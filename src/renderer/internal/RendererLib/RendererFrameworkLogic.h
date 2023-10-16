//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/IRendererSceneEventSender.h"
#include "internal/SceneGraph/Scene/SceneActionCollection.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"
#include "internal/PlatformAbstraction/Collections/Pair.h"
#include "internal/PlatformAbstraction/PlatformLock.h"
#include "internal/Core/Utils/Warnings.h"
#include "internal/Components/ISceneRendererHandler.h"

#include <unordered_map>
#include <string>

namespace ramses::internal
{
    class ISceneGraphConsumerComponent;
    class IResourceConsumerComponent;
    class RendererCommandBuffer;
    class IConnectionStatusUpdateNotifier;

    class RendererFrameworkLogic
        : public ISceneRendererHandler
        , public IRendererSceneEventSender
    {
    public:
        RendererFrameworkLogic(
            ISceneGraphConsumerComponent& sgc,
            RendererCommandBuffer& rendererCommandBuffer,
            PlatformLock& frameworkLock);
        ~RendererFrameworkLogic() override;

        // ISceneRendererHandler
        void handleInitializeScene(const SceneInfo& sceneInfo, const Guid& providerID) override;
        void handleSceneUpdate(const SceneId& sceneId, SceneUpdate&& sceneUpdate, const Guid& providerID) override;
        void handleNewSceneAvailable(const SceneInfo& newScene, const Guid& providerID) override;
        void handleSceneBecameUnavailable(const SceneId& unavailableScene, const Guid& providerID) override;

        // IRendererSceneEventSender
        void sendSubscribeScene(SceneId sceneId) override;
        void sendUnsubscribeScene(SceneId sceneId) override;
        void sendSceneStateChanged(SceneId masterScene, SceneId referencedScene, RendererSceneState newState) override;
        void sendSceneFlushed(SceneId masterScene, SceneId referencedScene, SceneVersionTag tag) override;
        void sendDataLinked(SceneId masterScene, SceneId providerScene, DataSlotId provider, SceneId consumerScene, DataSlotId consumer, bool success) override;
        void sendDataUnlinked(SceneId masterScene, SceneId consumerScene, DataSlotId consumer, bool success) override;

    private:
        PlatformLock&                 m_frameworkLock;
        ISceneGraphConsumerComponent& m_sceneGraphConsumerComponent;
        RendererCommandBuffer&        m_rendererCommands;

        HashMap<SceneId, std::pair<Guid, std::string> > m_sceneClients;
    };
}
