//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "ramses-renderer-api/DcsmContentControlConfig.h"

using namespace ramses;
using namespace testing;

TEST(ADcsmContentControlConfig, canBeEmpty)
{
    static constexpr Category cat1{ 111 };
    const DcsmContentControlConfig config{};
    EXPECT_EQ(nullptr, config.findCategoryInfo(cat1));
}

TEST(ADcsmContentControlConfig, canBeContructedWithCategories)
{
    static constexpr Category cat1{ 111 };
    static constexpr Category cat2{ 222 };
    static constexpr displayId_t disp1{ 1 };
    static constexpr displayId_t disp2{ 2 };
    static constexpr SizeInfo size1{ 1, 2 };
    static constexpr SizeInfo size2{ 3, 4 };

    const DcsmContentControlConfig config{ { { cat1, { size1, disp1 } }, { cat2, { size2, disp2 } } } };
    const auto catInfo1 = config.findCategoryInfo(cat1);
    const auto catInfo2 = config.findCategoryInfo(cat2);

    ASSERT_NE(nullptr, catInfo1);
    EXPECT_EQ(disp1, catInfo1->display);
    EXPECT_EQ(size1, catInfo1->size);

    ASSERT_NE(nullptr, catInfo2);
    EXPECT_EQ(disp2, catInfo2->display);
    EXPECT_EQ(size2, catInfo2->size);
}

TEST(ADcsmContentControlConfig, returnsNullWhenFindingUnregisteredCategory)
{
    static constexpr Category cat1{ 111 };
    static constexpr Category cat2{ 222 };
    static constexpr displayId_t disp1{ 1 };
    static constexpr SizeInfo size1{ 1, 2 };

    const DcsmContentControlConfig config{ { { cat1, { size1, disp1 } } } };
    const auto catInfo1 = config.findCategoryInfo(cat1);
    const auto catInfo2 = config.findCategoryInfo(cat2);

    EXPECT_NE(nullptr, catInfo1);
    EXPECT_EQ(nullptr, catInfo2);
}

TEST(ADcsmContentControlConfig, canAddCategoryAfterConstructing)
{
    static constexpr Category cat1{ 111 };
    static constexpr Category cat2{ 222 };
    static constexpr displayId_t disp1{ 1 };
    static constexpr displayId_t disp2{ 2 };
    static constexpr SizeInfo size1{ 1, 2 };
    static constexpr SizeInfo size2{ 3, 4 };

    DcsmContentControlConfig config{ { { cat1, { size1, disp1 } } } };

    EXPECT_EQ(StatusOK, config.addCategory(cat2, { size2, disp2 }));

    const auto catInfo1 = config.findCategoryInfo(cat1);
    const auto catInfo2 = config.findCategoryInfo(cat2);

    ASSERT_NE(nullptr, catInfo1);
    EXPECT_EQ(disp1, catInfo1->display);
    EXPECT_EQ(size1, catInfo1->size);

    ASSERT_NE(nullptr, catInfo2);
    EXPECT_EQ(disp2, catInfo2->display);
    EXPECT_EQ(size2, catInfo2->size);
}

TEST(ADcsmContentControlConfig, ignoresDupliciteCategoryGivenToConstructor)
{
    static constexpr Category cat1{ 111 };
    static constexpr displayId_t disp1{ 1 };
    static constexpr displayId_t disp2{ 2 };
    static constexpr SizeInfo size1{ 1, 2 };
    static constexpr SizeInfo size2{ 3, 4 };

    // passing different category info for same category ID
    const DcsmContentControlConfig config{ { { cat1, { size1, disp1 } }, { cat1, { size2, disp2 } } } };

    const auto catInfo1 = config.findCategoryInfo(cat1);

    ASSERT_NE(nullptr, catInfo1);
    EXPECT_EQ(disp1, catInfo1->display);
    EXPECT_EQ(size1, catInfo1->size);
}

TEST(ADcsmContentControlConfig, ignoresDupliciteCategoryAdded)
{
    static constexpr Category cat1{ 111 };
    static constexpr displayId_t disp1{ 1 };
    static constexpr displayId_t disp2{ 2 };
    static constexpr SizeInfo size1{ 1, 2 };
    static constexpr SizeInfo size2{ 3, 4 };

    DcsmContentControlConfig config{ { { cat1, { size1, disp1 } } } };
    // passing different category info for same category ID
    EXPECT_NE(StatusOK, config.addCategory(cat1, { size2, disp2 }));

    const auto catInfo1 = config.findCategoryInfo(cat1);

    ASSERT_NE(nullptr, catInfo1);
    EXPECT_EQ(disp1, catInfo1->display);
    EXPECT_EQ(size1, catInfo1->size);
}
