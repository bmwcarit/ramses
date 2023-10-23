//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/text/FontInstanceId.h"
#include <vector>

namespace ramses
{
    /**
    * @ingroup TextAPI
    * @brief A list of tuples from font instances and offsets into a string.
    *
    * Used to tell the text API which font should be used for which substring
    * when creating glyphs for a string. The list is used as this: take
    * entry N of the list, start at string offset 'N:beginOffset' and use
    * font 'N:fontInstance' until offset 'N+1:beginOffset'.
    */
    struct FontInstanceOffset
    {
        /// Font instance to use starting at this offset
        FontInstanceId fontInstance;
        /// Offset into string where to start
        std::size_t beginOffset;
    };

    /// Vector of FontInstanceOffset elements
    using FontInstanceOffsets = std::vector<FontInstanceOffset>;
}
