//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/text/Freetype2FontInstance.h"
#include "ramses/client/text/FontRegistry.h"
#include "impl/text/Quad.h"
#include "gtest/gtest.h"

namespace ramses::internal
{
    class AFreetype2FontInstance :  public testing::Test
    {
    public:
        static void SetUpTestSuite()
        {
            FRegistry = new FontRegistry;
            const auto fontId = FRegistry->createFreetype2Font("res/ramses-text-Roboto-Bold.ttf");
            const auto fontIdJp = FRegistry->createFreetype2Font("res/ramses-text-WenQuanYi-Micro-Hei.ttf");
            const auto fontThaiId = FRegistry->createFreetype2Font("res/ramses-text-NotoSansThai-Regular.ttf");

            FontInstanceId4 = FRegistry->createFreetype2FontInstanceWithHarfBuzz(fontId, 4);
            FontInstanceId10 = FRegistry->createFreetype2FontInstanceWithHarfBuzz(fontId, 10);
            FontInstanceIdLatin = FRegistry->createFreetype2FontInstance(fontId, 10);
            FontInstanceIdJapanese = FRegistry->createFreetype2FontInstance(fontIdJp, 10);
            FontInstanceIdThai = FRegistry->createFreetype2FontInstance(fontThaiId, 10);

            FontInstance4 = static_cast<Freetype2FontInstance*>(FRegistry->getFontInstance(FontInstanceId4));
            FontInstance10 = static_cast<Freetype2FontInstance*>(FRegistry->getFontInstance(FontInstanceId10));
            FontInstanceLatin = static_cast<Freetype2FontInstance*>(FRegistry->getFontInstance(FontInstanceIdLatin));
            FontInstanceJp    = static_cast<Freetype2FontInstance*>(FRegistry->getFontInstance(FontInstanceIdJapanese));
            FontInstanceThai  = static_cast<Freetype2FontInstance*>(FRegistry->getFontInstance(FontInstanceIdThai));
        }

        static void TearDownTestSuite()
        {
            delete FRegistry;
        }

    protected:
        static void ExpectGlyphMetricsEq(const GlyphMetrics& expected, const GlyphMetrics& actual)
        {
            EXPECT_EQ(expected.key.fontInstanceId, actual.key.fontInstanceId);
            EXPECT_EQ(expected.key.identifier.getValue(), actual.key.identifier.getValue());
            EXPECT_EQ(expected.posX, actual.posX);
            EXPECT_EQ(expected.posY, actual.posY);
            EXPECT_EQ(expected.width, actual.width);
            EXPECT_EQ(expected.height, actual.height);
            EXPECT_EQ(expected.advance, actual.advance);
        }

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
        static FontInstanceId FontInstanceIdLatin;
        static FontInstanceId FontInstanceIdJapanese;
        static FontInstanceId FontInstanceIdThai;
        static Freetype2FontInstance* FontInstance4;
        static Freetype2FontInstance* FontInstance10;
        static Freetype2FontInstance* FontInstanceLatin;
        static Freetype2FontInstance* FontInstanceJp;
        static Freetype2FontInstance* FontInstanceThai;
    };

    FontRegistry*          AFreetype2FontInstance::FRegistry(nullptr);
    FontInstanceId         AFreetype2FontInstance::FontInstanceId4(0u);
    FontInstanceId         AFreetype2FontInstance::FontInstanceId10(0u);
    FontInstanceId         AFreetype2FontInstance::FontInstanceIdLatin(0u);
    FontInstanceId         AFreetype2FontInstance::FontInstanceIdJapanese(0u);
    FontInstanceId         AFreetype2FontInstance::FontInstanceIdThai(0u);
    Freetype2FontInstance* AFreetype2FontInstance::FontInstance4(nullptr);
    Freetype2FontInstance* AFreetype2FontInstance::FontInstance10(nullptr);
    Freetype2FontInstance* AFreetype2FontInstance::FontInstanceLatin(nullptr);
    Freetype2FontInstance* AFreetype2FontInstance::FontInstanceJp(nullptr);
    Freetype2FontInstance* AFreetype2FontInstance::FontInstanceThai(nullptr);

