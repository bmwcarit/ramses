//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses-client-api/RamsesClient.h"

#include "ramses-text/GlyphGeometry.h"
#include "ramses-text-api/Glyph.h"
#include "ramses-text/GlyphTextureAtlas.h"
#include "ramses-text-api/FontRegistry.h"

namespace
{
    constexpr uint32_t AtlasTextureWidth = 12;
    constexpr uint32_t AtlasTextureHeight = 20;
    static const ramses::FontInstanceId FakeFontId{ 13u };
}

namespace ramses
{
    class AGlyphTextureAtlas : public testing::Test
    {
    public:
        AGlyphTextureAtlas()
            : m_client("test glyph atlas", m_framework)
            , m_scene(*m_client.createScene(1u))
            , m_atlas(m_scene, QuadSize(AtlasTextureWidth, AtlasTextureHeight))
        {
        }

        GlyphGeometry createTestGlyphGeometry(GlyphMetricsVector glyphs)
        {
            for (const auto& glyph : glyphs)
            {
                GlyphData forgedGlyphData(glyph.width * glyph.height);
                m_atlas.registerGlyph(glyph.key, QuadSize(glyph.width, glyph.height), std::move(forgedGlyphData));
            }
            return m_atlas.mapGlyphsAndCreateGeometry(glyphs);
        }

        bool glyphMappingsOverlap(const GlyphGeometry& glyphGeometry1, const GlyphGeometry& glyphGeometry2) const
        {
            if (glyphGeometry1.atlasPage != glyphGeometry2.atlasPage)
            {
                return false;
            }

            const float x1_lo = glyphGeometry1.texcoords[2];
            const float y1_lo = glyphGeometry1.texcoords[3];
            const float x1_hi = glyphGeometry1.texcoords[6];
            const float y1_hi = glyphGeometry1.texcoords[7];

            const float x2_lo = glyphGeometry2.texcoords[2];
            const float y2_lo = glyphGeometry2.texcoords[3];
            const float x2_hi = glyphGeometry2.texcoords[6];
            const float y2_hi = glyphGeometry2.texcoords[7];

            const float x_lo = std::max(x1_lo, x2_lo);
            const float y_lo = std::max(y1_lo, y2_lo);
            const float x_hi = std::min(x1_hi, x2_hi);
            const float y_hi = std::min(y1_hi, y2_hi);

            return (x_hi > x_lo) && (y_hi > y_lo);
        }

        void verifyGlyphMapping(const GlyphGeometry& glyphGeometry, float x_lo_expected, float y_lo_expected, float x_hi_expected, float y_hi_expected) const
        {
            EXPECT_FLOAT_EQ(glyphGeometry.texcoords[2], x_lo_expected);
            EXPECT_FLOAT_EQ(glyphGeometry.texcoords[3], y_lo_expected);
            EXPECT_FLOAT_EQ(glyphGeometry.texcoords[6], x_hi_expected);
            EXPECT_FLOAT_EQ(glyphGeometry.texcoords[7], y_hi_expected);
        }

        void expectGeometrySize(size_t numGlyphs, GlyphGeometry const& glyphGeometry)
        {
            EXPECT_EQ(glyphGeometry.positions.size(), numGlyphs * 8u);
            EXPECT_EQ(glyphGeometry.indices.size(), numGlyphs * 6u);
            EXPECT_EQ(glyphGeometry.texcoords.size(), numGlyphs * 8u);
        }

