//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"

#include "ramses-logic/EPropertyType.h"

namespace rlogic
{
    TEST(PropertyTypeToEnumTypeTrait, ConvertsSupportedTypesToCorrectEnum)
    {
        constexpr EPropertyType floatType = PropertyTypeToEnum<float>::TYPE;
        constexpr EPropertyType vec2fType = PropertyTypeToEnum<vec2f>::TYPE;
        constexpr EPropertyType vec3fType = PropertyTypeToEnum<vec3f>::TYPE;
        constexpr EPropertyType vec4fType = PropertyTypeToEnum<vec4f>::TYPE;
        constexpr EPropertyType int32Type = PropertyTypeToEnum<int32_t>::TYPE;
        constexpr EPropertyType int64Type = PropertyTypeToEnum<int64_t>::TYPE;
        constexpr EPropertyType vec2iType = PropertyTypeToEnum<vec2i>::TYPE;
        constexpr EPropertyType vec3iType = PropertyTypeToEnum<vec3i>::TYPE;
        constexpr EPropertyType vec4iType = PropertyTypeToEnum<vec4i>::TYPE;
        constexpr EPropertyType boolType = PropertyTypeToEnum<bool>::TYPE;
        constexpr EPropertyType stringType = PropertyTypeToEnum<std::string>::TYPE;
        constexpr EPropertyType arrayType = PropertyTypeToEnum<std::vector<float>>::TYPE;
        EXPECT_EQ(floatType, EPropertyType::Float);
        EXPECT_EQ(vec2fType, EPropertyType::Vec2f);
        EXPECT_EQ(vec3fType, EPropertyType::Vec3f);
        EXPECT_EQ(vec4fType, EPropertyType::Vec4f);
        EXPECT_EQ(int32Type, EPropertyType::Int32);
        EXPECT_EQ(int64Type, EPropertyType::Int64);
        EXPECT_EQ(vec2iType, EPropertyType::Vec2i);
        EXPECT_EQ(vec3iType, EPropertyType::Vec3i);
        EXPECT_EQ(vec4iType, EPropertyType::Vec4i);
        EXPECT_EQ(boolType, EPropertyType::Bool);
        EXPECT_EQ(stringType, EPropertyType::String);
        EXPECT_EQ(arrayType, EPropertyType::Array);
    }

    TEST(PropertyTypeToEnumTypeTrait, ConvertsPropertyEnumToType)
    {
        static_assert(std::is_same_v<int32_t, PropertyEnumToType<EPropertyType::Int32>::TYPE>, "wrong traits");
        static_assert(std::is_same_v<int64_t, PropertyEnumToType<EPropertyType::Int64>::TYPE>, "wrong traits");
        static_assert(std::is_same_v<float, PropertyEnumToType<EPropertyType::Float>::TYPE>, "wrong traits");
        static_assert(std::is_same_v<vec2f, PropertyEnumToType<EPropertyType::Vec2f>::TYPE>, "wrong traits");
        static_assert(std::is_same_v<vec3f, PropertyEnumToType<EPropertyType::Vec3f>::TYPE>, "wrong traits");
        static_assert(std::is_same_v<vec4f, PropertyEnumToType<EPropertyType::Vec4f>::TYPE>, "wrong traits");
        static_assert(std::is_same_v<vec2i, PropertyEnumToType<EPropertyType::Vec2i>::TYPE>, "wrong traits");
        static_assert(std::is_same_v<vec3i, PropertyEnumToType<EPropertyType::Vec3i>::TYPE>, "wrong traits");
        static_assert(std::is_same_v<vec4i, PropertyEnumToType<EPropertyType::Vec4i>::TYPE>, "wrong traits");
        static_assert(std::is_same_v<std::string, PropertyEnumToType<EPropertyType::String>::TYPE>, "wrong traits");
        static_assert(std::is_same_v<bool, PropertyEnumToType<EPropertyType::Bool>::TYPE>, "wrong traits");
        static_assert(std::is_same_v<std::vector<float>, PropertyEnumToType<EPropertyType::Array>::TYPE>, "wrong traits");
    }

    TEST(IsPrimitivePropertyTypeTrait, IsTrueOnlyForPrimitiveProperties)
    {
        static_assert(IsPrimitiveProperty<float>::value, "wrong traits");
        static_assert(IsPrimitiveProperty<vec2f>::value, "wrong traits");
        static_assert(IsPrimitiveProperty<vec3f>::value, "wrong traits");
        static_assert(IsPrimitiveProperty<vec4f>::value, "wrong traits");
        static_assert(IsPrimitiveProperty<int32_t>::value, "wrong traits");
        static_assert(IsPrimitiveProperty<int64_t>::value, "wrong traits");
        static_assert(IsPrimitiveProperty<vec2i>::value, "wrong traits");
        static_assert(IsPrimitiveProperty<vec3i>::value, "wrong traits");
        static_assert(IsPrimitiveProperty<vec4i>::value, "wrong traits");
        static_assert(IsPrimitiveProperty<bool>::value, "wrong traits");
        static_assert(IsPrimitiveProperty<std::string>::value, "wrong traits");

        static_assert(!IsPrimitiveProperty<size_t>::value, "wrong traits");
        static_assert(!IsPrimitiveProperty<std::vector<float>>::value, "wrong traits");
    }

