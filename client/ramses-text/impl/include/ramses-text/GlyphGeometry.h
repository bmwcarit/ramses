//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_GLYPHGEOMETRY_H
#define RAMSES_GLYPHGEOMETRY_H

#include "stdint.h"
#include <vector>

namespace ramses
{
    struct GlyphGeometry
    {
        std::vector<uint16_t> indices;
        std::vector<float>    positions;
        std::vector<float>    texcoords;
        size_t atlasPage = std::numeric_limits<size_t>::max();
    };

    static_assert(std::is_nothrow_move_constructible<GlyphGeometry>::value &&
        std::is_nothrow_move_assignable<GlyphGeometry>::value, "GlyphGeometry must be movable");
}

#endif
