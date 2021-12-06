//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERINGCONTEXT_H
#define RAMSES_RENDERINGCONTEXT_H

#include "RendererAPI/Types.h"
#include "RendererAPI/SceneRenderExecutionIterator.h"
#include "SceneAPI/Viewport.h"
#include "SceneAPI/RenderState.h"
#include "Math3d/Vector4.h"

namespace ramses_internal
{
    struct RenderingContext
    {
        DeviceResourceHandle displayBufferDeviceHandle;
        uint32_t viewportWidth = 0u;
        uint32_t viewportHeight = 0u;
        SceneRenderExecutionIterator renderFrom;

        uint32_t displayBufferClearPending = EClearFlags_None;
        Vector4 displayBufferClearColor;
        bool displayBufferDepthDiscard = false;
    };
}

#endif
