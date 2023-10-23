//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/text/HarfbuzzFontInstance.h"
#include "ramses/client/text/FontRegistry.h"
#include "impl/text/Quad.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "fmt/format.h"

namespace ramses
{
    inline bool operator==(const GlyphMetrics& a, const GlyphMetrics& b)
    {
        return (a.key == b.key)
            && (a.posX == b.posX)
            && (a.posY == b.posY)
            && (a.width == b.width)
            && (a.height == b.height)
            && (a.advance == b.advance);
    }

    inline void PrintTo(const GlyphMetrics& param, ::std::ostream* os)
    {
        *os << fmt::format("GlyphMetrics key:({},font:{}) x:{} y:{} w:{} h:{} adv:{}",
                           param.key.identifier.getValue(),
                           param.key.fontInstanceId.getValue(),
                           param.posX, param.posY, param.width, param.height, param.advance);
    }
}

namespace ramses::internal
{
    class AHarfbuzzFontInstance : public testing::Test
    {
    public:
        static void SetUpTestSuite()
        {
            FRegistry = new FontRegistry;
            const auto fontId = FRegistry->createFreetype2Font("res/ramses-text-Roboto-Bold.ttf");
            const auto fontArabicId = FRegistry->createFreetype2Font("res/ramses-text-DroidKufi-Regular.ttf");
            const auto fontJapaneseId = FRegistry->createFreetype2Font("res/ramses-text-WenQuanYi-Micro-Hei.ttf");
            const auto fontThaiId     = FRegistry->createFreetype2Font("res/ramses-text-NotoSansThai-Regular.ttf");

            FontInstanceId4 = FRegistry->createFreetype2FontInstanceWithHarfBuzz(fontId, 4);
            FontInstanceId10 = FRegistry->createFreetype2FontInstanceWithHarfBuzz(fontId, 10);
            FontInstanceArabicId = FRegistry->createFreetype2FontInstanceWithHarfBuzz(fontArabicId, 10);
            FontInstanceJapaneseId = FRegistry->createFreetype2FontInstanceWithHarfBuzz(fontJapaneseId, 10);
            FontInstanceThaiId     = FRegistry->createFreetype2FontInstanceWithHarfBuzz(fontThaiId, 10);

            FontInstance4 = static_cast<Freetype2FontInstance*>(FRegistry->getFontInstance(FontInstanceId4));
            FontInstance10 = static_cast<Freetype2FontInstance*>(FRegistry->getFontInstance(FontInstanceId10));
            FontInstanceArabic = FRegistry->getFontInstance(FontInstanceArabicId);
            FontInstanceJapanese = FRegistry->getFontInstance(FontInstanceJapaneseId);
            FontInstanceThai     = FRegistry->getFontInstance(FontInstanceThaiId);
        }

        static void TearDownTestSuite()
        {
            delete FRegistry;
        }

    protected:
        static GlyphMetricsVector GetPositionedGlyphs(const std::u32string& str, IFontInstance& fontInstance)
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
        static FontInstanceId FontInstanceJapaneseId;
        static FontInstanceId FontInstanceThaiId;

        // intentionally use base pointer
        static Freetype2FontInstance* FontInstance4;
        static Freetype2FontInstance* FontInstance10;
        static IFontInstance*         FontInstanceArabic;
        static IFontInstance*         FontInstanceJapanese;
        static IFontInstance*         FontInstanceThai;
    };

    FontRegistry*          AHarfbuzzFontInstance::FRegistry(nullptr);
    FontInstanceId         AHarfbuzzFontInstance::FontInstanceId4(0u);
    FontInstanceId         AHarfbuzzFontInstance::FontInstanceId10(0u);
    FontInstanceId         AHarfbuzzFontInstance::FontInstanceArabicId(0u);
    FontInstanceId         AHarfbuzzFontInstance::FontInstanceJapaneseId(0u);
    FontInstanceId         AHarfbuzzFontInstance::FontInstanceThaiId(0u);

