//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/text/GlyphTextureAtlas.h"
#include "internal/Core/Utils/LogMacros.h"
#include <cassert>
#include <algorithm>

namespace ramses::internal
{
    GlyphTextureAtlas::GlyphTextureAtlas(ramses::Scene& scene, QuadSize const& pageSize)
        : m_scene(scene)
        , m_pageSize(pageSize)
    {
    }

    GlyphTextureAtlas::~GlyphTextureAtlas()
    {
        m_glyphAtlasPages.clear();
    }

    GlyphTexturePage& GlyphTextureAtlas::getPage(size_t atlasPage)
    {
        assert(atlasPage < m_glyphAtlasPages.size());
        return *m_glyphAtlasPages[atlasPage];
    }

    GlyphTexturePage const& GlyphTextureAtlas::getPage(size_t atlasPage) const
    {
        assert(atlasPage < m_glyphAtlasPages.size());
        return *m_glyphAtlasPages[atlasPage];
    }

    size_t GlyphTextureAtlas::createNewPage()
    {
        m_glyphAtlasPages.push_back(std::make_unique<GlyphTexturePage>(m_scene, m_pageSize));
        return m_glyphAtlasPages.size() - 1;
    }

    void GlyphTextureAtlas::registerGlyph(const GlyphKey& key, const QuadSize& size, GlyphData&& data)
    {
        assert(data.empty() == (size.getArea() == 0u)); // assert zero size if data empty or non-zero size if data not empty
        m_glyphInfoMap.insert(std::make_pair(key, GlyphInfo{ size, std::move(data), {} }));
    }

    bool GlyphTextureAtlas::isGlyphRegistered(const GlyphKey& key) const
    {
        return m_glyphInfoMap.count(key) == 1;
    }

    bool GlyphTextureAtlas::findMappingForPage(size_t atlasPage, const GlyphMetricsVector& glyphs)
    {
        std::vector<GlyphKey> tomap;
        std::vector<GlyphKey> mapped;
        categorizeGlyphs(atlasPage, glyphs, tomap, mapped);

        std::vector<Quad> glyphsOnPage;
        for (auto const& glyphkey : tomap)
        {
            GlyphInfo& glyphInfo = m_glyphInfoMap.at(glyphkey);
            const QuadSize sizeInAtlas(glyphInfo.size.x + 2, glyphInfo.size.y + 2); // padding requires 2 more pixels for each dimension

            const GlyphTexturePage::QuadIndex freeQuadOnPage = getPage(atlasPage).findFreeSpace(sizeInAtlas);
            if (freeQuadOnPage != std::numeric_limits<GlyphTexturePage::QuadIndex>::max())
            {
                const QuadOffset origin = getPage(atlasPage).claimSpace(freeQuadOnPage, sizeInAtlas);
                glyphsOnPage.emplace_back(origin, sizeInAtlas);
            }
            else
            {
                // that didn't work out. revert claimed space and return with fail
                for (auto const& torevert : glyphsOnPage)
                {
                    getPage(atlasPage).releaseSpace(torevert);
                }
                return false;
            }
        }

        // actually put new glyphs on page
        assert(glyphsOnPage.size() == tomap.size());
        auto it = glyphsOnPage.begin();
        for (auto const& glyphkey : tomap)
        {
            GlyphInfo& glyphInfo = m_glyphInfoMap.at(glyphkey);
            glyphInfo.glyphMapping.emplace(atlasPage, GlyphMapping{ 1u, *it });
            getPage(atlasPage).updateDataWithPadding(*it, &glyphInfo.data[0], m_cacheForGlyphPageDataUpdate);
            it++;
        }

        // increase ref count on the glyphs already there
        for (auto const& glyphkey : mapped)
        {
            GlyphInfo& glyphInfo = m_glyphInfoMap.at(glyphkey);
            auto mappingIt = glyphInfo.glyphMapping.find(atlasPage);
            assert(mappingIt != glyphInfo.glyphMapping.end());
            ++mappingIt->second.refCount;
        }

        return true;
    }

    void GlyphTextureAtlas::categorizeGlyphs(size_t atlasPage, const GlyphMetricsVector& glyphs, std::vector<GlyphKey>& tomap, std::vector<GlyphKey>& mapped)
    {
        assert(tomap.empty());
        assert(mapped.empty());
        tomap.reserve(glyphs.size());
        mapped.reserve(glyphs.size());
        for (auto const& glyph : glyphs)
        {
            if (glyph.height == 0 || glyph.width == 0)
                continue;

            GlyphInfo& glyphInfo = m_glyphInfoMap.at(glyph.key);
            if (glyphInfo.glyphMapping.end() != glyphInfo.glyphMapping.find(atlasPage))
            {
                if (mapped.end() == std::find(mapped.begin(), mapped.end(), glyph.key))
                    mapped.push_back(glyph.key);
            }
            else
            {
                if (tomap.end() == std::find(tomap.begin(), tomap.end(), glyph.key))
                    tomap.push_back(glyph.key);
            }
        }
    }

