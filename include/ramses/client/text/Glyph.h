//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/text/FontInstanceId.h"
#include "ramses/framework/StronglyTypedValue.h"
#include <vector>
#include <limits>
#include <functional>

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
    using GlyphId = StronglyTypedValue<uint32_t, std::numeric_limits<uint32_t>::max(), GlyphIdTag>;

    /**
    * @ingroup TextAPI
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
        * @brief Equal comparison operator
        * @param[in] rhs The glyph key to be compared with
        * @returns True if glyph keys are identical, false otherwise
        */
        bool operator==(const GlyphKey& rhs) const
        {
            return identifier == rhs.identifier && fontInstanceId == rhs.fontInstanceId;
        }

        /**
        * @brief Unequal comparison operator
        * @param[in] rhs The glyph key to be compared with
        * @returns False if glyph keys are identical, true otherwise
        */
        bool operator!=(const GlyphKey& rhs) const
        {
            return !(*this == rhs);
        }

        /// Glyph id
        GlyphId identifier;
        /// Font instance id
        FontInstanceId fontInstanceId;
    };

    /// Vector of GlyphKey elements
    using GlyphKeyVector = std::vector<GlyphKey>;
}

namespace std
{
    /// Hasher for GlyphKey for use in STL hash maps
    template <>
    struct hash<ramses::GlyphKey>
    {
        /**
        * @brief Hasher implementation
        * @param k Value to be hashed
        * @returns Hash usable in STL hash maps.
        */
        size_t operator()(const ramses::GlyphKey& k) const
        {
            static_assert(sizeof(ramses::GlyphKey::identifier) <= sizeof(uint32_t), "Adapt hashing function!");
            static_assert(sizeof(ramses::GlyphKey::fontInstanceId) <= sizeof(uint32_t), "Adapt hashing function!");
            const uint64_t val = (uint64_t(k.identifier.getValue()) << 32) | k.fontInstanceId.getValue();
            return hash<uint64_t>()(val);
        }
    };
}
