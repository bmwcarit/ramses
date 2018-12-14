//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_GLYPH_H
#define RAMSES_GLYPH_H

#include "ramses-text-api/FontInstanceId.h"
#include "ramses-framework-api/StronglyTypedValue.h"
#include <vector>
#include <limits>

namespace ramses
{
    /**
    * @brief Stores 2-dimensional glyph pixel data as a flat memory array
    */
    using GlyphData = std::vector<uint8_t>;

    /**
    * @brief An empty struct to make GlyphId a strong type
    */
    struct GlyphIdTag {};

    /**
    * @brief A strongly typed integer to distinguish between different glyphs
    */
    using GlyphId = StronglyTypedValue<uint32_t, GlyphIdTag>;

    /**
    * @brief A constant value representing an invalid GlyphId
    */
    static const GlyphId InvalidGlyphId(std::numeric_limits<GlyphId::BaseType>::max());

    /**
    * @brief GlyphKey identifies a glyph with a specific font instance.
    */
    struct GlyphKey
    {
        /**
        * @brief Constructor for GlyphKey that initializes members to default values.
        */
        GlyphKey() = default;

        /**
        * @brief Constructor for GlyphKey that initializes members to passed values.
        * @param[in] _identifier The glyph id
        * @param[in] _fontInstanceId The font instance id
        */
        GlyphKey(GlyphId _identifier, FontInstanceId _fontInstanceId)
            : identifier(_identifier)
            , fontInstanceId(_fontInstanceId)
        {
        }

        /**
        * @brief Comparison operator
        * @param[in] rhs The glyph key to be compared with
        * @returns True if glyph keys are identical, false otherwise
        */
        bool operator==(const GlyphKey& rhs) const
        {
            return identifier == rhs.identifier && fontInstanceId == rhs.fontInstanceId;
        }

        /// Glyph id
        GlyphId identifier = InvalidGlyphId;
        /// Font instance id
        FontInstanceId fontInstanceId = InvalidFontInstanceId;
    };

    /// Vector of GlyphKey elements
    using GlyphKeyVector = std::vector<GlyphKey>;
}

#endif