        void expectGeometry(int left, int right, int bottom, int top, GlyphGeometry const& geo, int i = 0, int xoffset = 0, int yoffset = 0)
        {
            constexpr int padding = 1;
            constexpr float halfPadding = padding / 2.f;

            EXPECT_EQ(geo.positions[i * 8 + 0], left - halfPadding);
            EXPECT_EQ(geo.positions[i * 8 + 1], bottom - halfPadding);

            EXPECT_EQ(geo.positions[i * 8 + 2], left - halfPadding);
            EXPECT_EQ(geo.positions[i * 8 + 3], top + halfPadding);

            EXPECT_EQ(geo.positions[i * 8 + 4], right + halfPadding);
            EXPECT_EQ(geo.positions[i * 8 + 5], top + halfPadding);

            EXPECT_EQ(geo.positions[i * 8 + 6], right + halfPadding);
            EXPECT_EQ(geo.positions[i * 8 + 7], bottom - halfPadding);

            EXPECT_EQ(geo.indices[i * 6 + 0], i * 4 + 2);
            EXPECT_EQ(geo.indices[i * 6 + 1], i * 4 + 1);
            EXPECT_EQ(geo.indices[i * 6 + 2], i * 4 + 0);

            EXPECT_EQ(geo.indices[i * 6 + 3], i * 4 + 3);
            EXPECT_EQ(geo.indices[i * 6 + 4], i * 4 + 2);
            EXPECT_EQ(geo.indices[i * 6 + 5], i * 4 + 0);

            EXPECT_NEAR(geo.texcoords[i * 8 + 0], (xoffset + padding - halfPadding) / AtlasTextureWidth, 0.0001f);
            EXPECT_NEAR(geo.texcoords[i * 8 + 1], (yoffset + top - bottom + padding + halfPadding) / AtlasTextureHeight, 0.0001f);

            EXPECT_NEAR(geo.texcoords[i * 8 + 2], (xoffset + padding - halfPadding) / AtlasTextureWidth, 0.0001f);
            EXPECT_NEAR(geo.texcoords[i * 8 + 3], (yoffset + padding - halfPadding) / AtlasTextureHeight, 0.0001f);

            EXPECT_NEAR(geo.texcoords[i * 8 + 4], (xoffset + right - left + padding + halfPadding) / AtlasTextureWidth, 0.0001f);
            EXPECT_NEAR(geo.texcoords[i * 8 + 5], (yoffset + padding - halfPadding) / AtlasTextureHeight, 0.0001f);

            EXPECT_NEAR(geo.texcoords[i * 8 + 6], (xoffset + right - left + padding + halfPadding) / AtlasTextureWidth, 0.0001f);
            EXPECT_NEAR(geo.texcoords[i * 8 + 7], (yoffset + top - bottom + padding + halfPadding) / AtlasTextureHeight, 0.0001f);
        }

    protected:
        RamsesFramework m_framework;
        RamsesClient m_client;
        Scene& m_scene;

        FontRegistry m_fontRegistry;
        GlyphTextureAtlas m_atlas;
    };

    TEST_F(AGlyphTextureAtlas, MapsGlyphWhichFitsEntirelyInOneTextureAndLeavesLotsOfFreeSpace)
    {
        const GlyphMetricsVector glyphs = { {GlyphKey(GlyphId('a'), FakeFontId), 3, 5, 0, 0, 0} };
        auto glyphGeometry = createTestGlyphGeometry(glyphs);

        verifyGlyphMapping(glyphGeometry, 0.041666668f, 0.025f, 0.375f, 0.32499999f);
        EXPECT_EQ(0u, glyphGeometry.atlasPage);
    }

    TEST_F(AGlyphTextureAtlas, MapsGlyphWhichBarelyFitsInOneTexture)
    {
        const GlyphMetricsVector glyphs = { { GlyphKey(GlyphId('a'), FakeFontId), 10, 18, 0, 0, 0 } };
        auto glyphGeometry = createTestGlyphGeometry(glyphs);

        verifyGlyphMapping(glyphGeometry, 0.041666668f, 0.025f, 0.95833331f, 0.97500002f);
        EXPECT_EQ(0u, glyphGeometry.atlasPage);
    }

