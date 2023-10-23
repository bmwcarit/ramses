//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"

namespace ramses::internal
{
    class GPUResource
    {
    public:
        inline GPUResource(uint32_t gpuAddress, uint32_t totalSizeInBytes)
            : m_GPUAddress(gpuAddress)
            , m_sizeInBytes(totalSizeInBytes)
        {
        }

        virtual ~GPUResource() = default;

        [[nodiscard]] inline uint32_t getGPUAddress() const
        {
            return m_GPUAddress;
        }

        [[nodiscard]] inline uint32_t getTotalSizeInBytes() const
        {
            return m_sizeInBytes;
        }

    private:
        const uint32_t m_GPUAddress;
        const uint32_t m_sizeInBytes;
    };
}
