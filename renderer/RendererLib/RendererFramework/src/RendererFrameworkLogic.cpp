//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererFramework/RendererFrameworkLogic.h"
#include "Components/ISceneGraphConsumerComponent.h"
#include "Components/IResourceConsumerComponent.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "Math3d/CameraMatrixHelper.h"
#include "Utils/LogMacros.h"
#include "TransportCommon/IConnectionStatusUpdateNotifier.h"
#include "Components/ManagedResource.h"
#include "Components/SceneGraphComponent.h"
#include "SceneReferencing/SceneReferenceEvent.h"

namespace ramses_internal
{
    RendererFrameworkLogic::RendererFrameworkLogic(
        IConnectionStatusUpdateNotifier& connectionStatusUpdateNotifier,
        IResourceConsumerComponent& res,
        ISceneGraphConsumerComponent& sgc,
        RendererCommandBuffer& rendererCommandBuffer,
        PlatformLock& frameworkLock)
        : m_frameworkLock(frameworkLock)
        , m_connectionStatusUpdateNotifier(connectionStatusUpdateNotifier)
        , m_sceneGraphConsumerComponent(sgc)
        , m_resourceComponent(res)
        , m_rendererCommands(rendererCommandBuffer)
    {
        m_connectionStatusUpdateNotifier.registerForConnectionUpdates(this);
        m_sceneGraphConsumerComponent.setSceneRendererServiceHandler(this);
    }

    RendererFrameworkLogic::~RendererFrameworkLogic()
    {
        m_connectionStatusUpdateNotifier.unregisterForConnectionUpdates(this);
        m_sceneGraphConsumerComponent.setSceneRendererServiceHandler(nullptr);
    }

    void RendererFrameworkLogic::handleNewScenesAvailable(const SceneInfoVector& newScenes, const Guid& providerID, EScenePublicationMode mode)
    {
        for(const auto& newScene : newScenes)
        {
            auto existingSceneIt = m_sceneClients.find(newScene.sceneID);
            if (existingSceneIt != m_sceneClients.end() && existingSceneIt->value.first == providerID)
            {
                LOG_WARN(CONTEXT_RENDERER, "RendererFrameworkLogic::handleNewScenesAvailable: duplicate publish ofscene: " << newScene.sceneID.getValue() << " @ " << providerID << " name:" << newScene.friendlyName << ". Will unpublish first");
                doHandleSceneBecameUnavailable(newScene.sceneID, providerID);
            }

            if (!m_sceneClients.contains(newScene.sceneID))
            {
                LOG_INFO(CONTEXT_RENDERER, "RendererFrameworkLogic::handleNewScenesAvailable: scene published: " << newScene.sceneID.getValue() << " @ " << providerID << " name:" << newScene.friendlyName << " publicationmode: " << EnumToString(newScene.publicationMode));

                m_rendererCommands.publishScene(newScene.sceneID, mode);
                m_sceneClients.put(newScene.sceneID, std::make_pair(providerID, newScene.friendlyName));
            }
            else
            {
                LOG_WARN(CONTEXT_RENDERER, "RendererFrameworkLogic::handleNewScenesAvailable: ignore publish for duplicate scene: " << newScene.sceneID.getValue() << " @ " << providerID << " name:" << newScene.friendlyName);
            }
        }
    }

    void RendererFrameworkLogic::handleScenesBecameUnavailable(const SceneInfoVector& unavailableScenes, const Guid& providerID)
    {
        for(const auto& scene : unavailableScenes)
        {
            if (m_sceneClients.contains(scene.sceneID))
            {
                doHandleSceneBecameUnavailable(scene.sceneID, providerID);
            }
            else
            {
                LOG_WARN(CONTEXT_RENDERER, "RendererFrameworkLogic::handleScenesBecameUnavailable: ignore unpublish for unknown scene: " << scene.sceneID.getValue() << " by " << providerID);
            }
        }
    }

    void RendererFrameworkLogic::doHandleSceneBecameUnavailable(const SceneId& sceneId, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_RENDERER, "RendererFrameworkLogic::doHandleSceneBecameUnavailable: scene unpublished: " << sceneId.getValue() << " by " << providerID);
        assert(m_sceneClients.contains(sceneId));