    Freetype2FontInstance* AHarfbuzzFontInstance::FontInstance4(nullptr);
    Freetype2FontInstance* AHarfbuzzFontInstance::FontInstance10(nullptr);
    IFontInstance*         AHarfbuzzFontInstance::FontInstanceArabic(nullptr);
    IFontInstance*         AHarfbuzzFontInstance::FontInstanceJapanese(nullptr);
    IFontInstance*         AHarfbuzzFontInstance::FontInstanceThai(nullptr);

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
        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(U"%", *FontInstance4);

        ASSERT_EQ(1u, positionedGlyphs.size());
        const GlyphKey key{ GlyphId(9u), FontInstanceId4 };
        EXPECT_EQ(GlyphMetrics({ key, 3u, 4u, 0, -1, 3 }), positionedGlyphs.front());
    }

    TEST_F(AHarfbuzzFontInstance, ObtainsMultipleGlyphMetrics)
    {
        const std::u32string str = U" abc 123 ._! ";
        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance4);

        ASSERT_EQ(13u, positionedGlyphs.size());
        auto it = positionedGlyphs.cbegin();

        EXPECT_EQ(GlyphMetrics({{ GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }), *it++); //' '
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 69u), FontInstanceId4 }, 3u, 4u, 0, -1, 2 }), *it++); //a
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 70u), FontInstanceId4 }, 3u, 4u, 0, -1, 2 }), *it++); //b
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 71u), FontInstanceId4 }, 2u, 4u, 0, -1, 2 }), *it++); //c
        EXPECT_EQ(GlyphMetrics({{ GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }), *it++); //' '
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 21u), FontInstanceId4 }, 2u, 3u, 0,  0, 2 }), *it++); //1
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 22u), FontInstanceId4 }, 3u, 3u, 0,  0, 2 }), *it++); //2
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 23u), FontInstanceId4 }, 3u, 4u, 0, -1, 2 }), *it++); //3
        EXPECT_EQ(GlyphMetrics({{ GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }), *it++); //' '
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 18u), FontInstanceId4 }, 1u, 2u, 0, -1, 1 }), *it++); //.
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 67u), FontInstanceId4 }, 2u, 1u, 0, -1, 2 }), *it++); //_
        EXPECT_EQ(GlyphMetrics({{ GlyphId(  5u), FontInstanceId4 }, 1u, 4u, 0, -1, 1 }), *it++); //!
        EXPECT_EQ(GlyphMetrics({{ GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }), *it++); //' '
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, ObtainsMultipleGlyphMetricsFromInstancesWithDifferentSizes)
    {
        const std::u32string str1 = U" 123 ._! ";
        const std::u32string str2 = U" abc ";
        const GlyphMetricsVector positionedGlyphs1 = GetPositionedGlyphs(str1, *FontInstance4);
        const GlyphMetricsVector positionedGlyphs2 = GetPositionedGlyphs(str2, *FontInstance10);

        ASSERT_EQ(9u, positionedGlyphs1.size());
        ASSERT_EQ(5u, positionedGlyphs2.size());

        auto it1 = positionedGlyphs1.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }), *it1++); //' '
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 21u), FontInstanceId4 }, 2u, 3u, 0,  0, 2 }), *it1++); //1
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 22u), FontInstanceId4 }, 3u, 3u, 0,  0, 2 }), *it1++); //2
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 23u), FontInstanceId4 }, 3u, 4u, 0, -1, 2 }), *it1++); //3
        EXPECT_EQ(GlyphMetrics({{ GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }), *it1++); //' '
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 18u), FontInstanceId4 }, 1u, 2u, 0, -1, 1 }), *it1++); //.
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 67u), FontInstanceId4 }, 2u, 1u, 0, -1, 2 }), *it1++); //_
        EXPECT_EQ(GlyphMetrics({{ GlyphId(  5u), FontInstanceId4 }, 1u, 4u, 0, -1, 1 }), *it1++); //!
        EXPECT_EQ(GlyphMetrics({{ GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }), *it1++); //' '
        EXPECT_EQ(it1, positionedGlyphs1.cend());

        auto it2 = positionedGlyphs2.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(  4u), FontInstanceId10 }, 0u, 0u, 0,  0, 2 }), *it2++); //' '
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 69u), FontInstanceId10 }, 6u, 5u, 0,  0, 5 }), *it2++); //a
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 70u), FontInstanceId10 }, 6u, 8u, 0,  0, 6 }), *it2++); //b
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 71u), FontInstanceId10 }, 5u, 5u, 0,  0, 5 }), *it2++); //c
        EXPECT_EQ(GlyphMetrics({{ GlyphId(  4u), FontInstanceId10 }, 0u, 0u, 0,  0, 2 }), *it2++); //' '
        EXPECT_EQ(it2, positionedGlyphs2.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanLoadACharacterGlyphBitmapData)
    {
        QuadSize glyphBitmapSize;
        const GlyphData bitmapData = FontInstance4->loadGlyphBitmapData(FontInstance4->getGlyphId(U'@'), glyphBitmapSize.x, glyphBitmapSize.y);
        EXPECT_EQ(4u, glyphBitmapSize.x);
        EXPECT_EQ(4u, glyphBitmapSize.y);

        EXPECT_EQ(bitmapData, std::vector<uint8_t>({ 0x1e, 0x57, 0x54, 0x7,
                                                     0x66, 0x6f, 0x65, 0x51,
                                                     0x5e, 0x8b, 0x96, 0x36,
                                                     0x41, 0x58, 0x28, 0x0 }));
    }

    TEST_F(AHarfbuzzFontInstance, ReportsUnsupportedCharCode)
    {
        EXPECT_FALSE(FontInstance10->supportsCharacter(0x19aa));
    }
    //////
    //////

    TEST_F(AHarfbuzzFontInstance, ObtainsMultipleGlyphMetricsWithReshaping)
    {
        //logical order
        const std::u32string str = {32, 1577, 1581, 1608, 1604, 32, }; //U" ةحول ";
        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstanceArabic);

        ASSERT_EQ(6u, positionedGlyphs.size());
        auto it = positionedGlyphs.cbegin();

        EXPECT_EQ(GlyphMetrics({{ GlyphId(  3u), FontInstanceArabicId }, 0u, 0u,  0,  0, 3 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(154u), FontInstanceArabicId }, 7u, 8u,  0,  0, 6 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(  8u), FontInstanceArabicId }, 7u, 5u, -1,  0, 6 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 47u), FontInstanceArabicId }, 6u, 7u,  0, -2, 6 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 35u), FontInstanceArabicId }, 4u, 8u, -1,  0, 3 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(  3u), FontInstanceArabicId }, 0u, 0u,  0,  0, 3 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, ObtainsMultipleGlyphMetricsWithReshapingForVisualOrder)
    {
        //visual order
        const std::u32string str = {0x0020, 0xfedd, 0xfeee, 0xfea3, 0xfe93, 0x0020, }; //U" ةحول ";
        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstanceArabic);

        ASSERT_EQ(6u, positionedGlyphs.size());
        auto it = positionedGlyphs.cbegin();

        EXPECT_EQ(GlyphMetrics({{ GlyphId(  3u), FontInstanceArabicId }, 0u, 0u,   0,  0, 3 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 34u), FontInstanceArabicId }, 6u, 10u,  0, -2, 7 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 47u), FontInstanceArabicId }, 6u, 7u,   0, -2, 6 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(  8u), FontInstanceArabicId }, 7u, 5u,  -1,  0, 6 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(153u), FontInstanceArabicId }, 6u, 7u,   0,  0, 6 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(  3u), FontInstanceArabicId }, 0u, 0u,   0,  0, 3 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, ObtainsMultipleGlyphMetricsWithReshaping2)
    {
        // logical order
        const std::u32string str = {32, 1575, 1604, 1593, 1585, 1576, 1610, 1577, 32, }; //U" العربية ";
        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstanceArabic);

        ASSERT_EQ(8u, positionedGlyphs.size());
        auto it = positionedGlyphs.cbegin();

        EXPECT_EQ(GlyphMetrics({{ GlyphId(  3u), FontInstanceArabicId }, 0u, 0u,  0,  0, 3 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 51u), FontInstanceArabicId }, 8u, 8u,  0,  0, 7 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 27u), FontInstanceArabicId }, 6u, 5u, -1,  0, 5 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 13u), FontInstanceArabicId }, 5u, 7u, -1, -2, 4 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(135u), FontInstanceArabicId }, 5u, 7u, -1, -2, 3 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(216u), FontInstanceArabicId }, 4u, 7u, -1, -2, 4 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(153u), FontInstanceArabicId }, 6u, 7u,  0,  0, 6 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(  3u), FontInstanceArabicId }, 0u, 0u,  0,  0, 3 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, ObtainsMultipleGlyphMetricsWithReshapingForVisualOrder2)
    {
        // visual order
        const std::u32string str = {0x0020, 0xfe94, 0xfef4, 0xfe91, 0xfeae, 0xfecc, 0xfedf, 0xfe8d, 0x0020, }; //U" العربية ";
        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstanceArabic);

        ASSERT_EQ(9u, positionedGlyphs.size());
        auto it = positionedGlyphs.cbegin();

        EXPECT_EQ(GlyphMetrics({{ GlyphId(  3u), FontInstanceArabicId }, 0u, 0u,  0,  0, 3 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(154u), FontInstanceArabicId }, 7u, 8u,  0,  0, 6 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(217u), FontInstanceArabicId }, 6u, 7u, -1, -2, 4 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(134u), FontInstanceArabicId }, 4u, 7u, -1, -2, 3 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 13u), FontInstanceArabicId }, 5u, 7u, -1, -2, 4 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 28u), FontInstanceArabicId }, 8u, 5u, -1,  0, 6 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 35u), FontInstanceArabicId }, 4u, 8u, -1,  0, 3 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(  5u), FontInstanceArabicId }, 1u, 8u,  1,  0, 3 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(  3u), FontInstanceArabicId }, 0u, 0u,  0,  0, 3 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

