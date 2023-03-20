//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTLINE_H
#define RAMSES_TEXTLINE_H

#include "ramses-text-api/GlyphMetrics.h"
#include <limits>

namespace ramses
{
    /**
    * @brief An empty struct to make TextLineId a strong type
    */
    struct TextLineIdTag {};

    /**
    * @brief A strongly typed integer to distinguish between different text lines
    */
    using TextLineId = StronglyTypedValue<uint32_t, std::numeric_limits<uint32_t>::max(), TextLineIdTag>;

    class MeshNode;
    class ArrayBuffer;

    /**
    * @brief Groups the scene objects needed to render a text line
    */
    struct TextLine
    {
        /// Mesh node that represents the text
        MeshNode*           meshNode = nullptr;
        /// Index to the atlas page containing the glyphs
        std::size_t         atlasPage = std::numeric_limits<std::size_t>::max();
        /// Glyph metrics of the original string characters
        GlyphMetricsVector  glyphs;
        /// Stores vertex data for the text line quads
        ArrayBuffer*        positions = nullptr;
        /// Stores texture coordinate data for the text line quads
        ArrayBuffer*        textureCoordinates = nullptr;
        /// Stores index data for the text line quads
        ArrayBuffer*        indices = nullptr;
    };

    static_assert(std::is_move_constructible<TextLine>::value, "TextLine must be movable");
    static_assert(std::is_move_assignable<TextLine>::value, "TextLine must be movable");
}

#endif
