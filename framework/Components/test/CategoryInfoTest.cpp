//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "Components/DcsmTypes.h"
#include "Components/CategoryInfo.h"
#include "Utils/BinaryOutputStream.h"

namespace ramses_internal
{
    class ACategoryInfo : public testing::Test
    {
    public:
        ACategoryInfo()
        {
            filled.setCategorySize(12, 34, 56, 78);
            filled.setRenderSize(88,99);
            filled.setSafeArea(4,3,2,1);
        }

        CategoryInfo serializeDeserialize(const CategoryInfo& ref)
        {
            const auto vec = ref.toBinary();
            EXPECT_TRUE(vec.size() > 0);
            return CategoryInfo(vec);
        }

        CategoryInfo filled;
        CategoryInfo empty;
    };

    TEST(CategoryInfo, hasComparison)
    {
        CategoryInfo defaultConstructed;
        CategoryInfo explicitZero(0u, 0u);
        CategoryInfo explicitValuesSet;
        explicitValuesSet.setCategorySize(0, 0, 3, 4);
        CategoryInfo explicitValuesConstructor{ 3, 4 };

        EXPECT_FALSE(defaultConstructed == explicitZero); // zero 'has value'
        EXPECT_TRUE(defaultConstructed != explicitZero);
        EXPECT_TRUE(explicitValuesSet == explicitValuesConstructor);
        EXPECT_FALSE(explicitValuesSet != explicitValuesConstructor);

        explicitValuesSet.setCategorySize(1, 2, 3, 4);
        EXPECT_FALSE(explicitValuesSet == explicitValuesConstructor);
    }

    TEST(CategoryInfo, defaultValues)
    {
        CategoryInfo value;
        EXPECT_EQ(0u, value.getCategoryX());
        EXPECT_EQ(0u, value.getCategoryY());
        EXPECT_EQ(0u, value.getCategoryWidth());
        EXPECT_EQ(0u, value.getCategoryHeight());
        EXPECT_FALSE(value.hasCategorySizeChange());
        EXPECT_EQ(0u, value.getSafeAreaX());
        EXPECT_EQ(0u, value.getSafeAreaY());
        EXPECT_EQ(0u, value.getSafeAreaWidth());
        EXPECT_EQ(0u, value.getSafeAreaHeight());
        EXPECT_FALSE(value.hasSafeAreaSizeChange());
        EXPECT_EQ(0u, value.getRenderSizeWidth());
        EXPECT_EQ(0u, value.getRenderSizeHeight());
        EXPECT_FALSE(value.hasRenderSizeChange());
    }

    TEST(CategoryInfo, setCategorySize)
    {
        CategoryInfo value;
        EXPECT_FALSE(value.hasCategorySizeChange());

        value.setCategorySize(1, 2, 3, 4);
        EXPECT_TRUE(value.hasCategorySizeChange());
        EXPECT_EQ(1u, value.getCategoryX());
        EXPECT_EQ(2u, value.getCategoryY());
        EXPECT_EQ(3u, value.getCategoryWidth());
        EXPECT_EQ(4u, value.getCategoryHeight());
    }

    TEST(CategoryInfo, setSafeAreaSize)
    {
        CategoryInfo value;
        EXPECT_FALSE(value.hasSafeAreaSizeChange());

        value.setSafeArea(1, 2, 3, 4);
        EXPECT_TRUE(value.hasSafeAreaSizeChange());
        EXPECT_EQ(1u, value.getSafeAreaX());
        EXPECT_EQ(2u, value.getSafeAreaY());
        EXPECT_EQ(3u, value.getSafeAreaWidth());
        EXPECT_EQ(4u, value.getSafeAreaHeight());
    }

    TEST(CategoryInfo, setRenderSize)
    {
        CategoryInfo value;
        EXPECT_FALSE(value.hasRenderSizeChange());

        value.setRenderSize(1, 2);
        EXPECT_TRUE(value.hasRenderSizeChange());
        EXPECT_EQ(1u, value.getRenderSizeWidth());
        EXPECT_EQ(2u, value.getRenderSizeHeight());
    }