#if !defined(USE_HARFBUZZ_LEGACY_SHAPING)
    TEST_F(AHarfbuzzFontInstance, CanShapeJapaneseCombiningCharacters)
    {
        const std::u32string str = {0x30D5, 0x3099, 0x30C6, 0x3099, }; //decompositions of U+30D6 and U+30C7

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstanceJapanese);
        ASSERT_EQ(2u, positionedGlyphs.size());

        auto it = positionedGlyphs.begin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(1615u), FontInstanceJapaneseId }, 9u,  10u, 1, -1, 10 }), *it++); //U+30D6 = U+30D5, U+3099
        EXPECT_EQ(GlyphMetrics({{ GlyphId(1600u), FontInstanceJapaneseId }, 10u, 10u, 0, -1, 10 }), *it++); //U+30C7 = U+30C6, U+3099
        EXPECT_EQ(it, positionedGlyphs.end());
    }

    TEST_F(AHarfbuzzFontInstance, CanShapeFrenchCombinedCharacters)
    {
        const std::u32string str = { 0x0061, 0x0301, }; //decomposition of U+00E1

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(1u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(667u), FontInstanceId10 }, 6u, 8u, 0, 0, 5 }), *it++); //U+00E1
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanShapeThaiCombinedCharacters)
    {
        const std::u32string str = { 0x0E1B, 0x0E31, 0x0E4A, 0x0E21, 0x0E19, 0x0E49, 0x0E33, 0x0E21, 0x0E31, }; //U"ปั๊มน้ำมั"

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstanceThai);
        ASSERT_EQ(10u, positionedGlyphs.size()); //creates 10 glyphs: U+0E33 is decomposed to 2 glyphs


        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(80u), FontInstanceThaiId }, 5u, 8u,  1, 0, 6 }), *it++); //U+0E1B
