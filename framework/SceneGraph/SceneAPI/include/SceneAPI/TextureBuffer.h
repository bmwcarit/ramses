//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTERNAL_TEXTUREBUFFER_H
#define RAMSES_INTERNAL_TEXTUREBUFFER_H

#include "Collections/Vector.h"
#include "SceneAPI/TextureEnums.h"
#include "SceneAPI/MipMapSize.h"
#include "Resource/TextureMetaInfo.h"
#include <numeric>

namespace ramses_internal
{
    using MipMapData = Vector<Vector<Byte>>;

    struct TextureBuffer
    {
        TextureBuffer() = default;
        TextureBuffer(const TextureBuffer&) = default;
        TextureBuffer& operator=(const  TextureBuffer&) = default;
        TextureBuffer(TextureBuffer&&) = default;
        TextureBuffer& operator=(TextureBuffer&&) = default;

        ETextureFormat      textureFormat = ETextureFormat_Invalid;
        MipMapDimensions    mipMapDimensions;
        MipMapData          mipMapData;

        static UInt32 GetMipMapDataSizeInBytes(const TextureBuffer& buffer)
        {
            return std::accumulate(buffer.mipMapData.cbegin(), buffer.mipMapData.cend(), 0u, [](UInt32 val, const Vector<Byte>& data) { return val + static_cast<UInt32>(data.size()); });
        }
    };

    static_assert(std::is_nothrow_move_constructible<TextureBuffer>::value &&
        std::is_nothrow_move_assignable<TextureBuffer>::value, "TextureBuffer must be movable");
}

#endif
