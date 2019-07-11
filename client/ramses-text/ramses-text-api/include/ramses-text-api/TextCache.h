//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTCACHE_H
#define RAMSES_TEXTCACHE_H

#include "ramses-text-api/TextLine.h"
#include "ramses-text-api/FontInstanceOffsets.h"
#include <string>

namespace ramses
{
    class Scene;
    class Effect;
    class IFontAccessor;
    class TextCacheImpl;

    /**
    * @brief Stores text data - texture atlas, meshes, glyph bitmap data. It is a cache because the
    * content can be re-generated when necessary, e.g. when cached glyphs take up too much memory.
    */
    class RAMSES_API TextCache
    {
    public:
        /**
        * @brief Constructor for text cache.
        *
        * Choose carefully the size of the atlas textures. Too small will prevent creation of
        * larger strings, because not all of the glyphs will fit on a single page. Too large
        * pages take up more memory than actually needed.
        *
        * @param[in] scene Scene to use when creating meshes from string glyphs.
        * @param[in] fontAccessor Font accessor to be used for getting font instance objects
        * @param[in] atlasTextureWidth Width for the texture atlas that gets created to store glyphs
        * @param[in] atlasTextureHeight Height for the texture atlas that gets created to store glyphs
        */
        TextCache(Scene& scene, IFontAccessor& fontAccessor, uint32_t atlasTextureWidth, uint32_t atlasTextureHeight);

        /**
        * @brief Destructor for text cache that cleans up any objects created using the text cache.
        *        The scene object passed to the text cache constructor must be still valid at the time
        *        of the text cache destruction.
        */
        ~TextCache();

        /**
        * @brief Create and get glyph metrics for a string using a font instance
        * @param[in] str The string for which to create glyph metrics
        * @param[in] font Id of the font instance to be used for creating the glyph metrics vector.
        *                 The font instance must be available at the font accessor passed in the
        *                 constructor of the text cache.
        * @return The glyph metrics vector created
        */
        GlyphMetricsVector      getPositionedGlyphs(const std::u32string& str, FontInstanceId font);

        /**
        * @brief Create and get glyph metrics for a string using a list of font instances and offsets
        * @param[in] str The string for which to create glyph metrics
        * @param[in] fontOffsets The font offsets created from font cascade to be used for creating the glyph metrics vector.
        *                 The font instances within the font cascade must all be available at the font accessor passed in the
        *                 constructor of the text cache. Also see docs of FontInstanceOffsets
        * @return The glyph metrics created
        */
        GlyphMetricsVector      getPositionedGlyphs(const std::u32string& str, const FontInstanceOffsets& fontOffsets);

        /**
        * @brief Create the scene objects, e.g., mesh and appearance...etc, needed for rendering a text line (represented by glyph metrics)
        * @param[in] glyphs The glyph metrics for which to create a text line
        * @param[in] effect The effect used for creating the appearance of the text line and rendering the meshes
        * @return Id of the text line created
        */
        TextLineId              createTextLine(const GlyphMetricsVector& glyphs, const Effect& effect);

        /**
        * @brief Get a const pointer to a (previously created) text line object
        * @param[in] textId Id of the text line object to get
        * @return A pointer to the text line object, or nullptr on failure
        */
        TextLine const*         getTextLine(TextLineId textId) const;

        /**
        * @brief Get a (non-const) pointer to a (previously created) text line object
        * @param[in] textId Id of the text line object to get
        * @return A pointer to the text line object, or nullptr on failure
        */
        TextLine*               getTextLine(TextLineId textId);

        /**
        * @brief Delete an existing text line object
        * @param[in] textId Id of the text line object to delete
        * @return True on success, false otherwise
        */
        bool                    deleteTextLine(TextLineId textId);

        /**
        * Stores internal data for implementation specifics of TextCache.
        */
        class TextCacheImpl* impl;

        /**
         * @brief Deleted copy constructor
         * @param other unused
         */
        TextCache(const TextCache& other) = delete;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        TextCache& operator=(const TextCache& other) = delete;
    };
}

#endif
