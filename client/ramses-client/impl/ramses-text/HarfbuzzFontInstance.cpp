//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-text/HarfbuzzFontInstance.h"
#include "Utils/Warnings.h"
#include "Utils/LogMacros.h"
#include <vector>
#include <cassert>

PUSH_DISABLE_C_STYLE_CAST_WARNING
#include <hb-ft.h>
POP_DISABLE_C_STYLE_CAST_WARNING

namespace ramses
{
    static int32_t RoundHBFixedToInt(int32_t fixed)
    {
        if (fixed >= 0)
            return (fixed + 32) / 64;
        else
            return (fixed - 32) / 64;
    }

#if !defined(USE_HARFBUZZ_LEGACY_SHAPING)
    static hb_script_t guessScript(hb_unicode_funcs_t* unicodeFuncs, std::u32string::const_iterator charIt, std::u32string::const_iterator charsEnd)
    {
        hb_script_t result = HB_SCRIPT_COMMON;

        while (charIt != charsEnd)
        {
            const hb_script_t hbScript = hb_unicode_script(unicodeFuncs, *charIt);
            if ((HB_SCRIPT_COMMON != hbScript) && (HB_SCRIPT_INHERITED != hbScript) && (HB_SCRIPT_UNKNOWN != hbScript))
            {
                result = hbScript;
                break;
            }
            ++charIt;
        }

        return result;
    }
#endif

    HarfbuzzFontInstance::HarfbuzzFontInstance(FontInstanceId id, FT_Face fontFace, uint32_t pixelSize, bool forceAutohinting)
        : Freetype2FontInstance(id, fontFace, pixelSize, forceAutohinting)
    {
        m_hbFont = hb_ft_font_create(m_face, nullptr);
        if (m_hbFont == nullptr)
            LOG_ERROR(CONTEXT_TEXT, "HarfbuzzFontInstance::HarfbuzzFontInstance Could not create harfbuzz font");
        assert(m_hbFont);

        m_hbBuffer = hb_buffer_create();
        if (m_hbBuffer == nullptr)
            LOG_ERROR(CONTEXT_TEXT, "HarfbuzzFontInstance::HarfbuzzFontInstance Could not create harfbuzz buffer");
        assert(m_hbBuffer);
    }

    HarfbuzzFontInstance::~HarfbuzzFontInstance()
    {
        hb_buffer_destroy(m_hbBuffer);
        if (m_hbFont != nullptr)
            hb_font_destroy(m_hbFont);
    }

