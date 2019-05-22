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

#include "PlatformAbstraction/PlatformLock.h"
#include "Collections/Pair.h"
#include "ISceneGraphSender.h"
#include "SceneAPI/SceneId.h"
#include "Collections/HashMap.h"
#include "TransportCommon/IConnectionStatusListener.h"
#include "Collections/HashSet.h"
#include "SceneAPI/SceneSizeInformation.h"
#include "Utils/IPeriodicLogSupplier.h"

namespace ramses_internal
{
    class Guid;
    class ClientSceneLogicBase;
    class IConnectionStatusUpdateNotifier;
    class ICommunicationSystem;
    class SceneActionCollection;

    const uint64_t SceneActionList_CounterWrapAround = 10000;

    class SceneGraphComponent final : public ISceneGraphProviderComponent, public ISceneGraphSender, public ISceneGraphConsumerComponent, public IConnectionStatusListener, public IPeriodicLogSupplier
    {
    public:
        SceneGraphComponent(const Guid& myID, ICommunicationSystem& communicationSystem, IConnectionStatusUpdateNotifier& connectionStatusUpdateNotifier, PlatformLock& frameworkLock);
        virtual ~SceneGraphComponent() override;

        virtual void setSceneRendererServiceHandler(ISceneRendererServiceHandler* sceneRendererHandler) override;
        virtual void setSceneProviderServiceHandler(ISceneProviderServiceHandler* handler) override;

        virtual void sendCreateScene(const Guid& to, const SceneInfo& sceneInfo, EScenePublicationMode mode) override;
        virtual void sendSceneActionList(const std::vector<Guid>& toVec, SceneActionCollection&& sceneAction, SceneId sceneId, EScenePublicationMode mode) override;
        virtual void sendPublishScene(SceneId sceneId, EScenePublicationMode mode, const String& name) override;
        virtual void sendUnpublishScene(SceneId sceneId, EScenePublicationMode mode) override;
        virtual void subscribeScene(const Guid& to, SceneId sceneId) override;
        virtual void unsubscribeScene(const Guid& to, SceneId sceneId) override;
        virtual void newParticipantHasConnected(const Guid& guid) override;
        virtual void participantHasDisconnected(const Guid& guid) override;

        virtual void handleCreateScene(ClientScene& scene, bool enableLocalOnlyOptimization) override;
        virtual void handlePublishScene(SceneId sceneId, EScenePublicationMode publicationMode) override;
        virtual void handleUnpublishScene(SceneId sceneId) override;
        virtual void handleFlush(SceneId sceneId, ESceneFlushMode flushMode, const FlushTimeInformation& flushTimeInfo, SceneVersionTag versionTag) override;
        virtual void handleRemoveScene(SceneId sceneId) override;
        virtual void handleSceneSubscription(SceneId sceneId, const Guid& subscriber) override;
        virtual void handleSceneUnsubscription(SceneId sceneId, const Guid& subscriber) override;

        virtual void triggerLogMessageForPeriodicLog() override;

        void disconnectFromNetwork();

    private:
        ISceneRendererServiceHandler* m_sceneRendererHandler;
        ISceneProviderServiceHandler* m_sceneProviderHandler;
        Guid m_myID;
        ICommunicationSystem& m_communicationSystem;
        IConnectionStatusUpdateNotifier& m_connectionStatusUpdateNotifier;

        PlatformLock& m_frameworkLock;

        struct PublishedSceneInfo
        {
            EScenePublicationMode publicationMode = EScenePublicationMode_Unpublished;
            String name;

            PublishedSceneInfo()
            {}

            PublishedSceneInfo(EScenePublicationMode publicationMode_, const String& name_)
                : publicationMode(publicationMode_)
                , name(name_)
            {}
        };

        typedef HashMap<SceneId, PublishedSceneInfo> SceneIdModePublishedSceneInfoMap;
        SceneIdModePublishedSceneInfoMap m_publishedScenes;

        typedef HashMap<SceneId, ClientSceneLogicBase*> ClientSceneLogicMap;
        ClientSceneLogicMap m_clientSceneLogicMap;

        typedef std::pair<Guid, SceneId> Subscription;
        typedef HashMap<Subscription, uint64_t > SceneActionListCountPerSubscription;
        SceneActionListCountPerSubscription m_subscriptions;
    };
}

#endif
