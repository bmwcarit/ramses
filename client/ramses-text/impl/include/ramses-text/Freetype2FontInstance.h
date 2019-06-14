//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FREETYPE2FONTINSTANCE_H
#define RAMSES_FREETYPE2FONTINSTANCE_H

#include "ramses-text-api/IFontInstance.h"
#include "ramses-text/Freetype2Headers.h"
#include "ramses-text-api/Glyph.h"
#include "ramses-text/FontData.h"
#include <unordered_map>

namespace ramses
{
    class Freetype2Font;

    class Freetype2FontInstance : public IFontInstance
    {
    public:
        Freetype2FontInstance(FontInstanceId id, FT_Library freetypeLib, const FontData& font, uint32_t pixelSize, bool forceAutohinting);
        virtual ~Freetype2FontInstance();

        virtual bool      supportsCharacter(char32_t character) const override final;
        virtual int       getHeight() const override;
        virtual int       getAscender() const override;
        virtual int       getDescender() const override;

        virtual void      loadAndAppendGlyphMetrics(std::u32string::const_iterator charsBegin, std::u32string::const_iterator charsEnd, GlyphMetricsVector& positionedGlyphs) override;
        virtual GlyphData loadGlyphBitmapData(GlyphId glyphId, uint32_t& sizeX, uint32_t& sizeY) override final;

        GlyphId getGlyphId(char32_t character) const;

    protected:
        struct GlyphBitmapData;

        const GlyphMetrics*    getGlyphMetricsData(GlyphId glyphId);
        const GlyphBitmapData* getGlyphBitmapData(GlyphId glyphId);
        bool                   loadGlyph(GlyphId glyphId);
        void                   activateSize() const;
        int32_t                getKerningAdvance(GlyphId glyphIdentifier1, GlyphId glyphIdentifier2) const;

        FontInstanceId          m_id;
        const FontData&         m_font;
        FT_Face                 m_face = nullptr;
        FT_Size                 m_size = nullptr;
        bool                    m_forceAutohinting = false;
        int                     m_height = 0;
        int                     m_ascender = 0;
        int                     m_descender = 0;

        struct GlyphBitmapData
        {
            GlyphData data;
            uint32_t  width;
            uint32_t  height;
        };

        // The reason for separation of the metrics and bitmap data cache
        // is that bitmap loading is relatively heavy and not needed for determining text layout
        // which needs metrics only.
        std::unordered_map<GlyphId, GlyphMetrics> m_glyphMetricsCache;
        std::unordered_map<GlyphId, GlyphBitmapData> m_glyphBitmapCache;
    };
}

#endif
