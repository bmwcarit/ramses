//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERBUFFERGPURESOURCE_H
#define RAMSES_RENDERBUFFERGPURESOURCE_H

#include "Platform_Base/GpuResource.h"

namespace ramses_internal
{
    class RenderBufferGPUResource : public GPUResource
    {
    public:
        RenderBufferGPUResource(UInt32 gpuAddress, UInt32 width, UInt32 height, ERenderBufferType type, ETextureFormat format, UInt32 sampleCount, ERenderBufferAccessMode accessMode)
            : GPUResource(gpuAddress, width * height * GetTexelSizeFromFormat(format) * std::max(1u, sampleCount))
            , m_type(type)
            , m_format(format)
            , m_sampleCount(sampleCount)
            , m_width(width)
            , m_height(height)
            , m_accessMode(accessMode)
        {
        }

        ERenderBufferType getType() const
        {
            return m_type;
        }

        ETextureFormat getStorageFormat() const
        {
            return m_format;
        }

        UInt32 getWidth() const
        {
            return m_width;
        }

        UInt32 getHeight() const
        {
            return m_height;
        }

        UInt32 getSampleCount() const
        {
            return m_sampleCount;
        }

        ERenderBufferAccessMode getAccessMode() const
        {
            return m_accessMode;
        }

    private:
        const ERenderBufferType m_type;
        const ETextureFormat m_format;
        const UInt32 m_sampleCount;
        const UInt32 m_width;
        const UInt32 m_height;
        const ERenderBufferAccessMode m_accessMode;
    };
}

#endif
