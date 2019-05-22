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
#include "Scene/ESceneFlushMode.h"
#include "Scene/ClientScene.h"
#include "Animation/AnimationSystemFactory.h"
#include "Scene/Scene.h"

namespace ramses_internal
{
    class ISceneGraphSender;
    class StatisticCollectionScene;
    struct FlushTimeInformation;

    class ClientSceneLogicBase
    {
    public:
        ClientSceneLogicBase(ISceneGraphSender& sceneGraphSender, ClientScene& scene, const Guid& clientAddress);
        virtual ~ClientSceneLogicBase();

        void publish(EScenePublicationMode publicationMode);
        void unpublish();
        Bool isPublished() const;
        void addSubscriber(const Guid& newSubscriber);
        void removeSubscriber(const Guid& subscriber);

        std::vector<Guid> getWaitingAndActiveSubscribers() const;

        virtual void flushSceneActions(ESceneFlushMode flushMode, const FlushTimeInformation& flushTimeInfo, SceneVersionTag versionTag) = 0;

        const char* getSceneStateString() const;

    protected:
        virtual void postAddSubscriber() {};
        void sendSceneToWaitingSubscribers(const IScene& scene, const FlushTimeInformation& flushTimeInfo, SceneVersionTag versionTag);
        void printFlushInfo(StringOutputStream& sos, const char* name, const SceneActionCollection& collection, ESceneFlushMode flushMode) const;

        ISceneGraphSender&     m_scenegraphSender;
        const Guid             m_myID;
        const SceneId          m_sceneId;
        ClientScene&           m_scene;

        typedef std::vector<Guid> AddressVector;
        AddressVector  m_subscribersActive;
        AddressVector  m_subscribersWaitingForScene;
        EScenePublicationMode m_scenePublicationMode;

        UInt64                 m_flushCounter = 0u;
        AnimationSystemFactory m_animationSystemFactory;
    };
}

#endif
