//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/text/Glyph.h"

namespace ramses
{
    /**
    * @ingroup TextAPI
    * @brief GlyphMetrics describes a glyph's position and dimensions in the rasterized glyph bitmap space.
    *
    * Width and height are the pixel dimensions of the glyph - must match the pixel data of
    * the glyph provided by its corresponding font instance.
    * GlyphKey uniquely identifies the glyph by referencing its origin character and font instance.
    * The posX/posY values are local pixel values in the coordinate system of the text line.
    * The advance value is the local advance (see Freetype2 docs) for each glyph.
    */
    struct GlyphMetrics
    {
        /// Glyph key (GlyphId + FontInstanceId)
        GlyphKey key;
        /// Glyph width
        uint32_t width = 0u;
        /// Glyph height
        uint32_t height = 0u;
        /// Glyph offset on x-axis
        int32_t  posX = 0;
        /// Glyph offset on y-axis
        int32_t  posY = 0;
        /// Glyph advance (distance from origin of this glyph to where origin of next glyph should start)
        int32_t  advance = 0;
    };

    /// Vector of GlyphMetrics elements
    using GlyphMetricsVector = std::vector<GlyphMetrics>;
}
