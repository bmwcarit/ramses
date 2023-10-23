//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses/framework/EDataType.h"
#include "impl/DataTypesImpl.h"
#include "impl/DataTypeUtils.h"
#include "internal/SceneGraph/SceneAPI/EDataType.h"

namespace ramses::internal
{
    using namespace testing;

    TEST(Datatype, CanBeConstructedFromInitializerList)
    {
        [[maybe_unused]] vec2f     v2f{1.f, 2.f};
        [[maybe_unused]] vec3f     v3f{1.f, 2.f, 3.f};
        [[maybe_unused]] vec4f     v4f{1.f, 2.f, 3.f, 4.f};
        [[maybe_unused]] vec2i     v2i{1, 2};
        [[maybe_unused]] vec3i     v3i{1, 2, 3};
        [[maybe_unused]] vec4i     v4i{1, 2, 3, 4};
        [[maybe_unused]] matrix22f m2{1.f, 2.f, 3.f, 4.f};
        [[maybe_unused]] matrix33f m3{1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f};
        [[maybe_unused]] matrix44f m4{1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f};
    }

    TEST(Datatype, CanBeCopyConstructed)
    {
        [[maybe_unused]] vec2f     v2f{vec2f{1.f, 2.f}};
        [[maybe_unused]] vec3f     v3f{vec3f{1.f, 2.f, 3.f}};
        [[maybe_unused]] vec4f     v4f{vec4f{1.f, 2.f, 3.f, 4.f}};
        [[maybe_unused]] vec2i     v2i{vec2i{1, 2}};
        [[maybe_unused]] vec3i     v3i{vec3i{1, 2, 3}};
        [[maybe_unused]] vec4i     v4i{vec4i{1, 2, 3, 4}};
        [[maybe_unused]] matrix22f m2{matrix22f{1.f, 2.f, 3.f, 4.f}};
        [[maybe_unused]] matrix33f m3{matrix33f{1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f}};
        [[maybe_unused]] matrix44f m4{matrix44f{1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f}};
    }

    TEST(EDatatype, CanReturnNumberOfComponentsAtCompiletime)
    {
        static_assert(1u == GetNumberOfComponents(ramses::EDataType::UInt16), "Invalid number of components");
        static_assert(1u == GetNumberOfComponents(ramses::EDataType::UInt32), "Invalid number of components");
        static_assert(1u == GetNumberOfComponents(ramses::EDataType::Float), "Invalid number of components");
        static_assert(2u == GetNumberOfComponents(ramses::EDataType::Vector2F), "Invalid number of components");
        static_assert(3u == GetNumberOfComponents(ramses::EDataType::Vector3F), "Invalid number of components");
        static_assert(4u == GetNumberOfComponents(ramses::EDataType::Vector4F), "Invalid number of components");
        static_assert(1u == GetNumberOfComponents(ramses::EDataType::ByteBlob), "Invalid number of components");
        static_assert(1u == GetNumberOfComponents(ramses::EDataType::Bool), "Invalid number of components");
        static_assert(1u == GetNumberOfComponents(ramses::EDataType::Int32), "Invalid number of components");
        static_assert(2u == GetNumberOfComponents(ramses::EDataType::Vector2I), "Invalid number of components");
        static_assert(3u == GetNumberOfComponents(ramses::EDataType::Vector3I), "Invalid number of components");
        static_assert(4u == GetNumberOfComponents(ramses::EDataType::Vector4I), "Invalid number of components");
        static_assert(4u == GetNumberOfComponents(ramses::EDataType::Matrix22F), "Invalid number of components");
        static_assert(9u == GetNumberOfComponents(ramses::EDataType::Matrix33F), "Invalid number of components");
        static_assert(16u == GetNumberOfComponents(ramses::EDataType::Matrix44F), "Invalid number of components");

        static_assert(0u == GetNumberOfComponents(ramses::EDataType::TextureSampler2D), "Invalid number of components");
        static_assert(0u == GetNumberOfComponents(ramses::EDataType::TextureSampler2DMS), "Invalid number of components");
        static_assert(0u == GetNumberOfComponents(ramses::EDataType::TextureSampler3D), "Invalid number of components");
        static_assert(0u == GetNumberOfComponents(ramses::EDataType::TextureSamplerCube), "Invalid number of components");
        static_assert(0u == GetNumberOfComponents(ramses::EDataType::TextureSamplerExternal), "Invalid number of components");
    }

