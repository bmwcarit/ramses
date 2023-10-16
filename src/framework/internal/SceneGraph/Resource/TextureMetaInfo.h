//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "internal/SceneGraph/SceneAPI/TextureEnums.h"
#include "internal/PlatformAbstraction/Collections/Pair.h"

#include <cstdint>
#include <cassert>
#include <array>

namespace ramses::internal
{
    using MipDataSizeVector = std::vector<uint32_t>;
    using TextureSwizzleArray = std::array<ETextureChannelColor, 4>;
    constexpr TextureSwizzleArray DefaultTextureSwizzleArray{ ETextureChannelColor::Red, ETextureChannelColor::Green, ETextureChannelColor::Blue, ETextureChannelColor::Alpha };

    struct TextureMetaInfo
    {
        explicit TextureMetaInfo(uint32_t width = 0u, uint32_t height = 0u, uint32_t depth = 0u, EPixelStorageFormat format = EPixelStorageFormat::Invalid, bool generateMipChain = false, const TextureSwizzleArray swizzle = {}, MipDataSizeVector mipDataSizes = {})
            : m_width(width)
            , m_height(height)
            , m_depth(depth)
            , m_format(format)
            , m_generateMipChain(generateMipChain)
            , m_dataSizes(std::move(mipDataSizes))
            , m_swizzle(swizzle)
        {
        }

        uint32_t                    m_width;
        uint32_t                    m_height;
        uint32_t                    m_depth;
        EPixelStorageFormat         m_format;
        bool                        m_generateMipChain;
        MipDataSizeVector           m_dataSizes;
        TextureSwizzleArray         m_swizzle;
    };
}
