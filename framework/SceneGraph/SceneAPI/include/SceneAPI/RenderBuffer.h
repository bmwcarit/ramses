//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTERNAL_RENDERBUFFER_H
#define RAMSES_INTERNAL_RENDERBUFFER_H

#include "SceneAPI/TextureEnums.h"

namespace ramses_internal
{
    struct RenderBuffer
    {
        RenderBuffer() = default;
        RenderBuffer(uint32_t _width, uint32_t _height, ERenderBufferType _type, ETextureFormat _format, ERenderBufferAccessMode _accessMode, uint32_t _sampleCount)
            : width(_width)
            , height(_height)
            , type(_type)
            , format(_format)
            , accessMode(_accessMode)
            , sampleCount(_sampleCount)
        {
        }

        bool operator==(const RenderBuffer& other) const
        {
            return type == other.type
                && format == other.format
                && width == other.width
                && height == other.height
                && accessMode == other.accessMode
                && sampleCount == other.sampleCount;
        }

        bool operator!=(const RenderBuffer& other) const
        {
            return !this->operator==(other);
        }

        uint32_t                  width = 0u;
        uint32_t                  height = 0u;
        ERenderBufferType       type = ERenderBufferType_InvalidBuffer;
        ETextureFormat          format = ETextureFormat::Invalid;
        ERenderBufferAccessMode accessMode = ERenderBufferAccessMode_Invalid;
        uint32_t                  sampleCount = 0u;
    };
}

#endif
