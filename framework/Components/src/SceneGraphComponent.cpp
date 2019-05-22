//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/SceneGraphComponent.h"
#include "Components/ClientSceneLogicShadowCopy.h"
#include "Components/ClientSceneLogicDirect.h"
#include "Scene/ClientScene.h"
#include "Scene/SceneActionUtils.h"
#include "TransportCommon/IConnectionStatusUpdateNotifier.h"
#include "TransportCommon/ICommunicationSystem.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "Utils/LogMacros.h"
#include "Scene/SceneActionCollection.h"

namespace ramses_internal
{
    SceneGraphComponent::SceneGraphComponent(const Guid& myID, ICommunicationSystem& communicationSystem, IConnectionStatusUpdateNotifier& connectionStatusUpdateNotifier, PlatformLock& frameworkLock)
        : m_sceneRendererHandler(0)
        , m_sceneProviderHandler(0)
        , m_myID(myID)
        , m_communicationSystem(communicationSystem)
        , m_connectionStatusUpdateNotifier(connectionStatusUpdateNotifier)
        , m_frameworkLock(frameworkLock)
    {
        m_connectionStatusUpdateNotifier.registerForConnectionUpdates(this);
    }

    SceneGraphComponent::~SceneGraphComponent()
    {
        m_connectionStatusUpdateNotifier.unregisterForConnectionUpdates(this);

        for (auto logic : m_clientSceneLogicMap)
        {
            delete logic.value;
        }
    }

