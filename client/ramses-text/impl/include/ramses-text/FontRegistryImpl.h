//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FONTREGISTRYIMPL_H
#define RAMSES_FONTREGISTRYIMPL_H

#include "ramses-text-api/IFontInstance.h"
#include "ramses-text/Freetype2Headers.h"
#include "ramses-text/FontData.h"
#include "ramses-text/Logger.h"
#include <unordered_map>
#include <memory>

namespace ramses
{
    class SharedFTLibrary
    {
    public:
        SharedFTLibrary();
        ~SharedFTLibrary();

        FT_Library get();

    private:
        static FT_Library Ft2Library;
        static size_t NumRefs;
    };

    class FontRegistryImpl
    {
    public:
        FontRegistryImpl() = default;

        IFontInstance*          getFontInstance(FontInstanceId fontInstanceId) const;

        FontId                  createFreetype2Font(const char* fontPath);
        FontInstanceId          createFreetype2FontInstance(FontId fontId, uint32_t size, bool forceAutohinting);
        FontInstanceId          createFreetype2FontInstanceWithHarfBuzz(FontId fontId, uint32_t size, bool forceAutohinting);

        bool                    deleteFont(FontId fontId);
        bool                    deleteFontInstance(FontInstanceId fontInstance);

        FontId          registerFont(uint32_t fontType, const char* fontPath);
        const FontData* getFontData(FontId fontId) const;
        FontInstanceId  reserveFontInstanceId();
        void            registerFontInstance(FontInstanceId fontInstanceId, std::unique_ptr<IFontInstance> fontInstance);

    private:
        FontRegistryImpl(const FontRegistryImpl&) = delete;
        FontRegistryImpl& operator=(const FontRegistryImpl&) = delete;
        FontRegistryImpl(FontRegistryImpl&&) = delete;
        FontRegistryImpl& operator=(FontRegistryImpl&&) = delete;

        static std::vector<uint8_t> LoadFile(const char* fontFileName);

        SharedFTLibrary m_ft2Library;

        using Fonts = std::unordered_map<FontId, std::unique_ptr<FontData>>;
        Fonts m_fonts;
        FontId m_lastFontId{ 0u };

        using FontInstances = std::unordered_map<FontInstanceId, std::unique_ptr<IFontInstance>>;
        FontInstances m_fontInstances;
        FontInstanceId m_lastFontInstanceId{ 0u };

        static constexpr uint32_t Freetype2FontType = 0x1;
    };
}

#endif
