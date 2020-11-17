//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-text-api/LayoutUtils.h"
#include <algorithm>

namespace ramses
{
    namespace LayoutUtils
    {
        template <typename Iter>
        static StringBoundingBox getBoundingBoxForString(Iter first, Iter last)
        {
            if (first >= last)
                return { 0, 0, 0, 0, 0 };

            int32_t xmin = std::numeric_limits<int32_t>::max();
            int32_t xmax = std::numeric_limits<int32_t>::min();
            int32_t ymin = std::numeric_limits<int32_t>::max();
            int32_t ymax = std::numeric_limits<int32_t>::min();

            int32_t totalAdvance = 0;
            for (; first != last; ++first)
            {
                // glyphs with no bitmap data do not contribute to bounding box, take only their advance
                if (first->width > 0u && first->height > 0u)
                {
                    xmin = std::min<int32_t>(xmin, totalAdvance + first->posX);
                    xmax = std::max<int32_t>(xmax, totalAdvance + first->posX + first->width);
                    ymin = std::min<int32_t>(ymin, first->posY);
                    ymax = std::max<int32_t>(ymax, first->posY + first->height);
                }

                totalAdvance += first->advance;
            }

            StringBoundingBox bbox;
            bbox.offsetX = xmin;
            bbox.offsetY = ymin;
            bbox.width = xmax - xmin;
            bbox.height = ymax - ymin;
            bbox.combinedAdvance = totalAdvance;

            return bbox;
        }

        StringBoundingBox GetBoundingBoxForString(GlyphMetricsVector::const_iterator first, GlyphMetricsVector::const_iterator last)
        {
            return getBoundingBoxForString(first, last);
        }

        StringBoundingBox GetBoundingBoxForString(const GlyphMetricsVector::const_reverse_iterator& first, const GlyphMetricsVector::const_reverse_iterator& last)
        {
            return getBoundingBoxForString(first, last);
        }
    }
}
