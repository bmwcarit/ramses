//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/TextureEnums.h"

namespace ramses::internal
{
    struct RenderBuffer
    {
        RenderBuffer() = default;
        RenderBuffer(uint32_t _width, uint32_t _height, EPixelStorageFormat _format, ERenderBufferAccessMode _accessMode, uint32_t _sampleCount)
            : width(_width)
            , height(_height)
            , format(_format)
            , accessMode(_accessMode)
            , sampleCount(_sampleCount)
        {
        }

        bool operator==(const RenderBuffer& other) const
        {
            return format == other.format
                && width == other.width
                && height == other.height
                && accessMode == other.accessMode
                && sampleCount == other.sampleCount;
        }

        bool operator!=(const RenderBuffer& other) const
        {
            return !this->operator==(other);
        }

        uint32_t                width = 0u;
        uint32_t                height = 0u;
        EPixelStorageFormat     format = EPixelStorageFormat::Invalid;
        ERenderBufferAccessMode accessMode = ERenderBufferAccessMode::ReadWrite;
        uint32_t                sampleCount = 0u;
    };
}
