//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ramses-text-api/FontRegistry.h"
#include "ramses-text/HarfbuzzFontInstance.h"
#include "ramses-text/Quad.h"

namespace ramses
{
    class AHarfbuzzFontInstance : public testing::Test
    {
    public:
        static void SetUpTestCase()
        {
            FRegistry = new FontRegistry;
            const auto fontId = FRegistry->createFreetype2Font("res/ramses-text-Roboto-Bold.ttf");
            const auto fontArabicId = FRegistry->createFreetype2Font("res/ramses-text-DroidKufi-Regular.ttf");

            FontInstanceId4 = FRegistry->createFreetype2FontInstanceWithHarfBuzz(fontId, 4);
            FontInstanceId10 = FRegistry->createFreetype2FontInstanceWithHarfBuzz(fontId, 10);
            FontInstanceArabicId = FRegistry->createFreetype2FontInstanceWithHarfBuzz(fontArabicId, 10);

            FontInstance4 = static_cast<Freetype2FontInstance*>(FRegistry->getFontInstance(FontInstanceId4));
            FontInstance10 = static_cast<Freetype2FontInstance*>(FRegistry->getFontInstance(FontInstanceId10));
            FontInstanceArabic = FRegistry->getFontInstance(FontInstanceArabicId);
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
        static FontInstanceId FontInstanceArabicId;
        // intentionally use base pointer
        static Freetype2FontInstance* FontInstance4;
        static Freetype2FontInstance* FontInstance10;
        static IFontInstance*         FontInstanceArabic;
    };

    FontRegistry*          AHarfbuzzFontInstance::FRegistry(nullptr);
    FontInstanceId         AHarfbuzzFontInstance::FontInstanceId4(0u);
    FontInstanceId         AHarfbuzzFontInstance::FontInstanceId10(0u);
    FontInstanceId         AHarfbuzzFontInstance::FontInstanceArabicId(0u);
    Freetype2FontInstance* AHarfbuzzFontInstance::FontInstance4(nullptr);
    Freetype2FontInstance* AHarfbuzzFontInstance::FontInstance10(nullptr);
    IFontInstance*         AHarfbuzzFontInstance::FontInstanceArabic(nullptr);

    //////
    /// These tests test base class functionality with no HB reshaping involved, essentially equivalent to freetype2 font instance
    //////
    TEST_F(AHarfbuzzFontInstance, ComputesHeightFromFontData)
    {
        EXPECT_EQ(5 , FontInstance4 ->getHeight());
        EXPECT_EQ(12, FontInstance10->getHeight());
    }

    TEST_F(AHarfbuzzFontInstance, ComputesAscenderFromFontData)
    {
        EXPECT_EQ(4, FontInstance4->getAscender());
        EXPECT_EQ(10, FontInstance10->getAscender());
    }

    TEST_F(AHarfbuzzFontInstance, ComputesDescenderFromFontData)
    {
        EXPECT_EQ(-1, FontInstance4->getDescender());
        EXPECT_EQ(-3, FontInstance10->getDescender());
    }

    TEST_F(AHarfbuzzFontInstance, ObtainsGlyphMetrics)
    {
        const GlyphMetricsVector positionedGlyphs = getPositionedGlyphs(U"%", *FontInstance4);

        ASSERT_EQ(1u, positionedGlyphs.size());
        const GlyphKey key{ GlyphId(9u), FontInstanceId4 };
        expectGlyphMetricsEq({ key, 3u, 4u, 0, -1, 3 }, positionedGlyphs.front());
    }

    TEST_F(AHarfbuzzFontInstance, ObtainsMultipleGlyphMetrics)
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

    TEST_F(AHarfbuzzFontInstance, ObtainsMultipleGlyphMetricsFromInstancesWithDifferentSizes)
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

