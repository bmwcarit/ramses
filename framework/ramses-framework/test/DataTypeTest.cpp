//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses-framework-api/DataTypes.h"
#include "ramses-framework-api/EDataType.h"
#include "SceneAPI/EDataType.h"
#include "DataTypeUtils.h"

namespace ramses
{
    using namespace testing;

    TEST(Datatype, CanBeConstructedFromInitializerList)
    {
        vec2f v2f{ 1.f, 2.f };
        vec3f v3f{ 1.f, 2.f, 3.f };
        vec4f v4f{ 1.f, 2.f, 3.f, 4.f };
        vec2i v2i{ 1, 2 };
        vec3i v3i{ 1, 2, 3 };
        vec4i v4i{ 1, 2, 3, 4 };
        matrix22f m2{ 1.f, 2.f, 3.f, 4.f };
        matrix33f m3{ 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f };
        matrix44f m4{ 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f };
        UNUSED(v2f);
        UNUSED(v3f);
        UNUSED(v4f);
        UNUSED(v2i);
        UNUSED(v3i);
        UNUSED(v4i);
        UNUSED(m2);
        UNUSED(m3);
        UNUSED(m4);
    }

    TEST(Datatype, CanBeConstructedFromStdArray)
    {
        vec2f v2f{ std::array<float, 2>{{ 1.f, 2.f }} };
        vec3f v3f{ std::array<float, 3>{{ 1.f, 2.f, 3.f }} };
        vec4f v4f{ std::array<float, 4>{{ 1.f, 2.f, 3.f, 4.f }} };
        vec2i v2i{ std::array<int32_t, 2>{{ 1, 2 }} };
        vec3i v3i{ std::array<int32_t, 3>{{ 1, 2, 3 }} };
        vec4i v4i{ std::array<int32_t, 4>{{ 1, 2, 3, 4 }} };
        matrix22f m2{ std::array<float, 4>{{ 1.f, 2.f, 3.f, 4.f }} };
        matrix33f m3{ std::array<float, 9>{{ 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f }} };
        matrix44f m4{ std::array<float, 16>{{ 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f }} };
        UNUSED(v2f);
        UNUSED(v3f);
        UNUSED(v4f);
        UNUSED(v2i);
        UNUSED(v3i);
        UNUSED(v4i);
        UNUSED(m2);
        UNUSED(m3);
        UNUSED(m4);
    }

    TEST(Datatype, CanBeCopyConstructed)
    {
        vec2f v2f{ vec2f{ 1.f, 2.f } };
        vec3f v3f{ vec3f{ 1.f, 2.f, 3.f } };
        vec4f v4f{ vec4f{ 1.f, 2.f, 3.f, 4.f } };
        vec2i v2i{ vec2i{ 1, 2 } };
        vec3i v3i{ vec3i{ 1, 2, 3 } };
        vec4i v4i{ vec4i{ 1, 2, 3, 4 } };
        matrix22f m2{ matrix22f{ 1.f, 2.f, 3.f, 4.f } };
        matrix33f m3{ matrix33f{ 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f } };
        matrix44f m4{ matrix44f{ 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f } };
        UNUSED(v2f);
        UNUSED(v3f);
        UNUSED(v4f);
        UNUSED(v2i);
        UNUSED(v3i);
        UNUSED(v4i);
        UNUSED(m2);
        UNUSED(m3);
        UNUSED(m4);
    }

    TEST(Datatype, CanBeCastToStdArray)
    {
        std::array<float, 2>   arrv2f = vec2f{ 1.f, 2.f };
        std::array<float, 3>   arrv3f = vec3f{ 1.f, 2.f, 3.f };
        std::array<float, 4>   arrv4f = vec4f{ 1.f, 2.f, 3.f, 4.f };
        std::array<int32_t, 2> arrv2i = vec2i{ 1, 2 };
        std::array<int32_t, 3> arrv3i = vec3i{ 1, 2, 3 };
        std::array<int32_t, 4> arrv4i = vec4i{ 1, 2, 3, 4 };
        std::array<float, 4>   arrm2 = matrix22f{ 1.f, 2.f, 3.f, 4.f };
        std::array<float, 9>   arrm3 = matrix33f{ 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f };
        std::array<float, 16>  arrm4 = matrix44f{ 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f };
        UNUSED(arrv2f);
        UNUSED(arrv3f);
        UNUSED(arrv4f);
        UNUSED(arrv2i);
        UNUSED(arrv3i);
        UNUSED(arrv4i);
        UNUSED(arrm2);
        UNUSED(arrm3);
        UNUSED(arrm4);
    }

