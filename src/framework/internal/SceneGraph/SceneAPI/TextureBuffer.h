//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "internal/SceneGraph/SceneAPI/TextureEnums.h"
#include "internal/SceneGraph/Resource/TextureMetaInfo.h"
#include "internal/Core/Math3d/Quad.h"
#include "internal/Core/Utils/AssertMovable.h"
#include <numeric>
#include <vector>

namespace ramses::internal
{
    struct MipMap
    {
        uint32_t              width = 0u;
        uint32_t              height = 0u;
        Quad                usedRegion;
        std::vector<std::byte>   data;
    };
    using MipMaps = std::vector<MipMap>;

    struct TextureBuffer
    {
        EPixelStorageFormat textureFormat = EPixelStorageFormat::Invalid;
        MipMaps             mipMaps;

        static uint32_t GetMipMapDataSizeInBytes(const TextureBuffer& buffer)
        {
            return std::accumulate(buffer.mipMaps.cbegin(), buffer.mipMaps.cend(), 0u, [](uint32_t val, const MipMap& mip) { return val + static_cast<uint32_t>(mip.data.size()); });
        }
    };

    ASSERT_MOVABLE(TextureBuffer)
}
