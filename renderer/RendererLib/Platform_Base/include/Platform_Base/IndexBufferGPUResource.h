//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INDEXBUFFERGPURESOURCE_H
#define RAMSES_INDEXBUFFERGPURESOURCE_H

#include "Platform_Base/GpuResource.h"

namespace ramses_internal
{
    class IndexBufferGPUResource : public GPUResource
    {
    public:
        IndexBufferGPUResource(UInt32 gpuAddress, UInt32 totalSizeInBytes, UInt32 elementSizeInBytes)
            : GPUResource(gpuAddress, totalSizeInBytes)
            , m_elementSizeInBytes(elementSizeInBytes)
        {
        }

        UInt32 getElementSizeInBytes() const
        {
            return m_elementSizeInBytes;
        }

    private:
        const UInt32 m_elementSizeInBytes;
    };
}

#endif
