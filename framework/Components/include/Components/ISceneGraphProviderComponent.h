//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ISCENEGRAPHPROVIDERCOMPONENT_H
#define RAMSES_ISCENEGRAPHPROVIDERCOMPONENT_H

#include "Scene/EScenePublicationMode.h"
#include "Scene/ESceneFlushMode.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/SceneVersionTag.h"

namespace ramses_internal
{
    class Guid;
    class ClientScene;
    class ISceneProviderServiceHandler;
    struct FlushTimeInformation;

    class ISceneGraphProviderComponent
    {
    public:
        virtual ~ISceneGraphProviderComponent() {}
        virtual void setSceneProviderServiceHandler(ISceneProviderServiceHandler* handler) = 0;
        virtual void handleCreateScene(ClientScene& scene, bool enableLocalOnlyOptimization) = 0;
        virtual void handlePublishScene(SceneId sceneId, EScenePublicationMode publicationMode) = 0;
        virtual void handleUnpublishScene(SceneId sceneId) = 0;
        virtual void handleFlush(SceneId sceneId, ESceneFlushMode flushMode, const FlushTimeInformation& flushTimeInfo, SceneVersionTag versionTag) = 0;
        virtual void handleRemoveScene(SceneId sceneId) = 0;
        virtual void handleSceneSubscription(SceneId sceneId, const Guid& subscriber) = 0;
        virtual void handleSceneUnsubscription(SceneId sceneId, const Guid& subscriber) = 0;
    };
}

#endif
