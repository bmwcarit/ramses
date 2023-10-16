//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformBase/GpuResource.h"

namespace ramses::internal
{
    class IndexBufferGPUResource : public GPUResource
    {
    public:
        IndexBufferGPUResource(uint32_t gpuAddress, uint32_t totalSizeInBytes, uint32_t elementSizeInBytes)
            : GPUResource(gpuAddress, totalSizeInBytes)
            , m_elementSizeInBytes(elementSizeInBytes)
        {
        }

        [[nodiscard]] uint32_t getElementSizeInBytes() const
        {
            return m_elementSizeInBytes;
        }

    private:
        const uint32_t m_elementSizeInBytes;
    };
}
