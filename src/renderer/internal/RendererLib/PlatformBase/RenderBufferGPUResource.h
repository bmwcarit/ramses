//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformBase/GpuResource.h"
#include "internal/SceneGraph/SceneAPI/TextureEnums.h"

namespace ramses::internal
{
    class RenderBufferGPUResource : public GPUResource
    {
    public:
        RenderBufferGPUResource(uint32_t gpuAddress, uint32_t width, uint32_t height, EPixelStorageFormat format, uint32_t sampleCount, ERenderBufferAccessMode accessMode)
            : GPUResource(gpuAddress, width * height * GetTexelSizeFromFormat(format) * std::max(1u, sampleCount))
            , m_format(format)
            , m_sampleCount(sampleCount)
            , m_width(width)
            , m_height(height)
            , m_accessMode(accessMode)
        {
        }

        [[nodiscard]] EPixelStorageFormat getStorageFormat() const
        {
            return m_format;
        }

        [[nodiscard]] uint32_t getWidth() const
        {
            return m_width;
        }

        [[nodiscard]] uint32_t getHeight() const
        {
            return m_height;
        }

        [[nodiscard]] uint32_t getSampleCount() const
        {
            return m_sampleCount;
        }

        [[nodiscard]] ERenderBufferAccessMode getAccessMode() const
        {
            return m_accessMode;
        }

    private:
        EPixelStorageFormat m_format;
        uint32_t m_sampleCount;
        uint32_t m_width;
        uint32_t m_height;
        ERenderBufferAccessMode m_accessMode;
    };
}
