//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/text/FontInstanceId.h"
#include "ramses/client/text/FontInstanceOffsets.h"
#include <string>
#include <vector>

namespace ramses
{
    class IFontAccessor;

    /// vector of FontInstanceId for use in @ref FontCascade
    using OrderedFontList = std::vector<FontInstanceId>;

    /**
     * @ingroup TextAPI
     * @brief struct to define a font cascade
     *
     * Allows to define a list of fonts that are tried sequentially for character
     * availability. The first font that supports a given character is returned.
     * Also supports generic fallback characters and blacklisted characters.
     */
    struct FontCascade
    {
        /// interface to use to resolve FontInstanceId to a FontInstance
        const IFontAccessor&        fontAccessor;

        /// String of characters that will be removed from a processed string
        std::u32string              charsToRemove;

        /// Ordered list of font instance ids to try for each character
        OrderedFontList             fontPriorityList;

        /// Fallback font used for fallbackChar when none in fontPriorityList matched
        FontInstanceId              fallbackFont;

        /// Fallback character when no match was found in fontPriorityList
        char32_t                    fallbackChar;

        /**
         * @brief Process a character string according to given FontCascade
         *
         * Creates a string with characters removed or replaced according to given FontInstance. Additionally creates FontInstanceOffsets
         * specifying the font instance to use for all returned characters.
         * Intended to be used in conjunction with @ref TextCache::getPositionedGlyphs.
         *
         * @param[in] fontCascade the font cascade used to process str
         * @param[in] str the character string to process
         * @param[out] fontOffsets vector of FontOffset describing the font to use
         * @return The processed string
         */
        static RAMSES_API std::u32string FilterAndFindFontInstancesForString(const FontCascade& fontCascade, const std::u32string& str, FontInstanceOffsets& fontOffsets);
    };

}
