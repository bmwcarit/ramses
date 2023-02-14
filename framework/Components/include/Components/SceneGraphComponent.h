//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEGRAPHCOMPONENT_H
#define RAMSES_SCENEGRAPHCOMPONENT_H

#include "ISceneGraphProviderComponent.h"
#include "ISceneGraphConsumerComponent.h"

#include "ISceneGraphSender.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/SceneSizeInformation.h"
#include "TransportCommon/IConnectionStatusListener.h"
#include "TransportCommon/ServiceHandlerInterfaces.h"
#include "ISceneProviderEventConsumer.h"
#include "ERendererToClientEventType.h"
#include "ramses-framework-api/EFeatureLevel.h"
#include "Utils/IPeriodicLogSupplier.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "Collections/HashMap.h"
#include "Collections/HashSet.h"
#include "Collections/Pair.h"
#include <unordered_map>

namespace ramses_internal
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
            ramses::EFeatureLevel featureLevel);
        virtual ~SceneGraphComponent() override;

        virtual void setSceneRendererHandler(ISceneRendererHandler* sceneRendererHandler) override;

        // ISceneGraphSender
        virtual void sendCreateScene(const Guid& to, const SceneId& sceneId, EScenePublicationMode mode) override;
        virtual void sendSceneUpdate(const std::vector<Guid>& toVec, SceneUpdate&& sceneUpdate, SceneId sceneId, EScenePublicationMode mode, StatisticCollectionScene& sceneStatistics) override;
        virtual void sendPublishScene(SceneId sceneId, EScenePublicationMode mode, const String& name) override;
        virtual void sendUnpublishScene(SceneId sceneId, EScenePublicationMode mode) override;

        // ISceneGraphConsumerComponent
        virtual void subscribeScene(const Guid& to, SceneId sceneId) override;
        virtual void unsubscribeScene(const Guid& to, SceneId sceneId) override;
        virtual void sendSceneReferenceEvent(const Guid& to, SceneReferenceEvent const& event) override;
        virtual void sendResourceAvailabilityEvent(const Guid& to, ResourceAvailabilityEvent const& event) override;

        // IConnectionStatusListener
        virtual void newParticipantHasConnected(const Guid& connnectedParticipant) override;
        virtual void participantHasDisconnected(const Guid& disconnnectedParticipant) override;

        // ISceneGraphProviderComponent
        virtual void handleCreateScene(ClientScene& scene, bool enableLocalOnlyOptimization, ISceneProviderEventConsumer& eventConsumer) override;
        virtual void handlePublishScene(SceneId sceneId, EScenePublicationMode publicationMode) override;
        virtual void handleUnpublishScene(SceneId sceneId) override;
        virtual bool handleFlush(SceneId sceneId, const FlushTimeInformation& flushTimeInfo, SceneVersionTag versionTag) override;
        virtual void handleRemoveScene(SceneId sceneId) override;

        // ISceneProviderServiceHandler
        virtual void handleSubscribeScene(const SceneId& sceneId, const Guid& consumerID) override;
        virtual void handleUnsubscribeScene(const SceneId& sceneId, const Guid& consumerID) override;
        virtual void handleRendererEvent(const SceneId& sceneId, const std::vector<Byte>& data, const Guid& rendererID) override;

        // ISceneRendererServiceHandler
        virtual void handleInitializeScene(const SceneId& sceneId, const Guid& providerID) override;
        virtual void handleSceneUpdate(const SceneId& sceneId, absl::Span<const Byte> actionData, const Guid& providerID) override;
        virtual void handleNewScenesAvailable(const SceneInfoVector& newScenes, const Guid& providerID, ramses::EFeatureLevel featureLevel) override;
        virtual void handleScenesBecameUnavailable(const SceneInfoVector& unavailableScenes, const Guid& providerID) override;
        virtual void handleSceneNotAvailable(const SceneId& sceneId, const Guid& providerID) override;

        // IPeriodicLogSupplier
        virtual void triggerLogMessageForPeriodicLog() override;

        void connectToNetwork();
        void disconnectFromNetwork();

        // for testing only
        const ClientSceneLogicBase* getClientSceneLogicForScene(SceneId sceneId) const;

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

        ramses::EFeatureLevel m_featureLevel = ramses::EFeatureLevel_01;

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

#endif
