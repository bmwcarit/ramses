//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FONTREGISTRY_H
#define RAMSES_FONTREGISTRY_H

#include "ramses-text-api/IFontAccessor.h"
#include "ramses-text-api/IFontInstance.h"

namespace ramses
{
    /**
    * @brief Font registry can be used to load Freetype2 fonts and create font instances (optionally with Harfbuzz).
    *        These are owned and managed by FontRegistry.
    */
    class RAMSES_API FontRegistry : public IFontAccessor
    {
    public:
        /**
        * @brief Constructor for FontRegistry.
        */
        FontRegistry();

        /**
        * @brief Destructor for FontRegistry. Destroys all created fonts
        */
        virtual ~FontRegistry() override;

        /**
        * @brief Get font instance object corresponding to given id
        *
        * @param[in] fontInstanceId The id of font instance
        * @return The font instance object, nullptr if not found
        */
        virtual IFontInstance*  getFontInstance(FontInstanceId fontInstanceId) const override;

        /**
        * @brief Load Freetype2 font from file
        *
        * @param[in] fontPath The file path to the font
        * @return The font id, InvalidFontId on error
        */
        FontId                  createFreetype2Font(const char* fontPath);

        /**
        * @brief Create Freetype2 font instance
        *
        * @param[in] fontId The id of the font from which to create a font instance
        * @param[in] size Size (height in rasterized texels of glyphs) of the font
        * @param[in] forceAutohinting Force autohinting (a flag for FT2 library)
        * @return The font instance id, InvalidFontInstanceId on error
        */
        FontInstanceId          createFreetype2FontInstance(FontId fontId, uint32_t size, bool forceAutohinting = false);

        /**
        * @brief Create Freetype2 font instance with Harfbuzz shaping
        *
        * @param[in] fontId The id of the font from which to create a font instance
        * @param[in] size Size (in rasterized texels of glyphs) of the font
        * @param[in] forceAutohinting Force autohinting (a flag for FT2 library)
        * @return The font instance id, InvalidFontInstanceId on error
        */
        FontInstanceId          createFreetype2FontInstanceWithHarfBuzz(FontId fontId, uint32_t size, bool forceAutohinting = false);

        /**
        * @brief Delete an existing font
        *
        * @param[in] fontId The id of the font to be deleted
        * @return True on success, false otherwise
        */
        bool                    deleteFont(FontId fontId);

        /**
        * @brief Delete an existing font instance
        *
        * @param[in] fontInstance The id of the font instance to be deleted
        * @return True on success, false otherwise
        */
        bool                    deleteFontInstance(FontInstanceId fontInstance);

        /**
        * Stores internal data for implementation specifics of FontRegistry.
        */
        class FontRegistryImpl& impl;

        /**
         * @brief Deleted copy constructor
         * @param other unused
         */
        FontRegistry(const FontRegistry& other) = delete;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        FontRegistry& operator=(const FontRegistry& other) = delete;
    };
}

#endif
