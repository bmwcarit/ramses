//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURESWIZZLE_H
#define RAMSES_TEXTURESWIZZLE_H

#include "TextureEnums.h"

namespace ramses
{
    /**
    * @ingroup CoreAPI
    * @brief Information of how color channels of a texture are reordered or set to fixed value (one, zero).
    * For example swizzling a texture with a red triangle (R:1, G:0, B:0, A:1) to TextureSwizzle(Blue, Green, Red, Alpha) would turn the triangle blue.
    * The resulting color channels would be: (R:0, G:0, B:1, A:1). The red input color channel was basically rerouted to the blue output color channel.
    * The color channels can also be fully overridden with a value of 0 (ETextureChannelColor::Zero) or 1 (ETextureChannelColor::One).
    */
    struct TextureSwizzle
    {
        /// The red color channel of texture that can be swizzled to any of ETextureChannelColor
        ETextureChannelColor channelRed = ETextureChannelColor::Red;
        /// The green color channel of texture that can be swizzled to any of ETextureChannelColor
        ETextureChannelColor channelGreen = ETextureChannelColor::Green;
        /// The blue color channel of texture that can be swizzled to any of ETextureChannelColor
        ETextureChannelColor channelBlue = ETextureChannelColor::Blue;
        /// The alpha color channel of texture that can be swizzled to any of ETextureChannelColor
        ETextureChannelColor channelAlpha = ETextureChannelColor::Alpha;
    };
}

#endif
