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
#include "TransportCommon/IConnectionStatusUpdateNotifier.h"
#include "TransportCommon/ICommunicationSystem.h"
#include "Utils/LogMacros.h"
#include "Scene/SceneActionCollection.h"
#include "SceneReferencing/SceneReferenceEvent.h"
#include "Components/ISceneRendererHandler.h"
#include "TransportCommon/SceneUpdateStreamDeserializer.h"
#include "Components/SceneUpdate.h"
#include "TransportCommon/SceneUpdateSerializer.h"
#include "Components/ResourceAvailabilityEvent.h"
#include "Components/IResourceProviderComponent.h"
#include "Components/SceneUpdate.h"

namespace ramses_internal
{
    SceneGraphComponent::SceneGraphComponent(const Guid& myID, ICommunicationSystem& communicationSystem, IConnectionStatusUpdateNotifier& connectionStatusUpdateNotifier, IResourceProviderComponent& res, PlatformLock& frameworkLock)
        : m_sceneRendererHandler(nullptr)
        , m_myID(myID)
        , m_communicationSystem(communicationSystem)
        , m_connectionStatusUpdateNotifier(connectionStatusUpdateNotifier)
        , m_frameworkLock(frameworkLock)
        , m_resourceComponent(res)
    {
        m_connectionStatusUpdateNotifier.registerForConnectionUpdates(this);
        m_communicationSystem.setSceneProviderServiceHandler(this);
        m_communicationSystem.setSceneRendererServiceHandler(this);
    }

    SceneGraphComponent::~SceneGraphComponent()
    {
        m_connectionStatusUpdateNotifier.unregisterForConnectionUpdates(this);
        m_communicationSystem.setSceneProviderServiceHandler(nullptr);
        m_communicationSystem.setSceneRendererServiceHandler(nullptr);

        for (auto logic : m_clientSceneLogicMap)
        {
            delete logic.value;
        }
    }

    void SceneGraphComponent::setSceneRendererHandler(ISceneRendererHandler* sceneRendererHandler)
    {
        PlatformGuard guard(m_frameworkLock);

        if (nullptr != m_sceneRendererHandler && nullptr != sceneRendererHandler)
        {
            LOG_FATAL(CONTEXT_FRAMEWORK, "SceneGraphComponent::setSceneGraphConsumer: SceneGraphComponent already has a scene graph consumer. This probably means that two RamsesRenderer were initialized with the same RamsesFramework. This might cause further issues!");
            assert(false && "Prevented by HL logic");
        }

        // cannot modify renderer after connect/before disconnect
        assert(m_remoteScenes.empty());

        m_sceneRendererHandler = sceneRendererHandler;

        if (m_sceneRendererHandler)
        {
            // new renderer: publish all scenes
            for (const auto& sceneInfo : m_locallyPublishedScenes)
                m_sceneRendererHandler->handleNewSceneAvailable(sceneInfo.value, m_myID);
        }
        else
        {
            // renderer gone: unsubscribe self from all scenes
            for(const auto& publishedScene : m_locallyPublishedScenes)
            {
                if (ClientSceneLogicBase** sceneLogic = m_clientSceneLogicMap.get(publishedScene.key))
                    (*sceneLogic)->removeSubscriber(m_myID);
            }
        }
    }

