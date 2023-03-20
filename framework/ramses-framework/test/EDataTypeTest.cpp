//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses-framework-api/EDataType.h"
#include "SceneAPI/EDataType.h"

namespace ramses
{
    using namespace testing;

    TEST(EDatatype, CanReturnNumberOfComponentsAtCompiletime)
    {
        static_assert(1u == GetNumberOfComponents(EDataType::Float), "EDataType::Float should have 1 component");
        static_assert(2u == GetNumberOfComponents(EDataType::Vector2F), "EDataType::Vector2F should have 2 components");
    }

    TEST(EDatatype, CanReturnSizeOfComponentAtCompiletime)
    {
        static_assert(sizeof(uint16_t) == GetSizeOfComponent(EDataType::UInt16), "component of EDataType::UInt16 should have size of uint16_t");
        static_assert(sizeof(uint32_t) == GetSizeOfComponent(EDataType::UInt32), "component of EDataType::UInt32 should have size of uint32_t");
        static_assert(sizeof(float) == GetSizeOfComponent(EDataType::Float), "component of EDataType::Float should have size of float");
        static_assert(sizeof(float) == GetSizeOfComponent(EDataType::Vector2F), "component of EDataType::Vector2F should have size of float");
        static_assert(sizeof(float) == GetSizeOfComponent(EDataType::Vector3F), "component of EDataType::Vector3F should have size of float");
        static_assert(sizeof(float) == GetSizeOfComponent(EDataType::Vector4F), "component of EDataType::Vector4F should have size of float");
    }

    TEST(EDatatype, CanReturnSizesAtCompiletime)
    {
        static_assert(sizeof(float) == GetSizeOfDataType(EDataType::Float), "EDataType::Float is sizeof float");
        static_assert(sizeof(float) * 2u == GetSizeOfDataType(EDataType::Vector2F), "EDataType::Vector2F should be as large as two floats");
    }

    TEST(EDataType, EnsureGivesSameSizeOnPublicAPI)
    {
        EXPECT_EQ(ramses::GetSizeOfDataType(ramses::EDataType::UInt16), ramses_internal::EnumToSize(ramses_internal::EDataType::UInt16));
        EXPECT_EQ(ramses::GetSizeOfDataType(ramses::EDataType::UInt32), ramses_internal::EnumToSize(ramses_internal::EDataType::UInt32));
        EXPECT_EQ(ramses::GetSizeOfDataType(ramses::EDataType::Float), ramses_internal::EnumToSize(ramses_internal::EDataType::Float));
        EXPECT_EQ(ramses::GetSizeOfDataType(ramses::EDataType::Vector2F), ramses_internal::EnumToSize(ramses_internal::EDataType::Vector2F));
        EXPECT_EQ(ramses::GetSizeOfDataType(ramses::EDataType::Vector3F), ramses_internal::EnumToSize(ramses_internal::EDataType::Vector3F));
        EXPECT_EQ(ramses::GetSizeOfDataType(ramses::EDataType::Vector4F), ramses_internal::EnumToSize(ramses_internal::EDataType::Vector4F));
    }

    TEST(EDataType, EnsureGivesSameNumberOfComponentsOnPublicAPI)
    {
        EXPECT_EQ(ramses::GetNumberOfComponents(ramses::EDataType::UInt16), ramses_internal::EnumToNumComponents(ramses_internal::EDataType::UInt16));
        EXPECT_EQ(ramses::GetNumberOfComponents(ramses::EDataType::UInt32), ramses_internal::EnumToNumComponents(ramses_internal::EDataType::UInt32));
        EXPECT_EQ(ramses::GetNumberOfComponents(ramses::EDataType::Float), ramses_internal::EnumToNumComponents(ramses_internal::EDataType::Float));
        EXPECT_EQ(ramses::GetNumberOfComponents(ramses::EDataType::Vector2F), ramses_internal::EnumToNumComponents(ramses_internal::EDataType::Vector2F));
        EXPECT_EQ(ramses::GetNumberOfComponents(ramses::EDataType::Vector3F), ramses_internal::EnumToNumComponents(ramses_internal::EDataType::Vector3F));
        EXPECT_EQ(ramses::GetNumberOfComponents(ramses::EDataType::Vector4F), ramses_internal::EnumToNumComponents(ramses_internal::EDataType::Vector4F));
    }
}
