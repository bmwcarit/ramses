//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#include <gtest/gtest.h>
#include "ramses-text-api/FontRegistry.h"
#include "ramses-text/Freetype2FontInstance.h"
#include "ramses-text/Quad.h"

namespace ramses
{
    class AFreetype2FontInstance :  public testing::Test
    {
    public:
        static void SetUpTestCase()
        {
            FRegistry = new FontRegistry;
            const auto fontId = FRegistry->createFreetype2Font("res/ramses-text-Roboto-Bold.ttf");

            FontInstanceId4 = FRegistry->createFreetype2FontInstanceWithHarfBuzz(fontId, 4);
            FontInstanceId10 = FRegistry->createFreetype2FontInstanceWithHarfBuzz(fontId, 10);

            FontInstance4 = static_cast<Freetype2FontInstance*>(FRegistry->getFontInstance(FontInstanceId4));
            FontInstance10 = static_cast<Freetype2FontInstance*>(FRegistry->getFontInstance(FontInstanceId10));
        }

        static void TearDownTestCase()
        {
            delete FRegistry;
        }

    protected:
        void expectGlyphMetricsEq(const GlyphMetrics& expected, const GlyphMetrics& actual)
        {
            EXPECT_EQ(expected.key.fontInstanceId, actual.key.fontInstanceId);
            EXPECT_EQ(expected.key.identifier.getValue(), actual.key.identifier.getValue());
            EXPECT_EQ(expected.posX, actual.posX);
            EXPECT_EQ(expected.posY, actual.posY);
            EXPECT_EQ(expected.width, actual.width);
            EXPECT_EQ(expected.height, actual.height);
            EXPECT_EQ(expected.advance, actual.advance);
        }

        GlyphMetricsVector getPositionedGlyphs(const std::u32string& str, IFontInstance& fontInstance)
        {
            GlyphMetricsVector ret;
            ret.reserve(str.size());
            fontInstance.loadAndAppendGlyphMetrics(str.cbegin(), str.cend(), ret);
            return ret;
        }

        static FontRegistry*  FRegistry;
        static FontInstanceId FontInstanceId4;
        static FontInstanceId FontInstanceId10;
        static Freetype2FontInstance* FontInstance4;
        static Freetype2FontInstance* FontInstance10;
    };

    FontRegistry*          AFreetype2FontInstance::FRegistry(nullptr);
    FontInstanceId         AFreetype2FontInstance::FontInstanceId4(0u);
    FontInstanceId         AFreetype2FontInstance::FontInstanceId10(0u);
    Freetype2FontInstance* AFreetype2FontInstance::FontInstance4(nullptr);
    Freetype2FontInstance* AFreetype2FontInstance::FontInstance10(nullptr);

    TEST_F(AFreetype2FontInstance, ComputesHeightFromFontData)
    {
        EXPECT_EQ(5 , FontInstance4 ->getHeight());
        EXPECT_EQ(12, FontInstance10->getHeight());
    }

    TEST_F(AFreetype2FontInstance, ComputesAscenderFromFontData)
    {
        EXPECT_EQ(4, FontInstance4->getAscender());
        EXPECT_EQ(9, FontInstance10->getAscender());
    }

    TEST_F(AFreetype2FontInstance, ComputesDescenderFromFontData)
    {
        EXPECT_EQ(-1, FontInstance4->getDescender());
        EXPECT_EQ(-2, FontInstance10->getDescender());
    }

    TEST_F(AFreetype2FontInstance, ObtainsGlyphMetrics)
    {
        const GlyphMetricsVector positionedGlyphs = getPositionedGlyphs(U"%", *FontInstance4);

        ASSERT_EQ(1u, positionedGlyphs.size());
        const GlyphKey key{ GlyphId(9u), FontInstanceId4 };
        expectGlyphMetricsEq({ key, 3u, 4u, 0, -1, 3 }, positionedGlyphs.front());
    }

