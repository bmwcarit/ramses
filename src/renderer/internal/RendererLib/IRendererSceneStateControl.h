//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneGraph/SceneAPI/DataSlot.h"

namespace ramses::internal
{
    class IRendererSceneStateControl
    {
    public:
        virtual void handleSceneSubscriptionRequest(SceneId sceneId) = 0;
        virtual void handleSceneUnsubscriptionRequest(SceneId sceneId, bool indirect) = 0;
        virtual void handleSceneMappingRequest(SceneId sceneId) = 0;
        virtual void handleSceneUnmappingRequest(SceneId sceneId) = 0;
        virtual void handleSceneShowRequest(SceneId sceneId) = 0;
        virtual void handleSceneHideRequest(SceneId sceneId) = 0;
        virtual bool handleSceneDisplayBufferAssignmentRequest(SceneId sceneId, OffscreenBufferHandle buffer, int32_t sceneRenderOrder) = 0;

        virtual ~IRendererSceneStateControl() = default;
    };
}
