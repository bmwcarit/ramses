//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTUREMATHUTILS_H
#define RAMSES_TEXTUREMATHUTILS_H

#include "PlatformAbstraction/PlatformTypes.h"
#include <algorithm>
#include <cmath>
#include <cassert>

namespace ramses_internal
{
    class TextureMathUtils
    {
    public:
        static uint32_t GetLowerMipSize(uint32_t size)
        {
            return std::max<uint32_t>(1u, size >> 1);
        }

        static uint32_t GetMipSize(uint32_t mipLevel, uint32_t baseMipSize)
        {
            uint32_t mipSize = baseMipSize;
            for (uint32_t i = 0u; i < mipLevel; ++i)
            {
                mipSize = GetLowerMipSize(mipSize);
            }

            return mipSize;
        }

        static uint32_t GetMipLevelCount(uint32_t width, uint32_t height, uint32_t depth)
        {
            const uint32_t maxDimension = std::max(width, std::max(height, depth));
            assert(maxDimension > 0u);
            // add 0.5 so that floating precision errors do not cause value to fall below the integral part
            // 0.5 is small enough not to affect the log2 result and big enough to avoid the potential precision error
            // see glTexStorage2D OpenGL docs for this formula
            const float mipCountUnfloored = std::log2(static_cast<float>(maxDimension) + 0.5f);
            return 1u + static_cast<uint32_t>(mipCountUnfloored);
        }

        static uint32_t GetTotalMemoryUsedByMipmappedTexture(uint32_t bytesPerTexel, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevelCount)
        {
            uint32_t totalSizeInBytes = 0u;
            for (uint32_t i = 0u; i < mipLevelCount; ++i)
            {
                totalSizeInBytes += width * height * depth * bytesPerTexel;
                width = TextureMathUtils::GetLowerMipSize(width);
                height = TextureMathUtils::GetLowerMipSize(height);
                depth = TextureMathUtils::GetLowerMipSize(depth);
            }
            return totalSizeInBytes;
        }
    };
}

#endif
