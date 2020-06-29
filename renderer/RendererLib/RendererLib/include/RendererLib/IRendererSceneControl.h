//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IRENDERERSCENECONTROL_H
#define RAMSES_IRENDERERSCENECONTROL_H

#include "RendererAPI/Types.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/DataSlot.h"

namespace ramses_internal
{
    class IRendererSceneControl
    {
    public:
        virtual void handleSceneSubscriptionRequest(SceneId sceneId) = 0;
        virtual void handleSceneUnsubscriptionRequest(SceneId sceneId, bool indirect) = 0;
        virtual void handleSceneMappingRequest(SceneId sceneId, DisplayHandle handle) = 0;
        virtual void handleSceneUnmappingRequest(SceneId sceneId) = 0;
        virtual void handleSceneShowRequest(SceneId sceneId) = 0;
        virtual void handleSceneHideRequest(SceneId sceneId) = 0;
        virtual bool handleSceneDisplayBufferAssignmentRequest(SceneId sceneId, OffscreenBufferHandle buffer, int32_t sceneRenderOrder) = 0;
        virtual void handleSceneDataLinkRequest(SceneId providerSceneId, DataSlotId providerId, SceneId consumerSceneId, DataSlotId consumerId) = 0;
        virtual void handleBufferToSceneDataLinkRequest(OffscreenBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId) = 0;
        virtual void handleDataUnlinkRequest(SceneId consumerSceneId, DataSlotId consumerId) = 0;

        virtual ~IRendererSceneControl() = default;
    };
}

#endif
