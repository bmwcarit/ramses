//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-text/FontRegistryImpl.h"
#include "ramses-text/Freetype2FontInstance.h"
#include "ramses-text/HarfbuzzFontInstance.h"
#include <fstream>
#include <assert.h>

namespace ramses
{
    FT_Library SharedFTLibrary::Ft2Library = nullptr;
    size_t SharedFTLibrary::NumRefs = 0u;

    SharedFTLibrary::SharedFTLibrary()
    {
        if (Ft2Library == nullptr)
        {
            const int32_t error = FT_Init_FreeType(&Ft2Library);
            if (error)
            {
                Ft2Library = nullptr;
                LOG_TEXT_ERROR("SharedFTLibrary: Failed to initialize FreeType with error " << error);
            }
        }
        NumRefs++;
    }

    SharedFTLibrary::~SharedFTLibrary()
    {
        assert(NumRefs > 0u);
        NumRefs--;
        if (NumRefs == 0u && Ft2Library != nullptr)
        {
            FT_Done_FreeType(Ft2Library);
            Ft2Library = nullptr;
        }
    }

    FT_Library SharedFTLibrary::get()
    {
        return Ft2Library;
    }

    IFontInstance* FontRegistryImpl::getFontInstance(FontInstanceId fontInstanceId) const
    {
        auto it = m_fontInstances.find(fontInstanceId);
        return it != m_fontInstances.cend() ? it->second.get() : nullptr;
    }

    FontId FontRegistryImpl::createFreetype2Font(const char* fontPath)
    {
        return registerFont(Freetype2FontType, fontPath);
    }

    bool FontRegistryImpl::deleteFont(FontId fontId)
    {
        if (m_fonts.erase(fontId) == 0u)
        {
            LOG_TEXT_ERROR("FontRegistryImpl::deleteFont: Cannot delete font " << fontId.getValue() << ", no such entry");
            return false;
        }
        return true;
    }

    FontInstanceId FontRegistryImpl::createFreetype2FontInstance(FontId fontId, uint32_t size, bool forceAutohinting)
    {
        const FontData* fontData = getFontData(fontId);
        if (fontData == nullptr || fontData->type != Freetype2FontType)
        {
            LOG_TEXT_ERROR("FontRegistry: Failed to create font instance, fontId " << fontId.getValue() << " does not exist or is of invalid type");
            return InvalidFontInstanceId;
        }

        const FontInstanceId fontInstanceId = reserveFontInstanceId();
        registerFontInstance(fontInstanceId, std::unique_ptr<IFontInstance>{ new Freetype2FontInstance(fontInstanceId, m_ft2Library.get(), *fontData, size, forceAutohinting) });

        return fontInstanceId;
    }

    FontInstanceId FontRegistryImpl::createFreetype2FontInstanceWithHarfBuzz(FontId fontId, uint32_t size, bool forceAutohinting)
    {
        const FontData* fontData = getFontData(fontId);
        if (fontData == nullptr || fontData->type != Freetype2FontType)
        {
            LOG_TEXT_ERROR("FontRegistry: Failed to create font instance, fontId " << fontId.getValue() << " does not exist or is of invalid type");
            return InvalidFontInstanceId;
        }

        const FontInstanceId fontInstanceId = reserveFontInstanceId();
        registerFontInstance(fontInstanceId, std::unique_ptr<IFontInstance>{ new HarfbuzzFontInstance(fontInstanceId, m_ft2Library.get(), *fontData, size, forceAutohinting) });

        return fontInstanceId;
    }

    bool FontRegistryImpl::deleteFontInstance(FontInstanceId fontInstance)
    {
        if (m_fontInstances.erase(fontInstance) == 0u)
        {
            LOG_TEXT_ERROR("FontRegistryImpl::deleteFontInstance: Cannot delete font instance " << fontInstance.getValue() << ", no such entry");
            return false;
        }
        return true;
    }

    ramses::FontId FontRegistryImpl::registerFont(uint32_t fontType, const char* fontPath)
    {
        auto fontBinaryData = LoadFile(fontPath);
        if (fontBinaryData.empty())
        {
            LOG_TEXT_ERROR("FontRegistry: Failed to load font from file " << fontPath);
            return InvalidFontId;
        }

        const FontId fontId = m_lastFontId;
        m_lastFontId.getReference()++;

        assert(m_fonts.count(fontId) == 0u);
        m_fonts.insert(std::make_pair(fontId, std::unique_ptr<FontData>(new FontData{ fontType, std::move(fontBinaryData) })));

        return fontId;
    }

    const FontData* FontRegistryImpl::getFontData(FontId fontId) const
    {
        const auto fontIt = m_fonts.find(fontId);
        return (fontIt == m_fonts.cend() ? nullptr : fontIt->second.get());
    }

    FontInstanceId FontRegistryImpl::reserveFontInstanceId()
    {
        const FontInstanceId fontInstanceId = m_lastFontInstanceId;
        m_lastFontInstanceId.getReference()++;

        assert(m_fontInstances.count(fontInstanceId) == 0u);
        return fontInstanceId;
    }

    void FontRegistryImpl::registerFontInstance(FontInstanceId fontInstanceId, std::unique_ptr<IFontInstance> fontInstance)
    {
        assert(m_fontInstances.count(fontInstanceId) == 0u);
        m_fontInstances.insert(std::make_pair(fontInstanceId, std::move(fontInstance)));
    }

    std::vector<uint8_t> FontRegistryImpl::LoadFile(const char* fontFileName)
    {
        std::ifstream input(fontFileName, std::ios::binary | std::ios::ate);
        const auto size = input.tellg();
        if (input.fail() || size < 0)
            return {};
        std::vector<uint8_t> vec(static_cast<std::size_t>(size));
        input.seekg(0);
        input.read(reinterpret_cast<char*>(vec.data()), vec.size());
        return vec;
    }
}
