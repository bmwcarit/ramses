//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXT_IFONTINSTANCE_H
#define RAMSES_TEXT_IFONTINSTANCE_H

#include "ramses-text-api/GlyphMetrics.h"
#include <stdint.h>
#include <string>

namespace ramses
{
    /**
    * @brief Interface for font instance that can be used to query glyph metadata and bitmaps
    */
    class RAMSES_API IFontInstance
    {
    public:
        /**
        * @brief Empty destructor
        */
        virtual ~IFontInstance() = default;

        /**
        * @brief Check if font instance supports a char using it's UTF32 char code
        * @param[in] character The UTF32 char code of the character
        * @return True if the character is supported, false otherwise
        */
        virtual bool supportsCharacter(char32_t character) const = 0;

        /**
        * @brief Get the line height of the font instance
        * @return Line height (in rasterized texels of glyphs)
        */
        virtual int getHeight() const = 0;

        /**
        * @brief Get the line ascender of the font instance (see Freetype2 docs)
        * @return Line ascender (in rasterized texels of glyphs)
        */
        virtual int getAscender() const = 0;

        /**
        * @brief Get the line descender of the font instance (see Freetype2 docs)
        * @return Line descender (in rasterized texels of glyphs)
        */
        virtual int getDescender() const = 0;

        /**
        * @brief Load the glyphs metrics for all characters in a string and appends them to the provided vector of GLYPHS
        * @param[in] charsBegin Iterator for the beginning of characters from UTF32 string that should be loaded
        * @param[in] charsEnd Iterator for the end of characters from UTF32 string that should be loaded
        * @param[out] positionedGlyphs Vector of glyphs to which newly loaded glyph metrics are appended
        */
        virtual void loadAndAppendGlyphMetrics(std::u32string::const_iterator charsBegin, std::u32string::const_iterator charsEnd, GlyphMetricsVector& positionedGlyphs) = 0;

        /**
        * @brief Load the glyph data for a specific glyph id
        * @param[in] glyphId Id of glyph for which to load data
        * @param[out] sizeX Width of glyph data
        * @param[out] sizeY Height of glyph data
        * @return The glyph data if glyphId is found, or empty glyph data otherwise
        */
        virtual GlyphData loadGlyphBitmapData(GlyphId glyphId, uint32_t& sizeX, uint32_t& sizeY) = 0;
    };
}

#endif
