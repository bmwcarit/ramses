//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"
#include "internal/RendererLib/SceneRenderExecutionIterator.h"
#include "internal/SceneGraph/SceneAPI/Viewport.h"
#include "internal/SceneGraph/SceneAPI/RenderState.h"

namespace ramses::internal
{
    struct RenderingContext
    {
        DeviceResourceHandle displayBufferDeviceHandle;
        uint32_t viewportWidth = 0u;
        uint32_t viewportHeight = 0u;
        SceneRenderExecutionIterator renderFrom;

        ClearFlags displayBufferClearPending = EClearFlag::None;
        glm::vec4 displayBufferClearColor{};
        bool displayBufferDepthDiscard = false;
    };
}