    TEST_F(AGlyphTextureAtlas, MapsTwoGlyphsWhichFitInOneTextureHorizontally)
    {
        const GlyphMetricsVector glyphLeft = { {GlyphKey(GlyphId('a'), FakeFontId), 3, 5, 0, 0, 0} };
        auto glyphGeometryLeft = createTestGlyphGeometry(glyphLeft);
        const GlyphMetricsVector glyphRight = { { GlyphKey(GlyphId('b'), FakeFontId), 3, 5, 0, 0, 0 } };
        auto glyphGeometryRight = createTestGlyphGeometry(glyphRight);

        verifyGlyphMapping(glyphGeometryLeft, 0.041666668f, 0.025f, 0.375f, 0.32499999f);
        verifyGlyphMapping(glyphGeometryRight, 0.45833334f, 0.025f, 0.79166669f, 0.32499999f);
        EXPECT_FALSE(glyphMappingsOverlap(glyphGeometryRight, glyphGeometryLeft));
        EXPECT_EQ(0u, glyphGeometryLeft.atlasPage);
        EXPECT_EQ(0u, glyphGeometryRight.atlasPage);
    }

    TEST_F(AGlyphTextureAtlas, MapsTwoGlyphsWhichFitInOneTextureButSecondGlyphIsTooWideToFitHorizontally)
    {
        // Reordered by stability sorter -> bigger glyph comes first
        const GlyphMetricsVector glyphTop = { { GlyphKey(GlyphId('a'), FakeFontId), 9, 5, 0, 0, 0 } };
        auto glyphGeometryTop = createTestGlyphGeometry(glyphTop);
        const GlyphMetricsVector glyphBottom = { { GlyphKey(GlyphId('b'), FakeFontId), 3, 5, 0, 0, 0 } };
        auto glyphGeometryBottom = createTestGlyphGeometry(glyphBottom);

        verifyGlyphMapping(glyphGeometryTop, 0.041666668f, 0.025f, 0.875f, 0.32499999f);
        verifyGlyphMapping(glyphGeometryBottom, 0.041666668f, 0.375f, 0.375f, 0.67500001f);
        EXPECT_FALSE(glyphMappingsOverlap(glyphGeometryTop, glyphGeometryBottom));
        EXPECT_EQ(0u, glyphGeometryTop.atlasPage);
        EXPECT_EQ(0u, glyphGeometryBottom.atlasPage);
    }

    TEST_F(AGlyphTextureAtlas, MapsTwoGlyphsWhichDontFitInASingleTexture)
    {
        const GlyphMetricsVector glyphMappedToTex1 = { { GlyphKey(GlyphId('a'), FakeFontId), 10, 18, 0, 0, 0 } };
        const GlyphMetricsVector glyphMappedToTex2 = { { GlyphKey(GlyphId('b'), FakeFontId), 3, 5, 0, 0, 0 } };
        auto glyphGeometryMappedToTex1 = createTestGlyphGeometry(glyphMappedToTex1);
        auto glyphGeometryMappedToTex2 = createTestGlyphGeometry(glyphMappedToTex2);

        verifyGlyphMapping(glyphGeometryMappedToTex1, 0.041666668f, 0.025f, 0.95833331f, 0.97500002f);
        verifyGlyphMapping(glyphGeometryMappedToTex2, 0.041666668f, 0.025f, 0.375f, 0.32499999f);
        EXPECT_FALSE(glyphMappingsOverlap(glyphGeometryMappedToTex1, glyphGeometryMappedToTex2));
        EXPECT_EQ(0u, glyphGeometryMappedToTex1.atlasPage);
        EXPECT_EQ(1u, glyphGeometryMappedToTex2.atlasPage);
    }

    TEST_F(AGlyphTextureAtlas, MapsZeroAreaGlyphAndDoesNotCreateGeometryForIt)
    {
        const GlyphMetricsVector glyphs =
        {
            { GlyphKey(GlyphId('a'), FakeFontId), 10, 8, 0, 0, 0 },
            { GlyphKey(GlyphId(' '), FakeFontId), 0, 0, 0, 0, 0 },
        };

        auto glyphGeometry = createTestGlyphGeometry(glyphs);
        expectGeometrySize(1, glyphGeometry);

        m_atlas.unmapGlyphsFromPage(glyphs, glyphGeometry.atlasPage);
    }