    TEST(GetLuaPrimitiveTypeNameFunction, ProvidesNameForSupportedTypeEnumValues)
    {
        EXPECT_STREQ("Float", GetLuaPrimitiveTypeName(EPropertyType::Float));
        EXPECT_STREQ("Vec2f", GetLuaPrimitiveTypeName(EPropertyType::Vec2f));
        EXPECT_STREQ("Vec3f", GetLuaPrimitiveTypeName(EPropertyType::Vec3f));
        EXPECT_STREQ("Vec4f", GetLuaPrimitiveTypeName(EPropertyType::Vec4f));
        EXPECT_STREQ("Int32", GetLuaPrimitiveTypeName(EPropertyType::Int32));
        EXPECT_STREQ("Int64", GetLuaPrimitiveTypeName(EPropertyType::Int64));
        EXPECT_STREQ("Vec2i", GetLuaPrimitiveTypeName(EPropertyType::Vec2i));
        EXPECT_STREQ("Vec3i", GetLuaPrimitiveTypeName(EPropertyType::Vec3i));
        EXPECT_STREQ("Vec4i", GetLuaPrimitiveTypeName(EPropertyType::Vec4i));
        EXPECT_STREQ("Bool", GetLuaPrimitiveTypeName(EPropertyType::Bool));
        EXPECT_STREQ("String", GetLuaPrimitiveTypeName(EPropertyType::String));
        EXPECT_STREQ("Struct", GetLuaPrimitiveTypeName(EPropertyType::Struct));
        EXPECT_STREQ("Array", GetLuaPrimitiveTypeName(EPropertyType::Array));
    }

    TEST(PropertyTypeCheck, ChecksPropertyTypeToBeStoredInDataArray)
    {
        EXPECT_TRUE(CanPropertyTypeBeStoredInDataArray(EPropertyType::Float));
        EXPECT_TRUE(CanPropertyTypeBeStoredInDataArray(EPropertyType::Vec2f));
        EXPECT_TRUE(CanPropertyTypeBeStoredInDataArray(EPropertyType::Vec3f));
        EXPECT_TRUE(CanPropertyTypeBeStoredInDataArray(EPropertyType::Vec4f));
        EXPECT_TRUE(CanPropertyTypeBeStoredInDataArray(EPropertyType::Int32));
        EXPECT_TRUE(CanPropertyTypeBeStoredInDataArray(EPropertyType::Vec2i));
        EXPECT_TRUE(CanPropertyTypeBeStoredInDataArray(EPropertyType::Vec3i));
        EXPECT_TRUE(CanPropertyTypeBeStoredInDataArray(EPropertyType::Vec4i));
        EXPECT_TRUE(CanPropertyTypeBeStoredInDataArray(EPropertyType::Array));

        EXPECT_FALSE(CanPropertyTypeBeStoredInDataArray(EPropertyType::Bool));
        EXPECT_FALSE(CanPropertyTypeBeStoredInDataArray(EPropertyType::Struct));
        EXPECT_FALSE(CanPropertyTypeBeStoredInDataArray(EPropertyType::String));
        EXPECT_FALSE(CanPropertyTypeBeStoredInDataArray(EPropertyType::Int64));

        auto invalidType(static_cast<EPropertyType>(10000));
        EXPECT_FALSE(CanPropertyTypeBeStoredInDataArray(invalidType));
    }

    TEST(PropertyTypeCheck, ChecksPropertyTypeToBeAnimatable)
    {
        EXPECT_TRUE(CanPropertyTypeBeAnimated(EPropertyType::Float));
        EXPECT_TRUE(CanPropertyTypeBeAnimated(EPropertyType::Vec2f));
        EXPECT_TRUE(CanPropertyTypeBeAnimated(EPropertyType::Vec3f));
        EXPECT_TRUE(CanPropertyTypeBeAnimated(EPropertyType::Vec4f));
        EXPECT_TRUE(CanPropertyTypeBeAnimated(EPropertyType::Int32));
        EXPECT_TRUE(CanPropertyTypeBeAnimated(EPropertyType::Vec2i));
        EXPECT_TRUE(CanPropertyTypeBeAnimated(EPropertyType::Vec3i));
        EXPECT_TRUE(CanPropertyTypeBeAnimated(EPropertyType::Vec4i));
        EXPECT_TRUE(CanPropertyTypeBeAnimated(EPropertyType::Array));

        EXPECT_FALSE(CanPropertyTypeBeAnimated(EPropertyType::Bool));
        EXPECT_FALSE(CanPropertyTypeBeAnimated(EPropertyType::Struct));
        EXPECT_FALSE(CanPropertyTypeBeAnimated(EPropertyType::String));
        EXPECT_FALSE(CanPropertyTypeBeAnimated(EPropertyType::Int64));
    }
}
