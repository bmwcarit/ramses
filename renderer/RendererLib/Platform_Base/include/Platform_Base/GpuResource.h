//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_GPURESOURCE_H
#define RAMSES_GPURESOURCE_H

#include "RendererAPI/Types.h"

namespace ramses_internal
{
    class GPUResource
    {
    public:
        inline GPUResource(uint32_t gpuAddress, uint32_t totalSizeInBytes)
            : m_GPUAddress(gpuAddress)
            , m_sizeInBytes(totalSizeInBytes)
        {
        }

        virtual ~GPUResource() {}

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

#endif
