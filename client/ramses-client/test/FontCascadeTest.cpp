//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-text-api/FontCascade.h"
#include "ramses-text-api/TextCache.h"
#include "ramses-text-api/FontRegistry.h"
#include "ramses-client-api/RamsesClient.h"
#include <gtest/gtest.h>

namespace ramses
{
    class AFontCascade :  public testing::Test
    {
    public:
        AFontCascade()
            : m_excludedChars(U"~")
        {
        }

        static void SetUpTestCase()
        {
            FRegistry = new FontRegistry;
            LatinFont = FRegistry->createFreetype2Font("./res/ramses-text-Roboto-Bold.ttf");
            LatinFontInstance = FRegistry->createFreetype2FontInstance(LatinFont, 12);
            ArabicFont = FRegistry->createFreetype2Font("./res/ramses-text-DroidKufi-Regular.ttf");
            ArabicFontInstance = FRegistry->createFreetype2FontInstance(ArabicFont, 12);
        }

        static void TearDownTestCase()
        {
            delete FRegistry;
        }

    protected:
        const std::u32string m_excludedChars;

        const FontCascade m_fontCascadeLatinOnly{ *FRegistry,{ m_excludedChars },{ LatinFontInstance }, LatinFontInstance, 0xFFFD };
        const FontCascade m_fontCascadeLatinThenArabic{ *FRegistry, m_excludedChars,{ LatinFontInstance, ArabicFontInstance }, LatinFontInstance, 0xFFFD };
        const FontCascade m_fontCascadeArabicThenLatin{ *FRegistry, m_excludedChars,{ ArabicFontInstance, LatinFontInstance }, LatinFontInstance, 0xFFFD };

        static FontRegistry*    FRegistry;
        static FontId           ArabicFont;
        static FontInstanceId   ArabicFontInstance;
        static FontId           LatinFont;
        static FontInstanceId   LatinFontInstance;
    };

    FontRegistry*    AFontCascade::FRegistry(nullptr);
    FontId           AFontCascade::ArabicFont(0u);
    FontInstanceId   AFontCascade::ArabicFontInstance(0u);
    FontId           AFontCascade::LatinFont(0u);
    FontInstanceId   AFontCascade::LatinFontInstance(0u);

    TEST_F(AFontCascade, CreatesOnlyOneClusterIfAllCharsContainedInTheFirstFont)
    {
        const std::u32string str = U"abcABC";
        FontInstanceOffsets fontOffsets;
        const auto filteredStr = FontCascade::FilterAndFindFontInstancesForString(m_fontCascadeLatinOnly, str, fontOffsets);

        EXPECT_EQ(str, filteredStr);
        ASSERT_EQ(1u, fontOffsets.size());
        EXPECT_EQ(LatinFontInstance, fontOffsets.front().fontInstance);
        EXPECT_EQ(0u, fontOffsets.front().beginOffset);
    }

    TEST_F(AFontCascade, FiltersBlacklistedChars)
    {
        const std::u32string str = U"~abc~ABC~";
        FontInstanceOffsets fontOffsets;
        const auto filteredStr = FontCascade::FilterAndFindFontInstancesForString(m_fontCascadeLatinOnly, str, fontOffsets);

        EXPECT_EQ(U"abcABC", filteredStr);
        ASSERT_EQ(1u, fontOffsets.size());
        EXPECT_EQ(LatinFontInstance, fontOffsets.front().fontInstance);
        EXPECT_EQ(0u, fontOffsets.front().beginOffset);
    }

    TEST_F(AFontCascade, picksCharactersFromFirstAppearanceInCascade)
    {
        const std::u32string str = U" ش2";
        FontInstanceOffsets fontOffsets;
        const auto filteredStr = FontCascade::FilterAndFindFontInstancesForString(m_fontCascadeLatinThenArabic, str, fontOffsets);

        RamsesFramework framework;
        RamsesClient& client(*framework.createClient("test"));
        TextCache textCache(*client.createScene(sceneId_t(1u)), *FRegistry, 64u, 64u);

        const auto positionedGlyphs = textCache.getPositionedGlyphs(filteredStr, fontOffsets);
        ASSERT_EQ(3u, positionedGlyphs.size());

        // 0: in both
        EXPECT_EQ(LatinFontInstance, positionedGlyphs[0].key.fontInstanceId);
        EXPECT_EQ(4u, positionedGlyphs[0].key.identifier.getValue());
        EXPECT_EQ(0u, positionedGlyphs[0].width);
        EXPECT_EQ(0u, positionedGlyphs[0].height);
        EXPECT_EQ(0,  positionedGlyphs[0].posX);
        EXPECT_EQ(0,  positionedGlyphs[0].posY);
        EXPECT_EQ(3 , positionedGlyphs[0].advance);

        // 1: arabic only
        EXPECT_EQ(ArabicFontInstance, positionedGlyphs[1].key.fontInstanceId);
        EXPECT_EQ(175u, positionedGlyphs[1].key.identifier.getValue());
        EXPECT_EQ(14u,  positionedGlyphs[1].width);
        EXPECT_EQ(13u,  positionedGlyphs[1].height);
        EXPECT_EQ(0,    positionedGlyphs[1].posX);
        EXPECT_EQ(-3,   positionedGlyphs[1].posY);
        EXPECT_EQ(15 ,  positionedGlyphs[1].advance);

        // 2: latin only
        EXPECT_EQ(LatinFontInstance, positionedGlyphs[2].key.fontInstanceId);
        EXPECT_EQ(22u, positionedGlyphs[2].key.identifier.getValue());
        EXPECT_EQ(7u,  positionedGlyphs[2].width);
        EXPECT_EQ(9u,  positionedGlyphs[2].height);
        EXPECT_EQ(0,   positionedGlyphs[2].posX);
        EXPECT_EQ(0,   positionedGlyphs[2].posY);
        EXPECT_EQ(7 ,  positionedGlyphs[2].advance);
    }
}
