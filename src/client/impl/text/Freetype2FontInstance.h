//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/text/IFontInstance.h"
#include "impl/text/Freetype2Wrapper.h"
#include "ramses/client/text/Glyph.h"
#include <unordered_map>
#include <unordered_set>

namespace ramses::internal
{
    class Freetype2FontInstance : public IFontInstance
    {
    public:
        Freetype2FontInstance(FontInstanceId id, FT_Face fontFace, uint32_t pixelSize, bool forceAutohinting);
        ~Freetype2FontInstance() override;

        [[nodiscard]] bool      supportsCharacter(char32_t character) const final override;
        [[nodiscard]] int       getHeight() const override;
        [[nodiscard]] int       getAscender() const override;
        [[nodiscard]] int       getDescender() const override;

        void                 loadAndAppendGlyphMetrics(std::u32string::const_iterator charsBegin, std::u32string::const_iterator charsEnd, GlyphMetricsVector& positionedGlyphs) override;
        GlyphData            loadGlyphBitmapData(GlyphId glyphId, uint32_t& sizeX, uint32_t& sizeY) final override;
        std::unordered_set<char32_t> getAllSupportedCharacters() override;

        [[nodiscard]] GlyphId getGlyphId(char32_t character) const;

    protected:
        struct GlyphBitmapData;

        const GlyphMetrics*    getGlyphMetricsData(GlyphId glyphId);
        const GlyphBitmapData* getGlyphBitmapData(GlyphId glyphId);
        bool                   loadGlyph(GlyphId glyphId);
        void                   activateSize() const;
        [[nodiscard]] int32_t                getKerningAdvance(GlyphId glyphIdentifier1, GlyphId glyphIdentifier2) const;
        void                   cacheAllSupportedCharacters();

        FontInstanceId          m_id;
        FT_Face                 m_face = nullptr;
        FT_Size                 m_size = nullptr;
        bool                    m_forceAutohinting = false;
        int                     m_height = 0;
        int                     m_ascender = 0;
        int                     m_descender = 0;
        bool                    m_allSupportedCharactersCached = false;

        struct GlyphBitmapData
        {
            GlyphData data;
            uint32_t  width = 0;
            uint32_t  height = 0;
        };

        // The reason for separation of the metrics and bitmap data cache
        // is that bitmap loading is relatively heavy and not needed for determining text layout
        // which needs metrics only.
        std::unordered_map<GlyphId, GlyphMetrics> m_glyphMetricsCache;
        std::unordered_map<GlyphId, GlyphBitmapData> m_glyphBitmapCache;
        mutable std::unordered_map<char32_t, bool> m_supportedCharacters;

    private:
        static constexpr bool IsBidiMarker(char32_t character);
    };
}
