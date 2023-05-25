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
        inline GPUResource(UInt32 gpuAddress, UInt32 totalSizeInBytes)
            : m_GPUAddress(gpuAddress)
            , m_sizeInBytes(totalSizeInBytes)
        {
        }

        virtual ~GPUResource() {}

        [[nodiscard]] inline UInt32 getGPUAddress() const
        {
            return m_GPUAddress;
        }

        [[nodiscard]] inline UInt32 getTotalSizeInBytes() const
        {
            return m_sizeInBytes;
        }

    private:
        const UInt32 m_GPUAddress;
        const UInt32 m_sizeInBytes;
    };
}

#endif
