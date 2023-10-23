//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/text/GlyphTextureAtlas.h"
#include "ramses/client/text/TextLine.h"
#include "ramses/client/text/FontInstanceOffsets.h"
#include <unordered_map>
#include <string>

namespace ramses
{
    class Scene;
    class MeshNode;
    class Effect;
    class IFontAccessor;
}

namespace ramses::internal
{
    class TextCacheImpl
    {
    public:
        TextCacheImpl(ramses::Scene& scene, IFontAccessor& fontAccessor, uint32_t atlasTextureWidth, uint32_t atlasTextureHeight);
        ~TextCacheImpl() = default;

        GlyphMetricsVector      getPositionedGlyphs(const std::u32string& str, FontInstanceId font);
        GlyphMetricsVector      getPositionedGlyphs(const std::u32string& str, const FontInstanceOffsets& fontOffsets);

        TextLineId              createTextLine(const GlyphMetricsVector& glyphs, const Effect& effect);
        TextLine const*         getTextLine(TextLineId textId) const;
        TextLine*               getTextLine(TextLineId textId);
        bool                    deleteTextLine(TextLineId textId);

        TextCacheImpl(const TextCacheImpl&) = delete;
        TextCacheImpl& operator=(const TextCacheImpl&) = delete;
        TextCacheImpl(TextCacheImpl&&) = delete;
        TextCacheImpl& operator=(TextCacheImpl&&) = delete;

    private:
        ramses::Scene& m_scene;
        IFontAccessor& m_fontAccessor;
        GlyphTextureAtlas m_textureAtlas;

        using Texts = std::unordered_map<TextLineId, TextLine>;
        Texts m_textLines;

        TextLineId m_textIdCounter{ 0u };
    };
}