    TEST(EDatatype, CanReturnNumberOfComponentsAtCompiletime)
    {
        static_assert(1u == GetNumberOfComponents(EDataType::UInt16), "Invalid number of components");
        static_assert(1u == GetNumberOfComponents(EDataType::UInt32), "Invalid number of components");
        static_assert(1u == GetNumberOfComponents(EDataType::Float), "Invalid number of components");
        static_assert(2u == GetNumberOfComponents(EDataType::Vector2F), "Invalid number of components");
        static_assert(3u == GetNumberOfComponents(EDataType::Vector3F), "Invalid number of components");
        static_assert(4u == GetNumberOfComponents(EDataType::Vector4F), "Invalid number of components");
        static_assert(1u == GetNumberOfComponents(EDataType::ByteBlob), "Invalid number of components");
        static_assert(1u == GetNumberOfComponents(EDataType::Int32), "Invalid number of components");
        static_assert(2u == GetNumberOfComponents(EDataType::Vector2I), "Invalid number of components");
        static_assert(3u == GetNumberOfComponents(EDataType::Vector3I), "Invalid number of components");
        static_assert(4u == GetNumberOfComponents(EDataType::Vector4I), "Invalid number of components");
        static_assert(4u == GetNumberOfComponents(EDataType::Matrix22F), "Invalid number of components");
        static_assert(9u == GetNumberOfComponents(EDataType::Matrix33F), "Invalid number of components");
        static_assert(16u == GetNumberOfComponents(EDataType::Matrix44F), "Invalid number of components");

        static_assert(0u == GetNumberOfComponents(EDataType::TextureSampler2D), "Invalid number of components");
        static_assert(0u == GetNumberOfComponents(EDataType::TextureSampler2DMS), "Invalid number of components");
        static_assert(0u == GetNumberOfComponents(EDataType::TextureSampler3D), "Invalid number of components");
        static_assert(0u == GetNumberOfComponents(EDataType::TextureSamplerCube), "Invalid number of components");
        static_assert(0u == GetNumberOfComponents(EDataType::TextureSamplerExternal), "Invalid number of components");
    }

    TEST(EDatatype, CanReturnSizeOfComponentAtCompiletime)
    {
        static_assert(sizeof(uint16_t) == GetSizeOfComponent(EDataType::UInt16), "component of EDataType::UInt16 should have size of uint16_t");
        static_assert(sizeof(uint32_t) == GetSizeOfComponent(EDataType::UInt32), "component of EDataType::UInt32 should have size of uint32_t");
        static_assert(sizeof(float) == GetSizeOfComponent(EDataType::Float), "component of EDataType::Float should have size of float");
        static_assert(sizeof(float) == GetSizeOfComponent(EDataType::Vector2F), "component of EDataType::Vector2F should have size of float");
        static_assert(sizeof(float) == GetSizeOfComponent(EDataType::Vector3F), "component of EDataType::Vector3F should have size of float");
        static_assert(sizeof(float) == GetSizeOfComponent(EDataType::Vector4F), "component of EDataType::Vector4F should have size of float");
        static_assert(sizeof(ramses::Byte) == GetSizeOfComponent(EDataType::ByteBlob), "component of EDataType::ByteBlob should have size of byte");
        static_assert(sizeof(int32_t) == GetSizeOfComponent(EDataType::Int32), "Invalid component size");
        static_assert(sizeof(int32_t) == GetSizeOfComponent(EDataType::Vector2I), "Invalid component size");
        static_assert(sizeof(int32_t) == GetSizeOfComponent(EDataType::Vector3I), "Invalid component size");
        static_assert(sizeof(int32_t) == GetSizeOfComponent(EDataType::Vector4I), "Invalid component size");
        static_assert(sizeof(float) == GetSizeOfComponent(EDataType::Matrix22F), "Invalid component size");
        static_assert(sizeof(float) == GetSizeOfComponent(EDataType::Matrix33F), "Invalid component size");
        static_assert(sizeof(float) == GetSizeOfComponent(EDataType::Matrix44F), "Invalid component size");

        static_assert(0u == GetSizeOfComponent(EDataType::TextureSampler2D), "Invalid component size");
        static_assert(0u == GetSizeOfComponent(EDataType::TextureSampler2DMS), "Invalid component size");
        static_assert(0u == GetSizeOfComponent(EDataType::TextureSampler3D), "Invalid component size");
        static_assert(0u == GetSizeOfComponent(EDataType::TextureSamplerCube), "Invalid component size");
        static_assert(0u == GetSizeOfComponent(EDataType::TextureSamplerExternal), "Invalid component size");
    }