    TEST_F(AGlyphTextureAtlas, CreatesGeometryForSingleGlyph)
    {
        constexpr int right = 10;
        constexpr int top = 18;
        constexpr int left = 0;
        constexpr int bottom = 0;

        auto glyphGeometry = createTestGlyphGeometry({ { GlyphKey(GlyphId('a'), FakeFontId), right - left, top - bottom, left, bottom, 0 } });
        EXPECT_EQ(0u, glyphGeometry.atlasPage);

        expectGeometrySize(1u, glyphGeometry);
        expectGeometry(left, right, bottom, top, glyphGeometry);
    }

    TEST_F(AGlyphTextureAtlas, CreatesGeometryForSingleGlyphWithOffsetNotZero)
    {
        constexpr int right = 10;
        constexpr int top = 18;
        constexpr int left = 2;
        constexpr int bottom = 3;

        auto glyphGeometry = createTestGlyphGeometry({ { GlyphKey(GlyphId('a'), FakeFontId), right - left, top - bottom, left, bottom, 0 } });
        EXPECT_EQ(0u, glyphGeometry.atlasPage);

        expectGeometrySize(1u, glyphGeometry);
        expectGeometry(left, right, bottom, top, glyphGeometry);
    }

    TEST_F(AGlyphTextureAtlas, CreatesMultipleGeometriesForGlyphsInSamePage)
    {
        constexpr int right = 10;
        constexpr int top = 8;
        constexpr int left = 0;
        constexpr int bottom = 0;

        const GlyphMetricsVector glyphs =
        {
            { GlyphKey(GlyphId('a'), FakeFontId), right - left, top - bottom, left, bottom, 0 },
            { GlyphKey(GlyphId('b'), FakeFontId), right - left, top - bottom, left, bottom, 0 },
        };
        const auto geometry = createTestGlyphGeometry(glyphs);
        EXPECT_EQ(0u, geometry.atlasPage);

        expectGeometrySize(glyphs.size(), geometry);
        expectGeometry(left, right, bottom, top, geometry, 0, 0, 0);
        expectGeometry(left, right, bottom, top, geometry, 1, 0, top - bottom + 2);
    }

    TEST_F(AGlyphTextureAtlas, CreatesMultipleGeometriesForGlyphsInSamePageWhenOffsetIsNotZero)
    {
        constexpr int right = 10;
        constexpr int top = 8;
        constexpr int left = 3;
        constexpr int bottom = 2;

        const GlyphMetricsVector glyphs =
        {
            { GlyphKey(GlyphId('a'), FakeFontId), right - left, top - bottom, left, bottom, 0 },
            { GlyphKey(GlyphId('b'), FakeFontId), right - left, top - bottom, left, bottom, 0 },
        };
        const auto geometry = createTestGlyphGeometry(glyphs);
        EXPECT_EQ(0u, geometry.atlasPage);

        expectGeometrySize(glyphs.size(), geometry);
        expectGeometry(left, right, bottom, top, geometry, 0, 0, 0);
        expectGeometry(left, right, bottom, top, geometry, 1, 0, top - bottom + 2);
    }

    TEST_F(AGlyphTextureAtlas, CreatesGeometryForStringWithSomeEmptyChars)
    {
        const GlyphMetricsVector glyphs =
        {
            { GlyphKey(GlyphId(' '), FakeFontId), 0u, 0u,  0,  0, 3 },
            { GlyphKey(GlyphId('a'), FakeFontId), 2u, 3u,  1, -1, 4 },
            { GlyphKey(GlyphId(' '), FakeFontId), 0u, 0u,  0,  0, 3 },
            { GlyphKey(GlyphId('b'), FakeFontId), 3u, 2u, -1,  1, 1 },
            { GlyphKey(GlyphId(' '), FakeFontId), 0u, 0u,  0,  0, 3 },
        };
        const auto geometry = createTestGlyphGeometry(glyphs);
        EXPECT_EQ(0u, geometry.atlasPage);

        // 2 chars with valid geometry
        expectGeometrySize(2u, geometry);
        expectGeometry(4, 6, -1, 2, geometry, 0);
        expectGeometry(9, 12, 1, 3, geometry, 1, 4);
    }