    TEST(EDatatype, CanReturnSizeOfComponentAtCompiletime)
    {
        static_assert(sizeof(uint16_t) == GetSizeOfComponent(ramses::EDataType::UInt16), "component of EDataType::UInt16 should have size of uint16_t");
        static_assert(sizeof(uint32_t) == GetSizeOfComponent(ramses::EDataType::UInt32), "component of EDataType::UInt32 should have size of uint32_t");
        static_assert(sizeof(float) == GetSizeOfComponent(ramses::EDataType::Float), "component of EDataType::Float should have size of float");
        static_assert(sizeof(float) == GetSizeOfComponent(ramses::EDataType::Vector2F), "component of EDataType::Vector2F should have size of float");
        static_assert(sizeof(float) == GetSizeOfComponent(ramses::EDataType::Vector3F), "component of EDataType::Vector3F should have size of float");
        static_assert(sizeof(float) == GetSizeOfComponent(ramses::EDataType::Vector4F), "component of EDataType::Vector4F should have size of float");
        static_assert(sizeof(std::byte) == GetSizeOfComponent(ramses::EDataType::ByteBlob), "component of EDataType::ByteBlob should have size of byte");
        static_assert(sizeof(bool) == GetSizeOfComponent(ramses::EDataType::Bool), "Invalid component size");
        static_assert(sizeof(int32_t) == GetSizeOfComponent(ramses::EDataType::Int32), "Invalid component size");
        static_assert(sizeof(int32_t) == GetSizeOfComponent(ramses::EDataType::Vector2I), "Invalid component size");
        static_assert(sizeof(int32_t) == GetSizeOfComponent(ramses::EDataType::Vector3I), "Invalid component size");
        static_assert(sizeof(int32_t) == GetSizeOfComponent(ramses::EDataType::Vector4I), "Invalid component size");
        static_assert(sizeof(float) == GetSizeOfComponent(ramses::EDataType::Matrix22F), "Invalid component size");
        static_assert(sizeof(float) == GetSizeOfComponent(ramses::EDataType::Matrix33F), "Invalid component size");
        static_assert(sizeof(float) == GetSizeOfComponent(ramses::EDataType::Matrix44F), "Invalid component size");

        static_assert(0u == GetSizeOfComponent(ramses::EDataType::TextureSampler2D), "Invalid component size");
        static_assert(0u == GetSizeOfComponent(ramses::EDataType::TextureSampler2DMS), "Invalid component size");
        static_assert(0u == GetSizeOfComponent(ramses::EDataType::TextureSampler3D), "Invalid component size");
        static_assert(0u == GetSizeOfComponent(ramses::EDataType::TextureSamplerCube), "Invalid component size");
        static_assert(0u == GetSizeOfComponent(ramses::EDataType::TextureSamplerExternal), "Invalid component size");
    }

    TEST(EDatatype, CanReturnSizesAtCompiletime)
    {
        static_assert(sizeof(uint16_t) == GetSizeOfDataType(ramses::EDataType::UInt16), "Invalid data type size");
        static_assert(sizeof(uint32_t) == GetSizeOfDataType(ramses::EDataType::UInt32), "Invalid data type size");
        static_assert(sizeof(float) == GetSizeOfDataType(ramses::EDataType::Float), "Invalid data type size");
        static_assert(sizeof(ramses::vec2f) == GetSizeOfDataType(ramses::EDataType::Vector2F), "Invalid data type size");
        static_assert(sizeof(ramses::vec3f) == GetSizeOfDataType(ramses::EDataType::Vector3F), "Invalid data type size");
        static_assert(sizeof(ramses::vec4f) == GetSizeOfDataType(ramses::EDataType::Vector4F), "Invalid data type size");
        static_assert(sizeof(std::byte) == GetSizeOfDataType(ramses::EDataType::ByteBlob), "Invalid data type size");
        static_assert(sizeof(bool) == GetSizeOfDataType(ramses::EDataType::Bool), "Invalid data type size");
        static_assert(sizeof(int32_t) == GetSizeOfDataType(ramses::EDataType::Int32), "Invalid data type size");
        static_assert(sizeof(ramses::vec2i) == GetSizeOfDataType(ramses::EDataType::Vector2I), "Invalid data type size");
        static_assert(sizeof(ramses::vec3i) == GetSizeOfDataType(ramses::EDataType::Vector3I), "Invalid data type size");
        static_assert(sizeof(ramses::vec4i) == GetSizeOfDataType(ramses::EDataType::Vector4I), "Invalid data type size");
        static_assert(sizeof(ramses::matrix22f) == GetSizeOfDataType(ramses::EDataType::Matrix22F), "Invalid data type size");
        static_assert(sizeof(ramses::matrix33f) == GetSizeOfDataType(ramses::EDataType::Matrix33F), "Invalid data type size");
        static_assert(sizeof(ramses::matrix44f) == GetSizeOfDataType(ramses::EDataType::Matrix44F), "Invalid data type size");

        static_assert(0u == GetSizeOfDataType(ramses::EDataType::TextureSampler2D), "Invalid data type size");
        static_assert(0u == GetSizeOfDataType(ramses::EDataType::TextureSampler2DMS), "Invalid data type size");
        static_assert(0u == GetSizeOfDataType(ramses::EDataType::TextureSampler3D), "Invalid data type size");
        static_assert(0u == GetSizeOfDataType(ramses::EDataType::TextureSamplerCube), "Invalid data type size");
        static_assert(0u == GetSizeOfDataType(ramses::EDataType::TextureSamplerExternal), "Invalid data type size");
    }

