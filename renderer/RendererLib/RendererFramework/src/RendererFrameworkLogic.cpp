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
#include "Components/SceneUpdate.h"

namespace ramses_internal
{
    RendererFrameworkLogic::RendererFrameworkLogic(
        IResourceConsumerComponent& res,
        ISceneGraphConsumerComponent& sgc,
        RendererCommandBuffer& rendererCommandBuffer,
        PlatformLock& frameworkLock)
        : m_frameworkLock(frameworkLock)
        , m_sceneGraphConsumerComponent(sgc)
        , m_resourceComponent(res)
        , m_rendererCommands(rendererCommandBuffer)
    {
        m_sceneGraphConsumerComponent.setSceneRendererHandler(this);
    }

    RendererFrameworkLogic::~RendererFrameworkLogic()
    {
        m_sceneGraphConsumerComponent.setSceneRendererHandler(nullptr);
    }

    void RendererFrameworkLogic::handleNewSceneAvailable(const SceneInfo& newScene, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_RENDERER, "RendererFrameworkLogic::handleNewScenesAvailable: scene published: " << newScene.sceneID << " @ " << providerID << " name:" << newScene.friendlyName << " publicationmode: " << EnumToString(newScene.publicationMode));

        if (m_sceneClients.contains(newScene.sceneID))
        {
            LOG_WARN(CONTEXT_RENDERER, "RendererFrameworkLogic::handleNewScenesAvailable: ignore already published scene: " << newScene.sceneID << " @ " << providerID << " name:" << newScene.friendlyName << " publicationmode: " << EnumToString(newScene.publicationMode));
            return;
        }
        m_sceneClients.put(newScene.sceneID, std::make_pair(providerID, newScene.friendlyName));
        m_rendererCommands.publishScene(newScene.sceneID, newScene.publicationMode);
    }

    void RendererFrameworkLogic::handleSceneBecameUnavailable(const SceneId& sceneId, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_RENDERER, "RendererFrameworkLogic::handleSceneBecameUnavailable: scene unpublished: " << sceneId << " by " << providerID);

        assert(m_sceneClients.contains(sceneId));
        m_sceneClients.remove(sceneId);
        m_rendererCommands.unpublishScene(sceneId);
    }

    void RendererFrameworkLogic::handleInitializeScene(const SceneInfo& sceneInfo, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_RENDERER, "RendererFrameworkLogic::handleInitializeScene: scene unpublished: " << sceneInfo.sceneID << " by " << providerID);

        assert(m_sceneClients.contains(sceneInfo.sceneID));
        m_rendererCommands.receiveScene(sceneInfo);
    }

    void RendererFrameworkLogic::requestResourceAsyncronouslyFromFramework(const ResourceContentHashVector& ids, const ResourceRequesterID& requesterID, const SceneId& sceneId)
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
            LOG_ERROR(CONTEXT_RENDERER, "RendererFrameworkLogic::requestResourceAsyncronouslyFromFramework:  could not request resources because sceneid" << sceneId
                << " was not mappable to a provider");
        }
    }

    void RendererFrameworkLogic::cancelResourceRequest(const ResourceContentHash& resourceHash, const ResourceRequesterID& requesterID)
    {
        PlatformGuard guard(m_frameworkLock);
        m_resourceComponent.cancelResourceRequest(resourceHash, requesterID);
    }

    ManagedResourceVector RendererFrameworkLogic::popArrivedResources(const ResourceRequesterID& requesterID)
    {
        PlatformGuard guard(m_frameworkLock);
        return m_resourceComponent.popArrivedResources(requesterID);
    }

    void RendererFrameworkLogic::handleSceneUpdate(const SceneId& sceneId, SceneUpdate&& sceneUpdate, const Guid& /*providerID*/)
    {
        m_rendererCommands.enqueueActionsForScene(sceneId, std::move(sceneUpdate));
    }

    void RendererFrameworkLogic::sendSubscribeScene(SceneId sceneId)
    {
        PlatformGuard guard(m_frameworkLock);
        auto it = m_sceneClients.find(sceneId);
        if (it == m_sceneClients.end())
        {
            LOG_WARN(CONTEXT_RENDERER, "RendererFrameworkLogic::sendSubscribeScene: can't send subscribe scene " << sceneId << " because provider unknown");
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
            LOG_WARN(CONTEXT_RENDERER, "RendererFrameworkLogic::sendUnsubscribeScene: can't send subscribe scene " << sceneId << " because provider unknown");
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
}
