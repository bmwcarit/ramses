//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformBase/GpuResource.h"

namespace ramses::internal
{
    class VertexArrayGPUResource : public GPUResource
    {
    public:
        VertexArrayGPUResource(uint32_t gpuAddress, DeviceResourceHandle indexBufferHandle)
            : GPUResource(gpuAddress, 0u)
            , m_indexBufferHandle(indexBufferHandle)
        {
        }

        [[nodiscard]] DeviceResourceHandle getIndexBufferHandle() const
        {
            return m_indexBufferHandle;
        }

    private:
        DeviceResourceHandle m_indexBufferHandle;
    };
}
