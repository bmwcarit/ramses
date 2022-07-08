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
#include "ramses-framework-api/DcsmApiTypes.h"

namespace ramses
{
    TEST(ACategoryInfoUpdate, constructionAndEquality)
    {
        CategoryInfoUpdate defaultConstructed;
        CategoryInfoUpdate setAfterward;
        CategoryInfoUpdate constructorZero(SizeInfo{ 0, 0 }, Rect{0, 0, 0, 0}, Rect{0, 0, 0, 0}, CategoryInfoUpdate::Layout::Drive);
        CategoryInfoUpdate constructorAllSizes(SizeInfo{ 4, 5 }, Rect{0, 0, 4, 5}, Rect{1, 2, 3, 4}, CategoryInfoUpdate::Layout::Drive);
        CategoryInfoUpdate constructorWithDefaultForSafeRect(SizeInfo{ 4, 5 }, Rect{0, 0, 4, 5});
        CategoryInfoUpdate constructorSafeRectSetToZero(SizeInfo{ 4, 5 }, Rect{0, 0, 4, 5}, Rect{0, 0, 0, 0}, CategoryInfoUpdate::Layout::Drive);

        EXPECT_FALSE(constructorZero == defaultConstructed);
        EXPECT_FALSE(constructorZero == constructorWithDefaultForSafeRect);
        EXPECT_TRUE(defaultConstructed == setAfterward);

        EXPECT_FALSE(constructorWithDefaultForSafeRect == constructorSafeRectSetToZero);

        EXPECT_FALSE(constructorAllSizes == defaultConstructed);
        EXPECT_FALSE(constructorAllSizes == constructorWithDefaultForSafeRect);
        EXPECT_FALSE(constructorAllSizes == constructorSafeRectSetToZero);
        EXPECT_FALSE(constructorAllSizes == setAfterward);

        setAfterward.setRenderSize({ 4, 5 });
        setAfterward.setCategoryRect({ 0, 0, 4, 5 });
        setAfterward.setSafeRect({ 1, 2, 3, 4 });
        setAfterward.setActiveLayout(CategoryInfoUpdate::Layout::Drive);
        EXPECT_TRUE(constructorAllSizes == setAfterward);
    }

    TEST(ACategoryInfoUpdate, emptyObjectHasNoValues)
    {
        CategoryInfoUpdate update;
        EXPECT_FALSE(update.hasCategoryRectUpdate());
        EXPECT_FALSE(update.hasRenderSizeUpdate());
        EXPECT_FALSE(update.hasSafeRectUpdate());
        //defaults
        EXPECT_EQ(0u, update.getCategoryRect().x);
        EXPECT_EQ(0u, update.getCategoryRect().y);
        EXPECT_EQ(0u, update.getCategoryRect().width);
        EXPECT_EQ(0u, update.getCategoryRect().height);
        EXPECT_EQ(0u, update.getRenderSize().width);
        EXPECT_EQ(0u, update.getRenderSize().height);
        EXPECT_EQ(0u, update.getSafeRect().x);
        EXPECT_EQ(0u, update.getSafeRect().y);
        EXPECT_EQ(0u, update.getSafeRect().width);
        EXPECT_EQ(0u, update.getSafeRect().height);
        EXPECT_EQ(CategoryInfoUpdate::Layout::Drive, update.getActiveLayout());
    }

    TEST(ACategoryInfoUpdate, constructorOnlySetsUserProvidedValues)
    {
        const SizeInfo renderSize(1, 2);
        const Rect categoryRect(1, 2, 3, 4);
        const Rect safeRect(5, 6, 7, 8);
        const CategoryInfoUpdate::Layout layout = CategoryInfoUpdate::Layout::Gallery;

        {
            CategoryInfoUpdate ciu;
            EXPECT_FALSE(ciu.hasRenderSizeUpdate());
            EXPECT_FALSE(ciu.hasCategoryRectUpdate());
            EXPECT_FALSE(ciu.hasSafeRectUpdate());
            EXPECT_FALSE(ciu.hasActiveLayoutUpdate());
        }
        {
            CategoryInfoUpdate ciu(renderSize, categoryRect);
            EXPECT_TRUE(ciu.hasRenderSizeUpdate());
            EXPECT_TRUE(ciu.hasCategoryRectUpdate());
            EXPECT_FALSE(ciu.hasSafeRectUpdate());
            EXPECT_FALSE(ciu.hasActiveLayoutUpdate());

            EXPECT_EQ(renderSize, ciu.getRenderSize());
            EXPECT_EQ(categoryRect, ciu.getCategoryRect());
        }
        {
            CategoryInfoUpdate ciu(renderSize, categoryRect, safeRect);
            EXPECT_TRUE(ciu.hasRenderSizeUpdate());
            EXPECT_TRUE(ciu.hasCategoryRectUpdate());
            EXPECT_TRUE(ciu.hasSafeRectUpdate());
            EXPECT_FALSE(ciu.hasActiveLayoutUpdate());

            EXPECT_EQ(renderSize, ciu.getRenderSize());
            EXPECT_EQ(categoryRect, ciu.getCategoryRect());
            EXPECT_EQ(safeRect, ciu.getSafeRect());
        }
        {
            CategoryInfoUpdate ciu(renderSize, categoryRect, safeRect, layout);
            EXPECT_TRUE(ciu.hasRenderSizeUpdate());
            EXPECT_TRUE(ciu.hasCategoryRectUpdate());
            EXPECT_TRUE(ciu.hasSafeRectUpdate());
            EXPECT_TRUE(ciu.hasActiveLayoutUpdate());

            EXPECT_EQ(renderSize, ciu.getRenderSize());
            EXPECT_EQ(categoryRect, ciu.getCategoryRect());
            EXPECT_EQ(safeRect, ciu.getSafeRect());
            EXPECT_EQ(layout, ciu.getActiveLayout());
        }
    }