    TEST(EDatatype, CanReturnSizesAtCompiletime)
    {
        static_assert(sizeof(uint16_t) == GetSizeOfDataType(EDataType::UInt16), "Invalid data type size");
        static_assert(sizeof(uint32_t) == GetSizeOfDataType(EDataType::UInt32), "Invalid data type size");
        static_assert(sizeof(float) == GetSizeOfDataType(EDataType::Float), "Invalid data type size");
        static_assert(sizeof(ramses::vec2f) == GetSizeOfDataType(EDataType::Vector2F), "Invalid data type size");
        static_assert(sizeof(ramses::vec3f) == GetSizeOfDataType(EDataType::Vector3F), "Invalid data type size");
        static_assert(sizeof(ramses::vec4f) == GetSizeOfDataType(EDataType::Vector4F), "Invalid data type size");
        static_assert(sizeof(ramses::Byte) == GetSizeOfDataType(EDataType::ByteBlob), "Invalid data type size");
        static_assert(sizeof(int32_t) == GetSizeOfDataType(EDataType::Int32), "Invalid data type size");
        static_assert(sizeof(ramses::vec2i) == GetSizeOfDataType(EDataType::Vector2I), "Invalid data type size");
        static_assert(sizeof(ramses::vec3i) == GetSizeOfDataType(EDataType::Vector3I), "Invalid data type size");
        static_assert(sizeof(ramses::vec4i) == GetSizeOfDataType(EDataType::Vector4I), "Invalid data type size");
        static_assert(sizeof(ramses::matrix22f) == GetSizeOfDataType(EDataType::Matrix22F), "Invalid data type size");
        static_assert(sizeof(ramses::matrix33f) == GetSizeOfDataType(EDataType::Matrix33F), "Invalid data type size");
        static_assert(sizeof(ramses::matrix44f) == GetSizeOfDataType(EDataType::Matrix44F), "Invalid data type size");

        static_assert(0u == GetSizeOfDataType(EDataType::TextureSampler2D), "Invalid data type size");
        static_assert(0u == GetSizeOfDataType(EDataType::TextureSampler2DMS), "Invalid data type size");
        static_assert(0u == GetSizeOfDataType(EDataType::TextureSampler3D), "Invalid data type size");
        static_assert(0u == GetSizeOfDataType(EDataType::TextureSamplerCube), "Invalid data type size");
        static_assert(0u == GetSizeOfDataType(EDataType::TextureSamplerExternal), "Invalid data type size");
    }

