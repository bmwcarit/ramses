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
#include "Scene/SceneActionCollection.h"
#include "TransportCommon/IConnectionStatusListener.h"
#include "Collections/HashMap.h"
#include "Collections/Pair.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "TransportCommon/ServiceHandlerInterfaces.h"
#include "Utils/Warnings.h"
#include <unordered_map>
#include "Scene/EScenePublicationMode.h"

namespace ramses_internal
{
    class ISceneGraphConsumerComponent;
    class IResourceConsumerComponent;
    class RendererCommandBuffer;
    class IConnectionStatusUpdateNotifier;

    class RendererFrameworkLogic
        : public ISceneRendererServiceHandler
        , public IResourceProvider
        , private IConnectionStatusListener
    {
    public:
        RendererFrameworkLogic(
            IConnectionStatusUpdateNotifier& connectionStatusUpdateNotifier,
            IResourceConsumerComponent& res,
            ISceneGraphConsumerComponent& sgc,
            RendererCommandBuffer& rendererCommandBuffer,
            PlatformLock& frameworkLock);
        virtual ~RendererFrameworkLogic();

        // ISceneRendererServiceHandler
        virtual void handleInitializeScene(const SceneInfo& sceneInfo, const Guid& providerID) override;
        virtual void handleSceneNotAvailable(const SceneId& sceneId, const Guid& providerID) override;
        virtual void handleSceneActionList(const SceneId& sceneId, SceneActionCollection&& actions, const uint64_t& counter, const Guid& providerID) override;
        virtual void handleNewScenesAvailable(const SceneInfoVector& newScenes, const Guid& providerID, EScenePublicationMode mode) override;
        virtual void handleScenesBecameUnavailable(const SceneInfoVector& unavailableScenes, const Guid& providerID) override;

        // IResourceProviderServiceHandler
        virtual void requestResourceAsyncronouslyFromFramework(const ResourceContentHashVector& ids, const RequesterID& requesterID, const SceneId& sceneId) override;
        virtual void cancelResourceRequest(const ResourceContentHash& resourceHash, const RequesterID& requesterID) override;
        virtual ManagedResourceVector popArrivedResources(const RequesterID& requesterID) override;

        virtual void newParticipantHasConnected(const Guid& guid) override;
        virtual void participantHasDisconnected(const Guid& guid) override;

    private:
        bool isSceneActionListCounterValid(const SceneId& sceneId, const uint64_t& counter);
        PlatformLock&                 m_frameworkLock;
        IConnectionStatusUpdateNotifier& m_connectionStatusUpdateNotifier;
        ISceneGraphConsumerComponent& m_sceneGraphConsumerComponent;
        IResourceConsumerComponent&   m_resourceComponent;
        RendererCommandBuffer&        m_rendererCommands;

        HashMap<SceneId, std::pair<Guid, String> > m_sceneClients;
        std::unordered_map<SceneId, SceneActionCollection>   m_bufferedSceneActionsPerScene;
        std::unordered_map<SceneId, uint64_t>   m_lastReceivedListCounter;
    };
}

#endif
