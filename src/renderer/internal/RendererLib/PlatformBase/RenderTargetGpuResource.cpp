//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/PlatformBase/RenderTargetGpuResource.h"


namespace ramses::internal
{
    RenderTargetGPUResource::RenderTargetGPUResource(uint32_t gpuAddress)
        : GPUResource(gpuAddress, 0u)
    {
    }
}