    TEST_F(AGlyphTextureAtlas, CreatesGeometryForStringWithSomeEmptyCharsWithZeroAdvance)
    {
        const GlyphMetricsVector glyphs =
        {
            { GlyphKey(GlyphId(' '), FakeFontId), 0u, 0u,  0,  0, 0 },
            { GlyphKey(GlyphId('a'), FakeFontId), 2u, 3u,  1, -1, 4 },
            { GlyphKey(GlyphId(' '), FakeFontId), 0u, 0u,  0,  0, 0 },
            { GlyphKey(GlyphId('b'), FakeFontId), 3u, 2u, -1,  1, 1 },
            { GlyphKey(GlyphId(' '), FakeFontId), 0u, 0u,  0,  0, 0 },
        };
        const auto geometry = createTestGlyphGeometry(glyphs);
        EXPECT_EQ(0u, geometry.atlasPage);

        // 2 chars with valid geometry
        expectGeometrySize(2u, geometry);
        expectGeometry(1, 3, -1, 2, geometry, 0);
        expectGeometry(3, 6, 1, 3, geometry, 1, 4);
    }

    TEST_F(AGlyphTextureAtlas, ReleasesGlyphFromPageAfterUnmappingAsOftenAsMappingIt)
    {
        {
            const GlyphMetricsVector glyphs =
            {
                { GlyphKey(GlyphId('a'), FakeFontId), 10, 8, 0, 0, 0 },
                { GlyphKey(GlyphId('b'), FakeFontId), 10, 8, 0, 0, 0 }
            };
            const auto geometry = createTestGlyphGeometry(glyphs);

            EXPECT_EQ(0u, geometry.atlasPage);
            m_atlas.unmapGlyphsFromPage(glyphs, 0u);
        }

        const GlyphMetricsVector glyphs =
        {
            { GlyphKey(GlyphId('c'), FakeFontId), 10, 8, 0, 0, 0 },
            { GlyphKey(GlyphId('d'), FakeFontId), 10, 8, 0, 0, 0 }
        };
        const auto geometry = createTestGlyphGeometry(glyphs);

        // TODO(Violin) does not work yet
        // EXPECT_EQ(geometry.atlasPage, 0u);
    }

    TEST_F(AGlyphTextureAtlas, DoesNotReleaseGlyphAfterMappingItMoreThanUnmappingIt)
    {
        {
            const GlyphMetricsVector glyph1 = { { GlyphKey(GlyphId('a'), FakeFontId), 10, 8, 0, 0, 0 } };
            const GlyphMetricsVector glyph2 = { { GlyphKey(GlyphId('b'), FakeFontId), 10, 8, 0, 0, 0 } };
            const auto geometry1 = createTestGlyphGeometry(glyph1);
            const auto geometry2 = createTestGlyphGeometry(glyph2);
            const auto geometry3 = m_atlas.mapGlyphsAndCreateGeometry(glyph1);

            EXPECT_EQ(0u, geometry1.atlasPage);
            EXPECT_EQ(0u, geometry2.atlasPage);
            EXPECT_EQ(0u, geometry3.atlasPage);

            m_atlas.unmapGlyphsFromPage(glyph1, 0u);
            m_atlas.unmapGlyphsFromPage(glyph2, 0u);
        }

        const GlyphMetricsVector glyph1 = { { GlyphKey(GlyphId('c'), FakeFontId), 10, 8, 0, 0, 0 } };
        const GlyphMetricsVector glyph2 = { { GlyphKey(GlyphId('d'), FakeFontId), 10, 8, 0, 0, 0 } };
        const auto geometry1 = createTestGlyphGeometry(glyph1);
        const auto geometry2 = createTestGlyphGeometry(glyph2);

        // TODO(Violin) does not work yet
        //EXPECT_EQ(geometry1.atlasPage, 0u);
        EXPECT_EQ(1u, geometry2.atlasPage);
    }