    TEST_F(AFreetype2FontInstance, ObtainsMultipleGlyphMetrics)
    {
        const std::u32string str = U" abc 123 ._! ";
        const GlyphMetricsVector positionedGlyphs = getPositionedGlyphs(str, *FontInstance4);

        ASSERT_EQ(13u, positionedGlyphs.size());
        auto it = positionedGlyphs.cbegin();

        expectGlyphMetricsEq({ { GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }, *it++); //' '
        expectGlyphMetricsEq({ { GlyphId( 69u), FontInstanceId4 }, 3u, 4u, 0, -1, 2 }, *it++); //a
        expectGlyphMetricsEq({ { GlyphId( 70u), FontInstanceId4 }, 3u, 4u, 0, -1, 2 }, *it++); //b
        expectGlyphMetricsEq({ { GlyphId( 71u), FontInstanceId4 }, 2u, 4u, 0, -1, 2 }, *it++); //c
        expectGlyphMetricsEq({ { GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }, *it++); //' '
        expectGlyphMetricsEq({ { GlyphId( 21u), FontInstanceId4 }, 2u, 3u, 0,  0, 2 }, *it++); //1
        expectGlyphMetricsEq({ { GlyphId( 22u), FontInstanceId4 }, 3u, 3u, 0,  0, 2 }, *it++); //2
        expectGlyphMetricsEq({ { GlyphId( 23u), FontInstanceId4 }, 3u, 4u, 0, -1, 2 }, *it++); //3
        expectGlyphMetricsEq({ { GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }, *it++); //' '
        expectGlyphMetricsEq({ { GlyphId( 18u), FontInstanceId4 }, 1u, 2u, 0, -1, 1 }, *it++); //.
        expectGlyphMetricsEq({ { GlyphId( 67u), FontInstanceId4 }, 2u, 1u, 0, -1, 2 }, *it++); //_
        expectGlyphMetricsEq({ { GlyphId(  5u), FontInstanceId4 }, 1u, 4u, 0, -1, 1 }, *it++); //!
        expectGlyphMetricsEq({ { GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }, *it++); //' '
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AFreetype2FontInstance, ObtainsMultipleGlyphMetricsFromInstancesWithDifferentSizes)
    {
        const std::u32string str1 = U" 123 ._! ";
        const std::u32string str2 = U" abc ";
        const GlyphMetricsVector positionedGlyphs1 = getPositionedGlyphs(str1, *FontInstance4);
        const GlyphMetricsVector positionedGlyphs2 = getPositionedGlyphs(str2, *FontInstance10);

        ASSERT_EQ(9u, positionedGlyphs1.size());
        ASSERT_EQ(5u, positionedGlyphs2.size());

        auto it1 = positionedGlyphs1.cbegin();
        expectGlyphMetricsEq({ { GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }, *it1++); //' '
        expectGlyphMetricsEq({ { GlyphId( 21u), FontInstanceId4 }, 2u, 3u, 0,  0, 2 }, *it1++); //1
        expectGlyphMetricsEq({ { GlyphId( 22u), FontInstanceId4 }, 3u, 3u, 0,  0, 2 }, *it1++); //2
        expectGlyphMetricsEq({ { GlyphId( 23u), FontInstanceId4 }, 3u, 4u, 0, -1, 2 }, *it1++); //3
        expectGlyphMetricsEq({ { GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }, *it1++); //' '
        expectGlyphMetricsEq({ { GlyphId( 18u), FontInstanceId4 }, 1u, 2u, 0, -1, 1 }, *it1++); //.
        expectGlyphMetricsEq({ { GlyphId( 67u), FontInstanceId4 }, 2u, 1u, 0, -1, 2 }, *it1++); //_
        expectGlyphMetricsEq({ { GlyphId(  5u), FontInstanceId4 }, 1u, 4u, 0, -1, 1 }, *it1++); //!
        expectGlyphMetricsEq({ { GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }, *it1++); //' '
        EXPECT_EQ(it1, positionedGlyphs1.cend());

        auto it2 = positionedGlyphs2.cbegin();
        expectGlyphMetricsEq({ { GlyphId(  4u), FontInstanceId10 }, 0u, 0u, 0,  0, 2 }, *it2++); //' '
        expectGlyphMetricsEq({ { GlyphId( 69u), FontInstanceId10 }, 6u, 5u, 0,  0, 5 }, *it2++); //a
        expectGlyphMetricsEq({ { GlyphId( 70u), FontInstanceId10 }, 6u, 8u, 0,  0, 6 }, *it2++); //b
        expectGlyphMetricsEq({ { GlyphId( 71u), FontInstanceId10 }, 5u, 5u, 0,  0, 5 }, *it2++); //c
        expectGlyphMetricsEq({ { GlyphId(  4u), FontInstanceId10 }, 0u, 0u, 0,  0, 2 }, *it2++); //' '
        EXPECT_EQ(it2, positionedGlyphs2.cend());
    }

    TEST_F(AFreetype2FontInstance, CanLoadACharacterGlyphBitmapData)
    {
        QuadSize glyphBitmapSize;
        const GlyphData bitmapData = FontInstance4->loadGlyphBitmapData(FontInstance4->getGlyphId(U'@'), glyphBitmapSize.x, glyphBitmapSize.y);
        EXPECT_EQ(4u, glyphBitmapSize.x);
        EXPECT_EQ(4u, glyphBitmapSize.y);

        const uint8_t expect[] =
        {
            0x1e, 0x57, 0x54, 0x7,
            0x66, 0x6f, 0x65, 0x51,
            0x5e, 0x8b, 0x96, 0x37,
            0x41, 0x58, 0x28, 0x0
        };
        ASSERT_EQ(16u, bitmapData.size());
        for (size_t i = 0; i < bitmapData.size(); ++i)
            EXPECT_EQ(expect[i], bitmapData[i]);
    }

    TEST_F(AFreetype2FontInstance, ReportsSupportedCharCodes)
    {
        const std::u32string str1 = U" 123 ._! ";
        for (auto c : str1)
            EXPECT_TRUE(FontInstance4->supportsCharacter(c));

        const std::u32string str2 = U" abc ";
        for (auto c : str2)
            EXPECT_TRUE(FontInstance10->supportsCharacter(c));
    }

    TEST_F(AFreetype2FontInstance, ReportsSupportedCharCodesAfterAlreadyLoadedTheirMetrics)
    {
        const std::u32string str1 = U" 123 ._! ";
        const std::u32string str2 = U" abc ";
        const GlyphMetricsVector positionedGlyphs1 = getPositionedGlyphs(str1, *FontInstance4);
        const GlyphMetricsVector positionedGlyphs2 = getPositionedGlyphs(str2, *FontInstance10);
        EXPECT_FALSE(positionedGlyphs1.empty());
        EXPECT_FALSE(positionedGlyphs2.empty());

        for (auto c : str1)
            EXPECT_TRUE(FontInstance4->supportsCharacter(c));

        for (auto c : str2)
            EXPECT_TRUE(FontInstance10->supportsCharacter(c));
    }

    TEST_F(AFreetype2FontInstance, ReportsUnsupportedCharCode)
    {
        EXPECT_FALSE(FontInstance10->supportsCharacter(0x19aa));
    }
}
