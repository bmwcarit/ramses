//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/text/Quad.h"
#include <unordered_map>

namespace ramses
{
    struct GlyphMapping final
    {
        // Violin: there is no working around this ref count, because glyphs
        // can be used multiple times in different strings, and someone needs
        // to remember how many times are they used, because fetching them from
        // a font is expensive
        uint32_t refCount;
        Quad quad;
    };

    using GlyphMappings = std::unordered_map<size_t, GlyphMapping>;
}