    void SceneGraphComponent::setSceneRendererServiceHandler(ISceneRendererServiceHandler* sceneRendererHandler)
    {
        PlatformGuard guard(m_frameworkLock);

        // TODO Tobias Handle properly the case that two renderers are created with the same framework
        if (NULL != m_sceneRendererHandler && NULL != sceneRendererHandler)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "SceneGraphComponent::setSceneGraphConsumer: SceneGraphComponent already has a scene graph consumer. This probably means that two RamsesRenderer were initialized with the same RamsesFramework. This might cause further issues!");
        }

        m_sceneRendererHandler = sceneRendererHandler;
        m_communicationSystem.setSceneRendererServiceHandler(sceneRendererHandler);

        if (m_sceneRendererHandler && m_publishedScenes.count() > 0)
        {
            SceneInfoVector newLocalScenes;
            SceneInfoVector newRemoteScenes;
            for (const auto& sceneInfo : m_publishedScenes)
            {
                if (sceneInfo.value.publicationMode == EScenePublicationMode_LocalAndRemote)
                    newRemoteScenes.push_back(SceneInfo(sceneInfo.key, sceneInfo.value.name));
                else
                    newLocalScenes.push_back(SceneInfo(sceneInfo.key, sceneInfo.value.name));

            }

            if(!newLocalScenes.empty())
                m_sceneRendererHandler->handleNewScenesAvailable(newLocalScenes, m_myID, EScenePublicationMode_LocalOnly);

            if (!newRemoteScenes.empty())
                m_sceneRendererHandler->handleNewScenesAvailable(newRemoteScenes, m_myID, EScenePublicationMode_LocalAndRemote);
        }
    }

    void SceneGraphComponent::setSceneProviderServiceHandler(ISceneProviderServiceHandler* handler)
    {
        PlatformGuard guard(m_frameworkLock);
        m_sceneProviderHandler = handler;
        m_communicationSystem.setSceneProviderServiceHandler(handler);
    }

    void SceneGraphComponent::sendCreateScene(const Guid& to, const SceneInfo& sceneInfo, EScenePublicationMode mode)
    {
        if (m_myID == to)
        {
            if (m_sceneRendererHandler)
            {
                m_sceneRendererHandler->handleInitializeScene(sceneInfo, m_myID);
            }
        }
        else
        {
            assert(mode != EScenePublicationMode_LocalOnly);
            UNUSED(mode);
            m_communicationSystem.sendInitializeScene(to, sceneInfo);
            m_subscriptions[Subscription(to, sceneInfo.sceneID)] = 1u;
            LOG_DEBUG(CONTEXT_FRAMEWORK, "SceneGraphComponent::sendCreateScene: initialize counter for sceneid " << sceneInfo.sceneID << " to 1u");
        }
    }

    void SceneGraphComponent::sendSceneActionList(const std::vector<Guid>& toVec, SceneActionCollection&& sceneAction, SceneId sceneId, EScenePublicationMode mode)
    {
        UNUSED(mode);

        // send to network (no ownership transfer)
        bool sendToSelf = false;
        for (const auto& to : toVec)
        {
            if (m_myID == to)
            {
                sendToSelf = true;
            }
            else
            {
                assert(mode != EScenePublicationMode_LocalOnly);
                const uint64_t currentCounter = m_subscriptions[Subscription(to, sceneId)];
                assert(currentCounter != 0);
                const uint64_t numberOfChunksSent = m_communicationSystem.sendSceneActionList(to, sceneId, sceneAction, currentCounter);
                if (numberOfChunksSent > 0)
                {
                    LOG_DEBUG(CONTEXT_FRAMEWORK, "SceneGraphComponent::sendSceneActionList: to " << to << ", counter for sceneid " << sceneId << " started at " << currentCounter << " sent " << numberOfChunksSent << " chunks");
                }
                else
                {
                    LOG_WARN(CONTEXT_FRAMEWORK, "SceneGraphComponent::sendSceneActionList: sceneid " << sceneId << ", counter " << currentCounter << ", to " << to << " failed. 0 chunks sent");
                }
                uint64_t nextCounter = currentCounter + numberOfChunksSent;

                // wrap around case
                if (nextCounter == SceneActionList_CounterWrapAround)
                {
                    nextCounter = 1u;
                }
                m_subscriptions[Subscription(to, sceneId)] = nextCounter;
                LOG_DEBUG(CONTEXT_FRAMEWORK, "SceneGraphComponent::sendSceneActionList: set counter for sceneid " << sceneId << " to " << nextCounter);
            }
        }

        // transfer ownership of sceneAction to local renderer
        if (sendToSelf && m_sceneRendererHandler)
        {
            m_sceneRendererHandler->handleSceneActionList(sceneId, std::move(sceneAction), 0u, m_myID);
        }
    }

    void SceneGraphComponent::sendPublishScene(SceneId sceneId, EScenePublicationMode mode, const String& name)
    {
        LOG_INFO(CONTEXT_FRAMEWORK, "SceneGraphComponent::publishScene: publishing scene: " << sceneId.getValue() << " mode: " << EnumToString(mode));

        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId, name));

        if (m_sceneRendererHandler)
        {
            m_sceneRendererHandler->handleNewScenesAvailable(newScenes, m_myID, mode);
        }
        if (mode != EScenePublicationMode_LocalOnly)
        {
            m_communicationSystem.broadcastNewScenesAvailable(newScenes);
        }

        m_publishedScenes.put(sceneId, PublishedSceneInfo(mode, name));
    }

    void SceneGraphComponent::sendUnpublishScene(SceneId sceneId, EScenePublicationMode mode)
    {
        LOG_DEBUG(CONTEXT_FRAMEWORK, "SceneGraphComponent::unpublishScene: unpublishing scene: " << sceneId.getValue() << " mode: " << EnumToString(mode));

        assert(m_publishedScenes.contains(sceneId));
        PublishedSceneInfo pubInfo = *m_publishedScenes.get(sceneId);
        m_publishedScenes.remove(sceneId);

        SceneInfoVector unavailableScenes;
        unavailableScenes.push_back(SceneInfo(sceneId, pubInfo.name));

        if (m_sceneRendererHandler)
        {
            m_sceneRendererHandler->handleScenesBecameUnavailable(unavailableScenes, m_myID);
        }
        if (mode != EScenePublicationMode_LocalOnly)
        {
            m_communicationSystem.broadcastScenesBecameUnavailable(unavailableScenes);
        }
    }

    void SceneGraphComponent::subscribeScene(const Guid& to, SceneId sceneId)
    {
        if (m_myID == to)
        {
            if (m_sceneProviderHandler)
            {
                LOG_INFO(CONTEXT_FRAMEWORK, "SceneGraphComponent::subscribeScene: subscribing to local scene " << sceneId.getValue());
                PlatformGuard guard(m_frameworkLock);
                m_sceneProviderHandler->handleSubscribeScene(sceneId, m_myID);
            }
        }
        else
        {
            LOG_INFO(CONTEXT_FRAMEWORK, "SceneGraphComponent::subscribeScene: subscribing to scene " << sceneId.getValue() << " from " << to);
            m_communicationSystem.sendSubscribeScene(to, sceneId);
        }
    }

    void SceneGraphComponent::unsubscribeScene(const Guid& to, SceneId sceneId)
    {
        if (m_myID == to)
        {
            if (m_sceneProviderHandler)
            {
                PlatformGuard guard(m_frameworkLock);
                m_sceneProviderHandler->handleUnsubscribeScene(sceneId, m_myID);
            }
        }
        else
        {
            m_communicationSystem.sendUnsubscribeScene(to, sceneId);
        }
    }

    void SceneGraphComponent::disconnectFromNetwork()
    {
        PlatformGuard guard(m_frameworkLock);
        LOG_INFO(CONTEXT_FRAMEWORK, "SceneGraphComponent::disconnectFromNetwork");

        // send unpublish for all localAndRemoteScenes on network
        SceneInfoVector scenesToUnpublish;
        for (const auto& p : m_publishedScenes)
        {
            if (p.value.publicationMode != EScenePublicationMode_LocalOnly)
            {
                scenesToUnpublish.push_back(SceneInfo(p.key, p.value.name));
            }
        }
        m_communicationSystem.broadcastScenesBecameUnavailable(scenesToUnpublish);

        // remove all subscribers from CSL
        for (const auto& p : m_clientSceneLogicMap)
        {
            ClientSceneLogicBase* sceneLogic = p.value;
            const std::vector<Guid> subscribers(sceneLogic->getWaitingAndActiveSubscribers());
            for (const auto& sub : subscribers)
            {
                if (sub != m_myID)
                {
                    sceneLogic->removeSubscriber(sub);
                }
            }
        }

        // clear subscriptions, it is guaranteed that scenes sent to remote have entries here
        m_subscriptions.clear();

        LOG_INFO(CONTEXT_FRAMEWORK, "SceneGraphComponent::disconnectFromNetwork: done");
    }

    void SceneGraphComponent::newParticipantHasConnected(const Guid& to)
    {
        PlatformGuard guard(m_frameworkLock);
        if (m_publishedScenes.count() > 0)
        {
            SceneInfoVector availableScenes;
            for(const auto& publishedScene : m_publishedScenes)
            {
                if (publishedScene.value.publicationMode != EScenePublicationMode_LocalOnly)
                {
                    LOG_INFO(CONTEXT_FRAMEWORK, "SceneGraphComponent::newParticipantHasConnected: publishing scene to new particpant: " << to << " scene is: " << publishedScene.key.getValue() << " mode: " << EnumToString(publishedScene.value.publicationMode) << " from: " << m_myID);
                    availableScenes.push_back(SceneInfo(publishedScene.key, publishedScene.value.name));
                }
            }
            if (availableScenes.size() > 0)
            {
                m_communicationSystem.sendScenesAvailable(to, availableScenes);
            }
        }
    }

    void SceneGraphComponent::participantHasDisconnected(const Guid& disconnnectedParticipant)
    {
        LOG_INFO(CONTEXT_FRAMEWORK, "SceneGraphComponent::participantHasDisconnected: unsubscribing all scenes for particpant: " << disconnnectedParticipant);

        PlatformGuard guard(m_frameworkLock);
        for(const auto& publishedScene : m_publishedScenes)
        {
            handleSceneUnsubscription(publishedScene.key, disconnnectedParticipant);
        }
    }

    void SceneGraphComponent::handleCreateScene(ClientScene& scene, bool enableLocalOnlyOptimization)
    {
        const SceneId sceneId = scene.getSceneId();
        assert(!m_clientSceneLogicMap.contains(sceneId));
        ClientSceneLogicBase* sceneLogic = nullptr;
        if (enableLocalOnlyOptimization)
        {
            LOG_INFO(CONTEXT_CLIENT, "SceneGraphComponent::handleCreateScene: creating scene " << scene.getSceneId().getValue() << " (direct)");
            sceneLogic = new ClientSceneLogicDirect(*this, scene, m_myID);
        }
        else
        {
            LOG_INFO(CONTEXT_CLIENT, "SceneGraphComponent::handleCreateScene: creating scene " << scene.getSceneId().getValue() << " (shadow copy)");
            sceneLogic = new ClientSceneLogicShadowCopy(*this, scene, m_myID);
        }
        m_clientSceneLogicMap.put(sceneId, sceneLogic);
    }

    void SceneGraphComponent::handlePublishScene(SceneId sceneId, EScenePublicationMode publicationMode)
    {
        assert(m_clientSceneLogicMap.contains(sceneId));
        ClientSceneLogicBase& sceneLogic = **m_clientSceneLogicMap.get(sceneId);

        LOG_INFO(CONTEXT_CLIENT, "SceneGraphComponent::handlePublishScene:  " << sceneId.getValue() << " in mode " << EnumToString(publicationMode));
        sceneLogic.publish(publicationMode);
    }

    void SceneGraphComponent::handleUnpublishScene(SceneId sceneId)
    {
        assert(m_clientSceneLogicMap.contains(sceneId));
        ClientSceneLogicBase& sceneLogic = **m_clientSceneLogicMap.get(sceneId);

        LOG_INFO(CONTEXT_CLIENT, "SceneGraphComponent::handleUnpublishScene:  unpublishing scene " << sceneId.getValue());
        sceneLogic.unpublish();
    }

    void SceneGraphComponent::handleFlush(SceneId sceneId, ESceneFlushMode flushMode, const FlushTimeInformation& flushTimeInfo, SceneVersionTag versionTag)
    {
        assert(m_clientSceneLogicMap.contains(sceneId));

        ClientSceneLogicBase& sceneLogic = **m_clientSceneLogicMap.get(sceneId);

        sceneLogic.flushSceneActions(flushMode, flushTimeInfo, versionTag);
    }

    void SceneGraphComponent::handleRemoveScene(SceneId sceneId)
    {
        LOG_INFO(CONTEXT_CLIENT, "SceneGraphComponent::handleRemoveScene: " << sceneId.getValue());
        ClientSceneLogicBase* sceneLogic = *m_clientSceneLogicMap.get(sceneId);
        assert(sceneLogic != NULL);
        m_clientSceneLogicMap.remove(sceneId);
        delete sceneLogic;
    }

    void SceneGraphComponent::handleSceneSubscription(SceneId sceneId, const Guid& subscriber)
    {
        ClientSceneLogicBase** sceneLogic = m_clientSceneLogicMap.get(sceneId);
        if (sceneLogic != nullptr)
        {
            LOG_INFO(CONTEXT_CLIENT, "SceneGraphComponent::handleSceneSubscription: received scene subscription for scene " << sceneId.getValue() << " from " << subscriber);
            (*sceneLogic)->addSubscriber(subscriber);
        }
        else
        {
            LOG_WARN(CONTEXT_CLIENT, "SceneGraphComponent::handleSceneSubscription: received scene subscription for unknown scene " << sceneId.getValue() << " from " << subscriber);
        }
    }

    void SceneGraphComponent::handleSceneUnsubscription(SceneId sceneId, const Guid& subscriber)
    {
        ClientSceneLogicBase** sceneLogic = m_clientSceneLogicMap.get(sceneId);
        if (sceneLogic != nullptr)
        {
            LOG_INFO(CONTEXT_CLIENT, "SceneGraphComponent::handleSceneUnsubscription:  received scene unsubscription for scene " << sceneId.getValue() << " from " << subscriber);
            (*sceneLogic)->removeSubscriber(subscriber);
        }
        else
        {
            LOG_WARN(CONTEXT_CLIENT, "SceneGraphComponent::handleSceneUnsubscription:  received scene unsubscription for unknown scene " << sceneId.getValue() << " from " << subscriber);
        }
        LOG_DEBUG(CONTEXT_FRAMEWORK, "SceneGraphComponent::handleSceneUnsubscription: remove counter for sceneid " << sceneId);
        m_subscriptions.remove(Subscription(subscriber, sceneId));
    }

    void SceneGraphComponent::triggerLogMessageForPeriodicLog()
    {
        PlatformGuard guard(m_frameworkLock);
        LOG_INFO_F(CONTEXT_PERIODIC, ([&](StringOutputStream& sos) {
                    sos << "Client: " << m_clientSceneLogicMap.count() << " scene(s):";
                    Bool first = true;
                    for (const auto& m_clientScenesIter : m_clientSceneLogicMap)
                    {
                        if (first)
                        {
                            first = false;
                        }
                        else
                        {
                            sos << ",";
                        }
                        sos << " " << m_clientScenesIter.key.getValue() << " " << m_clientScenesIter.value->getSceneStateString();
                    }
                }));
    }
}
