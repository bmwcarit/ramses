//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"

#include "internals/TypeData.h"

namespace rlogic::internal
{
    class ATypeData : public ::testing::Test
    {
    };

    TEST_F(ATypeData, IsEqualToItself)
    {
        const TypeData data("name", EPropertyType::Bool);
        EXPECT_EQ(data, data);
    }

    TEST_F(ATypeData, IsNotEqualToAnotherTypeWithDifferentName)
    {
        const TypeData data1("name", EPropertyType::Int32);
        const TypeData data2("name2", EPropertyType::Int32);
        EXPECT_NE(data1, data2);
    }

    TEST_F(ATypeData, IsNotEqualToAnotherTypeWithDifferentType)
    {
        const TypeData data1("name", EPropertyType::Int32);
        const TypeData data2("name", EPropertyType::Bool);
        EXPECT_NE(data1, data2);
    }

    class AHierarchicalTypeData : public ::testing::Test
    {
    protected:

        HierarchicalTypeData m_data{
            TypeData("root", EPropertyType::Struct),
            {
                HierarchicalTypeData(TypeData("bool", EPropertyType::Bool),{}),
                HierarchicalTypeData(TypeData("array", EPropertyType::Array),{}),
            }};
    };

    TEST_F(AHierarchicalTypeData, IsEqualToItself)
    {
        EXPECT_EQ(m_data, m_data);
    }

    TEST_F(AHierarchicalTypeData, IsNotEqualToAnotherTypeWithDifferentName)
    {
        HierarchicalTypeData data1{ TypeData("name", EPropertyType::Struct), {} };
        HierarchicalTypeData data2{ TypeData("otherName", EPropertyType::Struct), {} };
        EXPECT_NE(data1, data2);
    }

    TEST_F(AHierarchicalTypeData, IsNotEqualToAnotherTypeWithDifferentType)
    {
        HierarchicalTypeData data1{ TypeData("name", EPropertyType::Struct), {} };
        HierarchicalTypeData data2{ TypeData("name", EPropertyType::Array), {} };
        EXPECT_NE(data1, data2);
    }

    TEST_F(AHierarchicalTypeData, IsNotEqualToAnotherTypeWithDifferentChildren)
    {
        HierarchicalTypeData noChildren{ TypeData("root", EPropertyType::Struct), std::vector<HierarchicalTypeData>() };
        HierarchicalTypeData moreChildren{ TypeData("root", EPropertyType::Struct), std::vector<HierarchicalTypeData>(3, HierarchicalTypeData(TypeData("bool", EPropertyType::Bool),{})) };
        HierarchicalTypeData fewerChildren{ TypeData("root", EPropertyType::Struct), std::vector<HierarchicalTypeData>(1, HierarchicalTypeData(TypeData("bool", EPropertyType::Bool),{})) };
        EXPECT_NE(m_data, noChildren);
        EXPECT_NE(m_data, moreChildren);
        EXPECT_NE(m_data, fewerChildren);
    }

    TEST_F(AHierarchicalTypeData, IsNotEqualToAnotherTypeWithAdditionalSubchildren)
    {
        HierarchicalTypeData moreSubchildren{
            TypeData("root", EPropertyType::Struct),
            {
                HierarchicalTypeData(TypeData("bool", EPropertyType::Bool),{}),
                HierarchicalTypeData(TypeData("array", EPropertyType::Array),
                    {
                        HierarchicalTypeData(TypeData("subchild", EPropertyType::Bool), {}),
                    }),
        } };
        EXPECT_NE(m_data, moreSubchildren);
    }

    TEST(StaticDataTypeHelper, CreatesPrimitiveType)
    {
        const HierarchicalTypeData expected{
            TypeData("name", EPropertyType::Vec4i), {}};
        EXPECT_EQ(expected, MakeType("name", EPropertyType::Vec4i));
    }

    TEST(StaticDataTypeHelper, CreatesArray)
    {
        const HierarchicalTypeData expected{
            TypeData("array", EPropertyType::Array),
            {
                HierarchicalTypeData(TypeData("", EPropertyType::Vec2f),{}),
                HierarchicalTypeData(TypeData("", EPropertyType::Vec2f),{})
        } };
        EXPECT_EQ(expected, MakeArray("array", 2, EPropertyType::Vec2f));
    }

    TEST(StaticDataTypeHelper, CreatesStruct)
    {
        const HierarchicalTypeData expected{
            TypeData("struct", EPropertyType::Struct),
            {
                HierarchicalTypeData(TypeData("a", EPropertyType::Float),{}),
                HierarchicalTypeData(TypeData("b", EPropertyType::String),{})
        } };
        EXPECT_EQ(expected, MakeStruct("struct", {{"a", EPropertyType::Float}, {"b", EPropertyType::String}}));
    }
}
