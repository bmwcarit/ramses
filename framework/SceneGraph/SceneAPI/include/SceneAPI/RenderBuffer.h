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
        RenderBuffer(UInt32 _width, UInt32 _height, ERenderBufferType _type, ETextureFormat _format, ERenderBufferAccessMode _accessMode, UInt32 _sampleCount)
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

        UInt32                  width = 0u;
        UInt32                  height = 0u;
        ERenderBufferType       type = ERenderBufferType_InvalidBuffer;
        ETextureFormat          format = ETextureFormat::Invalid;
        ERenderBufferAccessMode accessMode = ERenderBufferAccessMode_Invalid;
        UInt32                  sampleCount = 0u;
    };
}

#endif
