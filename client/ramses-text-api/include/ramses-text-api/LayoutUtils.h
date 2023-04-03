//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LAYOUTUTILS_H
#define RAMSES_LAYOUTUTILS_H

#include "ramses-text-api/GlyphMetrics.h"

namespace ramses
{
    namespace LayoutUtils
    {
        /**
        * @brief Helper data structure describing multiple glyphs metrics.
        *
        * StringBoundingBox is essentially a union of a set of GlyphMetrics.
        * It describes a bounding box of renderable glyphs (glyphs with non-empty bitmap data).
        * Width and height are dimensions of the bounding box.
        * The bounding box can be offset from origin using offsetX/Y.
        * Combined advance is X axis offset from origin where next StringBoundingBox would be if put on same line.
        * Similarly to GlyphMetrics, even when StringBoundingBox have zero width/height (which means there is no renderable data)
        * it can still have non-zero advance which should be taken into account by other StringBoundingBox if combined.
        */
        struct StringBoundingBox
        {
            /// X axis offset of bounding box from origin
            int32_t offsetX;
            /// Y axis offset of bounding box from origin
            int32_t offsetY;
            /// Bounding box width
            uint32_t width;
            /// Bounding box height
            uint32_t height;
            /// Advance to next StringBoundingBox
            int32_t combinedAdvance;
        };

        /**
        * @brief Compute a bounding box for a string represented by a range of GlyphMetrics.
        * @param[in] first Beginning of range of GlyphMetrics to compute bounding box
        * @param[in] last End of range of GlyphMetrics to compute bounding box, glyph pointed to by last is not included
        * @return The bounding box of the provided string, or bounding box of Zero values on failure
        */
        RAMSES_API StringBoundingBox GetBoundingBoxForString(GlyphMetricsVector::const_iterator first, GlyphMetricsVector::const_iterator last);

        /**
        * @copydoc GetBoundingBoxForString(GlyphMetricsVector::const_iterator first, GlyphMetricsVector::const_iterator last)
        **/
        RAMSES_API StringBoundingBox GetBoundingBoxForString(const GlyphMetricsVector::const_reverse_iterator& first, const GlyphMetricsVector::const_reverse_iterator& last);
    }
}

#endif
