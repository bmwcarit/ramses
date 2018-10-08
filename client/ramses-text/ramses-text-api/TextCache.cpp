//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-text-api/TextCache.h"
#include "ramses-text/TextCacheImpl.h"

namespace ramses
{
    TextCache::TextCache(Scene& scene, IFontAccessor& fontAccessor, uint32_t atlasTextureWidth, uint32_t atlasTextureHeight)
        : impl(new TextCacheImpl(scene, fontAccessor, atlasTextureWidth, atlasTextureHeight))
    {
    }

    TextCache::~TextCache()
    {
        delete impl;
    }

    GlyphMetricsVector TextCache::getPositionedGlyphs(const std::u32string& str, const FontInstanceOffsets& fontOffsets)
    {
        return impl->getPositionedGlyphs(str, fontOffsets);
    }

    GlyphMetricsVector TextCache::getPositionedGlyphs(const std::u32string& str, FontInstanceId font)
    {
        return impl->getPositionedGlyphs(str, font);
    }

    TextLineId TextCache::createTextLine(const GlyphMetricsVector& glyphs, const Effect& effect)
    {
        return impl->createTextLine(glyphs, effect);
    }

    TextLine const* TextCache::getTextLine(TextLineId textId) const
    {
        return impl->getTextLine(textId);
    }

    TextLine* TextCache::getTextLine(TextLineId textId)
    {
        return impl->getTextLine(textId);
    }

    bool TextCache::deleteTextLine(TextLineId textId)
    {
        return impl->deleteTextLine(textId);
    }
}
