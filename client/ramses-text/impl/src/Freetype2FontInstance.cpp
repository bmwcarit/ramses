//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-text/Freetype2FontInstance.h"
#include "ramses-text/Logger.h"
#include "ramses-text/Quad.h"
#include <assert.h>
#include <iostream>
#include <cstring>

namespace ramses
{
    Freetype2FontInstance::Freetype2FontInstance(FontInstanceId id, FT_Library freetypeLib, const FontData& font, uint32_t pixelSize, bool forceAutohinting)
        : m_id(id)
        , m_font(font)
        , m_forceAutohinting(forceAutohinting)
    {
        // TODO Violin check if face has to be created per instance, or is enough to have it per font

        FT_Open_Args fontDataArgs;
        fontDataArgs.flags = FT_OPEN_MEMORY;
        fontDataArgs.memory_base = m_font.data.data();
        fontDataArgs.memory_size = static_cast<FT_Long>(m_font.data.size());
        fontDataArgs.num_params = 0;

        // TODO Violin check what is font face 0 and if it is required to check for other faces
        int32_t error = FT_Open_Face(freetypeLib, &fontDataArgs, 0, &m_face);
        if (error != 0)
            LOG_TEXT_ERROR("Freetype2FontInstance: Failed to open face, FT error " << error);
        assert(0 == error);

        error = FT_New_Size(m_face, &m_size);
        if (error != 0)
            LOG_TEXT_ERROR("Freetype2FontInstance: Failed to initialize FT size, FT error " << error);
        assert(0 == error);

        activateSize();

        error = FT_Set_Pixel_Sizes(m_face, 0, pixelSize);
        if (error != 0)
            LOG_TEXT_ERROR("Freetype2FontInstance: Failed to set pixel sizes, FT error " << error);
        assert(0 == error);

        // TODO Violin store those parameters as integer, not float
        m_height = static_cast<float>(m_size->metrics.height) / 64.0f;
        m_ascender = static_cast<float>(m_size->metrics.ascender) / 64.0f;
        m_descender = static_cast<float>(m_size->metrics.descender) / 64.0f;
    }

    Freetype2FontInstance::~Freetype2FontInstance()
    {
        FT_Done_Size(m_size);
        FT_Done_Face(m_face);
    }

