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
#include "Resource/TextureMetaInfo.h"
#include "Math3d/Quad.h"
#include "Utils/AssertMovable.h"
#include <numeric>
#include <vector>

namespace ramses_internal
{
    struct MipMap
    {
        uint32_t              width = 0u;
        uint32_t              height = 0u;
        Quad                usedRegion;
        std::vector<Byte>   data;
    };
    using MipMaps = std::vector<MipMap>;

    struct TextureBuffer
    {
        ETextureFormat      textureFormat = ETextureFormat::Invalid;
        MipMaps             mipMaps;

        static uint32_t GetMipMapDataSizeInBytes(const TextureBuffer& buffer)
        {
            return std::accumulate(buffer.mipMaps.cbegin(), buffer.mipMaps.cend(), 0u, [](uint32_t val, const MipMap& mip) { return val + static_cast<uint32_t>(mip.data.size()); });
        }
    };

    ASSERT_MOVABLE(TextureBuffer)
}

#endif
