//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-text/Freetype2FontInstance.h"
#include "ramses-text/Quad.h"
#include "Utils/LogMacros.h"
#include "RamsesFrameworkTypesImpl.h"
#include "ramses-text/TextTypesImpl.h"
#include <cassert>

namespace ramses
{
    Freetype2FontInstance::Freetype2FontInstance(FontInstanceId id, FT_Face fontFace, uint32_t pixelSize, bool forceAutohinting)
        : m_id(id)
        , m_face(fontFace)
        , m_forceAutohinting(forceAutohinting)
    {
        int error = FT_New_Size(m_face, &m_size);
        if (error != 0)
            LOG_ERROR(CONTEXT_TEXT, "Freetype2FontInstance: Failed to initialize FT size, FT error " << error);
        assert(0 == error);

        activateSize();

        error = FT_Set_Pixel_Sizes(m_face, 0, pixelSize);
        if (error != 0)
            LOG_ERROR(CONTEXT_TEXT, "Freetype2FontInstance: Failed to set pixel sizes, FT error " << error);
        assert(0 == error);

        m_height = static_cast<int>(m_size->metrics.height / 64);
        m_ascender = static_cast<int>(m_size->metrics.ascender / 64);
        m_descender = static_cast<int>(m_size->metrics.descender / 64);
    }

    Freetype2FontInstance::~Freetype2FontInstance()
    {
        if (m_size)
            FT_Done_Size(m_size);
    }

    void Freetype2FontInstance::loadAndAppendGlyphMetrics(std::u32string::const_iterator charsBegin, std::u32string::const_iterator charsEnd, GlyphMetricsVector& positionedGlyphs)
    {
        activateSize();

        positionedGlyphs.reserve(positionedGlyphs.size() + std::distance(charsBegin, charsEnd));

        GlyphId lastGlyphId(0);
        for (auto it = charsBegin; it != charsEnd; ++it)
        {
            const GlyphId glyphId = getGlyphId(*it);
            const GlyphMetrics* glyphMetricsEntry = getGlyphMetricsData(glyphId);
            if (glyphMetricsEntry == nullptr)
                continue;

            GlyphMetrics glyphMetrics = *glyphMetricsEntry;

            // kerning starts from second character
            if (it != charsBegin)
            {
                const int32_t kerning = getKerningAdvance(lastGlyphId, glyphId);
                glyphMetrics.posX += kerning;
                glyphMetrics.advance += kerning;
            }

            positionedGlyphs.push_back(glyphMetrics);
            lastGlyphId = glyphId;
        }
    }

    GlyphData Freetype2FontInstance::loadGlyphBitmapData(GlyphId glyphId, uint32_t& sizeX, uint32_t& sizeY)
    {
        activateSize();

        const GlyphBitmapData* data = getGlyphBitmapData(glyphId);
        if (data == nullptr)
        {
            sizeX = 0u;
            sizeY = 0u;
            return{};
        }

        sizeX = data->width;
        sizeY = data->height;

        return data->data;
    }

    bool Freetype2FontInstance::supportsCharacter(char32_t charcode) const
    {
        const auto it = m_supportedCharacters.find(charcode);

        if (m_supportedCharacters.end() != it)
            return it->second;

        //if the font doesn't contain a glyph for the queried charcode it's not supported
        const bool isCharSupported = (0 != getGlyphId(charcode).getValue());
        m_supportedCharacters[charcode] = isCharSupported;
        return isCharSupported;
    }

    const GlyphMetrics* Freetype2FontInstance::getGlyphMetricsData(GlyphId glyphId)
    {
        const auto it = m_glyphMetricsCache.find(glyphId);
        if (it != m_glyphMetricsCache.cend())
            return &it->second;

        if (!loadGlyph(glyphId))
            return nullptr;

        const FT_Glyph_Metrics& glyphMetrics = m_face->glyph->metrics;

        GlyphMetrics metrics;
        metrics.key = GlyphKey(glyphId, m_id);
        metrics.width = static_cast<uint32_t>(glyphMetrics.width / 64);
        metrics.height = static_cast<uint32_t>(glyphMetrics.height / 64);
        metrics.posX = glyphMetrics.horiBearingX / 64;
        metrics.posY = (glyphMetrics.horiBearingY - glyphMetrics.height) / 64;
        metrics.advance = glyphMetrics.horiAdvance / 64;

        return &m_glyphMetricsCache.insert({ glyphId, std::move(metrics) }).first->second;
    }

