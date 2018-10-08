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
        * @brief Utility convenience methods to compute layout data out of positioned glyphs.
        */
        struct StringBoundingBox
        {
            /// offset on x-axis
            int32_t  offsetX;
            /// offset on y-axis
            int32_t  offsetY;
            /// box width
            uint32_t width;
            /// box height
            uint32_t height;
            /// box advance
            int32_t  combinedAdvance;
        };

        /**
        * @brief Get the bounding box for a string represented by glyph metrics
        * @param[in] first (Forward-) Iterator that represents the beginning of string to calculate the bounding box
        * @param[in] last (Forward-) Iterator that represents the end of string to calculate the bounding box
        * @return The bounding box of the provided string, or bounding box of Zero values on failure
        */
        RAMSES_API StringBoundingBox GetBoundingBoxForString(GlyphMetricsVector::const_iterator first, GlyphMetricsVector::const_iterator last);

        /**
        * @brief Get the bounding box for a string represented by glyph metrics. The glyph metrics are scanned in reverse order.
        * @param[in] first Reverse-Iterator that represents the end of string to calculate the bounding box
        * @param[in] last Reverse-Iterator that represents the beginning of string to calculate the bounding box
        * @return The bounding box of the provided string, or bounding box of Zero values on failure
        */
        RAMSES_API StringBoundingBox GetBoundingBoxForString(GlyphMetricsVector::const_reverse_iterator first, GlyphMetricsVector::const_reverse_iterator last);

        /**
        * @brief Find the substring represented by glyphs in a glyph metrics vector that fit into a line with specific pixel width
        * @param[in] first (Forward-) Iterator that represents the beginning of string to calculate the fitting substring of glyph metrics
        * @param[in] last (Forward-) Iterator that represents the end of string to calculate the fitting substring of glyph metrics
        * @return An iterator to the first glyph metrics element that does not fit into the line width. If the whole glyph metrics
        *         vector fits into the given line width then last is returned.
        */
        RAMSES_API GlyphMetricsVector::const_iterator         FindFittingSubstring(GlyphMetricsVector::const_iterator first, GlyphMetricsVector::const_iterator last, uint32_t maxWidth);

        /**
        * @brief Find the substring represented by glyphs in a glyph metrics vector that fit into a line with specific width. The glyph metrics are scanned in reverse order.
        * @param[in] first Reverse-Iterator that represents the end of string to calculate the fitting substring of glyph metrics
        * @param[in] last Reverse-Iterator that represents the beginning of string to calculate the fitting substring of glyph metrics
        * @return An iterator to the first glyph metrics element that does not fit into the line width. If the whole glyph metrics
        *         vector fits into the given line width then last is returned.
        */
        RAMSES_API GlyphMetricsVector::const_reverse_iterator FindFittingSubstring(GlyphMetricsVector::const_reverse_iterator first, GlyphMetricsVector::const_reverse_iterator last, uint32_t maxWidth);
    }
}

#endif