    TEST_F(ACategoryInfo, canCopyConstruct)
    {
        CategoryInfo emptyCopy(empty);
        EXPECT_EQ(empty, emptyCopy);

        CategoryInfo withSizeCopy(filled);
        EXPECT_EQ(filled, withSizeCopy);
    }

    TEST_F(ACategoryInfo, canCopyAssign)
    {
        CategoryInfo emptyCopy;
        emptyCopy = empty;
        EXPECT_EQ(empty, emptyCopy);

        CategoryInfo withSizeCopy;
        withSizeCopy = filled;
        EXPECT_EQ(filled, withSizeCopy);
    }

    TEST_F(ACategoryInfo, canMoveContruct)
    {
        CategoryInfo emptyCopy(empty);
        CategoryInfo emptyMoved(std::move(emptyCopy));
        EXPECT_EQ(empty, emptyMoved);

        CategoryInfo filledCopy(filled);
        CategoryInfo filledMoved(std::move(filledCopy));
        EXPECT_EQ(filled, filledMoved);
    }

    TEST_F(ACategoryInfo, canMoveAssign)
    {
        CategoryInfo emptyCopy(empty);
        CategoryInfo emptyMoved;
        emptyMoved = std::move(emptyCopy);
        EXPECT_EQ(empty, emptyMoved);

        CategoryInfo filledCopy(filled);
        CategoryInfo filledMoved;
        filledMoved = std::move(filledCopy);
        EXPECT_EQ(filled, filledMoved);
    }

    TEST_F(ACategoryInfo, canSerializeDeserializeEmpty)
    {
        EXPECT_EQ(empty, serializeDeserialize(empty));
    }

    TEST_F(ACategoryInfo, canSerializeDeserializeFilled)
    {
        EXPECT_EQ(filled, serializeDeserialize(filled));
    }

    TEST_F(ACategoryInfo, canSerializeDeserializeSomeSet)
    {
        CategoryInfo ci;
        ci.setCategorySize(4,3,2,1);
        ci.setRenderSize(5,6);
        ci.setSafeArea(7,8,9,0);
        EXPECT_EQ(ci, serializeDeserialize(ci));
    }

    TEST_F(ACategoryInfo, canSkipDeserializeUnknownTypes)
    {
        BinaryOutputStreamT<Byte> os;
        os << static_cast<uint32_t>(1) // version
            << static_cast<uint32_t>(2) // entries

            << static_cast<uint32_t>(55) // unknown type
            << static_cast<uint32_t>(16) // unknown size
            << static_cast<uint64_t>(2) // unknown data
            << static_cast<uint64_t>(3)

            << static_cast<uint32_t>(1) // category size change type
            << static_cast<uint32_t>(2) // x
            << static_cast<uint32_t>(2) // y
            << static_cast<uint32_t>(2) // size
            << static_cast<uint32_t>(2) // width
            << static_cast<uint32_t>(123); // height

        CategoryInfo ci(os.release());
        EXPECT_TRUE(ci.hasCategorySizeChange());
        EXPECT_EQ(2u, ci.getCategoryWidth());
        EXPECT_EQ(123u, ci.getCategoryHeight());
    }

    TEST_F(ACategoryInfo, ignoresUnexpectedCategoryInfoDataVersion)
    {
        BinaryOutputStreamT<Byte> os;
        os << static_cast<uint32_t>(100) // unsupported version
            << static_cast<uint32_t>(1) // entries

            << static_cast<uint32_t>(1) // size change type
            << static_cast<uint32_t>(2) // x
            << static_cast<uint32_t>(2) // y
            << static_cast<uint32_t>(2) // width
            << static_cast<uint8_t>(123); // height
        CategoryInfo ci(os.release());
        EXPECT_FALSE(ci.hasCategorySizeChange());
    }

}
