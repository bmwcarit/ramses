//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VERTEXBUFFERGPURESOURCE_H
#define RAMSES_VERTEXBUFFERGPURESOURCE_H

#include "Platform_Base/GpuResource.h"

namespace ramses_internal
{
    class VertexBufferGPUResource : public GPUResource
    {
    public:
        VertexBufferGPUResource(UInt32 gpuAddress, UInt32 totalSizeInBytes, UInt32 numComponentsPerElement)
            : GPUResource(gpuAddress, totalSizeInBytes)
            , m_numComponentsPerElement(numComponentsPerElement)
        {
        }

        UInt32 getNumComponentsPerElement() const
        {
            return m_numComponentsPerElement;
        }

    private:
        const UInt32 m_numComponentsPerElement;
    };
}

#endif