        m_rendererCommands.unpublishScene(sceneId);
        m_bufferedSceneActionsPerScene.erase(sceneId);
        m_sceneClients.remove(sceneId);
        m_lastReceivedListCounter.erase(sceneId);
    }

    void RendererFrameworkLogic::handleInitializeScene(const SceneInfo& sceneInfo, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_RENDERER, "RendererFrameworkLogic::handleInitializeScene:  scene: from:" << providerID <<
            " id:" << sceneInfo.sceneID.getValue() << " (" << sceneInfo.friendlyName << ")");

        // ensure clean state
        m_lastReceivedListCounter.erase(sceneInfo.sceneID);
        m_bufferedSceneActionsPerScene.erase(sceneInfo.sceneID);

        m_rendererCommands.receiveScene(sceneInfo);
    }

    void RendererFrameworkLogic::handleSceneNotAvailable(const SceneId& sceneId, const Guid& providerID)
    {
        LOG_WARN(CONTEXT_RENDERER, "RendererFrameworkLogic::handleSceneNotAvailable:  scene " << sceneId.getValue()
            << " not available at " << providerID);

        // TODO(tobias) report up to renderer API
    }

    void RendererFrameworkLogic::requestResourceAsyncronouslyFromFramework(const ResourceContentHashVector& ids, const RequesterID& requesterID, const SceneId& sceneId)
    {
        PlatformGuard guard(m_frameworkLock);

        const std::pair<Guid, String> *sceneIdToProviderID = m_sceneClients.get(sceneId);
        if (nullptr != sceneIdToProviderID)
        {
            const Guid providerID = sceneIdToProviderID->first;
            m_resourceComponent.requestResourceAsynchronouslyFromFramework(ids, requesterID, providerID);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererFrameworkLogic::requestResourceAsyncronouslyFromFramework:  could not request resources because sceneid" << sceneId.getValue()
                << " was not mappable to a provider");
        }
    }

    void RendererFrameworkLogic::cancelResourceRequest(const ResourceContentHash& resourceHash, const RequesterID& requesterID)
    {
        PlatformGuard guard(m_frameworkLock);
        m_resourceComponent.cancelResourceRequest(resourceHash, requesterID);
    }

    ManagedResourceVector RendererFrameworkLogic::popArrivedResources(const RequesterID& requesterID)
    {
        PlatformGuard guard(m_frameworkLock);
        return m_resourceComponent.popArrivedResources(requesterID);
    }

    void RendererFrameworkLogic::handleSceneActionList(const SceneId& sceneId, SceneActionCollection&& actions, const uint64_t& counter, const Guid& providerID)
    {
        LOG_TRACE(CONTEXT_RENDERER, "RendererFrameworkLogic::handleSceneActionList: for scene: " << sceneId << " counter: " << counter << " from provider:" << providerID);

        if (!m_sceneClients.contains(sceneId))
        {
            LOG_WARN(CONTEXT_RENDERER, "RendererFrameworkLogic::handleSceneActionList: received actions for unknown scene " << sceneId << " from " << providerID);
            return;
        }
        if (!isSceneActionListCounterValid(sceneId, counter))
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererFrameworkLogic::handleSceneActionList:  Counter MISMATCH: for scene: " << sceneId << " counter: " << counter  << " from provider:" << providerID);
            m_rendererCommands.unsubscribeScene(sceneId, true);
            return;
        }
        if (actions.empty())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererFrameworkLogic::handleSceneActionList: received action list is empty for scene " << sceneId << " from " << providerID);
            return;
        }

        auto bufferedActionsId = m_bufferedSceneActionsPerScene.find(sceneId);
        const Bool hasBufferedActions = (bufferedActionsId != m_bufferedSceneActionsPerScene.end());
        const Bool actionsHaveTrailingFlush = (actions.back().type() == ESceneActionId_Flush);

        if (!hasBufferedActions && actionsHaveTrailingFlush)
        {
            // no need for buffering, fully received flush
            m_rendererCommands.enqueueActionsForScene(sceneId, std::move(actions));
        }
        else
        {
            if (!hasBufferedActions)
            {
                m_bufferedSceneActionsPerScene.emplace(sceneId, std::move(actions));
            }
            else
            {
                bufferedActionsId->second.append(actions);

                if (actionsHaveTrailingFlush)
                {
                    m_rendererCommands.enqueueActionsForScene(sceneId, std::move(bufferedActionsId->second));
                    m_bufferedSceneActionsPerScene.erase(bufferedActionsId);
                }
            }
        }
    }

    void RendererFrameworkLogic::newParticipantHasConnected(const Guid& /*guid*/)
    {
    }

    void RendererFrameworkLogic::participantHasDisconnected(const Guid& clientID)
    {
        LOG_INFO(CONTEXT_RENDERER, "RendererFrameworkLogic::participantHasDisconnected:  client disconnected: " << clientID);

        SceneInfoVector unavailableScenes;
        for(const auto& sceneClient : m_sceneClients)
        {
            const std::pair<Guid, String>& clientInfo = sceneClient.value;
            if (clientInfo.first == clientID)
            {
                unavailableScenes.push_back(SceneInfo(sceneClient.key, clientInfo.second));
            }
        }

        if (unavailableScenes.size() > 0)
        {
            handleScenesBecameUnavailable(unavailableScenes, clientID);
        }
    }

    void RendererFrameworkLogic::sendSubscribeScene(SceneId sceneId)
    {
        PlatformGuard guard(m_frameworkLock);
        auto it = m_sceneClients.find(sceneId);
        if (it == m_sceneClients.end())
        {
            LOG_WARN(CONTEXT_RENDERER, "RendererFrameworkLogic::subscribeScene: can't send subscribe scene " << sceneId << " because provider unknown");
            return;
        }
        m_sceneGraphConsumerComponent.subscribeScene(it->value.first, sceneId);
    }

    void RendererFrameworkLogic::sendUnsubscribeScene(SceneId sceneId)
    {
        PlatformGuard guard(m_frameworkLock);
        auto it = m_sceneClients.find(sceneId);
        if (it == m_sceneClients.end())
        {
            LOG_WARN(CONTEXT_RENDERER, "RendererFrameworkLogic::subscribeScene: can't send subscribe scene " << sceneId << " because provider unknown");
            return;
        }
        m_sceneGraphConsumerComponent.unsubscribeScene(it->value.first, sceneId);
    }

    void RendererFrameworkLogic::sendSceneStateChanged(SceneId masterScene, SceneId referencedScene, RendererSceneState newState)
    {
        SceneReferenceEvent event(masterScene);
        event.type = SceneReferenceEventType::SceneStateChanged;
        event.referencedScene = referencedScene;
        event.sceneState = newState;

        PlatformGuard guard(m_frameworkLock);
        auto it = m_sceneClients.find(masterScene);
        if (it == m_sceneClients.end())
        {
            LOG_WARN(CONTEXT_RENDERER, "RendererFrameworkLogic::sendSceneStateChanged: can't send scene state changed event for scene " << masterScene << " because provider unknown");
            return;
        }

        LOG_INFO_P(CONTEXT_FRAMEWORK,
            "RendererFrameworkLogic::sendSceneStateChanged: sending scene state changed event (state {} / master {} / reffed {}) to {}",
            EnumToString(newState), masterScene, referencedScene, it->value.first);
        m_sceneGraphConsumerComponent.sendSceneReferenceEvent(it->value.first, event);
    }

    void RendererFrameworkLogic::sendSceneFlushed(SceneId masterScene, SceneId referencedScene, SceneVersionTag tag)
    {
        SceneReferenceEvent event(masterScene);
        event.type = SceneReferenceEventType::SceneFlushed;
        event.referencedScene = referencedScene;
        event.tag = tag;

        PlatformGuard guard(m_frameworkLock);
        auto it = m_sceneClients.find(masterScene);
        if (it == m_sceneClients.end())
        {
            LOG_WARN(CONTEXT_RENDERER, "RendererFrameworkLogic::sendSceneFlushed: can't send scene state changed event for scene " << masterScene << " because provider unknown");
            return;
        }

        LOG_INFO_P(CONTEXT_FRAMEWORK,
            "RendererFrameworkLogic::sendSceneFlushed: sending scene flushed event (tag {} / master {} / reffed {}) to {}",
            tag, masterScene, referencedScene, it->value.first);
        m_sceneGraphConsumerComponent.sendSceneReferenceEvent(it->value.first, event);
    }

    void RendererFrameworkLogic::sendDataLinked(SceneId masterScene, SceneId providerScene, DataSlotId provider, SceneId consumerScene, DataSlotId consumer, bool success)
    {
        SceneReferenceEvent event(masterScene);
        event.type = SceneReferenceEventType::DataLinked;
        event.providerScene = providerScene;
        event.dataProvider = provider;
        event.consumerScene = consumerScene;
        event.dataConsumer = consumer;
        event.status = success;

        PlatformGuard guard(m_frameworkLock);
        auto it = m_sceneClients.find(masterScene);
        if (it == m_sceneClients.end())
        {
            LOG_WARN(CONTEXT_RENDERER, "RendererFrameworkLogic::sendDataLinked: can't send scene state changed event for scene " << masterScene << " because provider unknown");
            return;
        }

        LOG_INFO_P(CONTEXT_FRAMEWORK,
            "RendererFrameworkLogic::sendDataLinked: sending data linked event (master {} / providerScene {} / provider {} / consumerScene {} / consumer {} / success {}) to {}",
            masterScene, providerScene, provider, consumerScene, consumer, success, it->value.first);
        m_sceneGraphConsumerComponent.sendSceneReferenceEvent(it->value.first, event);
    }

    void RendererFrameworkLogic::sendDataUnlinked(SceneId masterScene, SceneId consumerScene, DataSlotId consumer, bool success)
    {
        SceneReferenceEvent event(masterScene);
        event.type = SceneReferenceEventType::DataUnlinked;
        event.consumerScene = consumerScene;
        event.dataConsumer = consumer;
        event.status = success;

        PlatformGuard guard(m_frameworkLock);
        auto it = m_sceneClients.find(masterScene);
        if (it == m_sceneClients.end())
        {
            LOG_WARN(CONTEXT_RENDERER, "RendererFrameworkLogic::sendDataUnlinked: can't send scene state changed event for scene " << masterScene << " because provider unknown");
            return;
        }

        LOG_INFO_P(CONTEXT_FRAMEWORK,
            "RendererFrameworkLogic::sendDataLinked: sending data linked event (master {} / consumerScene {} / consumer {} / success {}) to {}",
            masterScene, consumerScene, consumer, success, it->value.first);
        m_sceneGraphConsumerComponent.sendSceneReferenceEvent(it->value.first, event);
    }

    bool RendererFrameworkLogic::isSceneActionListCounterValid(const SceneId& sceneId, const uint64_t& counter)
    {
        LOG_TRACE(CONTEXT_RENDERER, "RendererFrameworkLogic::isSceneActionListCounterValid: " << sceneId << "received counter:" << counter);
        // zero is valid for backward compatibility
        if (counter == 0u)
        {
            LOG_DEBUG(CONTEXT_RENDERER, "RendererFrameworkLogic::isSceneActionListCounterValid: accepting '0' for backward compatibility for scene: " << sceneId);
            return true;
        }

        // if its first list , then counter must start with 1u
        auto counterIter = m_lastReceivedListCounter.find(sceneId);
        if (counterIter == m_lastReceivedListCounter.end())
        {
            if (counter == 1u)
            {
                LOG_DEBUG(CONTEXT_RENDERER, "RendererFrameworkLogic::isSceneActionListCounterValid: init to 1 for scene: " << sceneId);
                m_lastReceivedListCounter[sceneId] = 1u;
                return true;
            }
            else
            {
                // first list was not started with 1 -> error
                LOG_ERROR(CONTEXT_RENDERER, "RendererFrameworkLogic::isSceneActionListCounterValid: mismatch for scene: " << sceneId << "expected to start with 1, but got" << counter);
                return false;
            }
        }
        else
        {
            const uint64_t lastreceived = counterIter->second;
            uint64_t nextExpected = lastreceived + 1u;
            if (nextExpected == SceneActionList_CounterWrapAround)
            {
                LOG_TRACE(CONTEXT_RENDERER, "RendererFrameworkLogic::isSceneActionListCounterValid: wrap around to '1' for scene: " << sceneId);
                nextExpected = 1u;
            }
            if (counter == nextExpected)
            {
                m_lastReceivedListCounter[sceneId] = counter;
                return true;
            }
            else
            {
                // number mismatch
                LOG_ERROR(CONTEXT_RENDERER, "RendererFrameworkLogic::isSceneActionListCounterValid: mismatch! Expected: " << nextExpected << " but received " << counter << " for scene " << sceneId);
                return false;
            }
        }
    }

}