    TEST_F(AGlyphTextureAtlas, TreatsTheSameCharWithDifferentFontInstancesAsDifferentGlyphs)
    {
        const auto glyph                            = GlyphKey(GlyphId('a'), ramses::FontInstanceId{ 3 });
        const auto glyphWithDifferentFontInstance   = GlyphKey(GlyphId('a'), ramses::FontInstanceId{ 4 });

        m_atlas.registerGlyph(glyph, QuadSize(10, 8), GlyphData(10, 8));
        EXPECT_FALSE(m_atlas.isGlyphRegistered(glyphWithDifferentFontInstance));
    }

    TEST_F(AGlyphTextureAtlas, CanMapFourOfTheSameGlyphsToTheSameSpace)
    {
        const GlyphMetricsVector glyphsA =
        {
            { GlyphKey(GlyphId('a'), FakeFontId), 10, 8, 0, 0, 0 },
            { GlyphKey(GlyphId('a'), FakeFontId), 10, 8, 0, 0, 0 },
            { GlyphKey(GlyphId('a'), FakeFontId), 10, 8, 0, 0, 0 },
            { GlyphKey(GlyphId('a'), FakeFontId), 10, 8, 0, 0, 0 },
        };

        const GlyphMetricsVector glyphsB =
        {
            { GlyphKey(GlyphId('b'), FakeFontId), 10, 8, 0, 0, 0 }
        };

        const auto geometryA = createTestGlyphGeometry(glyphsA);
        EXPECT_EQ(0u, geometryA.atlasPage);
        expectGeometrySize(4, geometryA);
        expectGeometry(0, 10, 0, 8, geometryA, 0, 0, 0);

        const auto geometryB = createTestGlyphGeometry(glyphsB);
        EXPECT_EQ(0u, geometryB.atlasPage);
        expectGeometrySize(1, geometryB);
        expectGeometry(0, 10, 0, 8, geometryB, 0, 0, 8 + 2);
    }

    TEST_F(AGlyphTextureAtlas, MapsNonFittingGlyphsToNewPageAndKeepsSpaceOnFirstPageOpen)
    {
        const GlyphMetricsVector glyphs1 =
        {
            { GlyphKey(GlyphId('a'), FakeFontId), 10, 5, 0, 0, 0 }
        };

        const GlyphMetricsVector glyphs1b =
        {
            { GlyphKey(GlyphId('1'), FakeFontId), 10, 4, 0, 0, 0 },
        };

        const GlyphMetricsVector glyphs2 =
        {
            { GlyphKey(GlyphId('a'), FakeFontId), 10, 5, 0, 0, 0 },
            { GlyphKey(GlyphId('b'), FakeFontId), 10, 4, 0, 0, 0 },
            { GlyphKey(GlyphId('c'), FakeFontId), 10, 4, 0, 0, 0 }
        };

        const GlyphMetricsVector glyphs3 =
        {
            { GlyphKey(GlyphId('d'), FakeFontId), 10, 4, 0, 0, 0 }
        };

        const GlyphMetricsVector glyphs4 =
        {
            { GlyphKey(GlyphId('e'), FakeFontId), 10, 4, 0, 0, 0 }
        };

        const auto geometry1 = createTestGlyphGeometry(glyphs1);
        EXPECT_EQ(0u, geometry1.atlasPage);

        const auto geometry1b = createTestGlyphGeometry(glyphs1b);
        EXPECT_EQ(0u, geometry1b.atlasPage);

        const auto geometry2 = createTestGlyphGeometry(glyphs2);
        EXPECT_EQ(1u, geometry2.atlasPage);

        const auto geometry3 = createTestGlyphGeometry(glyphs3);
        EXPECT_EQ(0u, geometry3.atlasPage);

        // test if refcount of first glyph was not increased by creating the second set of glyphs
        m_atlas.unmapGlyphsFromPage(glyphs1, 0u);

        const auto geometry4 = createTestGlyphGeometry(glyphs4);
        // TODO(Violin) does not work yet
        //EXPECT_EQ(0u, geometry4.atlasPage);
    }
}
