//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Scene/EScenePublicationMode.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneGraph/SceneAPI/SceneVersionTag.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/SceneTypes.h"

namespace ramses::internal
{
    class Guid;
    class ClientScene;
    class ISceneProviderServiceHandler;
    class ISceneProviderEventConsumer;
    struct FlushTimeInformation;

    class ISceneGraphProviderComponent
    {
    public:
        virtual ~ISceneGraphProviderComponent() = default;
        virtual void handleCreateScene(ClientScene& scene, bool enableLocalOnlyOptimization, ISceneProviderEventConsumer& eventInterface) = 0;
        virtual void handlePublishScene(SceneId sceneId, EScenePublicationMode publicationMode) = 0;
        virtual void handleUnpublishScene(SceneId sceneId) = 0;
        virtual bool handleFlush(SceneId sceneId, const FlushTimeInformation& flushTimeInfo, SceneVersionTag versionTag) = 0;
        virtual void handleRemoveScene(SceneId sceneId) = 0;
    };
}
