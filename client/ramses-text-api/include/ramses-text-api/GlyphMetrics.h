//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_GLYPHMETRICS_H
#define RAMSES_GLYPHMETRICS_H

#include "ramses-text-api/Glyph.h"

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
        uint32_t width;
        /// Glyph height
        uint32_t height;
        /// Glyph offset on x-axis
        int32_t  posX;
        /// Glyph offset on y-axis
        int32_t  posY;
        /// Glyph advance (distance from origin of this glyph to where origin of next glyph should start)
        int32_t  advance;
    };

    /// Vector of GlyphMetrics elements
    using GlyphMetricsVector = std::vector<GlyphMetrics>;
}

#endif
