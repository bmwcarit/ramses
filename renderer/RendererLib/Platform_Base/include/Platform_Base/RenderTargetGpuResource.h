//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERTARGETGPURESOURCE_H
#define RAMSES_RENDERTARGETGPURESOURCE_H

#include "Platform_Base/GpuResource.h"

namespace ramses_internal
{
    class RenderTargetGPUResource : public GPUResource
    {
    public:
        RenderTargetGPUResource(UInt32 gpuAddress);
    };
}

#endif