    TEST_F(AFreetype2FontInstance, ComputesHeightFromFontData)
    {
        EXPECT_EQ(5 , FontInstance4 ->getHeight());
        EXPECT_EQ(12, FontInstance10->getHeight());
    }

    TEST_F(AFreetype2FontInstance, ComputesAscenderFromFontData)
    {
        EXPECT_EQ(4, FontInstance4->getAscender());
        EXPECT_EQ(10, FontInstance10->getAscender());
    }

    TEST_F(AFreetype2FontInstance, ComputesDescenderFromFontData)
    {
        EXPECT_EQ(-1, FontInstance4->getDescender());
        EXPECT_EQ(-3, FontInstance10->getDescender());
    }

    TEST_F(AFreetype2FontInstance, ObtainsGlyphMetrics)
    {
        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(U"%", *FontInstance4);

        ASSERT_EQ(1u, positionedGlyphs.size());
        const GlyphKey key{ GlyphId(9u), FontInstanceId4 };
        ExpectGlyphMetricsEq({ key, 3u, 4u, 0, -1, 3 }, positionedGlyphs.front());
    }

    TEST_F(AFreetype2FontInstance, ObtainsMultipleGlyphMetrics)
    {
        const std::u32string str = U" abc 123 ._! ";
        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstance4);

        ASSERT_EQ(13u, positionedGlyphs.size());
        auto it = positionedGlyphs.cbegin();

