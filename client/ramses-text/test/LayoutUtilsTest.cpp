//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses-text-api/LayoutUtils.h"

namespace ramses
{
    class ALayoutUtils : public testing::Test
    {
    protected:
        void expectBBoxEq(const LayoutUtils::StringBoundingBox& expected, const LayoutUtils::StringBoundingBox& actual)
        {
            EXPECT_EQ(expected.offsetX,         actual.offsetX);
            EXPECT_EQ(expected.offsetY,         actual.offsetY);
            EXPECT_EQ(expected.width,           actual.width);
            EXPECT_EQ(expected.height,          actual.height);
            EXPECT_EQ(expected.combinedAdvance, actual.combinedAdvance);
        }

        GlyphMetricsVector m_positionedGlyphs
        {
            { GlyphKey(GlyphId(1u), FontInstanceId(1u)),  2u,  1u,  0,  5,  1 },
            { GlyphKey(GlyphId(2u), FontInstanceId(3u)),  7u, 13u, -1,  0, -2 },
            { GlyphKey(GlyphId(3u), FontInstanceId(5u)),  1u,  3u, -2,  3,  5 },
            { GlyphKey(GlyphId(4u), FontInstanceId(7u)), 11u,  7u,  5,  2,  3 },
            { GlyphKey(GlyphId(5u), FontInstanceId(9u)),  4u,  9u,  1, -1, -1 },
        };
    };

    TEST_F(ALayoutUtils, EmptyBBoxForEmptyString)
    {
        expectBBoxEq({ 0,0,0,0,0 }, LayoutUtils::GetBoundingBoxForString(m_positionedGlyphs.cbegin(), m_positionedGlyphs.cbegin()));
        expectBBoxEq({ 0,0,0,0,0 }, LayoutUtils::GetBoundingBoxForString(m_positionedGlyphs.cend(), m_positionedGlyphs.cend()));
        expectBBoxEq({ 0,0,0,0,0 }, LayoutUtils::GetBoundingBoxForString(m_positionedGlyphs.crbegin(), m_positionedGlyphs.crbegin()));
        expectBBoxEq({ 0,0,0,0,0 }, LayoutUtils::GetBoundingBoxForString(m_positionedGlyphs.crend(), m_positionedGlyphs.crend()));
    }

    TEST_F(ALayoutUtils, EmptyBBoxForInvalidRange)
    {
        expectBBoxEq({ 0,0,0,0,0 }, LayoutUtils::GetBoundingBoxForString(m_positionedGlyphs.cend(), m_positionedGlyphs.cbegin()));
        expectBBoxEq({ 0,0,0,0,0 }, LayoutUtils::GetBoundingBoxForString(m_positionedGlyphs.crend(), m_positionedGlyphs.crbegin()));
    }

    TEST_F(ALayoutUtils, GetsBBoxForString)
    {
        expectBBoxEq({ -3, -1, 23, 14, 6 }, LayoutUtils::GetBoundingBoxForString(m_positionedGlyphs.cbegin(), m_positionedGlyphs.cend()));
    }

    TEST_F(ALayoutUtils, GetsBBoxForStringInReverse)
    {
        expectBBoxEq({ 0, -1, 15, 14, 6 }, LayoutUtils::GetBoundingBoxForString(m_positionedGlyphs.crbegin(), m_positionedGlyphs.crend()));
    }

    TEST_F(ALayoutUtils, GetsBBoxForSubString)
    {
        expectBBoxEq({ -4, 0, 23, 13, 6 }, LayoutUtils::GetBoundingBoxForString(m_positionedGlyphs.cbegin() + 1, m_positionedGlyphs.cend() - 1));
    }

    TEST_F(ALayoutUtils, GetsBBoxForSubStringInReverse)
    {
        expectBBoxEq({ 1, 0, 15, 13, 6 }, LayoutUtils::GetBoundingBoxForString(m_positionedGlyphs.crbegin() + 1, m_positionedGlyphs.crend() - 1));
    }

