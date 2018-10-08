//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FRAMEBUFFERINFO_H
#define RAMSES_FRAMEBUFFERINFO_H

#include "RendererAPI/Types.h"
#include "SceneAPI/Viewport.h"
#include "Math3d/ProjectionParams.h"

namespace ramses_internal
{
    struct FrameBufferInfo
    {
        FrameBufferInfo(DeviceResourceHandle devHandle, const ProjectionParams& projParams, const Viewport& vport)
            : deviceHandle(devHandle)
            , projectionParams(projParams)
            , viewport(vport)
        {
        }

        DeviceResourceHandle deviceHandle;
        ProjectionParams     projectionParams;
        Viewport             viewport;
    };
}

#endif