        ExpectGlyphMetricsEq({ { GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }, *it++); //' '
        ExpectGlyphMetricsEq({ { GlyphId( 69u), FontInstanceId4 }, 3u, 4u, 0, -1, 2 }, *it++); //a
        ExpectGlyphMetricsEq({ { GlyphId( 70u), FontInstanceId4 }, 3u, 4u, 0, -1, 2 }, *it++); //b
        ExpectGlyphMetricsEq({ { GlyphId( 71u), FontInstanceId4 }, 2u, 4u, 0, -1, 2 }, *it++); //c
        ExpectGlyphMetricsEq({ { GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }, *it++); //' '
        ExpectGlyphMetricsEq({ { GlyphId( 21u), FontInstanceId4 }, 2u, 3u, 0,  0, 2 }, *it++); //1
        ExpectGlyphMetricsEq({ { GlyphId( 22u), FontInstanceId4 }, 3u, 3u, 0,  0, 2 }, *it++); //2
        ExpectGlyphMetricsEq({ { GlyphId( 23u), FontInstanceId4 }, 3u, 4u, 0, -1, 2 }, *it++); //3
        ExpectGlyphMetricsEq({ { GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }, *it++); //' '
        ExpectGlyphMetricsEq({ { GlyphId( 18u), FontInstanceId4 }, 1u, 2u, 0, -1, 1 }, *it++); //.
        ExpectGlyphMetricsEq({ { GlyphId( 67u), FontInstanceId4 }, 2u, 1u, 0, -1, 2 }, *it++); //_
        ExpectGlyphMetricsEq({ { GlyphId(  5u), FontInstanceId4 }, 1u, 4u, 0, -1, 1 }, *it++); //!
        ExpectGlyphMetricsEq({ { GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }, *it++); //' '
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AFreetype2FontInstance, ObtainsMultipleGlyphMetricsFromInstancesWithDifferentSizes)
    {
        const std::u32string str1 = U" 123 ._! ";
        const std::u32string str2 = U" abc ";
        const GlyphMetricsVector positionedGlyphs1 = GetPositionedGlyphs(str1, *FontInstance4);
        const GlyphMetricsVector positionedGlyphs2 = GetPositionedGlyphs(str2, *FontInstance10);

        ASSERT_EQ(9u, positionedGlyphs1.size());
        ASSERT_EQ(5u, positionedGlyphs2.size());

        auto it1 = positionedGlyphs1.cbegin();
        ExpectGlyphMetricsEq({ { GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }, *it1++); //' '
        ExpectGlyphMetricsEq({ { GlyphId( 21u), FontInstanceId4 }, 2u, 3u, 0,  0, 2 }, *it1++); //1
        ExpectGlyphMetricsEq({ { GlyphId( 22u), FontInstanceId4 }, 3u, 3u, 0,  0, 2 }, *it1++); //2
        ExpectGlyphMetricsEq({ { GlyphId( 23u), FontInstanceId4 }, 3u, 4u, 0, -1, 2 }, *it1++); //3
        ExpectGlyphMetricsEq({ { GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }, *it1++); //' '
        ExpectGlyphMetricsEq({ { GlyphId( 18u), FontInstanceId4 }, 1u, 2u, 0, -1, 1 }, *it1++); //.
        ExpectGlyphMetricsEq({ { GlyphId( 67u), FontInstanceId4 }, 2u, 1u, 0, -1, 2 }, *it1++); //_
        ExpectGlyphMetricsEq({ { GlyphId(  5u), FontInstanceId4 }, 1u, 4u, 0, -1, 1 }, *it1++); //!
        ExpectGlyphMetricsEq({ { GlyphId(  4u), FontInstanceId4 }, 0u, 0u, 0,  0, 1 }, *it1++); //' '
        EXPECT_EQ(it1, positionedGlyphs1.cend());

        auto it2 = positionedGlyphs2.cbegin();
        ExpectGlyphMetricsEq({ { GlyphId(  4u), FontInstanceId10 }, 0u, 0u, 0,  0, 2 }, *it2++); //' '
        ExpectGlyphMetricsEq({ { GlyphId( 69u), FontInstanceId10 }, 6u, 5u, 0,  0, 5 }, *it2++); //a
        ExpectGlyphMetricsEq({ { GlyphId( 70u), FontInstanceId10 }, 6u, 8u, 0,  0, 6 }, *it2++); //b
        ExpectGlyphMetricsEq({ { GlyphId( 71u), FontInstanceId10 }, 5u, 5u, 0,  0, 5 }, *it2++); //c
        ExpectGlyphMetricsEq({ { GlyphId(  4u), FontInstanceId10 }, 0u, 0u, 0,  0, 2 }, *it2++); //' '
        EXPECT_EQ(it2, positionedGlyphs2.cend());
    }

    TEST_F(AFreetype2FontInstance, CanLoadACharacterGlyphBitmapData)
    {
        QuadSize glyphBitmapSize;
        const GlyphData bitmapData = FontInstance4->loadGlyphBitmapData(FontInstance4->getGlyphId(U'@'), glyphBitmapSize.x, glyphBitmapSize.y);
        EXPECT_EQ(4u, glyphBitmapSize.x);
        EXPECT_EQ(4u, glyphBitmapSize.y);

        EXPECT_EQ(bitmapData, std::vector<uint8_t>({
            0x1e, 0x57, 0x54, 0x7,
            0x66, 0x6f, 0x65, 0x51,
            0x5e, 0x8b, 0x96, 0x36,
            0x41, 0x58, 0x28, 0x0
        }));
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
        const GlyphMetricsVector positionedGlyphs1 = GetPositionedGlyphs(str1, *FontInstance4);
        const GlyphMetricsVector positionedGlyphs2 = GetPositionedGlyphs(str2, *FontInstance10);
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

    TEST_F(AFreetype2FontInstance, ReportsAllSupportedChars)
    {
        const auto supportedChars = FontInstance10->getAllSupportedCharacters();
        EXPECT_TRUE(896u == supportedChars.size());
        EXPECT_TRUE(supportedChars.end() != supportedChars.find(165u));
        EXPECT_FALSE(supportedChars.end() != supportedChars.find(127u));
    }

    TEST_F(AFreetype2FontInstance, WillNotShapeJapaneseCombinedCharacters)
    {
        const std::u32string str = { 0x30D5, 0x3099, 0x30C4, 0x3099, }; //decompositions of U+30D6 and U+30C7

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstanceJp);
        ASSERT_EQ(4u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        ExpectGlyphMetricsEq({ { GlyphId(1614u), FontInstanceIdJapanese }, 8u,  9u, 1, -1, 10 }, *it++); //U+30D5
        ExpectGlyphMetricsEq({ { GlyphId(1556u), FontInstanceIdJapanese }, 3u,  3u, 7,  6, 10 }, *it++); //U+3099
        ExpectGlyphMetricsEq({ { GlyphId(1597u), FontInstanceIdJapanese }, 10u, 9u, 0, -1, 10 }, *it++); //U+30C6
        ExpectGlyphMetricsEq({ { GlyphId(1556u), FontInstanceIdJapanese }, 3u,  3u, 7,  6, 10 }, *it++); //U+3099
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AFreetype2FontInstance, WillNotShapeFrenchCombinedCharacters)
    {
        const std::u32string str = {0x0061, 0x0301, }; //decomposition of U+00E1

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstanceLatin);
        ASSERT_EQ(2u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        ExpectGlyphMetricsEq({ { GlyphId(69u), FontInstanceIdLatin},  6u, 5u,  0, 0, 5 }, *it++); //U+0061
        ExpectGlyphMetricsEq({ { GlyphId(169u), FontInstanceIdLatin}, 4u, 2u, -4, 6, 0 }, *it++); //U+0301
        EXPECT_EQ(it, positionedGlyphs.cend());
    }

    TEST_F(AFreetype2FontInstance, WillNotShapeThaiCombinedCharacters)
    {
        const std::u32string str = { 0x0E1B, 0x0E31, 0x0E4A, 0x0E21, 0x0E19, 0x0E49, 0x0E33, 0x0E21, 0x0E31, }; //'ปั๊มน้ำมั'

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstanceThai);
        ASSERT_EQ(9u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        ExpectGlyphMetricsEq({ { GlyphId(80u), FontInstanceIdThai}, 5u, 8u,  1, 0, 7 }, *it++);
        ExpectGlyphMetricsEq({ { GlyphId(45u), FontInstanceIdThai}, 5u, 2u, -4, 7, 0 }, *it++);
        ExpectGlyphMetricsEq({ { GlyphId(50u), FontInstanceIdThai}, 4u, 3u, -4, 7, 0 }, *it++);
        ExpectGlyphMetricsEq({ { GlyphId(56u), FontInstanceIdThai}, 5u, 6u,  1, 0, 7 }, *it++);
        ExpectGlyphMetricsEq({ { GlyphId(71u), FontInstanceIdThai}, 5u, 6u,  1, 0, 7 }, *it++);
        ExpectGlyphMetricsEq({ { GlyphId(47u), FontInstanceIdThai}, 4u, 2u, -4, 7, 0 }, *it++);
        ExpectGlyphMetricsEq({ { GlyphId(90u), FontInstanceIdThai}, 6u, 9u, -3, 0, 4 }, *it++); //0x0E33 stays 1 glyph (compared to HarfBuzz decomposition)
        ExpectGlyphMetricsEq({ { GlyphId(56u), FontInstanceIdThai}, 5u, 6u,  1, 0, 7 }, *it++);
        ExpectGlyphMetricsEq({ { GlyphId(45u), FontInstanceIdThai}, 5u, 2u, -4, 7, 0 }, *it++);

        EXPECT_EQ(it, positionedGlyphs.end());
    }

    TEST_F(AFreetype2FontInstance, WillNotShapeVietnameseCombinedCharacters)
    {
        const std::u32string str = { 0x00F4, 0x0309, }; //U"ổ"

        const GlyphMetricsVector positionedGlyphs = GetPositionedGlyphs(str, *FontInstanceLatin);
        ASSERT_EQ(2u, positionedGlyphs.size());

        auto it = positionedGlyphs.cbegin();
        ExpectGlyphMetricsEq({ { GlyphId(685u), FontInstanceIdLatin }, 6u, 8u,  0, 0, 6 }, *it++); //U+00F4
        ExpectGlyphMetricsEq({ { GlyphId(171u), FontInstanceIdLatin }, 3u, 3u, -4, 6, 0 }, *it++); //U+0309
        EXPECT_EQ(it, positionedGlyphs.end());
    }
}
