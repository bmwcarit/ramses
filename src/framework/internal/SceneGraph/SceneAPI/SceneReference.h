//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneGraph/SceneAPI/RendererSceneState.h"

namespace ramses::internal
{
    struct SceneReference
    {
        SceneId sceneId;
        RendererSceneState requestedState = RendererSceneState::Available;
        int32_t renderOrder = 0;
        bool flushNotifications = false;
    };
}