    void Freetype2FontInstance::loadAndAppendGlyphMetrics(std::u32string::const_iterator charsBegin, std::u32string::const_iterator charsEnd, GlyphMetricsVector& positionedGlyphs)
    {
        activateSize();

        positionedGlyphs.reserve(positionedGlyphs.size() + std::distance(charsBegin, charsEnd));

        GlyphId lastGlyphId(0);
        for (auto it = charsBegin; it != charsEnd; ++it)
        {
            const GlyphId glyphId = getGlyphId(*it);
            GlyphMetrics glyphMetrics = loadGlyphMetrics(glyphId);

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

    GlyphMetrics Freetype2FontInstance::loadGlyphMetrics(GlyphId glyphId)
    {
        assert(glyphId.getValue() != 0);

        // cache glyph metrics if not cached yet
        if (m_glyphMetricsCache.count(glyphId) == 0)
        {
            int32_t flags = FT_LOAD_DEFAULT;
            if (m_forceAutohinting)
                flags = FT_LOAD_FORCE_AUTOHINT;

            const uint32_t error = FT_Load_Glyph(m_face, glyphId.getValue(), flags);
            if (error != 0)
            {
                LOG_TEXT_ERROR("Freetype2FontInstance: Failed to load glyph " << glyphId.getValue() << ", FT error " << error);
                return { GlyphKey(glyphId, m_id), 0, 0, 0, 0, 0 };
            }

            m_glyphMetricsCache.emplace(glyphId, m_face->glyph->metrics);
        }

        const FT_Glyph_Metrics& glyphMetrics = m_glyphMetricsCache.at(glyphId);
        const int32_t bearingX = glyphMetrics.horiBearingX / 64;
        const int32_t bearingY = (glyphMetrics.horiBearingY - glyphMetrics.height) / 64;
        const uint32_t width = static_cast<uint32_t>(glyphMetrics.width / 64);
        const uint32_t height = static_cast<uint32_t>(glyphMetrics.height / 64);
        const int32_t localAdvance = glyphMetrics.horiAdvance / 64;

        return { GlyphKey(glyphId, m_id), width, height, bearingX, bearingY, localAdvance };
    }

    bool Freetype2FontInstance::supportsCharacter(char32_t charcode) const
    {
        // TODO Violin try to find a more expressive code for this check
        return 0 != getGlyphId(charcode).getValue();
    }

    GlyphData Freetype2FontInstance::loadGlyphBitmapData(GlyphId glyphIdentifier, uint32_t& sizeX, uint32_t& sizeY)
    {
        sizeX = 0u;
        sizeY = 0u;
        if (glyphIdentifier.getValue() == 0)
            return {};

        int32_t flags = FT_LOAD_DEFAULT;
        if (m_forceAutohinting)
            flags = FT_LOAD_FORCE_AUTOHINT;

        activateSize();
        uint32_t error = FT_Load_Glyph(m_face, glyphIdentifier.getValue(), flags);
        if (error)
        {
            LOG_TEXT_ERROR("Freetype2FontInstance::loadGlyphBitmapData:  FT_Load_Glyph failed - error: " << error);
            return {};
        }

        FT_Glyph ftGlyph = nullptr;
        error = FT_Get_Glyph(m_face->glyph, &ftGlyph);
        if (error || ftGlyph == nullptr)
        {
            LOG_TEXT_ERROR("Freetype2FontInstance::loadGlyphBitmapData:  FT_Get_Glyph failed - error: " << error);
            if (ftGlyph != nullptr)
                FT_Done_Glyph(ftGlyph);
            return {};
        }

        if (ftGlyph->format != FT_GLYPH_FORMAT_BITMAP)
        {
            error = FT_Glyph_To_Bitmap(&ftGlyph, FT_RENDER_MODE_NORMAL, 0, 1);
            if (error)
            {
                LOG_TEXT_ERROR("Freetype2FontInstance::loadGlyphBitmapData:  FT_Glyph_To_Bitmap failed - error: " << error);
                FT_Done_Glyph(ftGlyph);
                return {};
            }
        }

        const FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(ftGlyph);
        const QuadSize glyphBitmapSize(bitmapGlyph->bitmap.width, bitmapGlyph->bitmap.rows);
        sizeX = glyphBitmapSize.x;
        sizeY = glyphBitmapSize.y;
        const uint32_t numberPixels = glyphBitmapSize.getArea();
        GlyphData glyphBitmapData(numberPixels);
        // TODO remove this memcpy if possible
        // Check the last parameter of FT_Glyph_To_Bitmap - looks like it might be possible to take over the data by giving 0 instead of 1
        std::memcpy(glyphBitmapData.data(), bitmapGlyph->bitmap.buffer, numberPixels * sizeof(uint8_t));

        FT_Done_Glyph(ftGlyph);

        return glyphBitmapData;
    }

    void Freetype2FontInstance::activateSize() const
    {
        const uint32_t error = FT_Activate_Size(m_size);
        if (error)
            LOG_TEXT_ERROR("Freetype2FontInstance::activateSize:  FT_Activate_Size failed - error: " << error);
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
                LOG_TEXT_ERROR("Freetype2FontInstance::getKerningAdvance: Character not found for kerning; Character codes: " << glyphIdentifier1.getValue() << ", " << glyphIdentifier2.getValue());
        }

        return 0;
    }

    float Freetype2FontInstance::getHeight() const
    {
        return m_height;
    }

    float Freetype2FontInstance::getAscender() const
    {
        return m_ascender;
    }

    float Freetype2FontInstance::getDescender() const
    {
        return m_descender;
    }

    GlyphId Freetype2FontInstance::getGlyphId(char32_t charcode) const
    {
        activateSize();
        return GlyphId(FT_Get_Char_Index(m_face, charcode));
    }
}
