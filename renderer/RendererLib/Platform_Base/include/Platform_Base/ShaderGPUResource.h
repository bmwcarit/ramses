//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SHADERGPURESOURCE_H
#define RAMSES_SHADERGPURESOURCE_H

#include "Platform_Base/GpuResource.h"

namespace ramses_internal
{
    class ShaderGPUResource : public GPUResource
    {
    public:
        explicit ShaderGPUResource(UInt32 gpuAddress)
            : GPUResource(gpuAddress, 1u) // Giving shader size 1 byte as the footprint of shader in GPU memory is driver/platform specific and it should anyway not affect any measuring
        {
        }
    };
}

#endif
