//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "ramses-framework-api/CategoryInfoUpdate.h"
#include "Components/CategoryInfo.h"
#include "CategoryInfoUpdateImpl.h"

namespace ramses
{
    TEST(ACategoryInfoUpdate, constructionAndEquality)
    {
        CategoryInfoUpdate defaultConstructed;
        CategoryInfoUpdate explicitConstuctor(SizeInfo(4,5));
        CategoryInfoUpdate explicitConstuctorZero(SizeInfo(0, 0));
        CategoryInfoUpdate setAfterward;

        EXPECT_FALSE(explicitConstuctorZero == defaultConstructed);

        EXPECT_TRUE(defaultConstructed == setAfterward);
        EXPECT_FALSE(explicitConstuctor == defaultConstructed);
        EXPECT_FALSE(explicitConstuctor == setAfterward);

        setAfterward.setCategorySize({0, 0, 4,5});
        EXPECT_TRUE(explicitConstuctor == setAfterward);
    }

    TEST(ACategoryInfoUpdate, emptyObjectHasNoValues)
    {
        CategoryInfoUpdate update;
        EXPECT_FALSE(update.hasCategorySizeUpdate());
        EXPECT_FALSE(update.hasRenderSizeUpdate());
        EXPECT_FALSE(update.hasSafeAreaSizeUpdate());
        //defaults
        EXPECT_EQ(0u, update.getCategorySize().x);
        EXPECT_EQ(0u, update.getCategorySize().y);
        EXPECT_EQ(0u, update.getCategorySize().width);
        EXPECT_EQ(0u, update.getCategorySize().height);
        EXPECT_EQ(0u, update.getRenderSize().width);
        EXPECT_EQ(0u, update.getRenderSize().height);
        EXPECT_EQ(0u, update.getSafeAreaSize().x);
        EXPECT_EQ(0u, update.getSafeAreaSize().y);
        EXPECT_EQ(0u, update.getSafeAreaSize().width);
        EXPECT_EQ(0u, update.getSafeAreaSize().height);
    }

    TEST(ACategoryInfoUpdate, setCategorySize)
    {
        CategoryInfoUpdate update;

        EXPECT_FALSE(update.hasCategorySizeUpdate());
        update.setCategorySize({1,2,3,5});
        EXPECT_TRUE(update.hasCategorySizeUpdate());
        EXPECT_EQ(1u, update.getCategorySize().x);
        EXPECT_EQ(2u, update.getCategorySize().y);
        EXPECT_EQ(3u, update.getCategorySize().width);
        EXPECT_EQ(5u, update.getCategorySize().height);
    }

    TEST(ACategoryInfoUpdate, setSafeArea)
    {
        CategoryInfoUpdate update;

        EXPECT_FALSE(update.hasSafeAreaSizeUpdate());
        update.setSafeAreaSize({ 1,2,3,5 });
        EXPECT_TRUE(update.hasSafeAreaSizeUpdate());
        EXPECT_EQ(1u, update.getSafeAreaSize().x);
        EXPECT_EQ(2u, update.getSafeAreaSize().y);
        EXPECT_EQ(3u, update.getSafeAreaSize().width);
        EXPECT_EQ(5u, update.getSafeAreaSize().height);
    }

    TEST(ACategoryInfoUpdate, setRenderSize)
    {
        CategoryInfoUpdate update;

        EXPECT_FALSE(update.hasRenderSizeUpdate());
        update.setRenderSize({ 1,2});
        EXPECT_TRUE(update.hasRenderSizeUpdate());
        EXPECT_EQ(1u, update.getRenderSize().width);
        EXPECT_EQ(2u, update.getRenderSize().height);
    }

    TEST(ACategoryInfoUpdate, setCategorySizeMultipleTimesUpdatesValues)
    {
        CategoryInfoUpdate update;
        update.setCategorySize({1, 2, 3, 5});
        update.setCategorySize({3, 4, 13, 15});
        EXPECT_TRUE(update.hasCategorySizeUpdate());
        EXPECT_EQ(3u, update.getCategorySize().x);
        EXPECT_EQ(4u, update.getCategorySize().y);
        EXPECT_EQ(13u, update.getCategorySize().width);
        EXPECT_EQ(15u, update.getCategorySize().height);
    }

    TEST(ACategoryInfoUpdate, setRenderSizeMultipleTimesUpdatesValues)
    {
        CategoryInfoUpdate update;
        update.setRenderSize({ 1, 2});
        update.setRenderSize({ 3, 4});
        EXPECT_TRUE(update.hasRenderSizeUpdate());
        EXPECT_EQ(3u, update.getRenderSize().width);
        EXPECT_EQ(4u, update.getRenderSize().height);
    }

    TEST(ACategoryInfoUpdate, setSafeAreaSizeMultipleTimesUpdatesValues)
    {
        CategoryInfoUpdate update;
        update.setSafeAreaSize({ 1, 2, 3, 5 });
        update.setSafeAreaSize({ 3, 4, 13, 15 });
        EXPECT_TRUE(update.hasSafeAreaSizeUpdate());
        EXPECT_EQ(3u,  update.getSafeAreaSize().x);
        EXPECT_EQ(4u,  update.getSafeAreaSize().y);
        EXPECT_EQ(13u, update.getSafeAreaSize().width);
        EXPECT_EQ(15u, update.getSafeAreaSize().height);
    }

    TEST(ACategoryInfoUpdate, AssignFromInternalObject)
    {
        ramses_internal::CategoryInfo info;
        ramses::CategoryInfoUpdate update;

        update.impl.setCategoryInfo(info);
        EXPECT_FALSE(update.hasCategorySizeUpdate());
        EXPECT_FALSE(update.hasRenderSizeUpdate());
        EXPECT_FALSE(update.hasSafeAreaSizeUpdate());

        info.setCategorySize(1, 2, 3, 4);
        info.setRenderSize(11,22);
        info.setSafeArea(5,6,7,8);
        update.impl.setCategoryInfo(info);
        EXPECT_TRUE(update.hasCategorySizeUpdate());
        EXPECT_TRUE(update.hasSafeAreaSizeUpdate());
        EXPECT_TRUE(update.hasRenderSizeUpdate());
        EXPECT_EQ(1u, update.getCategorySize().x);
        EXPECT_EQ(2u, update.getCategorySize().y);
        EXPECT_EQ(3u, update.getCategorySize().width);
        EXPECT_EQ(4u, update.getCategorySize().height);
        EXPECT_EQ(11u, update.getRenderSize().width);
        EXPECT_EQ(22u, update.getRenderSize().height);
        EXPECT_EQ(5u, update.getSafeAreaSize().x);
        EXPECT_EQ(6u, update.getSafeAreaSize().y);
        EXPECT_EQ(7u, update.getSafeAreaSize().width);
        EXPECT_EQ(8u, update.getSafeAreaSize().height);
    }
}
