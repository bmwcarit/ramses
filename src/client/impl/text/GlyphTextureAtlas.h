//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/text/GlyphMapping.h"
#include "ramses/client/text/GlyphMetrics.h"
#include "impl/text/GlyphGeometry.h"
#include "impl/text/GlyphTexturePage.h"

#include <unordered_map>
#include <memory>

namespace ramses
{
    class Scene;
    class TextureSampler;
}

namespace ramses::internal
{
    class GlyphTextureAtlas
    {
    public:
        GlyphTextureAtlas(Scene& scene, QuadSize const& pageSize);
        ~GlyphTextureAtlas();

        void registerGlyph(const GlyphKey& key, const QuadSize& size, GlyphData&& data);
        bool isGlyphRegistered(const GlyphKey& key) const;

        GlyphGeometry mapGlyphsAndCreateGeometry(const GlyphMetricsVector& positionedGlyphVector);
        void unmapGlyphsFromPage(const GlyphMetricsVector& positionedGlyphVector, size_t atlasPage);

        const TextureSampler& getTextureSampler(size_t atlasPage) const;

        GlyphTextureAtlas(const GlyphTextureAtlas&) = delete;
        GlyphTextureAtlas& operator=(const GlyphTextureAtlas&) = delete;
        GlyphTextureAtlas(GlyphTextureAtlas&&) = delete;
        GlyphTextureAtlas& operator=(GlyphTextureAtlas&&) = delete;

    private:
        // Glyph page handling (i.e. texture resource related)
        size_t createNewPage();
        GlyphTexturePage& getPage(size_t atlasPage);
        GlyphTexturePage const& getPage(size_t atlasPage) const;

        bool findMappingForPage(size_t atlasPage, const GlyphMetricsVector& glyphs);
        GlyphGeometry createGlyphsGeometry(size_t atlasPage, const GlyphMetricsVector& glyphs);
        void categorizeGlyphs(size_t atlasPage, const GlyphMetricsVector& glyphs, std::vector<GlyphKey>& tomap, std::vector<GlyphKey>& mapped);

        Scene& m_scene;

        const QuadSize m_pageSize;

        GlyphTexturePage::GlyphPageData m_cacheForGlyphPageDataUpdate;

        using GlyphTexturePageVector = std::vector<std::unique_ptr<GlyphTexturePage>>;
        GlyphTexturePageVector m_glyphAtlasPages;

        struct GlyphInfo
        {
            QuadSize size;
            GlyphData data;
            GlyphMappings glyphMapping;
        };

        using GlyphInfoMap = std::unordered_map<GlyphKey, GlyphInfo>;
        GlyphInfoMap m_glyphInfoMap;
    };
}