    TEST_F(AHarfbuzzFontInstance, CanLoadACharacterGlyphBitmapData)
    {
        QuadSize glyphBitmapSize;
        const GlyphData bitmapData = FontInstance4->loadGlyphBitmapData(FontInstance4->getGlyphId(U'@'), glyphBitmapSize.x, glyphBitmapSize.y);
        EXPECT_EQ(4u, glyphBitmapSize.x);
        EXPECT_EQ(4u, glyphBitmapSize.y);

// (Violin) glyph loading produces minimally different results on Mingw
#if defined(__MINGW32__) || defined(__MINGW64__)
        EXPECT_THAT(bitmapData, testing::ContainerEq(std::vector<uint8_t>{ 0x1e, 0x57, 0x54, 0x7,
                                                                           0x66, 0x6f, 0x65, 0x51,
                                                                           0x5e, 0x8b, 0x96, 0x37,
                                                                           0x41, 0x58, 0x28, 0x0 }));
#else
        EXPECT_THAT(bitmapData, testing::ContainerEq(std::vector<uint8_t>{ 0x1e, 0x57, 0x54, 0x7,
                                                                           0x66, 0x6f, 0x65, 0x51,
                                                                           0x5e, 0x8b, 0x96, 0x36,
                                                                           0x41, 0x58, 0x28, 0x0 }));
#endif
    }

    TEST_F(AHarfbuzzFontInstance, ReportsUnsupportedCharCode)
    {
        EXPECT_FALSE(FontInstance10->supportsCharacter(0x19aa));
    }
    //////
    //////

    TEST_F(AHarfbuzzFontInstance, ObtainsMultipleGlyphMetricsWithReshaping)
    {
        const std::u32string str = {32, 1577, 1581, 1608, 1604, 32, }; //U" ةحول ";

        const GlyphMetricsVector positionedGlyphs = getPositionedGlyphs(str, *FontInstanceArabic);

        ASSERT_EQ(6u, positionedGlyphs.size());
        auto it = positionedGlyphs.cbegin();

        expectGlyphMetricsEq({ { GlyphId(  3u), FontInstanceArabicId }, 0u, 0u,  0,  0, 3 }, *it++);
        expectGlyphMetricsEq({ { GlyphId(154u), FontInstanceArabicId }, 7u, 8u,  0,  0, 6 }, *it++);
        expectGlyphMetricsEq({ { GlyphId(  8u), FontInstanceArabicId }, 7u, 5u, -1,  0, 6 }, *it++);
        expectGlyphMetricsEq({ { GlyphId( 47u), FontInstanceArabicId }, 6u, 7u,  0, -2, 6 }, *it++);
        expectGlyphMetricsEq({ { GlyphId( 35u), FontInstanceArabicId }, 4u, 8u, -1,  0, 3 }, *it++);
        expectGlyphMetricsEq({ { GlyphId(  3u), FontInstanceArabicId }, 0u, 0u,  0,  0, 3 }, *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, ObtainsMultipleGlyphMetricsWithReshaping2)
    {
        const std::u32string str = {32, 1575, 1604, 1593, 1585, 1576, 1610, 1577, 32, }; //U" العربية ";
        const GlyphMetricsVector positionedGlyphs = getPositionedGlyphs(str, *FontInstanceArabic);

        ASSERT_EQ(8u, positionedGlyphs.size());
        auto it = positionedGlyphs.cbegin();

        expectGlyphMetricsEq({ { GlyphId(  3u), FontInstanceArabicId }, 0u, 0u,  0,  0, 3 }, *it++);
        expectGlyphMetricsEq({ { GlyphId( 51u), FontInstanceArabicId }, 8u, 8u,  0,  0, 7 }, *it++);
        expectGlyphMetricsEq({ { GlyphId( 27u), FontInstanceArabicId }, 6u, 5u, -1,  0, 5 }, *it++);
        expectGlyphMetricsEq({ { GlyphId( 13u), FontInstanceArabicId }, 5u, 7u, -1, -2, 4 }, *it++);
        expectGlyphMetricsEq({ { GlyphId(135u), FontInstanceArabicId }, 5u, 7u, -1, -2, 3 }, *it++);
        expectGlyphMetricsEq({ { GlyphId(216u), FontInstanceArabicId }, 4u, 7u, -1, -2, 4 }, *it++);
        expectGlyphMetricsEq({ { GlyphId(153u), FontInstanceArabicId }, 6u, 7u,  0,  0, 6 }, *it++);
        expectGlyphMetricsEq({ { GlyphId(  3u), FontInstanceArabicId }, 0u, 0u,  0,  0, 3 }, *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }
}
