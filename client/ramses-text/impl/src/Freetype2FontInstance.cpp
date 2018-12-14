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

        int32_t error = FT_Open_Face(freetypeLib, &fontDataArgs, -1, &m_face);
        if (error != 0)
        {
            LOG_TEXT_ERROR("Freetype2FontInstance: Failed to open face, FT error " << error);
            assert(false);
            return;
        }
        if (m_face->num_faces < 1)
        {
            LOG_TEXT_ERROR("Freetype2FontInstance: no font faces found in font data");
            assert(false);
            return;
        }
        else if (m_face->num_faces > 1)
            LOG_TEXT_INFO("Freetype2FontInstance: current implementation does not support multiple faces, face with index 0 will be used (" << m_face->num_faces << " faces found in file)");

        error = FT_Open_Face(freetypeLib, &fontDataArgs, 0, &m_face);
        if (error != 0)
        {
            LOG_TEXT_ERROR("Freetype2FontInstance: Failed to open face, FT error " << error);
            assert(false);
            return;
        }

        error = FT_New_Size(m_face, &m_size);
        if (error != 0)
            LOG_TEXT_ERROR("Freetype2FontInstance: Failed to initialize FT size, FT error " << error);
        assert(0 == error);

        activateSize();

        error = FT_Set_Pixel_Sizes(m_face, 0, pixelSize);
        if (error != 0)
            LOG_TEXT_ERROR("Freetype2FontInstance: Failed to set pixel sizes, FT error " << error);
        assert(0 == error);

        m_height = m_size->metrics.height / 64;
        m_ascender = m_size->metrics.ascender / 64;
        m_descender = m_size->metrics.descender / 64;
    }

    Freetype2FontInstance::~Freetype2FontInstance()
    {
        if (m_size)
            FT_Done_Size(m_size);
        if (m_face)
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
        // TODO Violin try to find a more expressive code for this check
        return 0 != getGlyphId(charcode).getValue();
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
                LOG_TEXT_ERROR("Freetype2FontInstance::extractGlyphBitmapData:  FT_Get_Glyph failed - error: " << error);
                assert(ftGlyph == nullptr);
                return nullptr;
            }
            assert(ftGlyph != nullptr);

            if (ftGlyph->format != FT_GLYPH_FORMAT_BITMAP)
            {
                error = FT_Glyph_To_Bitmap(&ftGlyph, FT_RENDER_MODE_NORMAL, 0, 1);
                if (error)
                {
                    LOG_TEXT_ERROR("Freetype2FontInstance::extractGlyphBitmapData:  FT_Glyph_To_Bitmap failed - error: " << error);
                    FT_Done_Glyph(ftGlyph);
                    return nullptr;
                }
            }

            const FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(ftGlyph);
            const QuadSize glyphBitmapSize(bitmapGlyph->bitmap.width, bitmapGlyph->bitmap.rows);
            data.width = glyphBitmapSize.x;
            data.height = glyphBitmapSize.y;
            const uint32_t numberPixels = glyphBitmapSize.getArea();
            const uint8_t* bitmapBuffer = reinterpret_cast<uint8_t*>(bitmapGlyph->bitmap.buffer);
            data.data = GlyphData(bitmapBuffer, bitmapBuffer + numberPixels);
        }
        FT_Done_Glyph(ftGlyph);

        return &m_glyphBitmapCache.insert({ glyphId, std::move(data) }).first->second;
    }

    bool Freetype2FontInstance::loadGlyph(GlyphId glyphId)
    {
        if (glyphId.getValue() == 0)
        {
            LOG_TEXT_ERROR("Freetype2FontInstance: Failed to load glyph " << glyphId.getValue() << ", invalid character");
            return false;
        }

        // must call activateSize() before loading
        int32_t flags = FT_LOAD_DEFAULT;
        if (m_forceAutohinting)
            flags = FT_LOAD_FORCE_AUTOHINT;

        const uint32_t error = FT_Load_Glyph(m_face, glyphId.getValue(), flags);
        if (error != 0)
        {
            LOG_TEXT_ERROR("Freetype2FontInstance: Failed to load glyph " << glyphId.getValue() << ", FT error " << error);
            return false;
        }

        return true;
    }

    void Freetype2FontInstance::activateSize() const
    {
        const uint32_t error = FT_Activate_Size(m_size);
        if (error)
        {
            LOG_TEXT_ERROR("Freetype2FontInstance::activateSize:  FT_Activate_Size failed - error: " << error);
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
                LOG_TEXT_ERROR("Freetype2FontInstance::getKerningAdvance: Character not found for kerning; Character codes: " << glyphIdentifier1.getValue() << ", " << glyphIdentifier2.getValue());
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
}