    GlyphGeometry GlyphTextureAtlas::mapGlyphsAndCreateGeometry(const GlyphMetricsVector& positionedGlyphVector)
    {
        assert(positionedGlyphVector.end() == std::find_if(positionedGlyphVector.begin(), positionedGlyphVector.end(), [this](GlyphMetrics const& glyph)
        {
            return !isGlyphRegistered(glyph.key);
        }));

        size_t atlasPage = 0;
        bool success = false;
        for (; atlasPage < m_glyphAtlasPages.size(); ++atlasPage)
        {
            success = findMappingForPage(atlasPage, positionedGlyphVector);
            if (success)
                break;
        }

        if (!success)
        {
            // no results, so try new, empty page
            atlasPage = createNewPage();
            if (!findMappingForPage(atlasPage, positionedGlyphVector))
            {
                m_glyphAtlasPages.pop_back();
                LOG_ERROR(CONTEXT_TEXT, "GlyphTextureAtlas::mapGlyphsAndCreateGeometry failed - glyphs do not fit on one page, reduce string or increase atlas texture size");
                return {};
            }
        }

        return createGlyphsGeometry(atlasPage, positionedGlyphVector);
    }

    GlyphGeometry GlyphTextureAtlas::createGlyphsGeometry(size_t atlasPage, const GlyphMetricsVector& glyphs)
    {
        GlyphGeometry geometry;
        geometry.positions.reserve(glyphs.size() * 8);
        geometry.texcoords.reserve(glyphs.size() * 8);
        geometry.indices.reserve(glyphs.size() * 6);
        geometry.atlasPage = atlasPage;

        // Create vertices/texcoords
        uint16_t indicesCounter = 0;
        int32_t totalAdvance = 0;
        for (const auto& glyph : glyphs)
        {
            if (glyph.height == 0 || glyph.width == 0)
            {
                totalAdvance += glyph.advance;
                continue;
            }

            const GlyphKey& glyphKey = glyph.key;

            // Origin - 0.5, Size + 1: The geometry extends 0.5 texel in each direction.
            const float minX = static_cast<float>(totalAdvance + glyph.posX) - 0.5f;
            const float minY = static_cast<float>(glyph.posY) - 0.5f;
            const float maxX = minX + static_cast<float>(glyph.width) + 1.0f;
            const float maxY = minY + static_cast<float>(glyph.height) + 1.0f;

            geometry.positions.emplace_back(minX, minY);  // LowerLeft
            geometry.positions.emplace_back(minX, maxY);  // UpperLeft
            geometry.positions.emplace_back(maxX, maxY);  // UpperRight
            geometry.positions.emplace_back(maxX, minY);  // LowerRight

            const Quad& glyphQuad = m_glyphInfoMap.at(glyphKey).glyphMapping.at(atlasPage).quad;
            const auto lowerLeftX = static_cast<float>(glyphQuad.getOrigin().x) + 0.5f;
            const auto lowerLeftY = static_cast<float>(glyphQuad.getOrigin().y) + 0.5f;
            const QuadSize glyphSize = glyphQuad.getSize();
            const auto sizeX = static_cast<float>(glyphSize.x) - 1.0f;
            const auto sizeY = static_cast<float>(glyphSize.y) - 1.0f;

            const float upperRightX = lowerLeftX + sizeX;
            const float upperRightY = lowerLeftY + sizeY;

            const uint32_t textureWidth = m_pageSize.x;
            const uint32_t textureHeight = m_pageSize.y;
            const auto lowerLeftX_textureSpace = lowerLeftX / static_cast<float>(textureWidth);
            const auto lowerLeftY_textureSpace = lowerLeftY / static_cast<float>(textureHeight);
            const auto upperRightX_textureSpace = upperRightX / static_cast<float>(textureWidth);
            const auto upperRightY_textureSpace = upperRightY / static_cast<float>(textureHeight);

            geometry.texcoords.emplace_back(lowerLeftX_textureSpace, upperRightY_textureSpace);
            geometry.texcoords.emplace_back(lowerLeftX_textureSpace, lowerLeftY_textureSpace);
            geometry.texcoords.emplace_back(upperRightX_textureSpace, lowerLeftY_textureSpace);
            geometry.texcoords.emplace_back(upperRightX_textureSpace, upperRightY_textureSpace);

            geometry.indices.push_back(indicesCounter + 2);
            geometry.indices.push_back(indicesCounter + 1);
            geometry.indices.push_back(indicesCounter);
            geometry.indices.push_back(indicesCounter + 3);
            geometry.indices.push_back(indicesCounter + 2);
            geometry.indices.push_back(indicesCounter);

            indicesCounter += 4u;
            totalAdvance += glyph.advance;
        }

        return geometry;
    }

    void GlyphTextureAtlas::unmapGlyphsFromPage(const GlyphMetricsVector& positionedGlyphVector, size_t atlasPage)
    {
        std::vector<GlyphKey> tomap;
        std::vector<GlyphKey> mapped;
        categorizeGlyphs(atlasPage, positionedGlyphVector, tomap, mapped);
        assert(tomap.empty());

        for (const auto& glyphkey : mapped)
        {
            auto& glyphToPageMapping = m_glyphInfoMap.at(glyphkey).glyphMapping.at(atlasPage);
            assert(glyphToPageMapping.refCount != 0);
            --glyphToPageMapping.refCount;
            // TODO(Violin) implement explicit control of glyph memory
            // Idea: remove refcount altogether, cache glyph memory in IFontAccessor
        }
    }

    const ramses::TextureSampler& GlyphTextureAtlas::getTextureSampler(size_t atlasPage) const
    {
        return getPage(atlasPage).getSampler();
    }
}
