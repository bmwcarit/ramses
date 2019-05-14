//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FRAMEWORK_TEXTUREMETAINFO_H
#define RAMSES_FRAMEWORK_TEXTUREMETAINFO_H

#include <assert.h>
#include "PlatformAbstraction/PlatformTypes.h"
#include "Collections/Vector.h"
#include "SceneAPI/TextureEnums.h"
#include "Collections/Pair.h"

namespace ramses_internal
{
    using MipDataSizeVector = std::vector<UInt32>;

    struct TextureMetaInfo
    {
        TextureMetaInfo(UInt32 width = 0u, UInt32 height = 0u, UInt32 depth = 0u, ETextureFormat format = ETextureFormat_Invalid, Bool generateMipChain = false, const MipDataSizeVector& mipDataSizes = {})
            : m_width(width)
            , m_height(height)
            , m_depth(depth)
            , m_format(format)
            , m_generateMipChain(generateMipChain)
            , m_dataSizes(mipDataSizes)
        {
        }

        UInt32                  m_width;
        UInt32                  m_height;
        UInt32                  m_depth;
        ETextureFormat          m_format;
        Bool                    m_generateMipChain;
        MipDataSizeVector       m_dataSizes;
    };
}

#endif
