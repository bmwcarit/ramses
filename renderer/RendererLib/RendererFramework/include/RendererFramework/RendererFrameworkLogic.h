//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERFRAMEWORKLOGIC_H
#define RAMSES_RENDERERFRAMEWORKLOGIC_H

#include "RendererFramework/IResourceProvider.h"
#include "RendererFramework/IRendererSceneEventSender.h"
#include "Scene/SceneActionCollection.h"
#include "Collections/HashMap.h"
#include "Collections/Pair.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "Utils/Warnings.h"
#include "Components/ISceneRendererHandler.h"
#include <unordered_map>

namespace ramses_internal
{
    class ISceneGraphConsumerComponent;
    class IResourceConsumerComponent;
    class RendererCommandBuffer;
    class IConnectionStatusUpdateNotifier;

    class RendererFrameworkLogic
        : public ISceneRendererHandler
        , public IResourceProvider
        , public IRendererSceneEventSender
    {
    public:
        RendererFrameworkLogic(
            IResourceConsumerComponent& res,
            ISceneGraphConsumerComponent& sgc,
            RendererCommandBuffer& rendererCommandBuffer,
            PlatformLock& frameworkLock);
        virtual ~RendererFrameworkLogic();

        // ISceneRendererHandler
        virtual void handleInitializeScene(const SceneInfo& sceneInfo, const Guid& providerID) override;
        virtual void handleSceneUpdate(const SceneId& sceneId, SceneUpdate&& sceneUpdate, const Guid& providerID) override;
        virtual void handleNewSceneAvailable(const SceneInfo& newScene, const Guid& providerID) override;
        virtual void handleSceneBecameUnavailable(const SceneId& unavailableScene, const Guid& providerID) override;

        // IResourceProviderServiceHandler
        virtual void requestResourceAsyncronouslyFromFramework(const ResourceContentHashVector& ids, const ResourceRequesterID& requesterID, const SceneId& sceneId) override;
        virtual void cancelResourceRequest(const ResourceContentHash& resourceHash, const ResourceRequesterID& requesterID) override;
        virtual ManagedResourceVector popArrivedResources(const ResourceRequesterID& requesterID) override;

        // IRendererSceneEventSender
        virtual void sendSubscribeScene(SceneId sceneId) override;
        virtual void sendUnsubscribeScene(SceneId sceneId) override;
        virtual void sendSceneStateChanged(SceneId masterScene, SceneId referencedScene, RendererSceneState newState) override;
        virtual void sendSceneFlushed(SceneId masterScene, SceneId referencedScene, SceneVersionTag tag) override;
        virtual void sendDataLinked(SceneId masterScene, SceneId providerScene, DataSlotId provider, SceneId consumerScene, DataSlotId consumer, bool success) override;
        virtual void sendDataUnlinked(SceneId masterScene, SceneId consumerScene, DataSlotId consumer, bool success) override;

    private:
        PlatformLock&                 m_frameworkLock;
        ISceneGraphConsumerComponent& m_sceneGraphConsumerComponent;
        IResourceConsumerComponent&   m_resourceComponent;
        RendererCommandBuffer&        m_rendererCommands;

        HashMap<SceneId, std::pair<Guid, String> > m_sceneClients;
    };
}

#endif