    const Freetype2FontInstance::GlyphBitmapData* Freetype2FontInstance::getGlyphBitmapData(GlyphId glyphId)
    {
        const auto it = m_glyphBitmapCache.find(glyphId);
        if (it != m_glyphBitmapCache.cend())
            return &it->second;

        if (!loadGlyph(glyphId))
            return nullptr;

        GlyphBitmapData data;
        FT_Glyph ftGlyph = nullptr;
        auto error = FT_Get_Glyph(m_face->glyph, &ftGlyph);
        {
            if (error)
            {
                LOG_ERROR(CONTEXT_TEXT, "Freetype2FontInstance::extractGlyphBitmapData:  FT_Get_Glyph failed - error: " << error);
                assert(ftGlyph == nullptr);
                return nullptr;
            }
            assert(ftGlyph != nullptr);

            if (ftGlyph->format != FT_GLYPH_FORMAT_BITMAP)
            {
                error = FT_Glyph_To_Bitmap(&ftGlyph, FT_RENDER_MODE_NORMAL, nullptr, 1);
                if (error)
                {
                    LOG_ERROR(CONTEXT_TEXT, "Freetype2FontInstance::extractGlyphBitmapData:  FT_Glyph_To_Bitmap failed - error: " << error);
                    FT_Done_Glyph(ftGlyph);
                    return nullptr;
                }
            }

            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) safe because C-style inheritance, valid if ftGlyph->format == FT_GLYPH_FORMAT_BITMAP
            const FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(ftGlyph);
            const QuadSize glyphBitmapSize(bitmapGlyph->bitmap.width, bitmapGlyph->bitmap.rows);
            data.width = glyphBitmapSize.x;
            data.height = glyphBitmapSize.y;
            const uint32_t numberPixels = glyphBitmapSize.getArea();
            const uint8_t* bitmapBuffer = bitmapGlyph->bitmap.buffer;
            data.data = GlyphData(bitmapBuffer, bitmapBuffer + numberPixels);
        }
        FT_Done_Glyph(ftGlyph);

        return &m_glyphBitmapCache.insert({ glyphId, std::move(data) }).first->second;
    }

    bool Freetype2FontInstance::loadGlyph(GlyphId glyphId)
    {
        if (glyphId.getValue() == 0)
        {
            LOG_ERROR(CONTEXT_TEXT, "Freetype2FontInstance: Failed to load glyph " << glyphId << ", invalid character");
            return false;
        }

        // must call activateSize() before loading
        activateSize();

        int32_t flags = FT_LOAD_DEFAULT;
        if (m_forceAutohinting)
            flags = FT_LOAD_FORCE_AUTOHINT;

        const uint32_t error = FT_Load_Glyph(m_face, glyphId.getValue(), flags);
        if (error != 0)
        {
            LOG_ERROR(CONTEXT_TEXT, "Freetype2FontInstance: Failed to load glyph " << glyphId << ", FT error " << error);
            return false;
        }

        return true;
    }

    void Freetype2FontInstance::activateSize() const
    {
        const uint32_t error = FT_Activate_Size(m_size);
        if (error)
        {
            LOG_ERROR(CONTEXT_TEXT, "Freetype2FontInstance::activateSize:  FT_Activate_Size failed - error: " << error);
            assert(false);
        }
    }

    int32_t Freetype2FontInstance::getKerningAdvance(GlyphId glyphIdentifier1, GlyphId glyphIdentifier2) const
    {
        if (FT_HAS_KERNING(m_face))
        {
            if (glyphIdentifier1.getValue() != 0 && glyphIdentifier2.getValue() != 0)
            {
                FT_Vector delta;
                FT_Get_Kerning(m_face, glyphIdentifier1.getValue(), glyphIdentifier2.getValue(), FT_KERNING_DEFAULT, &delta);
                return delta.x / 64;
            }
            else
                LOG_ERROR(CONTEXT_TEXT, "Freetype2FontInstance::getKerningAdvance: Character not found for kerning; Character codes: " << glyphIdentifier1 << ", " << glyphIdentifier2);
        }

        return 0;
    }

    int Freetype2FontInstance::getHeight() const
    {
        return m_height;
    }

    int Freetype2FontInstance::getAscender() const
    {
        return m_ascender;
    }

    int Freetype2FontInstance::getDescender() const
    {
        return m_descender;
    }

    GlyphId Freetype2FontInstance::getGlyphId(char32_t charcode) const
    {
        activateSize();
        return GlyphId(FT_Get_Char_Index(m_face, charcode));
    }

    void Freetype2FontInstance::cacheAllSupportedCharacters()
    {
        assert(!m_allSupportedCharactersCached);
        FT_UInt gindex = 0;
        FT_ULong charcode = FT_Get_First_Char(m_face, &gindex);

        while (gindex != 0)
        {
            m_supportedCharacters[charcode] = true;
            charcode = FT_Get_Next_Char(m_face, charcode, &gindex);
        }
    }

    std::unordered_set<FT_ULong> Freetype2FontInstance::getAllSupportedCharacters()
    {
        if (!m_allSupportedCharactersCached)
        {
            cacheAllSupportedCharacters();
            m_allSupportedCharactersCached = true;
        }
        std::unordered_set<FT_ULong> allSupportedCharactersSet;

        for (const auto character : m_supportedCharacters)
        {
            if (character.second)
                allSupportedCharactersSet.insert(character.first);
        }
        return allSupportedCharactersSet;
    }
}
