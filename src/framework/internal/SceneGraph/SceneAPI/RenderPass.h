//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/SceneTypes.h"
#include "internal/Core/Utils/AssertMovable.h"
#include "impl/DataTypesImpl.h"

namespace ramses::internal
{
    struct RenderPass
    {
        bool                   isEnabled = true;
        CameraHandle           camera;
        RenderTargetHandle     renderTarget;
        int32_t                renderOrder = 0;
        glm::vec4              clearColor{ 0.f, 0.f, 0.f, 1.f };
        ClearFlags             clearFlags = EClearFlag::All;
        bool                   isRenderOnce = false;

        RenderGroupOrderVector renderGroups;
    };

    ASSERT_MOVABLE(RenderPass)
}