#ifdef FT_CONFIG_OPTION_USE_HARFBUZZ
        EXPECT_EQ(GlyphMetrics({{ GlyphId(46u), FontInstanceThaiId }, 3u, 2u, -5, 7, 0 }), *it++); //U+0E31.narrow, shaped for U+0E1B
        EXPECT_EQ(GlyphMetrics({{ GlyphId(52u), FontInstanceThaiId }, 4u, 2u, -5, 9, 0 }), *it++); //U+0E4A.small, shaped for U+0E1B
#else
        EXPECT_EQ(GlyphMetrics({{ GlyphId(46u), FontInstanceThaiId }, 3u, 2u, -5, 6, 0 }), *it++); //U+0E31.narrow, shaped for U+0E1B
        EXPECT_EQ(GlyphMetrics({{ GlyphId(52u), FontInstanceThaiId }, 4u, 3u, -5, 8, 0 }), *it++); //U+0E4A.small, shaped for U+0E1B
#endif
        EXPECT_EQ(GlyphMetrics({{ GlyphId(56u), FontInstanceThaiId }, 5u, 6u,  1, 0, 6 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(71u), FontInstanceThaiId }, 5u, 6u,  1, 0, 6 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(59u), FontInstanceThaiId }, 2u, 2u, -3, 6, 0 }), *it++); //U+0E4D, decomposition of U+0E33
