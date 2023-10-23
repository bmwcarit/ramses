//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/text/Freetype2FontInstance.h"

struct hb_font_t;
struct hb_buffer_t;

namespace ramses::internal
{
    class HarfbuzzFontInstance final : public Freetype2FontInstance
    {
    public:
        HarfbuzzFontInstance(FontInstanceId id, FT_Face fontFace, uint32_t pixelSize, bool forceAutohinting);
        ~HarfbuzzFontInstance() override;

        void loadAndAppendGlyphMetrics(std::u32string::const_iterator charsBegin, std::u32string::const_iterator charsEnd, GlyphMetricsVector& positionedGlyphs) override;

        HarfbuzzFontInstance(const HarfbuzzFontInstance&) = delete;
        HarfbuzzFontInstance(HarfbuzzFontInstance&&) = delete;
        HarfbuzzFontInstance operator=(const HarfbuzzFontInstance&) = delete;
        HarfbuzzFontInstance operator=(HarfbuzzFontInstance&&) = delete;

    private:
        void activateHBFontSize();

        hb_font_t* m_hbFont = nullptr;
        hb_buffer_t* m_hbBuffer =  nullptr;
    };
}
