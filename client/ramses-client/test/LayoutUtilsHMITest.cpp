//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-text-api/LayoutUtils.h"
#include "ramses-text-api/TextCache.h"
#include "ramses-text-api/FontCascade.h"
#include "ramses-text-api/FontRegistry.h"
#include "ramses-client-api/RamsesClient.h"
#include <gtest/gtest.h>
#include <algorithm>

namespace ramses
{
    class ALayoutUtilsHMI : public testing::Test
    {
    public:
        ALayoutUtilsHMI()
            : m_textCache(*SScene, *FRegistry, 64u, 64u)
        {
        }

        static void SetUpTestCase()
        {
            Framework = new RamsesFramework;
            Client = ALayoutUtilsHMI::Framework->createClient("test");
            SScene = Client->createScene(sceneId_t(1u));

            FRegistry = new FontRegistry;

            const FontId fontLatinId = FRegistry->createFreetype2Font("./res/ramses-text-Roboto-Regular.ttf");
            Latin24 = FRegistry->createFreetype2FontInstance(fontLatinId, 24u);

            const FontId fontArabicId = FRegistry->createFreetype2Font("./res/ramses-text-DroidKufi-Regular.ttf");
            Arabic24 = FRegistry->createFreetype2FontInstanceWithHarfBuzz(fontArabicId, 24u);
        }

        static void TearDownTestCase()
        {
            delete FRegistry;
            Framework->destroyClient(*Client);
            delete Framework;
        }

    protected:
        TextCache m_textCache;

        const std::u32string latinStringWithoutKerning = U"test";
        const std::u32string latinStringWithKerning = U"Test";
        const std::u32string latinSubstringWithKerning = U"ThisWillBeIgnored Test";
        const std::u32string latinStringWithBlacklistedCharacter = U"Te~st";
        const uint32_t substringOffset = 18u;

        static RamsesFramework* Framework;
        static RamsesClient* Client;
        static Scene* SScene;
        static FontRegistry* FRegistry;
        static FontInstanceId Latin24;
        static FontInstanceId Arabic24;
    };

    RamsesFramework* ALayoutUtilsHMI::Framework = nullptr;
    RamsesClient* ALayoutUtilsHMI::Client= nullptr;
    Scene* ALayoutUtilsHMI::SScene = nullptr;

    FontRegistry* ALayoutUtilsHMI::FRegistry(nullptr);
    FontInstanceId ALayoutUtilsHMI::Latin24(0u);
    FontInstanceId ALayoutUtilsHMI::Arabic24(0u);

    TEST_F(ALayoutUtilsHMI, FitsZeroCharactersIfFirstLetterIsBiggerThanProvidedMaxWidth)
    {
        const FontCascade fontCascade{ *FRegistry, {}, OrderedFontList{ Latin24 }, Latin24, 0xFFFD };
        FontInstanceOffsets fontOffsets;
        const auto str = FontCascade::FilterAndFindFontInstancesForString(fontCascade, latinStringWithoutKerning, fontOffsets);
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, fontOffsets);