    TEST(EDataType, EnsureGivesSameSizeOnPublicAPI)
    {
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::UInt16) == ramses_internal::EnumToSize(ramses_internal::EDataType::UInt16));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::UInt32) == ramses_internal::EnumToSize(ramses_internal::EDataType::UInt32));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Float) == ramses_internal::EnumToSize(ramses_internal::EDataType::Float));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Vector2F) == ramses_internal::EnumToSize(ramses_internal::EDataType::Vector2F));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Vector3F) == ramses_internal::EnumToSize(ramses_internal::EDataType::Vector3F));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Vector4F) == ramses_internal::EnumToSize(ramses_internal::EDataType::Vector4F));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::ByteBlob) == ramses_internal::EnumToSize(ramses_internal::EDataType::ByteBlob));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Int32) == ramses_internal::EnumToSize(ramses_internal::EDataType::Int32));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Vector2I) == ramses_internal::EnumToSize(ramses_internal::EDataType::Vector2I));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Vector3I) == ramses_internal::EnumToSize(ramses_internal::EDataType::Vector3I));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Vector4I) == ramses_internal::EnumToSize(ramses_internal::EDataType::Vector4I));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Matrix22F) == ramses_internal::EnumToSize(ramses_internal::EDataType::Matrix22F));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Matrix33F) == ramses_internal::EnumToSize(ramses_internal::EDataType::Matrix33F));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Matrix44F) == ramses_internal::EnumToSize(ramses_internal::EDataType::Matrix44F));

        // not relevant for sampler types
    }

    TEST(EDataType, EnsureGivesSameNumberOfComponentsOnPublicAPI)
    {
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::UInt16) == ramses_internal::EnumToNumComponents(ramses_internal::EDataType::UInt16));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::UInt32) == ramses_internal::EnumToNumComponents(ramses_internal::EDataType::UInt32));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Float) == ramses_internal::EnumToNumComponents(ramses_internal::EDataType::Float));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Vector2F) == ramses_internal::EnumToNumComponents(ramses_internal::EDataType::Vector2F));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Vector3F) == ramses_internal::EnumToNumComponents(ramses_internal::EDataType::Vector3F));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Vector4F) == ramses_internal::EnumToNumComponents(ramses_internal::EDataType::Vector4F));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::ByteBlob) == ramses_internal::EnumToNumComponents(ramses_internal::EDataType::ByteBlob));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Int32) == ramses_internal::EnumToNumComponents(ramses_internal::EDataType::Int32));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Vector2I) == ramses_internal::EnumToNumComponents(ramses_internal::EDataType::Vector2I));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Vector3I) == ramses_internal::EnumToNumComponents(ramses_internal::EDataType::Vector3I));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Vector4I) == ramses_internal::EnumToNumComponents(ramses_internal::EDataType::Vector4I));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Matrix22F) == ramses_internal::EnumToNumComponents(ramses_internal::EDataType::Matrix22F));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Matrix33F) == ramses_internal::EnumToNumComponents(ramses_internal::EDataType::Matrix33F));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Matrix44F) == ramses_internal::EnumToNumComponents(ramses_internal::EDataType::Matrix44F));

        // not relevant for sampler types
    }

    TEST(EDataType, IsUniformInputDataType)
    {
        static_assert(ramses::IsUniformInputDataType<int32_t>());
        static_assert(ramses::IsUniformInputDataType<float>());
        static_assert(ramses::IsUniformInputDataType<vec2i>());
        static_assert(ramses::IsUniformInputDataType<vec3i>());
        static_assert(ramses::IsUniformInputDataType<vec4i>());
        static_assert(ramses::IsUniformInputDataType<vec2f>());
        static_assert(ramses::IsUniformInputDataType<vec3f>());
        static_assert(ramses::IsUniformInputDataType<vec4f>());
        static_assert(ramses::IsUniformInputDataType<matrix22f>());
        static_assert(ramses::IsUniformInputDataType<matrix33f>());
        static_assert(ramses::IsUniformInputDataType<matrix44f>());

        static_assert(!ramses::IsUniformInputDataType<uint16_t>());
        static_assert(!ramses::IsUniformInputDataType<uint32_t>());
        static_assert(!ramses::IsUniformInputDataType<double>());
    }

    TEST(EDataType, IsArrayResourceDataType)
    {
        static_assert(ramses::IsArrayResourceDataType<uint16_t>());
        static_assert(ramses::IsArrayResourceDataType<uint32_t>());
        static_assert(ramses::IsArrayResourceDataType<float>());
        static_assert(ramses::IsArrayResourceDataType<vec2f>());
        static_assert(ramses::IsArrayResourceDataType<vec3f>());
        static_assert(ramses::IsArrayResourceDataType<vec4f>());
        static_assert(ramses::IsArrayResourceDataType<Byte>());

        static_assert(!ramses::IsArrayResourceDataType<int32_t>());
        static_assert(!ramses::IsArrayResourceDataType<double>());
        static_assert(!ramses::IsArrayResourceDataType<vec2i>());
        static_assert(!ramses::IsArrayResourceDataType<vec3i>());
        static_assert(!ramses::IsArrayResourceDataType<vec4i>());
        static_assert(!ramses::IsArrayResourceDataType<matrix22f>());
        static_assert(!ramses::IsArrayResourceDataType<matrix33f>());
        static_assert(!ramses::IsArrayResourceDataType<matrix44f>());
    }

    TEST(EDataType, GetEDataType)
    {
        static_assert(ramses::EDataType::UInt16 == ramses::GetEDataType<uint16_t>());
        static_assert(ramses::EDataType::UInt32 == ramses::GetEDataType<uint32_t>());
        static_assert(ramses::EDataType::Float == ramses::GetEDataType<float>());
        static_assert(ramses::EDataType::Vector2F == ramses::GetEDataType<vec2f>());
        static_assert(ramses::EDataType::Vector3F == ramses::GetEDataType<vec3f>());
        static_assert(ramses::EDataType::Vector4F == ramses::GetEDataType<vec4f>());
        static_assert(ramses::EDataType::ByteBlob == ramses::GetEDataType<Byte>());
        static_assert(ramses::EDataType::Int32 == ramses::GetEDataType<int32_t>());
        static_assert(ramses::EDataType::Vector2I == ramses::GetEDataType<vec2i>());
        static_assert(ramses::EDataType::Vector3I == ramses::GetEDataType<vec3i>());
        static_assert(ramses::EDataType::Vector4I == ramses::GetEDataType<vec4i>());
        static_assert(ramses::EDataType::Matrix22F == ramses::GetEDataType<matrix22f>());
        static_assert(ramses::EDataType::Matrix33F == ramses::GetEDataType<matrix33f>());
        static_assert(ramses::EDataType::Matrix44F == ramses::GetEDataType<matrix44f>());
    }

    TEST(EDataType, InternalToPublic)
    {
        EXPECT_EQ(ramses::EDataType::Int32, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::Int32));
        EXPECT_EQ(ramses::EDataType::UInt16, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::UInt16));
        EXPECT_EQ(ramses::EDataType::UInt32, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::UInt32));
        EXPECT_EQ(ramses::EDataType::Float, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::Float));
        EXPECT_EQ(ramses::EDataType::Vector2F, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::Vector2F));
        EXPECT_EQ(ramses::EDataType::Vector3F, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::Vector3F));
        EXPECT_EQ(ramses::EDataType::Vector4F, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::Vector4F));
        EXPECT_EQ(ramses::EDataType::Vector2I, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::Vector2I));
        EXPECT_EQ(ramses::EDataType::Vector3I, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::Vector3I));
        EXPECT_EQ(ramses::EDataType::Vector4I, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::Vector4I));
        EXPECT_EQ(ramses::EDataType::Matrix22F, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::Matrix22F));
        EXPECT_EQ(ramses::EDataType::Matrix33F, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::Matrix33F));
        EXPECT_EQ(ramses::EDataType::Matrix44F, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::Matrix44F));
        EXPECT_EQ(ramses::EDataType::TextureSampler2D, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::TextureSampler2D));
        EXPECT_EQ(ramses::EDataType::TextureSampler2DMS, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::TextureSampler2DMS));
        EXPECT_EQ(ramses::EDataType::TextureSampler3D, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::TextureSampler3D));
        EXPECT_EQ(ramses::EDataType::TextureSamplerCube, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::TextureSamplerCube));
        EXPECT_EQ(ramses::EDataType::TextureSamplerExternal, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::TextureSamplerExternal));

        EXPECT_EQ(ramses::EDataType::Float, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::FloatBuffer));
        EXPECT_EQ(ramses::EDataType::UInt16, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::UInt16Buffer));
        EXPECT_EQ(ramses::EDataType::Vector2F, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::Vector2Buffer));
        EXPECT_EQ(ramses::EDataType::Vector3F, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::Vector3Buffer));
        EXPECT_EQ(ramses::EDataType::Vector4F, DataTypeUtils::ConvertDataTypeFromInternal(ramses_internal::EDataType::Vector4Buffer));
    }

    TEST(EDataType, PublicToInternal)
    {
        EXPECT_EQ(ramses_internal::EDataType::Int32, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Int32));
        EXPECT_EQ(ramses_internal::EDataType::UInt16, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::UInt16));
        EXPECT_EQ(ramses_internal::EDataType::UInt32, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::UInt32));
        EXPECT_EQ(ramses_internal::EDataType::Float, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Float));
        EXPECT_EQ(ramses_internal::EDataType::Vector2F, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Vector2F));
        EXPECT_EQ(ramses_internal::EDataType::Vector3F, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Vector3F));
        EXPECT_EQ(ramses_internal::EDataType::Vector4F, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Vector4F));
        EXPECT_EQ(ramses_internal::EDataType::Vector2I, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Vector2I));
        EXPECT_EQ(ramses_internal::EDataType::Vector3I, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Vector3I));
        EXPECT_EQ(ramses_internal::EDataType::Vector4I, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Vector4I));
        EXPECT_EQ(ramses_internal::EDataType::Matrix22F, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Matrix22F));
        EXPECT_EQ(ramses_internal::EDataType::Matrix33F, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Matrix33F));
        EXPECT_EQ(ramses_internal::EDataType::Matrix44F, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Matrix44F));
        EXPECT_EQ(ramses_internal::EDataType::TextureSampler2D, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::TextureSampler2D));
        EXPECT_EQ(ramses_internal::EDataType::TextureSampler2DMS, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::TextureSampler2DMS));
        EXPECT_EQ(ramses_internal::EDataType::TextureSampler3D, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::TextureSampler3D));
        EXPECT_EQ(ramses_internal::EDataType::TextureSamplerCube, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::TextureSamplerCube));
        EXPECT_EQ(ramses_internal::EDataType::TextureSamplerExternal, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::TextureSamplerExternal));
    }
}
