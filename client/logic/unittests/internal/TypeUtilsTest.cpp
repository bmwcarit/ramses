//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internals/TypeUtils.h"

#include "gmock/gmock.h"

#include "LogicNodeDummy.h"

namespace ramses::internal
{
    class ATypeUtils : public ::testing::Test
    {
    protected:
        std::unique_ptr<PropertyImpl> createArrayProperty(size_t size, EPropertyType type)
        {
            auto property = std::make_unique<PropertyImpl>(MakeArray("", size, type), EPropertySemantics::BindingInput);
            property->setLogicNode(dummyNode);

            return property;
        }

        LogicNodeDummyImpl dummyNode{ "DummyNode" };
    };

    TEST_F(ATypeUtils, DistinguishesValidTypeEnumsFromInvalidOnes)
    {
        EXPECT_TRUE(TypeUtils::IsValidType(EPropertyType::Bool));
        EXPECT_TRUE(TypeUtils::IsValidType(EPropertyType::Int32));
        EXPECT_TRUE(TypeUtils::IsValidType(EPropertyType::Int64));
        EXPECT_TRUE(TypeUtils::IsValidType(EPropertyType::Float));
        EXPECT_TRUE(TypeUtils::IsValidType(EPropertyType::Vec2i));
        EXPECT_TRUE(TypeUtils::IsValidType(EPropertyType::Vec3i));
        EXPECT_TRUE(TypeUtils::IsValidType(EPropertyType::Vec4i));
        EXPECT_TRUE(TypeUtils::IsValidType(EPropertyType::Vec2f));
        EXPECT_TRUE(TypeUtils::IsValidType(EPropertyType::Vec3f));
        EXPECT_TRUE(TypeUtils::IsValidType(EPropertyType::Vec4f));
        EXPECT_TRUE(TypeUtils::IsValidType(EPropertyType::String));
        EXPECT_TRUE(TypeUtils::IsValidType(EPropertyType::Struct));
        EXPECT_TRUE(TypeUtils::IsValidType(EPropertyType::Array));

        auto invalidType(static_cast<EPropertyType>(10000));
        EXPECT_FALSE(TypeUtils::IsValidType(invalidType));
    }

    TEST_F(ATypeUtils, ReportsPropertyTypeTraits)
    {
        EXPECT_TRUE(TypeUtils::IsPrimitiveType(EPropertyType::Bool));
        EXPECT_TRUE(TypeUtils::IsPrimitiveType(EPropertyType::Int32));
        EXPECT_TRUE(TypeUtils::IsPrimitiveType(EPropertyType::Int64));
        EXPECT_TRUE(TypeUtils::IsPrimitiveType(EPropertyType::Float));
        EXPECT_TRUE(TypeUtils::IsPrimitiveType(EPropertyType::Vec2i));
        EXPECT_TRUE(TypeUtils::IsPrimitiveType(EPropertyType::Vec4f));
        EXPECT_TRUE(TypeUtils::IsPrimitiveType(EPropertyType::String));

        EXPECT_FALSE(TypeUtils::IsPrimitiveType(EPropertyType::Struct));
        EXPECT_FALSE(TypeUtils::IsPrimitiveType(EPropertyType::Array));

        EXPECT_TRUE(TypeUtils::CanHaveChildren(EPropertyType::Struct));
        EXPECT_TRUE(TypeUtils::CanHaveChildren(EPropertyType::Array));

        EXPECT_FALSE(TypeUtils::CanHaveChildren(EPropertyType::Bool));
        EXPECT_FALSE(TypeUtils::CanHaveChildren(EPropertyType::Vec2i));
    }

    TEST_F(ATypeUtils, DsitinguishesBetweenVectorAndNonVectorTypes)
    {
        EXPECT_TRUE(TypeUtils::IsPrimitiveVectorType(EPropertyType::Vec2i));
        EXPECT_TRUE(TypeUtils::IsPrimitiveVectorType(EPropertyType::Vec3i));
        EXPECT_TRUE(TypeUtils::IsPrimitiveVectorType(EPropertyType::Vec4i));
        EXPECT_TRUE(TypeUtils::IsPrimitiveVectorType(EPropertyType::Vec2f));
        EXPECT_TRUE(TypeUtils::IsPrimitiveVectorType(EPropertyType::Vec3f));
        EXPECT_TRUE(TypeUtils::IsPrimitiveVectorType(EPropertyType::Vec4f));

        EXPECT_FALSE(TypeUtils::IsPrimitiveVectorType(EPropertyType::Bool));
        EXPECT_FALSE(TypeUtils::IsPrimitiveVectorType(EPropertyType::Int32));
        EXPECT_FALSE(TypeUtils::IsPrimitiveVectorType(EPropertyType::Int64));
        EXPECT_FALSE(TypeUtils::IsPrimitiveVectorType(EPropertyType::Float));
        EXPECT_FALSE(TypeUtils::IsPrimitiveVectorType(EPropertyType::String));
        EXPECT_FALSE(TypeUtils::IsPrimitiveVectorType(EPropertyType::Struct));
        EXPECT_FALSE(TypeUtils::IsPrimitiveVectorType(EPropertyType::Array));
    }
}
