//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VERTEXARRAYGPURESOURCE_H
#define RAMSES_VERTEXARRAYGPURESOURCE_H

#include "Platform_Base/GpuResource.h"

namespace ramses_internal
{
    class VertexArrayGPUResource : public GPUResource
    {
    public:
        VertexArrayGPUResource(UInt32 gpuAddress, DeviceResourceHandle indexBufferHandle)
            : GPUResource(gpuAddress, 0u)
            , m_indexBufferHandle(indexBufferHandle)
        {
        }

        DeviceResourceHandle getIndexBufferHandle() const
        {
            return m_indexBufferHandle;
        }

    private:
        DeviceResourceHandle m_indexBufferHandle;
    };
}

#endif
