//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CLIENTSCENELOGICBASE_H
#define RAMSES_CLIENTSCENELOGICBASE_H

#include "Scene/EScenePublicationMode.h"
#include "Scene/ClientScene.h"
#include "Scene/Scene.h"

namespace ramses_internal
{
    class ISceneGraphSender;
    class StatisticCollectionScene;
    struct FlushTimeInformation;
    class IResourceProviderComponent;
    struct SceneUpdate;

    class ClientSceneLogicBase
    {
    public:
        ClientSceneLogicBase(ISceneGraphSender& sceneGraphSender, ClientScene& scene, IResourceProviderComponent& res, const Guid& clientAddress);
        virtual ~ClientSceneLogicBase();

        void publish(EScenePublicationMode publicationMode);
        void unpublish();
        [[nodiscard]] bool isPublished() const;
        void addSubscriber(const Guid& newSubscriber);
        void removeSubscriber(const Guid& subscriber);

        [[nodiscard]] std::vector<Guid> getWaitingAndActiveSubscribers() const;

        virtual bool flushSceneActions(const FlushTimeInformation& flushTimeInfo, SceneVersionTag versionTag) = 0;

        [[nodiscard]] const char* getSceneStateString() const;

    protected:
        enum class ResourceChangeState {
            MissingResource,
            NoChange,
            HasChanges,
        };

        virtual void postAddSubscriber() {};
        void sendSceneToWaitingSubscribers(const IScene& scene, const FlushTimeInformation& flushTimeInfo, SceneVersionTag versionTag);
        void printFlushInfo(StringOutputStream& sos, const char* name, const SceneUpdate& update) const;
        ResourceChangeState verifyAndGetResourceChanges(SceneUpdate& sceneUpdate, bool hasNewActions);
        void updateResourceStatistics();
        void fillStatisticsCollection();

        ISceneGraphSender&     m_scenegraphSender;
        IResourceProviderComponent& m_resourceComponent;
        const Guid             m_myID;
        const SceneId          m_sceneId;
        ClientScene&           m_scene;

        using AddressVector = std::vector<Guid>;
        AddressVector  m_subscribersActive;
        AddressVector  m_subscribersWaitingForScene;
        EScenePublicationMode m_scenePublicationMode;

        uint64_t m_flushCounter = 0u;
        ResourceContentHashVector m_lastFlushResourcesInUse;

        ResourceChanges m_resourceChangesSinceLastFlush; // keep container memory allocated
        ResourceContentHashVector m_currentFlushResourcesInUse; // keep container memory allocated

        // resource statistics gathered while flushing the last time
        std::array<uint64_t, EResourceStatisticIndex_NumIndices> m_resourceCount;
        std::array<uint64_t, EResourceStatisticIndex_NumIndices> m_resourceDataSize;
        std::array<uint64_t, EResourceStatisticIndex_NumIndices> m_resourceMaxSize;
    };
}

#endif
