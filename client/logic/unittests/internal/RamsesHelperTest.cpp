//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"

#include "internals/RamsesHelper.h"

namespace ramses::internal
{
    TEST(ARamsesHelper, ConvertsRamsesUniformTypeToEpropertyType)
    {
        EXPECT_EQ(EPropertyType::Int32, ConvertRamsesUniformTypeToPropertyType(ramses::EDataType::Int32));
        EXPECT_EQ(std::nullopt, ConvertRamsesUniformTypeToPropertyType(ramses::EDataType::UInt16));
        EXPECT_EQ(std::nullopt, ConvertRamsesUniformTypeToPropertyType(ramses::EDataType::UInt32));
        EXPECT_EQ(EPropertyType::Float, ConvertRamsesUniformTypeToPropertyType(ramses::EDataType::Float));
        EXPECT_EQ(EPropertyType::Vec2f, ConvertRamsesUniformTypeToPropertyType(ramses::EDataType::Vector2F));
        EXPECT_EQ(EPropertyType::Vec3f, ConvertRamsesUniformTypeToPropertyType(ramses::EDataType::Vector3F));
        EXPECT_EQ(EPropertyType::Vec4f, ConvertRamsesUniformTypeToPropertyType(ramses::EDataType::Vector4F));
        EXPECT_EQ(EPropertyType::Vec2i, ConvertRamsesUniformTypeToPropertyType(ramses::EDataType::Vector2I));
        EXPECT_EQ(EPropertyType::Vec3i, ConvertRamsesUniformTypeToPropertyType(ramses::EDataType::Vector3I));
        EXPECT_EQ(EPropertyType::Vec4i, ConvertRamsesUniformTypeToPropertyType(ramses::EDataType::Vector4I));
        EXPECT_EQ(std::nullopt, ConvertRamsesUniformTypeToPropertyType(ramses::EDataType::Matrix22F));
        EXPECT_EQ(std::nullopt, ConvertRamsesUniformTypeToPropertyType(ramses::EDataType::Matrix33F));
        EXPECT_EQ(std::nullopt, ConvertRamsesUniformTypeToPropertyType(ramses::EDataType::Matrix44F));
        EXPECT_EQ(std::nullopt, ConvertRamsesUniformTypeToPropertyType(ramses::EDataType::TextureSampler2D));
        EXPECT_EQ(std::nullopt, ConvertRamsesUniformTypeToPropertyType(ramses::EDataType::TextureSampler3D));
        EXPECT_EQ(std::nullopt, ConvertRamsesUniformTypeToPropertyType(ramses::EDataType::TextureSamplerCube));
        EXPECT_EQ(std::nullopt, ConvertRamsesUniformTypeToPropertyType(ramses::EDataType::TextureSamplerExternal));
    }
}