    void HarfbuzzFontInstance::loadAndAppendGlyphMetrics(std::u32string::const_iterator charsBegin, std::u32string::const_iterator charsEnd, GlyphMetricsVector& positionedGlyphs)
    {
        // Reshape given chars using HB resulting in list of (FT2) glyph indexes and their local offsets
        struct HBGlyphInfo
        {
            uint32_t glyphIndex;
            int32_t offsetX;
            int32_t offsetY;
            int32_t advance;
        };
        std::vector<HBGlyphInfo> hbGlyphInfos;
        hbGlyphInfos.reserve(std::distance(charsBegin, charsEnd));

        activateHBFontSize();

        unsigned clusterIdx = 0u;
        auto charIt = charsBegin;
        while (charIt != charsEnd)
        {
            hb_buffer_clear_contents(m_hbBuffer);
            hb_unicode_funcs_t* unicodeFuncs = hb_buffer_get_unicode_funcs(m_hbBuffer);

            hb_buffer_set_content_type(m_hbBuffer, HB_BUFFER_CONTENT_TYPE_UNICODE);
            // Always take LTR direction. With that Arabic reshaping works, but char codes must already come in the swapped order.
            hb_buffer_set_direction(m_hbBuffer, HB_DIRECTION_LTR);

#if !defined(USE_HARFBUZZ_LEGACY_SHAPING)
            // leading INHERITED, COMMON or UNKNOWN scripts have to be replaced by the first dedicated script (main script)
            // to allow contextual shaping: otherwise the main script characters created a buffer containing 1 character
            const hb_script_t hbScript = guessScript(unicodeFuncs, charIt, charsEnd);
            hb_buffer_set_script(m_hbBuffer, hbScript);

            // add chars using same script to buffer
            // - include chars with COMMON property to kern spaces, solidus, minus, etc. correct (8000+ characters)
            // - include chars with INHERITED property to shape diacritics correct (600+ characters)
            // - include chars with UNKNOWN property (private use area)
            while (charIt != charsEnd)
            {
                const hb_script_t cpScript = hb_unicode_script(unicodeFuncs, *charIt);
                if ((cpScript == hbScript) || (HB_SCRIPT_COMMON == cpScript) || (HB_SCRIPT_INHERITED == cpScript) || (HB_SCRIPT_UNKNOWN == cpScript))
#else
            const hb_script_t hbScript = hb_unicode_script(unicodeFuncs, *charIt);
            hb_buffer_set_script(m_hbBuffer, hbScript);

            // add chars using same script to buffer
            while (charIt != charsEnd)
            {
                if (hb_unicode_script(unicodeFuncs, *charIt) == hbScript)
#endif
                {
                    hb_buffer_add(m_hbBuffer, *charIt, clusterIdx);
                    charIt++;
                }
                else
                    break;
            }

            hb_shape(m_hbFont, m_hbBuffer, nullptr, 0);

            uint32_t glyphCount;
            hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(m_hbBuffer, &glyphCount);
            uint32_t glyphPosCount;
            hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(m_hbBuffer, &glyphPosCount);
            assert(glyphPosCount == glyphCount);

            for (uint32_t i = 0; i < glyphCount; i++)
            {
                HBGlyphInfo glyphInfo;
                glyphInfo.glyphIndex = glyph_info[i].codepoint;
                glyphInfo.advance = RoundHBFixedToInt(glyph_pos[i].x_advance);
                glyphInfo.offsetX = RoundHBFixedToInt(glyph_pos[i].x_offset);
                glyphInfo.offsetY = RoundHBFixedToInt(glyph_pos[i].y_offset);
                hbGlyphInfos.push_back(glyphInfo);
            }

            clusterIdx++;
        }

        // Use FT2 to load glyph metrics for reshaped glyphs,
        // adjust positions and advance according to HB data
        Freetype2FontInstance::activateSize();
        positionedGlyphs.reserve(positionedGlyphs.size() + hbGlyphInfos.size());

        GlyphId lastGlyphId(0);
        for (size_t i = 0u; i < hbGlyphInfos.size(); ++i)
        {
            const HBGlyphInfo& hbGlyphInfo = hbGlyphInfos[i];
            const GlyphId glyphId(hbGlyphInfo.glyphIndex);

            const GlyphMetrics* glyphMetricsEntry = getGlyphMetricsData(glyphId);
            if (glyphMetricsEntry == nullptr)
                continue;

            GlyphMetrics glyphMetrics = *glyphMetricsEntry;
            glyphMetrics.posX += hbGlyphInfo.offsetX;
            glyphMetrics.posY += hbGlyphInfo.offsetY;
            glyphMetrics.advance = hbGlyphInfo.advance; // advance is fully overridden by HB

            // kerning starts from second character
            if (i > 0u)
            {
                const int32_t kerning = getKerningAdvance(lastGlyphId, glyphId);
                glyphMetrics.posX += kerning;
                glyphMetrics.advance += kerning;
            }

            positionedGlyphs.push_back(glyphMetrics);
            lastGlyphId = glyphId;
        }
    }

    void HarfbuzzFontInstance::activateHBFontSize()
    {
        assert(m_hbFont != nullptr);
        const uint64_t x_scale = static_cast<uint64_t>(m_size->metrics.x_scale);
        const uint64_t y_scale = static_cast<uint64_t>(m_size->metrics.y_scale);
        const uint64_t units_per_EM = static_cast<uint64_t>(m_face->units_per_EM);

        // Setting the font size to the hb font, see hb_ft_font_create().
        // Needed for correct scaled offset values.
        hb_font_set_scale(m_hbFont,
            static_cast<int>((x_scale * units_per_EM + (1 << 15)) >> 16),
            static_cast<int>((y_scale * units_per_EM + (1 << 15)) >> 16));

        // also activate on freetype
        activateSize();
    }
}
