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
#include "PlatformAbstraction/PlatformMath.h"
#include <assert.h>

namespace ramses_internal
{
    class TextureMathUtils
    {
    public:
        static UInt32 GetLowerMipSize(UInt32 size)
        {
            return ramses_internal::max<UInt32>(1u, size >> 1);
        }

        static UInt32 GetMipSize(UInt32 mipLevel, UInt32 baseMipSize)
        {
            UInt32 mipSize = baseMipSize;
            for (UInt32 i = 0u; i < mipLevel; ++i)
            {
                mipSize = GetLowerMipSize(mipSize);
            }

            return mipSize;
        }

        static UInt32 GetMipLevelCount(UInt32 width, UInt32 height, UInt32 depth)
        {
            const UInt32 maxDimension = ramses_internal::max<UInt32>(width, ramses_internal::max<UInt32>(height, depth));
            assert(maxDimension > 0u);
            // add 0.5 so that floating precision errors do not cause value to fall below the integral part
            // 0.5 is small enough not to affect the log2 result and big enough to avoid the potential precision error
            // see glTexStorage2D OpenGL docs for this formula
            const float mipCountUnfloored = std::log2(static_cast<float>(maxDimension) + 0.5f);
            return 1u + static_cast<UInt32>(mipCountUnfloored);
        }

        static UInt32 GetTotalMemoryUsedByMipmappedTexture(UInt32 bytesPerTexel, UInt32 width, UInt32 height, UInt32 depth, UInt32 mipLevelCount)
        {
            UInt32 totalSizeInBytes = 0u;
            for (UInt32 i = 0u; i < mipLevelCount; ++i)
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