    TEST(EDataType, EnsureGivesSameSizeOnPublicAPI)
    {
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::UInt16) == ramses::internal::EnumToSize(ramses::internal::EDataType::UInt16));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::UInt32) == ramses::internal::EnumToSize(ramses::internal::EDataType::UInt32));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Float) == ramses::internal::EnumToSize(ramses::internal::EDataType::Float));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Vector2F) == ramses::internal::EnumToSize(ramses::internal::EDataType::Vector2F));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Vector3F) == ramses::internal::EnumToSize(ramses::internal::EDataType::Vector3F));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Vector4F) == ramses::internal::EnumToSize(ramses::internal::EDataType::Vector4F));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::ByteBlob) == ramses::internal::EnumToSize(ramses::internal::EDataType::ByteBlob));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Bool) == ramses::internal::EnumToSize(ramses::internal::EDataType::Bool));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Int32) == ramses::internal::EnumToSize(ramses::internal::EDataType::Int32));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Vector2I) == ramses::internal::EnumToSize(ramses::internal::EDataType::Vector2I));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Vector3I) == ramses::internal::EnumToSize(ramses::internal::EDataType::Vector3I));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Vector4I) == ramses::internal::EnumToSize(ramses::internal::EDataType::Vector4I));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Matrix22F) == ramses::internal::EnumToSize(ramses::internal::EDataType::Matrix22F));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Matrix33F) == ramses::internal::EnumToSize(ramses::internal::EDataType::Matrix33F));
        static_assert(ramses::GetSizeOfDataType(ramses::EDataType::Matrix44F) == ramses::internal::EnumToSize(ramses::internal::EDataType::Matrix44F));

        // not relevant for sampler types
    }

    TEST(EDataType, EnsureGivesSameNumberOfComponentsOnPublicAPI)
    {
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::UInt16) == ramses::internal::EnumToNumComponents(ramses::internal::EDataType::UInt16));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::UInt32) == ramses::internal::EnumToNumComponents(ramses::internal::EDataType::UInt32));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Float) == ramses::internal::EnumToNumComponents(ramses::internal::EDataType::Float));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Vector2F) == ramses::internal::EnumToNumComponents(ramses::internal::EDataType::Vector2F));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Vector3F) == ramses::internal::EnumToNumComponents(ramses::internal::EDataType::Vector3F));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Vector4F) == ramses::internal::EnumToNumComponents(ramses::internal::EDataType::Vector4F));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::ByteBlob) == ramses::internal::EnumToNumComponents(ramses::internal::EDataType::ByteBlob));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Bool) == ramses::internal::EnumToNumComponents(ramses::internal::EDataType::Bool));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Int32) == ramses::internal::EnumToNumComponents(ramses::internal::EDataType::Int32));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Vector2I) == ramses::internal::EnumToNumComponents(ramses::internal::EDataType::Vector2I));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Vector3I) == ramses::internal::EnumToNumComponents(ramses::internal::EDataType::Vector3I));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Vector4I) == ramses::internal::EnumToNumComponents(ramses::internal::EDataType::Vector4I));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Matrix22F) == ramses::internal::EnumToNumComponents(ramses::internal::EDataType::Matrix22F));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Matrix33F) == ramses::internal::EnumToNumComponents(ramses::internal::EDataType::Matrix33F));
        static_assert(ramses::GetNumberOfComponents(ramses::EDataType::Matrix44F) == ramses::internal::EnumToNumComponents(ramses::internal::EDataType::Matrix44F));

        // not relevant for sampler types
    }

    TEST(EDataType, IsUniformInputDataType)
    {
        static_assert(ramses::IsUniformInputDataType<bool>());
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
        static_assert(ramses::IsArrayResourceDataType<std::byte>());

        static_assert(!ramses::IsArrayResourceDataType<bool>());
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
        static_assert(ramses::EDataType::ByteBlob == ramses::GetEDataType<std::byte>());
        static_assert(ramses::EDataType::Bool == ramses::GetEDataType<bool>());
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
        EXPECT_EQ(ramses::EDataType::Bool, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::Bool));
        EXPECT_EQ(ramses::EDataType::Int32, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::Int32));
        EXPECT_EQ(ramses::EDataType::UInt16, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::UInt16));
        EXPECT_EQ(ramses::EDataType::UInt32, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::UInt32));
        EXPECT_EQ(ramses::EDataType::Float, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::Float));
        EXPECT_EQ(ramses::EDataType::Vector2F, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::Vector2F));
        EXPECT_EQ(ramses::EDataType::Vector3F, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::Vector3F));
        EXPECT_EQ(ramses::EDataType::Vector4F, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::Vector4F));
        EXPECT_EQ(ramses::EDataType::Vector2I, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::Vector2I));
        EXPECT_EQ(ramses::EDataType::Vector3I, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::Vector3I));
        EXPECT_EQ(ramses::EDataType::Vector4I, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::Vector4I));
        EXPECT_EQ(ramses::EDataType::Matrix22F, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::Matrix22F));
        EXPECT_EQ(ramses::EDataType::Matrix33F, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::Matrix33F));
        EXPECT_EQ(ramses::EDataType::Matrix44F, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::Matrix44F));
        EXPECT_EQ(ramses::EDataType::TextureSampler2D, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::TextureSampler2D));
        EXPECT_EQ(ramses::EDataType::TextureSampler2DMS, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::TextureSampler2DMS));
        EXPECT_EQ(ramses::EDataType::TextureSampler3D, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::TextureSampler3D));
        EXPECT_EQ(ramses::EDataType::TextureSamplerCube, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::TextureSamplerCube));
        EXPECT_EQ(ramses::EDataType::TextureSamplerExternal, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::TextureSamplerExternal));

        EXPECT_EQ(ramses::EDataType::Float, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::FloatBuffer));
        EXPECT_EQ(ramses::EDataType::UInt16, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::UInt16Buffer));
        EXPECT_EQ(ramses::EDataType::Vector2F, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::Vector2Buffer));
        EXPECT_EQ(ramses::EDataType::Vector3F, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::Vector3Buffer));
        EXPECT_EQ(ramses::EDataType::Vector4F, DataTypeUtils::ConvertDataTypeFromInternal(ramses::internal::EDataType::Vector4Buffer));
    }

    TEST(EDataType, PublicToInternal)
    {
        EXPECT_EQ(ramses::internal::EDataType::Bool, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Bool));
        EXPECT_EQ(ramses::internal::EDataType::Int32, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Int32));
        EXPECT_EQ(ramses::internal::EDataType::UInt16, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::UInt16));
        EXPECT_EQ(ramses::internal::EDataType::UInt32, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::UInt32));
        EXPECT_EQ(ramses::internal::EDataType::Float, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Float));
        EXPECT_EQ(ramses::internal::EDataType::Vector2F, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Vector2F));
        EXPECT_EQ(ramses::internal::EDataType::Vector3F, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Vector3F));
        EXPECT_EQ(ramses::internal::EDataType::Vector4F, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Vector4F));
        EXPECT_EQ(ramses::internal::EDataType::Vector2I, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Vector2I));
        EXPECT_EQ(ramses::internal::EDataType::Vector3I, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Vector3I));
        EXPECT_EQ(ramses::internal::EDataType::Vector4I, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Vector4I));
        EXPECT_EQ(ramses::internal::EDataType::Matrix22F, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Matrix22F));
        EXPECT_EQ(ramses::internal::EDataType::Matrix33F, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Matrix33F));
        EXPECT_EQ(ramses::internal::EDataType::Matrix44F, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::Matrix44F));
        EXPECT_EQ(ramses::internal::EDataType::TextureSampler2D, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::TextureSampler2D));
        EXPECT_EQ(ramses::internal::EDataType::TextureSampler2DMS, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::TextureSampler2DMS));
        EXPECT_EQ(ramses::internal::EDataType::TextureSampler3D, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::TextureSampler3D));
        EXPECT_EQ(ramses::internal::EDataType::TextureSamplerCube, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::TextureSamplerCube));
        EXPECT_EQ(ramses::internal::EDataType::TextureSamplerExternal, DataTypeUtils::ConvertDataTypeToInternal(ramses::EDataType::TextureSamplerExternal));
    }
}