    TEST_F(ALayoutUtils, FindsNoFittingForEmptyString)
    {
        EXPECT_EQ(m_positionedGlyphs.cbegin(), LayoutUtils::FindFittingSubstring(m_positionedGlyphs.cbegin(), m_positionedGlyphs.cbegin(), 1000u));
        EXPECT_EQ(m_positionedGlyphs.cend(), LayoutUtils::FindFittingSubstring(m_positionedGlyphs.cend(), m_positionedGlyphs.cend(), 1000u));
        EXPECT_EQ(m_positionedGlyphs.crbegin(), LayoutUtils::FindFittingSubstring(m_positionedGlyphs.crbegin(), m_positionedGlyphs.crbegin(), 1000u));
        EXPECT_EQ(m_positionedGlyphs.crend(), LayoutUtils::FindFittingSubstring(m_positionedGlyphs.crend(), m_positionedGlyphs.crend(), 1000u));
    }

    TEST_F(ALayoutUtils, FindsNoFittingForInvalidRange)
    {
        EXPECT_EQ(m_positionedGlyphs.cbegin(), LayoutUtils::FindFittingSubstring(m_positionedGlyphs.cend(), m_positionedGlyphs.cbegin(), 1000u));
        EXPECT_EQ(m_positionedGlyphs.crbegin(), LayoutUtils::FindFittingSubstring(m_positionedGlyphs.crend(), m_positionedGlyphs.crbegin(), 1000u));
    }

    TEST_F(ALayoutUtils, FindsNoFittingForZeroMaxWidth)
    {
        EXPECT_EQ(m_positionedGlyphs.cbegin(), LayoutUtils::FindFittingSubstring(m_positionedGlyphs.cbegin(), m_positionedGlyphs.cend(), 0u));
        EXPECT_EQ(m_positionedGlyphs.crbegin(), LayoutUtils::FindFittingSubstring(m_positionedGlyphs.crbegin(), m_positionedGlyphs.crend(), 0u));
    }

    TEST_F(ALayoutUtils, ReportsWholeGivenStringAsFittingIfMaxWidthEqualsItsWidth)
    {
        const auto bbox = LayoutUtils::GetBoundingBoxForString(m_positionedGlyphs.cbegin(), m_positionedGlyphs.cend());
        EXPECT_EQ(m_positionedGlyphs.cend(), LayoutUtils::FindFittingSubstring(m_positionedGlyphs.cbegin(), m_positionedGlyphs.cend(), bbox.width));
    }

    TEST_F(ALayoutUtils, ReportsWholeGivenStringAsNOTFittingIfMaxWidthJustBelowItsWidth)
    {
        const auto bbox = LayoutUtils::GetBoundingBoxForString(m_positionedGlyphs.cbegin(), m_positionedGlyphs.cend());
        EXPECT_NE(m_positionedGlyphs.cend(), LayoutUtils::FindFittingSubstring(m_positionedGlyphs.cbegin(), m_positionedGlyphs.cend(), bbox.width - 1u));
    }

    TEST_F(ALayoutUtils, FindsFittingSubstringForGivenMaxWidth)
    {
        EXPECT_EQ(m_positionedGlyphs.cbegin() + 3, LayoutUtils::FindFittingSubstring(m_positionedGlyphs.cbegin(), m_positionedGlyphs.cend(), 11u));
    }

    TEST_F(ALayoutUtils, FindsNoFittingIfFirstCharExceedsMaxWidth)
    {
        EXPECT_EQ(m_positionedGlyphs.cbegin(), LayoutUtils::FindFittingSubstring(m_positionedGlyphs.cbegin(), m_positionedGlyphs.cend(), 1u));
    }

    TEST_F(ALayoutUtils, FindsFittingFirstCharIfItsWidthEqualsMaxWidth)
    {
        EXPECT_EQ(m_positionedGlyphs.cbegin() + 1, LayoutUtils::FindFittingSubstring(m_positionedGlyphs.cbegin(), m_positionedGlyphs.cend(), 2u));
    }
}