    TEST(ACategoryInfoUpdate, setCategoryRect)
    {
        CategoryInfoUpdate update;

        EXPECT_FALSE(update.hasCategoryRectUpdate());
        update.setCategoryRect({1,2,3,5});
        EXPECT_TRUE(update.hasCategoryRectUpdate());
        EXPECT_EQ(1u, update.getCategoryRect().x);
        EXPECT_EQ(2u, update.getCategoryRect().y);
        EXPECT_EQ(3u, update.getCategoryRect().width);
        EXPECT_EQ(5u, update.getCategoryRect().height);
    }

    TEST(ACategoryInfoUpdate, setSafeRect)
    {
        CategoryInfoUpdate update;

        EXPECT_FALSE(update.hasSafeRectUpdate());
        update.setSafeRect({ 1,2,3,5 });
        EXPECT_TRUE(update.hasSafeRectUpdate());
        EXPECT_EQ(1u, update.getSafeRect().x);
        EXPECT_EQ(2u, update.getSafeRect().y);
        EXPECT_EQ(3u, update.getSafeRect().width);
        EXPECT_EQ(5u, update.getSafeRect().height);
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

    TEST(ACategoryInfoUpdate, setActiveLayout)
    {
        CategoryInfoUpdate update;

        EXPECT_FALSE(update.hasActiveLayoutUpdate());
        update.setActiveLayout(CategoryInfoUpdate::Layout::Sport_Road);
        EXPECT_TRUE(update.hasActiveLayoutUpdate());
        EXPECT_EQ(CategoryInfoUpdate::Layout::Sport_Road, update.getActiveLayout());
    }

    TEST(ACategoryInfoUpdate, setCategoryRectMultipleTimesUpdatesValues)
    {
        CategoryInfoUpdate update;
        update.setCategoryRect({1, 2, 3, 5});
        update.setCategoryRect({3, 4, 13, 15});
        EXPECT_TRUE(update.hasCategoryRectUpdate());
        EXPECT_EQ(3u, update.getCategoryRect().x);
        EXPECT_EQ(4u, update.getCategoryRect().y);
        EXPECT_EQ(13u, update.getCategoryRect().width);
        EXPECT_EQ(15u, update.getCategoryRect().height);
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

    TEST(ACategoryInfoUpdate, setSafeRectMultipleTimesUpdatesValues)
    {
        CategoryInfoUpdate update;
        update.setSafeRect({ 1, 2, 3, 5 });
        update.setSafeRect({ 3, 4, 13, 15 });
        EXPECT_TRUE(update.hasSafeRectUpdate());
        EXPECT_EQ(3u,  update.getSafeRect().x);
        EXPECT_EQ(4u,  update.getSafeRect().y);
        EXPECT_EQ(13u, update.getSafeRect().width);
        EXPECT_EQ(15u, update.getSafeRect().height);
    }

    TEST(ACategoryInfoUpdate, setActiveLayoutMultipleTimesUpdatesValues)
    {
        CategoryInfoUpdate update;
        update.setActiveLayout(CategoryInfoUpdate::Layout::Sport_Road);
        update.setActiveLayout(CategoryInfoUpdate::Layout::Gallery);
        EXPECT_TRUE(update.hasActiveLayoutUpdate());
        EXPECT_EQ(CategoryInfoUpdate::Layout::Gallery, update.getActiveLayout());
    }

    TEST(ACategoryInfoUpdate, AssignFromInternalObject)
    {
        ramses_internal::CategoryInfo info;
        ramses::CategoryInfoUpdate update;

        update.impl.setCategoryInfo(info);
        EXPECT_FALSE(update.hasCategoryRectUpdate());
        EXPECT_FALSE(update.hasRenderSizeUpdate());
        EXPECT_FALSE(update.hasSafeRectUpdate());
        EXPECT_FALSE(update.hasActiveLayoutUpdate());

        info.setCategoryRect(1, 2, 3, 4);
        info.setRenderSize(11,22);
        info.setSafeRect(5,6,7,8);
        info.setActiveLayout(ramses_internal::CategoryInfo::Layout::Autonomous);
        update.impl.setCategoryInfo(info);
        EXPECT_TRUE(update.hasCategoryRectUpdate());
        EXPECT_TRUE(update.hasSafeRectUpdate());
        EXPECT_TRUE(update.hasRenderSizeUpdate());
        EXPECT_TRUE(update.hasActiveLayoutUpdate());
        EXPECT_EQ(1u, update.getCategoryRect().x);
        EXPECT_EQ(2u, update.getCategoryRect().y);
        EXPECT_EQ(3u, update.getCategoryRect().width);
        EXPECT_EQ(4u, update.getCategoryRect().height);
        EXPECT_EQ(11u, update.getRenderSize().width);
        EXPECT_EQ(22u, update.getRenderSize().height);
        EXPECT_EQ(5u, update.getSafeRect().x);
        EXPECT_EQ(6u, update.getSafeRect().y);
        EXPECT_EQ(7u, update.getSafeRect().width);
        EXPECT_EQ(8u, update.getSafeRect().height);
        EXPECT_EQ(CategoryInfoUpdate::Layout::Autonomous, update.getActiveLayout());
    }
}