    // TODO(tobias) remove mode, already given with publish
    void SceneGraphComponent::sendCreateScene(const Guid& to, const SceneId& sceneId, EScenePublicationMode mode)
    {
        LOG_INFO(CONTEXT_FRAMEWORK, "SceneGraphComponent::sendCreateScene: sceneId " << sceneId << ", to " << to);

        const SceneInfo* info = m_locallyPublishedScenes.get(sceneId);
        if (!info)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "SceneGraphComponent::sendCreateScene: scene not published, sceneId " << sceneId);
            // return;   // TODO: lots of tests must be fixed for this check
        }

        if (m_myID == to)
        {
            if (m_sceneRendererHandler)
            {
                if (info)
                    m_sceneRendererHandler->handleInitializeScene(*info, m_myID);
                else
                    m_sceneRendererHandler->handleInitializeScene(SceneInfo(sceneId, "", mode), m_myID);
            }
        }
        else
        {
            assert(mode != EScenePublicationMode_LocalOnly);
            UNUSED(mode);
            m_communicationSystem.sendInitializeScene(to, sceneId);
        }
    }

    void SceneGraphComponent::sendSceneUpdate(const std::vector<Guid>& toVec, SceneUpdate&& sceneUpdate, SceneId sceneId, EScenePublicationMode /*mode*/, StatisticCollectionScene& sceneStatistics)
    {
        // send to network (no ownership transfer)
        bool sendToSelf = false;
        bool alreadyCompressed = false;
        for (const auto& to : toVec)
        {
            if (m_myID == to)
            {
                sendToSelf = true;
            }
            else
            {
                if (!alreadyCompressed)
                {
                    for (auto& resource : sceneUpdate.resources)
                    {
                        resource->compress(IResource::CompressionLevel::Realtime);
                    }
                    alreadyCompressed = true;
                }
                m_communicationSystem.sendSceneUpdate(to, sceneId, SceneUpdateSerializer(sceneUpdate, sceneStatistics));
            }
        }

        // send to self last to move sceneUpdate to local renderer
        if (sendToSelf && m_sceneRendererHandler)
            m_sceneRendererHandler->handleSceneUpdate(sceneId, std::move(sceneUpdate), m_myID);
    }

    void SceneGraphComponent::sendPublishScene(SceneId sceneId, EScenePublicationMode mode, const String& name)
    {
        LOG_INFO(CONTEXT_FRAMEWORK, "SceneGraphComponent::publishScene: publishing scene: " << sceneId << " mode: " << EnumToString(mode));

        SceneInfo info(sceneId, name, mode);

        if (m_sceneRendererHandler)
            m_sceneRendererHandler->handleNewSceneAvailable(info, m_myID);

        if (mode != EScenePublicationMode_LocalOnly)
            m_communicationSystem.broadcastNewScenesAvailable({info});

        m_locallyPublishedScenes.put(sceneId, info);
    }

    void SceneGraphComponent::sendUnpublishScene(SceneId sceneId, EScenePublicationMode mode)
    {
        LOG_DEBUG(CONTEXT_FRAMEWORK, "SceneGraphComponent::unpublishScene: unpublishing scene: " << sceneId << " mode: " << EnumToString(mode));

        assert(m_locallyPublishedScenes.contains(sceneId));
        const SceneInfo info = *m_locallyPublishedScenes.get(sceneId);
        m_locallyPublishedScenes.remove(sceneId);

        if (m_sceneRendererHandler)
            m_sceneRendererHandler->handleSceneBecameUnavailable(sceneId, m_myID);

        if (mode != EScenePublicationMode_LocalOnly)
            m_communicationSystem.broadcastScenesBecameUnavailable({info});
    }

    void SceneGraphComponent::subscribeScene(const Guid& to, SceneId sceneId)
    {
        if (m_myID == to)
        {
            LOG_INFO(CONTEXT_FRAMEWORK, "SceneGraphComponent::subscribeScene: subscribing to local scene " << sceneId);
            handleSubscribeScene(sceneId, m_myID);
        }
        else
        {
            LOG_INFO(CONTEXT_FRAMEWORK, "SceneGraphComponent::subscribeScene: subscribing to scene " << sceneId << " from " << to);
            m_communicationSystem.sendSubscribeScene(to, sceneId);
        }
    }

    void SceneGraphComponent::unsubscribeScene(const Guid& to, SceneId sceneId)
    {
        if (m_myID == to)
            handleUnsubscribeScene(sceneId, m_myID);
        else
            m_communicationSystem.sendUnsubscribeScene(to, sceneId);
    }

    void SceneGraphComponent::disconnectFromNetwork()
    {
        PlatformGuard guard(m_frameworkLock);
        LOG_INFO(CONTEXT_FRAMEWORK, "SceneGraphComponent::disconnectFromNetwork");

        // send unpublish for all localAndRemoteScenes on network
        SceneInfoVector scenesToUnpublish;
        for (const auto& p : m_locallyPublishedScenes)
        {
            if (p.value.publicationMode != EScenePublicationMode_LocalOnly)
                scenesToUnpublish.push_back(p.value);
        }
        if (!scenesToUnpublish.empty())
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

        LOG_INFO(CONTEXT_FRAMEWORK, "SceneGraphComponent::disconnectFromNetwork: done");
    }

    void SceneGraphComponent::newParticipantHasConnected(const Guid& connnectedParticipant)
    {
        PlatformGuard guard(m_frameworkLock);

        SceneInfoVector availableScenes;
        for(const auto& p : m_locallyPublishedScenes)
        {
            if (p.value.publicationMode != EScenePublicationMode_LocalOnly)
            {
                LOG_INFO(CONTEXT_FRAMEWORK, "SceneGraphComponent::newParticipantHasConnected: publishing scene to new particpant: " << connnectedParticipant << " scene is: " << p.key << " mode: " << EnumToString(p.value.publicationMode) << " from: " << m_myID);
                availableScenes.push_back(p.value);
            }
        }

        if (availableScenes.size() > 0)
            m_communicationSystem.sendScenesAvailable(connnectedParticipant, availableScenes);
    }

    void SceneGraphComponent::participantHasDisconnected(const Guid& disconnnectedParticipant)
    {
        LOG_INFO(CONTEXT_FRAMEWORK, "SceneGraphComponent::participantHasDisconnected: unsubscribing all scenes for particpant: " << disconnnectedParticipant);

        PlatformGuard guard(m_frameworkLock);
        for(const auto& publishedScene : m_locallyPublishedScenes)
        {
            if (ClientSceneLogicBase** sceneLogic = m_clientSceneLogicMap.get(publishedScene.key))
                (*sceneLogic)->removeSubscriber(disconnnectedParticipant);
        }

        for (auto it = m_remoteScenes.begin(); it != m_remoteScenes.end();)
        {
            if (it->second.provider == disconnnectedParticipant)
            {
                if (m_sceneRendererHandler)
                    m_sceneRendererHandler->handleSceneBecameUnavailable(it->first, disconnnectedParticipant);
                it = m_remoteScenes.erase(it);
            }
            else
                ++it;
        }
    }

    void SceneGraphComponent::handleCreateScene(ClientScene& scene, bool enableLocalOnlyOptimization, ISceneProviderEventConsumer& eventConsumer)
    {
        const SceneId sceneId = scene.getSceneId();
        assert(!m_clientSceneLogicMap.contains(sceneId));
        ClientSceneLogicBase* sceneLogic = nullptr;
        if (enableLocalOnlyOptimization)
        {
            LOG_INFO(CONTEXT_CLIENT, "SceneGraphComponent::handleCreateScene: creating scene " << scene.getSceneId() << " (direct)");
            sceneLogic = new ClientSceneLogicDirect(*this, scene, m_resourceComponent, m_myID);
        }
        else
        {
            LOG_INFO(CONTEXT_CLIENT, "SceneGraphComponent::handleCreateScene: creating scene " << scene.getSceneId() << " (shadow copy)");
            sceneLogic = new ClientSceneLogicShadowCopy(*this, scene, m_resourceComponent, m_myID);
        }
        m_sceneEventConsumers.put(sceneId, &eventConsumer);
        m_clientSceneLogicMap.put(sceneId, sceneLogic);
    }

    void SceneGraphComponent::handlePublishScene(SceneId sceneId, EScenePublicationMode publicationMode)
    {
        assert(m_clientSceneLogicMap.contains(sceneId));
        ClientSceneLogicBase& sceneLogic = **m_clientSceneLogicMap.get(sceneId);

        LOG_INFO(CONTEXT_CLIENT, "SceneGraphComponent::handlePublishScene:  " << sceneId << " in mode " << EnumToString(publicationMode));
        sceneLogic.publish(publicationMode);
    }

    void SceneGraphComponent::handleUnpublishScene(SceneId sceneId)
    {
        assert(m_clientSceneLogicMap.contains(sceneId));
        ClientSceneLogicBase& sceneLogic = **m_clientSceneLogicMap.get(sceneId);

        LOG_INFO(CONTEXT_CLIENT, "SceneGraphComponent::handleUnpublishScene:  unpublishing scene " << sceneId);
        sceneLogic.unpublish();
    }

    bool SceneGraphComponent::handleFlush(SceneId sceneId, const FlushTimeInformation& flushTimeInfo, SceneVersionTag versionTag)
    {
        assert(m_clientSceneLogicMap.contains(sceneId));

        ClientSceneLogicBase& sceneLogic = **m_clientSceneLogicMap.get(sceneId);

        return sceneLogic.flushSceneActions(flushTimeInfo, versionTag);
    }

    void SceneGraphComponent::handleRemoveScene(SceneId sceneId)
    {
        LOG_INFO(CONTEXT_CLIENT, "SceneGraphComponent::handleRemoveScene: " << sceneId);
        ClientSceneLogicBase* sceneLogic = *m_clientSceneLogicMap.get(sceneId);
        assert(sceneLogic != nullptr);
        m_clientSceneLogicMap.remove(sceneId);
        m_sceneEventConsumers.remove(sceneId);
        delete sceneLogic;
    }

    void SceneGraphComponent::handleSubscribeScene(const SceneId& sceneId, const Guid& consumerID)
    {
        ClientSceneLogicBase** sceneLogic = m_clientSceneLogicMap.get(sceneId);
        if (sceneLogic != nullptr)
        {
            LOG_INFO(CONTEXT_CLIENT, "SceneGraphComponent::handleSceneSubscription: received scene subscription for scene " << sceneId << " from " << consumerID);
            (*sceneLogic)->addSubscriber(consumerID);
        }
        else
        {
            LOG_WARN(CONTEXT_CLIENT, "SceneGraphComponent::handleSceneSubscription: received scene subscription for unknown scene " << sceneId << " from " << consumerID);
        }
    }

    void SceneGraphComponent::handleUnsubscribeScene(const SceneId& sceneId, const Guid& consumerID)
    {
        ClientSceneLogicBase** sceneLogic = m_clientSceneLogicMap.get(sceneId);
        if (sceneLogic != nullptr)
        {
            LOG_INFO(CONTEXT_CLIENT, "SceneGraphComponent::handleSceneUnsubscription:  received scene unsubscription for scene " << sceneId << " from " << consumerID);
            (*sceneLogic)->removeSubscriber(consumerID);
        }
        else
        {
            LOG_WARN(CONTEXT_CLIENT, "SceneGraphComponent::handleSceneUnsubscription:  received scene unsubscription for unknown scene " << sceneId << " from " << consumerID);
        }
    }

    void SceneGraphComponent::triggerLogMessageForPeriodicLog()
    {
        PlatformGuard guard(m_frameworkLock);
        LOG_INFO_F(CONTEXT_PERIODIC, ([&](StringOutputStream& sos) {
                    sos << "Client: " << m_clientSceneLogicMap.size() << " scene(s):";
                    bool first = true;
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
                        sos << " " << m_clientScenesIter.key << " " << m_clientScenesIter.value->getSceneStateString();
                    }
                }));
    }

    void SceneGraphComponent::sendSceneReferenceEvent(const Guid& to, SceneReferenceEvent const& event)
    {
        if (m_myID == to)
            forwardToSceneProviderEventConsumer(event);
        else
        {
            std::vector<Byte> dataBuffer;
            event.writeToBlob(dataBuffer);
            m_communicationSystem.sendRendererEvent(to, event.masterSceneId, dataBuffer);
        }
    }

    void SceneGraphComponent::sendResourceAvailabilityEvent(const Guid& to, ResourceAvailabilityEvent const& event)
    {
        if (m_myID == to)
            forwardToSceneProviderEventConsumer(event);
        else
        {
            std::vector<Byte> dataBuffer;
            event.writeToBlob(dataBuffer);
            m_communicationSystem.sendRendererEvent(to, event.sceneid, std::move(dataBuffer));
        }
    }

    void SceneGraphComponent::handleRendererEvent(const SceneId& sceneId, const std::vector<Byte>& data, const Guid& /*rendererID*/)
    {
        // First extract type of event, it is at the beginning
        // TODO(jonathan): check if we can improve type handling, handle in better framing format e.g.
        if (data.size() < sizeof(ERendererToClientEventType))
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "SceneGraphComponent::handleRendererEvent: invalid data size, ignoring event");
            return;
        }
        ERendererToClientEventType eventType;
        std::memcpy(&eventType, data.data(), sizeof(eventType));
        // then deserialize eventdata based on type
        switch (eventType)
        {
            case ERendererToClientEventType::SceneReferencingEvent:
            {
                SceneReferenceEvent event(sceneId);
                event.readFromBlob(data);
                forwardToSceneProviderEventConsumer(event);
                break;
            }
            case ERendererToClientEventType::ResourcesAvailableAtRendererEvent:
            {
                ResourceAvailabilityEvent event;
                event.readFromBlob(data);
                forwardToSceneProviderEventConsumer(event);
                break;
            }
            default:
                LOG_ERROR(CONTEXT_FRAMEWORK, "SceneGraphComponent::handleRendererEvent: unknown event type: " << eventType);
                break;
        }
    }

    void SceneGraphComponent::forwardToSceneProviderEventConsumer(SceneReferenceEvent const& event)
    {
        auto it = m_sceneEventConsumers.find(event.masterSceneId);
        if (it != m_sceneEventConsumers.end())
            it->value->handleSceneReferenceEvent(event, m_myID);
        else
            LOG_WARN(CONTEXT_CLIENT, "SceneGraphComponent::forwardToSceneProviderEventConsumer: trying to send event to local client, but no event handler registered for sceneId " << event.masterSceneId);
    }

    void SceneGraphComponent::forwardToSceneProviderEventConsumer(ResourceAvailabilityEvent const& event)
    {
        auto it = m_sceneEventConsumers.find(event.sceneid);
        if (it != m_sceneEventConsumers.end())
            it->value->handleResourceAvailabilityEvent(event, m_myID);
        else
            LOG_WARN(CONTEXT_CLIENT, "SceneGraphComponent::forwardToSceneProviderEventConsumer: trying to send event to local client, but no event handler registered for sceneId " << event.sceneid);
    }

    void SceneGraphComponent::handleInitializeScene(const SceneId& sceneId, const Guid& providerID)
    {
        if (!m_sceneRendererHandler)
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "SceneGraphComponent::handleInitializeScene: unexpected call because no renderer, scene " << sceneId << " from " << providerID);
            return;
        }

        LOG_INFO(CONTEXT_FRAMEWORK, "SceneGraphComponent::handleSceneActionList: sceneId: " << sceneId << ", by " << providerID);

        auto it = m_remoteScenes.find(sceneId);
        if (it == m_remoteScenes.end())
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "SceneGraphComponent::handleSceneActionList: received for unknown scene, sceneId: " << sceneId << ", by " << providerID);
            return;
        }
        if (it->second.provider != providerID)
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "SceneGraphComponent::handleSceneActionList: received from unexpected provider, sceneId: " << sceneId << ", by " <<
                     providerID << " but belongs to " << it->second.provider);
            return;
        }

        // start with fresh deinitializer
        // TODO(tobias) should already be cleared when unsub was sent ou for this scene
        it->second.sceneUpdateDeserializer = std::make_unique<SceneUpdateStreamDeserializer>();

        m_sceneRendererHandler->handleInitializeScene(it->second.info, providerID);
    }

    void SceneGraphComponent::handleSceneUpdate(const SceneId& sceneId, absl::Span<const Byte> actionData, const Guid& providerID)
    {
        if (!m_sceneRendererHandler)
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "SceneGraphComponent::handleSceneUpdate: unexpected call because no renderer, scene " << sceneId << " from " << providerID);
            return;
        }

        auto it = m_remoteScenes.find(sceneId);
        if (it == m_remoteScenes.end())
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "SceneGraphComponent::handleSceneUpdate: received actions for unknown scene " << sceneId << " from " << providerID);
            return;
        }
        if (it->second.provider != providerID)
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "SceneGraphComponent::handleSceneUpdate: received from unexpected provider, sceneId: " << sceneId << ", by " <<
                     providerID << " but belongs to " << it->second.provider);
            return;
        }
        if (actionData.empty())
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "SceneGraphComponent::handleSceneUpdate: data is empty, sceneId " << sceneId << " from " << providerID);
            return;
        }
        SceneUpdateStreamDeserializer* deserializer = it->second.sceneUpdateDeserializer.get();
        if (!deserializer)
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "SceneGraphComponent::handleSceneUpdate: scene was not initialized before sending actions, sceneId " << sceneId << " from " << providerID);
            return;
        }

        auto result = deserializer->processData(actionData);
        switch (result.result)
        {
        case SceneUpdateStreamDeserializer::ResultType::Empty:
            break;
        case SceneUpdateStreamDeserializer::ResultType::Failed:
            LOG_ERROR(CONTEXT_FRAMEWORK, "SceneGraphComponent::handleSceneUpdate: deserialization failed for scene: " << sceneId << " from provider:" << providerID);
            // TODO(tobias) handle properly by unsub scene or disconnect participant
            break;
        case SceneUpdateStreamDeserializer::ResultType::HasData:
            {
                SceneUpdate sceneUpdate;
                sceneUpdate.actions = std::move(result.actions);
                for (auto& res : result.resources)
                    sceneUpdate.resources.emplace_back(std::move(res));
                sceneUpdate.flushInfos = std::move(result.flushInfos);
                m_sceneRendererHandler->handleSceneUpdate(sceneId, std::move(sceneUpdate), providerID);
                break;
            }
        }
    }

    void SceneGraphComponent::handleNewScenesAvailable(const SceneInfoVector& newScenes, const Guid& providerID)
    {
        // TODO(tobias) also cross-check with locally published scenes (+ published by someone else?) and warn/ignore if exists

        for(const auto& newScene : newScenes)
        {
            LOG_INFO(CONTEXT_FRAMEWORK, "SceneGraphComponent::handleNewScenesAvailable: sceneId: " << newScene.sceneID << ", name " << newScene.friendlyName <<",  by " << providerID);

            auto existingSceneIt = m_remoteScenes.find(newScene.sceneID);
            if (existingSceneIt != m_remoteScenes.end() && existingSceneIt->second.provider == providerID)
            {
                LOG_WARN(CONTEXT_FRAMEWORK, "SceneGraphComponent::handleNewScenesAvailable: duplicate publish ofscene: " << newScene.sceneID.getValue() << " @ " << providerID << " name:" << newScene.friendlyName << ". Will unpublish first");
                if (m_sceneRendererHandler)
                    m_sceneRendererHandler->handleSceneBecameUnavailable(newScene.sceneID, providerID);
                m_remoteScenes.erase(newScene.sceneID);
            }

            if (m_remoteScenes.find(newScene.sceneID) == m_remoteScenes.end())
            {
                LOG_INFO(CONTEXT_FRAMEWORK, "SceneGraphComponent::handleNewScenesAvailable: scene published: " << newScene.sceneID.getValue() << " @ " << providerID << " name:" << newScene.friendlyName << " publicationmode: " << EnumToString(newScene.publicationMode));

                m_remoteScenes[newScene.sceneID] = ReceivedScene{newScene, providerID, nullptr};

                if (m_sceneRendererHandler)
                    m_sceneRendererHandler->handleNewSceneAvailable(SceneInfo(newScene.sceneID, newScene.friendlyName, EScenePublicationMode_LocalAndRemote), providerID);
            }
            else
            {
                LOG_WARN(CONTEXT_FRAMEWORK, "SceneGraphComponent::handleNewScenesAvailable: ignore publish for duplicate scene: " << newScene.sceneID.getValue() << " @ " << providerID << " name:" << newScene.friendlyName);
            }
        }
    }

    void SceneGraphComponent::handleScenesBecameUnavailable(const SceneInfoVector& unavailableScenes, const Guid& providerID)
    {
        for (const auto& scene : unavailableScenes)
        {
            LOG_INFO(CONTEXT_FRAMEWORK, "SceneGraphComponent::handleScenesBecameUnavailable: sceneId: " << scene.sceneID << ", name " << scene.friendlyName <<",  by " << providerID);

            auto it = m_remoteScenes.find(scene.sceneID);
            if (it != m_remoteScenes.end())
            {
                if (m_sceneRendererHandler)
                    m_sceneRendererHandler->handleSceneBecameUnavailable(scene.sceneID, providerID);
                m_remoteScenes.erase(it);
            }
            else
            {
                LOG_WARN(CONTEXT_FRAMEWORK, "SceneGraphComponent::handleScenesBecameUnavailable: ignore unpublish for unknown scene: " << scene.sceneID.getValue() << " by " << providerID);
            }
        }
    }

    void SceneGraphComponent::handleSceneNotAvailable(const SceneId& sceneId, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_FRAMEWORK, "SceneGraphComponent::handleSceneNotAvailable: ignoring from sceneId: " << sceneId <<",  by " << providerID);
    }

    const ClientSceneLogicBase* SceneGraphComponent::getClientSceneLogicForScene(SceneId sceneId) const
    {
        ClientSceneLogicBase const* const* result = m_clientSceneLogicMap.get(sceneId);
        return result ? *result : nullptr;
    }
}
