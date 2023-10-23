//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformBase/GpuResource.h"

namespace ramses::internal
{
    class ShaderGPUResource : public GPUResource
    {
    public:
        explicit ShaderGPUResource(uint32_t gpuAddress)
            : GPUResource(gpuAddress, 1u) // Giving shader size 1 byte as the footprint of shader in GPU memory is driver/platform specific and it should anyway not affect any measuring
        {
        }
    };
}