#ifdef FT_CONFIG_OPTION_USE_HARFBUZZ
        EXPECT_EQ(GlyphMetrics({{ GlyphId(49u), FontInstanceThaiId }, 3u, 2u, -3, 9, 0 }), *it++); //U+0E49.small, shaped for U+0E4D
#else
        EXPECT_EQ(GlyphMetrics({{ GlyphId(49u), FontInstanceThaiId }, 3u, 2u, -3, 8, 0 }), *it++); //U+0E49.small, shaped for U+0E4D
#endif
        EXPECT_EQ(GlyphMetrics({{ GlyphId(86u), FontInstanceThaiId }, 4u, 6u, -1, 0, 4 }), *it++); //U+0E32, decomposition of U+0E33
        EXPECT_EQ(GlyphMetrics({{ GlyphId(56u), FontInstanceThaiId }, 5u, 6u,  1, 0, 6 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(45u), FontInstanceThaiId }, 5u, 2u, -4, 7, 0 }), *it++);

        EXPECT_EQ(it, positionedGlyphs.end());
    }

    TEST_F(AHarfbuzzFontInstance, CanShapeVietnameseCombinedCharacters)
    {
        const std::u32string str = { 0x00F4, 0x0309, }; //U"ổ"

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(1u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(1107u), FontInstanceId10 }, 6u, 9u, 0, 0, 6 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.end());
    }

    TEST_F(AHarfbuzzFontInstance, ProducesEmptyListOfGlyphsWhenGivenEmptyString)
    {
        const std::u32string str = {}; //U""; empty string

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(0u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, ProducesEmptyListOfGlyphsWhenGivenControlCharOnly)
    {
        const std::u32string str = {0x000A, }; //U"\n"; newline character

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(0u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, ProducesEmptyListOfGlyphsWhenGivenUnavailablePuaChars)
    {
        const std::u32string str = {0xE000, 0xF8FF, }; //U""; private use area (PUA) block boundaries: U+E000..U+F8FF

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(0u, positionedGlyphs.size()); //not available in this font

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, ProducesEmptyListOfGlyphsWhenGivenUnavailableCjkChar)
    {
        const std::u32string str = {0xF900, }; //U"豈"; CJK Compatibility Ideographs are located just after PUA

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(0u, positionedGlyphs.size()); //not available in this font

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddSingleInheritedCharToBuffer)
    {
        const std::u32string str = {0x0301, }; //U"́"; combining acute accent

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(1u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(169u), FontInstanceId10 }, 4u, 2u, -4, 6, 0 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddMultipleInheritedCharsToBuffer)
    {
        const std::u32string str = {0x0301, 0x0301, 0x0301, }; // U"́́́"; combining acute accent (3x)

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(3u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(169u), FontInstanceId10 }, 4u, 2u, -4, 6, 0 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(169u), FontInstanceId10 }, 4u, 2u, -4, 6, 0 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(169u), FontInstanceId10 }, 4u, 2u, -4, 6, 0 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddInheritedCharAtBeginningToBuffer)
    {
        const std::u32string str = {0x0301, 0x0041, }; //U"́A"; combining acute accent + A

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(2u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(169u), FontInstanceId10 }, 4u, 2u, -4, 6, 0 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 37u), FontInstanceId10 }, 7u, 7u,  0, 0, 7 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddInheritedCharAtEndToBuffer)
    {
        const std::u32string str = {0x0041, 0x0301, }; //U"Á"; A + combining acute accent

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(1u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(640u), FontInstanceId10 }, 7u, 10u, 0, 0, 7 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddInheritedCharAtBeginningToBufferWithLatinMainScript)
    {
        const std::u32string str = {0x0301, 0x0041, 0x0042, 0x0043, 0x0044, }; //U"́ABCD"; combining acute accent + ABCD

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(5u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(169u), FontInstanceId10 }, 4u, 2u, -4, 6, 0 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 37u), FontInstanceId10 }, 7u, 7u,  0, 0, 7 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 38u), FontInstanceId10 }, 6u, 7u,  0, 0, 6 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 39u), FontInstanceId10 }, 7u, 7u,  0, 0, 7 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 40u), FontInstanceId10 }, 7u, 7u,  0, 0, 7 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddInheritedCharAtBeginningToBufferWithArabicMainScript)
    {
        // visual order
        const std::u32string str = {0x064b, 0xFEDD, 0xFEEE, 0xFEA3, 0xFE93, }; //U"ًةحول"; Arabic Fathatan + reordered Arabic text

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstanceArabic);
        ASSERT_EQ(5u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 98u), FontInstanceArabicId }, 4u,  3u,  0,  8, 0 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 34u), FontInstanceArabicId }, 6u, 10u,  0, -2, 7 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 47u), FontInstanceArabicId }, 6u,  7u,  0, -2, 6 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(  8u), FontInstanceArabicId }, 7u,  5u, -1,  0, 6 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(153u), FontInstanceArabicId }, 6u,  7u,  0,  0, 6 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddSingleCommonCharToBuffer)
    {
        const std::u32string str = {0x002F, }; //U"/";

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(1u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(19u), FontInstanceId10 }, 5u, 8u, -1, -1, 4 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddMultipleCommonCharsToBuffer)
    {
        const std::u32string str = {0x002F, 0x002F, }; // U"//";

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(2u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(19u), FontInstanceId10 }, 5u, 8u, -1, -1, 3 }), *it++); //contextual positioning
        EXPECT_EQ(GlyphMetrics({{ GlyphId(19u), FontInstanceId10 }, 5u, 8u, -1, -1, 4 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddCommonCharAtBeginningToBuffer)
    {
        const std::u32string str = {0x002F, 0x0041, }; //U"/A";

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(2u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(19u), FontInstanceId10 }, 5u, 8u, -1, -1, 4 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(37u), FontInstanceId10 }, 7u, 7u,  0,  0, 7 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddCommonCharAtEndToBuffer)
    {
        const std::u32string str = {0x0041, 0x002F, }; //U"A/";

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(2u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(37u), FontInstanceId10 }, 7u, 7u,  0,  0, 7 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(19u), FontInstanceId10 }, 5u, 8u, -1, -1, 4 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddCommonCharAtBeginningToBufferWithLatinMainScript)
    {
        const std::u32string str = {0x002F, 0x041, 0x042, 0x043, 0x044, }; //U"/ABCD";

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(5u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(19u), FontInstanceId10 }, 5u, 8u, -1, -1, 4 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(37u), FontInstanceId10 }, 7u, 7u,  0,  0, 7 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(38u), FontInstanceId10 }, 6u, 7u,  0,  0, 6 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(39u), FontInstanceId10 }, 7u, 7u,  0,  0, 7 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(40u), FontInstanceId10 }, 7u, 7u,  0,  0, 7 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddCommonCharAtBeginningToBufferWithArabicMainScript)
    {
        // visual order
        const std::u32string str = {0x061f, 0xfedd, 0xfeee, 0xfea3, 0xfe93, }; //U"؟ةحول"; Arabic question mark + reordered Arabic text

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstanceArabic);
        ASSERT_EQ(5u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 92u), FontInstanceArabicId }, 5u,  7u,  0,  0, 5 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 34u), FontInstanceArabicId }, 6u, 10u,  0, -2, 7 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 47u), FontInstanceArabicId }, 6u,  7u,  0, -2, 6 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(  8u), FontInstanceArabicId }, 7u,  5u, -1,  0, 6 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(153u), FontInstanceArabicId }, 6u,  7u,  0,  0, 6 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddUnknownCharToBuffer)
    {
        const std::u32string str = {0xEE01, }; //U""; PUA colon

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(1u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(440u), FontInstanceId10 }, 3u, 6u, 0, 1, 3 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddUnknownCharsToBuffer)
    {
        const std::u32string str = {0xEE01, 0xF6C3, }; //U""; PUA colon + PUA comma

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(2u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(440u), FontInstanceId10 }, 3u, 6u, 0,  1, 3 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(442u), FontInstanceId10 }, 2u, 2u, 0, -1, 0 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddUnknownCharAtBeginningToBuffer)
    {
        const std::u32string str = {0xF6C3, 0x0041, }; //U"A"; PUA comma + A

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(2u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(442u), FontInstanceId10 }, 2u, 2u, 0, -1, 0 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 37u), FontInstanceId10 }, 7u, 7u, 0,  0, 7 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddUnknownCharAtEndToBuffer)
    {
        const std::u32string str = {0x0041, 0xF6C3, }; //U"A"; A + PUA comma

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(2u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 37u), FontInstanceId10 }, 7u, 7u, 0,  0, 7 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(442u), FontInstanceId10 }, 2u, 2u, 0, -1, 0 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddInheritedCommonAndUnknownCharsToBuffer)
    {
        const std::u32string str = {0x0301, 0x002F, 0xF6C3, }; //U"́/"; combining acute accent + / + PUA comma

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(3u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(169u), FontInstanceId10 }, 4u, 2u, -4,  6, 0 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 19u), FontInstanceId10 }, 5u, 8u, -1, -1, 4 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(442u), FontInstanceId10 }, 2u, 2u,  0, -1, 0 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddInheritedCommonAndUnknownCharsAtBeginningToBuffer)
    {
        const std::u32string str = {0x0301, 0x002F, 0xF6C3, 0x0041, }; //U"́/A"; combining acute accent + / + PUA comma + A

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(4u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(169u), FontInstanceId10 }, 4u, 2u, -4,  6, 0 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 19u), FontInstanceId10 }, 5u, 8u, -1, -1, 4 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(442u), FontInstanceId10 }, 2u, 2u,  0, -1, 0 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 37u), FontInstanceId10 }, 7u, 7u,  0,  0, 7 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddInheritedCommonAndUnknownCharsAtEndToBuffer)
    {
        const std::u32string str = {0x0041, 0x0301, 0x002F, 0xF6C3, }; // U"Á/"; A + combining acute accent + / + PUA comma

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(3u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(640u), FontInstanceId10 }, 7u, 10u,  0,  0, 7 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 19u), FontInstanceId10 }, 5u,  8u, -1, -1, 4 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(442u), FontInstanceId10 }, 2u,  2u,  0, -1, 0 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddCommonUnknownAndInheritedCharsToBuffer)
    {
        const std::u32string str = {0x002F, 0xF6C3, 0x0301, }; //U"/́"; / + PUA comma + combining acute accent

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(3u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 19u), FontInstanceId10 }, 5u, 8u, -1, -1, 4 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(442u), FontInstanceId10 }, 2u, 2u,  0, -1, 0 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(169u), FontInstanceId10 }, 4u, 2u, -4,  6, 0 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddCommonUnknownAndInheritedCharsAtBeginningToBuffer)
    {
        const std::u32string str = {0x002F, 0xF6C3, 0x0301, 0x0041, }; //U"/́A"; / + PUA comma + combining acute accent + A

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(4u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 19u), FontInstanceId10 }, 5u, 8u, -1, -1, 4 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(442u), FontInstanceId10 }, 2u, 2u,  0, -1, 0 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(169u), FontInstanceId10 }, 4u, 2u, -4,  6, 0 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 37u), FontInstanceId10 }, 7u, 7u,  0,  0, 7 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddCommonUnknownAndInheritedCharsAtEndToBuffer)
    {
        const std::u32string str = {0x0041, 0x002F, 0xF6C3, 0x0301, }; // U"A/́"; A/ + PUA comma + combining acute accent

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(4u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 37u), FontInstanceId10 }, 7u, 7u,  0,  0, 7 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 19u), FontInstanceId10 }, 5u, 8u, -1, -1, 4 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(442u), FontInstanceId10 }, 2u, 2u,  0, -1, 0 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(169u), FontInstanceId10 }, 4u, 2u, -4,  6, 0 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddUnknownCommonAndInheritedCharsToBuffer)
    {
        const std::u32string str = {0xF6C3, 0x002F, 0x0301, }; //U"/́"; PUA comma + / + combining acute accent

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(3u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(442u), FontInstanceId10 }, 2u, 2u,  0, -1, 0 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 19u), FontInstanceId10 }, 5u, 8u, -1, -1, 4 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(169u), FontInstanceId10 }, 4u, 2u, -4,  6, 0 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddUnknownCommonAndInheritedCharsAtBeginningToBuffer)
    {
        const std::u32string str = {0xF6C3, 0x002F, 0x0301, 0x0041, }; //U"/́A"; / + PUA comma + combining acute accent + A

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(4u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId(442u), FontInstanceId10 }, 2u, 2u,  0, -1, 0 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 19u), FontInstanceId10 }, 5u, 8u, -1, -1, 4 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(169u), FontInstanceId10 }, 4u, 2u, -4,  6, 0 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 37u), FontInstanceId10 }, 7u, 7u,  0,  0, 7 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AHarfbuzzFontInstance, CanAddUnknownCommonAndInheritedCharsAtEndToBuffer)
    {
        const std::u32string str = {0x0041, 0xF6C3, 0x002F, 0x0301, }; // U"A/́"; A + PUA comma + / + combining acute accent

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance10);
        ASSERT_EQ(4u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 37u), FontInstanceId10 }, 7u, 7u,  0,  0, 7 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(442u), FontInstanceId10 }, 2u, 2u,  0, -1, 0 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId( 19u), FontInstanceId10 }, 5u, 8u, -1, -1, 4 }), *it++);
        EXPECT_EQ(GlyphMetrics({{ GlyphId(169u), FontInstanceId10 }, 4u, 2u, -4,  6, 0 }), *it++);
        EXPECT_EQ(it, positionedGlyphs.cend());
    }
#endif
}
