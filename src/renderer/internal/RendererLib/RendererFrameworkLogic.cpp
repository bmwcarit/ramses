//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RendererFrameworkLogic.h"
#include "internal/Components/ISceneGraphConsumerComponent.h"
#include "internal/RendererLib/RendererCommandBuffer.h"
#include "internal/Core/Math3d/CameraMatrixHelper.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Communication/TransportCommon/IConnectionStatusUpdateNotifier.h"
#include "internal/Components/ManagedResource.h"
#include "internal/Components/SceneGraphComponent.h"
#include "internal/SceneReferencing/SceneReferenceEvent.h"
#include "internal/Components/SceneUpdate.h"

namespace ramses::internal
{
    RendererFrameworkLogic::RendererFrameworkLogic(
        ISceneGraphConsumerComponent& sgc,
        RendererCommandBuffer& rendererCommandBuffer,
        PlatformLock& frameworkLock)
        : m_frameworkLock(frameworkLock)
        , m_sceneGraphConsumerComponent(sgc)
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
        m_rendererCommands.enqueueCommand(RendererCommand::ScenePublished{ newScene.sceneID, newScene.publicationMode });
    }

    void RendererFrameworkLogic::handleSceneBecameUnavailable(const SceneId& unavailableScene, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_RENDERER, "RendererFrameworkLogic::handleSceneBecameUnavailable: scene unpublished: " << unavailableScene << " by " << providerID);

        assert(m_sceneClients.contains(unavailableScene));
        m_sceneClients.remove(unavailableScene);
        m_rendererCommands.enqueueCommand(RendererCommand::SceneUnpublished{ unavailableScene });
    }

    void RendererFrameworkLogic::handleInitializeScene(const SceneInfo& sceneInfo, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_RENDERER, "RendererFrameworkLogic::handleInitializeScene: " << sceneInfo.sceneID << " by " << providerID);

        assert(m_sceneClients.contains(sceneInfo.sceneID));
        m_rendererCommands.enqueueCommand(RendererCommand::ReceiveScene{ sceneInfo });
    }

    void RendererFrameworkLogic::handleSceneUpdate(const SceneId& sceneId, SceneUpdate&& sceneUpdate, const Guid& /*providerID*/)
    {
        m_rendererCommands.enqueueCommand(RendererCommand::UpdateScene{ sceneId, std::move(sceneUpdate) });
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
