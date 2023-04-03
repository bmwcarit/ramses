//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"

#include "internals/RamsesHelper.h"

namespace rlogic::internal
{
    TEST(ARamsesHelper, ConvertsRamsesUniformTypeToEpropertyType)
    {
        EXPECT_EQ(std::nullopt, ConvertRamsesUniformTypeToPropertyType(ramses::EEffectInputDataType_Invalid));
        EXPECT_EQ(EPropertyType::Int32, ConvertRamsesUniformTypeToPropertyType(ramses::EEffectInputDataType_Int32));
        EXPECT_EQ(std::nullopt, ConvertRamsesUniformTypeToPropertyType(ramses::EEffectInputDataType_UInt16));
        EXPECT_EQ(std::nullopt, ConvertRamsesUniformTypeToPropertyType(ramses::EEffectInputDataType_UInt32));
        EXPECT_EQ(EPropertyType::Float, ConvertRamsesUniformTypeToPropertyType(ramses::EEffectInputDataType_Float));
        EXPECT_EQ(EPropertyType::Vec2f, ConvertRamsesUniformTypeToPropertyType(ramses::EEffectInputDataType_Vector2F));
        EXPECT_EQ(EPropertyType::Vec3f, ConvertRamsesUniformTypeToPropertyType(ramses::EEffectInputDataType_Vector3F));
        EXPECT_EQ(EPropertyType::Vec4f, ConvertRamsesUniformTypeToPropertyType(ramses::EEffectInputDataType_Vector4F));
        EXPECT_EQ(EPropertyType::Vec2i, ConvertRamsesUniformTypeToPropertyType(ramses::EEffectInputDataType_Vector2I));
        EXPECT_EQ(EPropertyType::Vec3i, ConvertRamsesUniformTypeToPropertyType(ramses::EEffectInputDataType_Vector3I));
        EXPECT_EQ(EPropertyType::Vec4i, ConvertRamsesUniformTypeToPropertyType(ramses::EEffectInputDataType_Vector4I));
        EXPECT_EQ(std::nullopt, ConvertRamsesUniformTypeToPropertyType(ramses::EEffectInputDataType_Matrix22F));
        EXPECT_EQ(std::nullopt, ConvertRamsesUniformTypeToPropertyType(ramses::EEffectInputDataType_Matrix33F));
        EXPECT_EQ(std::nullopt, ConvertRamsesUniformTypeToPropertyType(ramses::EEffectInputDataType_Matrix44F));
        EXPECT_EQ(std::nullopt, ConvertRamsesUniformTypeToPropertyType(ramses::EEffectInputDataType_TextureSampler2D));
        EXPECT_EQ(std::nullopt, ConvertRamsesUniformTypeToPropertyType(ramses::EEffectInputDataType_TextureSampler3D));
        EXPECT_EQ(std::nullopt, ConvertRamsesUniformTypeToPropertyType(ramses::EEffectInputDataType_TextureSamplerCube));
        EXPECT_EQ(std::nullopt, ConvertRamsesUniformTypeToPropertyType(ramses::EEffectInputDataType_TextureSamplerExternal));
    }
}
