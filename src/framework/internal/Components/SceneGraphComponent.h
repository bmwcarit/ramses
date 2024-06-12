//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ISceneGraphProviderComponent.h"
#include "ISceneGraphConsumerComponent.h"

#include "ISceneGraphSender.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneGraph/SceneAPI/SceneSizeInformation.h"
#include "internal/Communication/TransportCommon/IConnectionStatusListener.h"
#include "internal/Communication/TransportCommon/ServiceHandlerInterfaces.h"
#include "ISceneProviderEventConsumer.h"
#include "ERendererToClientEventType.h"
#include "ramses/framework/EFeatureLevel.h"
#include "internal/Core/Utils/IPeriodicLogSupplier.h"
#include "internal/PlatformAbstraction/PlatformLock.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"
#include "internal/PlatformAbstraction/Collections/HashSet.h"
#include "internal/PlatformAbstraction/Collections/Pair.h"
#include <unordered_map>

namespace ramses::internal
{
    class Guid;
    class ClientSceneLogicBase;
    class IConnectionStatusUpdateNotifier;
    class ICommunicationSystem;
    struct SceneUpdate;
    class ISceneRendererHandler;
    class SceneUpdateStreamDeserializer;
    class IResourceProviderComponent;

    class SceneGraphComponent final : public ISceneGraphProviderComponent,
                                      public ISceneGraphSender,
                                      public ISceneGraphConsumerComponent,
                                      public IConnectionStatusListener,
                                      public ISceneProviderServiceHandler,
                                      public ISceneRendererServiceHandler,
                                      public IPeriodicLogSupplier
    {
    public:
        SceneGraphComponent(
            const Guid& myID,
            ICommunicationSystem& communicationSystem,
            IConnectionStatusUpdateNotifier& connectionStatusUpdateNotifier,
            IResourceProviderComponent& res,
            PlatformLock& frameworkLock,
            EFeatureLevel featureLevel);
        ~SceneGraphComponent() override;

        void setSceneRendererHandler(ISceneRendererHandler* sceneRendererHandler) override;

        // ISceneGraphSender
        void sendCreateScene(const Guid& to, const SceneInfo& sceneInfo) override;
        void sendSceneUpdate(const std::vector<Guid>& toVec, SceneUpdate&& sceneUpdate, SceneId sceneId, EScenePublicationMode mode, StatisticCollectionScene& sceneStatistics) override;
        void sendPublishScene(const SceneInfo& sceneInfo) override;
        void sendUnpublishScene(SceneId sceneId, EScenePublicationMode mode) override;

        // ISceneGraphConsumerComponent
        void subscribeScene(const Guid& to, SceneId sceneId) override;
        void unsubscribeScene(const Guid& to, SceneId sceneId) override;
        void sendSceneReferenceEvent(const Guid& to, SceneReferenceEvent const& event) override;
        void sendResourceAvailabilityEvent(const Guid& to, ResourceAvailabilityEvent const& event) override;

        // IConnectionStatusListener
        void newParticipantHasConnected(const Guid& connnectedParticipant) override;
        void participantHasDisconnected(const Guid& disconnnectedParticipant) override;

        // ISceneGraphProviderComponent
        void handleCreateScene(ClientScene& scene, bool enableLocalOnlyOptimization, ISceneProviderEventConsumer& eventConsumer) override;
        void handlePublishScene(SceneId sceneId, EScenePublicationMode publicationMode) override;
        void handleUnpublishScene(SceneId sceneId) override;
        bool handleFlush(SceneId sceneId, const FlushTimeInformation& flushTimeInfo, SceneVersionTag versionTag) override;
        void handleRemoveScene(SceneId sceneId) override;

        // ISceneProviderServiceHandler
        void handleSubscribeScene(const SceneId& sceneId, const Guid& consumerID) override;
        void handleUnsubscribeScene(const SceneId& sceneId, const Guid& consumerID) override;
        void handleRendererEvent(const SceneId& sceneId, const std::vector<std::byte>& data, const Guid& rendererID) override;

        // ISceneRendererServiceHandler
        void handleInitializeScene(const SceneId& sceneId, const Guid& providerID) override;
        void handleSceneUpdate(const SceneId& sceneId, absl::Span<const std::byte> actionData, const Guid& providerID) override;
        void handleNewScenesAvailable(const SceneInfoVector& newScenes, const Guid& providerID, EFeatureLevel featureLevel) override;
        void handleScenesBecameUnavailable(const SceneInfoVector& unavailableScenes, const Guid& providerID) override;
        void handleSceneNotAvailable(const SceneId& sceneId, const Guid& providerID) override;

        // IPeriodicLogSupplier
        void triggerLogMessageForPeriodicLog() override;

        void connectToNetwork();
        void disconnectFromNetwork();

        // for testing only
        [[nodiscard]] const ClientSceneLogicBase* getClientSceneLogicForScene(SceneId sceneId) const;

    private:
        void forwardToSceneProviderEventConsumer(SceneReferenceEvent const& event);
        void forwardToSceneProviderEventConsumer(ResourceAvailabilityEvent const& event);

        ISceneRendererHandler* m_sceneRendererHandler;
        Guid m_myID;
        ICommunicationSystem& m_communicationSystem;
        IConnectionStatusUpdateNotifier& m_connectionStatusUpdateNotifier;

        PlatformLock& m_frameworkLock;

        HashMap<SceneId, SceneInfo> m_locallyPublishedScenes;

        using ClientSceneLogicMap = HashMap<SceneId, ClientSceneLogicBase *>;
        ClientSceneLogicMap m_clientSceneLogicMap;

        using SceneEventConsumerMap = HashMap<SceneId, ISceneProviderEventConsumer *>;
        SceneEventConsumerMap m_sceneEventConsumers;

        IResourceProviderComponent& m_resourceComponent;

        EFeatureLevel m_featureLevel = EFeatureLevel_Latest;

        struct ReceivedScene
        {
            SceneInfo info;
            Guid provider;
            std::unique_ptr<SceneUpdateStreamDeserializer> sceneUpdateDeserializer;
        };

        std::unordered_map<SceneId, ReceivedScene> m_remoteScenes;

        bool m_connected = false;
    };
}