        EXPECT_EQ(positionedGlyphs.cbegin(), LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin(), positionedGlyphs.cend(), 7u));
    }

    TEST_F(ALayoutUtilsHMI, CanFitTheWholeStringWhenItMatchesTheGivenMaxWidth)
    {
        const FontCascade fontCascade{ *FRegistry, {}, OrderedFontList{ Latin24 }, Latin24, 0xFFFD };
        FontInstanceOffsets fontOffsets;
        const auto str = FontCascade::FilterAndFindFontInstancesForString(fontCascade, latinStringWithoutKerning, fontOffsets);
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, fontOffsets);

        EXPECT_EQ(positionedGlyphs.cend(), LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin(), positionedGlyphs.cend(), 42u));

        const auto bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), positionedGlyphs.cend());
        EXPECT_EQ(41u, bbox.width);
        EXPECT_EQ(41, bbox.combinedAdvance);
    }

    TEST_F(ALayoutUtilsHMI, CanFitStringWithInvalidCharacters)
    {
        std::u32string textWithInvalidCharacters;
        textWithInvalidCharacters.push_back('t');
        textWithInvalidCharacters.push_back(0x19aa);
        textWithInvalidCharacters.push_back('e');
        textWithInvalidCharacters.push_back(0x19aa);
        textWithInvalidCharacters.push_back('s');
        textWithInvalidCharacters.push_back(0x19aa);
        textWithInvalidCharacters.push_back('t');
        textWithInvalidCharacters.push_back(0x19aa);
        textWithInvalidCharacters.push_back(0);

        const FontCascade fontCascade{ *FRegistry, {}, OrderedFontList{ Latin24 }, Latin24, 0xFFFD };
        FontInstanceOffsets fontOffsets;
        const auto str = FontCascade::FilterAndFindFontInstancesForString(fontCascade, textWithInvalidCharacters, fontOffsets);
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, fontOffsets);

        EXPECT_EQ(positionedGlyphs.cend(), LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin(), positionedGlyphs.cend(), 140u));

        const auto bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), positionedGlyphs.cend());
        EXPECT_EQ(140u, bbox.width);
        EXPECT_EQ(141, bbox.combinedAdvance);
    }

    TEST_F(ALayoutUtilsHMI, CanFitTheWholeStringWhenItMatchesTheGivenMaxWidth_WithExactSubstringLength_Reversed)
    {
        auto latinStringWithoutKerningReversed = latinStringWithoutKerning;
        std::reverse(latinStringWithoutKerningReversed.begin(), latinStringWithoutKerningReversed.end());

        const FontCascade fontCascade{ *FRegistry, {}, OrderedFontList{ Latin24 }, Latin24, 0xFFFD };
        FontInstanceOffsets fontOffsets;
        const auto str = FontCascade::FilterAndFindFontInstancesForString(fontCascade, latinStringWithoutKerningReversed, fontOffsets);
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, fontOffsets);

        EXPECT_EQ(positionedGlyphs.crend(), LayoutUtils::FindFittingSubstring(positionedGlyphs.crbegin(), positionedGlyphs.crend(), 42u));

        const auto bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), positionedGlyphs.cend());
        EXPECT_EQ(41u, bbox.width);
        EXPECT_EQ(41, bbox.combinedAdvance);
    }

    TEST_F(ALayoutUtilsHMI, CanFitTheWholeStringWhenItMatchesTheGivenMaxWidth_WithKerning)
    {
        const FontCascade fontCascade{ *FRegistry, {}, OrderedFontList{ Latin24 }, Latin24, 0xFFFD };
        FontInstanceOffsets fontOffsets;
        const auto str = FontCascade::FilterAndFindFontInstancesForString(fontCascade, latinStringWithKerning, fontOffsets);
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, fontOffsets);

        EXPECT_EQ(positionedGlyphs.cend(), LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin(), positionedGlyphs.cend(), 50u));

        const auto bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), positionedGlyphs.cend());
        EXPECT_EQ(47u, bbox.width);
        EXPECT_EQ(47, bbox.combinedAdvance);
    }

    TEST_F(ALayoutUtilsHMI, CalculatesBoundingBoxCorrectly)
    {
        const FontCascade fontCascade{ *FRegistry, {}, OrderedFontList{ Latin24 }, Latin24, 0xFFFD };
        FontInstanceOffsets fontOffsets;
        const auto str = FontCascade::FilterAndFindFontInstancesForString(fontCascade, latinStringWithKerning, fontOffsets);
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, fontOffsets);

        const auto bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), positionedGlyphs.cend());
        EXPECT_EQ(0, bbox.offsetX);
        EXPECT_EQ(0, bbox.offsetY);
        EXPECT_EQ(47u, bbox.width);
        EXPECT_EQ(17u, bbox.height);
        EXPECT_EQ(47, bbox.combinedAdvance);
    }

    TEST_F(ALayoutUtilsHMI, CanFitTheWholeStringWhenItMatchesTheGivenMaxWidth_WithKerning_AsSubstring)
    {
        const FontCascade fontCascade{ *FRegistry, {}, OrderedFontList{ Latin24 }, Latin24, 0xFFFD };
        FontInstanceOffsets fontOffsets;
        const auto str = FontCascade::FilterAndFindFontInstancesForString(fontCascade, latinSubstringWithKerning, fontOffsets);
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, fontOffsets);

        EXPECT_EQ(positionedGlyphs.cend(), LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin() + substringOffset, positionedGlyphs.cend(), 50u));

        const auto bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin() + substringOffset, positionedGlyphs.cend());
        EXPECT_EQ(0, bbox.offsetX);
        EXPECT_EQ(0, bbox.offsetY);
        EXPECT_EQ(47u, bbox.width);
        EXPECT_EQ(17u, bbox.height);
        EXPECT_EQ(47, bbox.combinedAdvance);
    }

    TEST_F(ALayoutUtilsHMI, CanFitTheWholeStringWhenItMatchesTheGivenMaxWidth_WithKerning_AsSubstring_Reversed)
    {
        auto latinSubstringWithKerningReversed = latinSubstringWithKerning;
        std::reverse(latinSubstringWithKerningReversed.begin(), latinSubstringWithKerningReversed.end());

        const FontCascade fontCascade{ *FRegistry, {}, OrderedFontList{ Latin24 }, Latin24, 0xFFFD };
        FontInstanceOffsets fontOffsets;
        const auto str = FontCascade::FilterAndFindFontInstancesForString(fontCascade, latinSubstringWithKerningReversed, fontOffsets);
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, fontOffsets);

        EXPECT_EQ(positionedGlyphs.crend(), LayoutUtils::FindFittingSubstring(positionedGlyphs.crbegin() + substringOffset, positionedGlyphs.crend(), 50u));

        const auto bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.crbegin() + substringOffset, positionedGlyphs.crend());
        EXPECT_EQ(0, bbox.offsetX);
        EXPECT_EQ(0, bbox.offsetY);
        EXPECT_EQ(47u, bbox.width);
        EXPECT_EQ(17u, bbox.height);
        EXPECT_EQ(47, bbox.combinedAdvance);
    }

    TEST_F(ALayoutUtilsHMI, FindsVaryingFittingSubstringsDependingOnMaxWidth)
    {
        // 'T' is 14 pixels, 'e' is 13 pixels, kerning for 'T' and 'e' is -1 pixels
        // test the corner cases with one and two chars

        const FontCascade fontCascade{ *FRegistry, {}, OrderedFontList{ Latin24 }, Latin24, 0xFFFD };
        FontInstanceOffsets fontOffsets;
        const auto str = FontCascade::FilterAndFindFontInstancesForString(fontCascade, latinStringWithKerning, fontOffsets);
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, fontOffsets);

        // First char fits perfectly
        auto subStrEndIt = LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin(), positionedGlyphs.cend(), 14u);
        EXPECT_EQ(positionedGlyphs.cbegin() + 1, subStrEndIt);
        auto bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), subStrEndIt);
        EXPECT_EQ(14u, bbox.width);
        EXPECT_EQ(14, bbox.combinedAdvance);

        // First char fits, some space left over
        subStrEndIt = LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin(), positionedGlyphs.cend(), 17u);
        EXPECT_EQ(positionedGlyphs.cbegin() + 1, subStrEndIt);
        bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), subStrEndIt);
        EXPECT_EQ(14u, bbox.width);
        EXPECT_EQ(14, bbox.combinedAdvance);

        // First two chars take one pixel more than max width
        subStrEndIt = LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin(), positionedGlyphs.cend(), 25u);
        EXPECT_EQ(positionedGlyphs.cbegin() + 1, subStrEndIt);
        bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), subStrEndIt);
        EXPECT_EQ(14u, bbox.width);
        EXPECT_EQ(14, bbox.combinedAdvance);

        // First two chars fit perfectly
        subStrEndIt = LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin(), positionedGlyphs.cend(), 26u);
        EXPECT_EQ(positionedGlyphs.cbegin() + 2, subStrEndIt);
        bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), subStrEndIt);
        EXPECT_EQ(26u, bbox.width);
        EXPECT_EQ(27, bbox.combinedAdvance);

        // First two chars fit, some space left over
        subStrEndIt = LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin(), positionedGlyphs.cend(), 28u);
        EXPECT_EQ(positionedGlyphs.cbegin() + 2, subStrEndIt);
        bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), subStrEndIt);
        EXPECT_EQ(26u, bbox.width);
        EXPECT_EQ(27, bbox.combinedAdvance);
    }

    TEST_F(ALayoutUtilsHMI, FindsVaryingFittingSubstringsDependingOnMaxWidth_AsSubstring)
    {
        // 'T' is 14 pixels, 'e' is 13 pixels, kerning for 'T' and 'e' is -1 pixels
        // test the corner cases with one and two chars

        const FontCascade fontCascade{ *FRegistry, {}, OrderedFontList{ Latin24 }, Latin24, 0xFFFD };
        FontInstanceOffsets fontOffsets;
        const auto str = FontCascade::FilterAndFindFontInstancesForString(fontCascade, latinSubstringWithKerning, fontOffsets);
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, fontOffsets);
        const auto subStrBeginIt = positionedGlyphs.cbegin() + substringOffset;

        // First char fits perfectly
        auto subStrEndIt = LayoutUtils::FindFittingSubstring(subStrBeginIt, positionedGlyphs.cend(), 14u);
        EXPECT_EQ(subStrBeginIt + 1, subStrEndIt);
        auto bbox = LayoutUtils::GetBoundingBoxForString(subStrBeginIt, subStrEndIt);
        EXPECT_EQ(14u, bbox.width);
        EXPECT_EQ(14, bbox.combinedAdvance);

        // First char fits, some space left over
        subStrEndIt = LayoutUtils::FindFittingSubstring(subStrBeginIt, positionedGlyphs.cend(), 17u);
        EXPECT_EQ(subStrBeginIt + 1, subStrEndIt);
        bbox = LayoutUtils::GetBoundingBoxForString(subStrBeginIt, subStrEndIt);
        EXPECT_EQ(14u, bbox.width);
        EXPECT_EQ(14, bbox.combinedAdvance);

        // First two chars take one pixel more than max width
        subStrEndIt = LayoutUtils::FindFittingSubstring(subStrBeginIt, positionedGlyphs.cend(), 25u);
        EXPECT_EQ(subStrBeginIt + 1, subStrEndIt);
        bbox = LayoutUtils::GetBoundingBoxForString(subStrBeginIt, subStrEndIt);
        EXPECT_EQ(14u, bbox.width);
        EXPECT_EQ(14, bbox.combinedAdvance);

        // First two chars fit perfectly
        subStrEndIt = LayoutUtils::FindFittingSubstring(subStrBeginIt, positionedGlyphs.cend(), 26u);
        EXPECT_EQ(subStrBeginIt + 2, subStrEndIt);
        bbox = LayoutUtils::GetBoundingBoxForString(subStrBeginIt, subStrEndIt);
        EXPECT_EQ(26u, bbox.width);
        EXPECT_EQ(27, bbox.combinedAdvance);

        // First two chars fit, some space left over
        subStrEndIt = LayoutUtils::FindFittingSubstring(subStrBeginIt, positionedGlyphs.cend(), 28u);
        EXPECT_EQ(subStrBeginIt + 2, subStrEndIt);
        bbox = LayoutUtils::GetBoundingBoxForString(subStrBeginIt, subStrEndIt);
        EXPECT_EQ(26u, bbox.width);
        EXPECT_EQ(27, bbox.combinedAdvance);
    }

    TEST_F(ALayoutUtilsHMI, CanFitTheWholeStringWithBlacklistedCharacterWhenItMatchesTheGivenMaxWidth)
    {
        const FontCascade fontCascade{ *FRegistry, U"~", OrderedFontList{ Latin24 }, Latin24, 0xFFFD };
        FontInstanceOffsets fontOffsets;
        const auto str = FontCascade::FilterAndFindFontInstancesForString(fontCascade, latinStringWithBlacklistedCharacter, fontOffsets);
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, fontOffsets);

        EXPECT_EQ(positionedGlyphs.cend(), LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin(), positionedGlyphs.cend(), 47u));

        const auto bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), positionedGlyphs.cend());
        EXPECT_EQ(0, bbox.offsetX);
        EXPECT_EQ(0, bbox.offsetY);
        EXPECT_EQ(47u, bbox.width);
        EXPECT_EQ(17u, bbox.height);
        EXPECT_EQ(47, bbox.combinedAdvance);
    }

    TEST_F(ALayoutUtilsHMI, CanFitTheWholeReshapedStringWhenTwoCharacterMergeToASingleGlyph)
    {
        const FontCascade fontCascade{ *FRegistry, {}, OrderedFontList{ Arabic24 }, Latin24, 0xFFFD };
        // Arabic "Lam" (ل) and "Alef" (ا) join together to a single glyph (لا).
        FontInstanceOffsets fontOffsets;
        const auto str = FontCascade::FilterAndFindFontInstancesForString(fontCascade, U"ال", fontOffsets);
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, fontOffsets);

        EXPECT_EQ(positionedGlyphs.cend(), LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin(), positionedGlyphs.cend(), 15u));

        const auto bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), positionedGlyphs.cend());
        EXPECT_EQ(14u, bbox.width);
        EXPECT_EQ(18u, bbox.height);
        EXPECT_EQ(1, bbox.offsetX);
        EXPECT_EQ(0, bbox.offsetY);
        EXPECT_EQ(16, bbox.combinedAdvance);
    }

    TEST_F(ALayoutUtilsHMI, CantFitAnyReshapedCharacterWhenTwoCharacterMergeToASingleGlyphWhichDoesntFit)
    {
        const FontCascade fontCascade{ *FRegistry, {}, OrderedFontList{ Arabic24 }, Latin24, 0xFFFD };
        // Arabic "Lam" (ل) and "Alef" (ا) join together to a single glyph (لا).
        FontInstanceOffsets fontOffsets;
        const auto str = FontCascade::FilterAndFindFontInstancesForString(fontCascade, U"ال", fontOffsets);
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, fontOffsets);

        const auto subStrEndIt = LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin(), positionedGlyphs.cend(), 12u);
        EXPECT_EQ(positionedGlyphs.cbegin(), subStrEndIt);

        const auto bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), subStrEndIt);
        EXPECT_EQ(0u, bbox.width);
        EXPECT_EQ(0, bbox.combinedAdvance);
    }

    TEST_F(ALayoutUtilsHMI, confidenceTest_ProvidesFittingReshapedCharacterCountWhenCharactersMergedSurroundedbyOtherCharacters)
    {
        const FontCascade fontCascade{ *FRegistry, {}, OrderedFontList{ Arabic24 }, Latin24, 0xFFFD };
        // Arabic "Lam" (ل) and "Alef" (ا) join together to a single glyph (لا).
        //  Character  Glyph(s)
        //  0 =>       1             "لا"
        //  1 =>
        //  2 =>       2             "ص"
        //  3 =>       3             "ح"
        //  4 =>       4             "ة"
        FontInstanceOffsets fontOffsets;
        const auto str = FontCascade::FilterAndFindFontInstancesForString(fontCascade, U"الصحة", fontOffsets);
        const auto positionedGlyphs = m_textCache.getPositionedGlyphs(str, fontOffsets);

        auto subStrEndIt = LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin(), positionedGlyphs.cend(), 64u);
        EXPECT_EQ(positionedGlyphs.cend(), subStrEndIt);
        auto bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), subStrEndIt);
        EXPECT_EQ(63u, bbox.width);
        EXPECT_EQ(65, bbox.combinedAdvance);

        subStrEndIt = LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin(), positionedGlyphs.cend(), 63u);
        EXPECT_EQ(positionedGlyphs.cend() - 1, subStrEndIt);
        bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), subStrEndIt);
        EXPECT_EQ(49u, bbox.width);
        EXPECT_EQ(51, bbox.combinedAdvance);

        subStrEndIt = LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin(), positionedGlyphs.cend(), 50u);
        EXPECT_EQ(positionedGlyphs.cend() - 1, subStrEndIt);
        bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), subStrEndIt);
        EXPECT_EQ(49u, bbox.width);
        EXPECT_EQ(51, bbox.combinedAdvance);

        subStrEndIt = LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin(), positionedGlyphs.cend(), 49u);
        EXPECT_EQ(positionedGlyphs.cend() - 2, subStrEndIt);
        bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), subStrEndIt);
        EXPECT_EQ(37u, bbox.width);
        EXPECT_EQ(37, bbox.combinedAdvance);

        subStrEndIt = LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin(), positionedGlyphs.cend(), 38u);
        EXPECT_EQ(positionedGlyphs.cend() - 2, subStrEndIt);
        bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), subStrEndIt);
        EXPECT_EQ(37u, bbox.width);
        EXPECT_EQ(37, bbox.combinedAdvance);

        subStrEndIt = LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin(), positionedGlyphs.cend(), 37u);
        EXPECT_EQ(positionedGlyphs.cend() - 3, subStrEndIt);
        bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), subStrEndIt);
        EXPECT_EQ(17u, bbox.width);
        EXPECT_EQ(17, bbox.combinedAdvance);

        subStrEndIt = LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin(), positionedGlyphs.cend(), 18u);
        EXPECT_EQ(positionedGlyphs.cend() - 3, subStrEndIt);
        bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), subStrEndIt);
        EXPECT_EQ(17u, bbox.width);
        EXPECT_EQ(17, bbox.combinedAdvance);

        subStrEndIt = LayoutUtils::FindFittingSubstring(positionedGlyphs.cbegin(), positionedGlyphs.cend(), 17u);
        EXPECT_EQ(positionedGlyphs.cbegin(), subStrEndIt);
        bbox = LayoutUtils::GetBoundingBoxForString(positionedGlyphs.cbegin(), subStrEndIt);
        EXPECT_EQ(0u, bbox.width);
        EXPECT_EQ(0, bbox.combinedAdvance);
    }
}
