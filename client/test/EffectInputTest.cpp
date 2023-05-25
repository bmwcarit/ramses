//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/AttributeInput.h"
#include "EffectInputImpl.h"
#include "SceneAPI/IScene.h"
#include "Utils/File.h"
#include "Utils/BinaryFileOutputStream.h"
#include "Utils/BinaryFileInputStream.h"

using namespace testing;

namespace ramses
{
    class AnEffectInput : public ::testing::Test
    {
    protected:
        void initializeInput(EffectInput& input)
        {
            constexpr ramses_internal::ResourceContentHash effectHash{ 1u, 0u };
            constexpr ramses_internal::EFixedSemantics semantics = ramses_internal::EFixedSemantics::ModelMatrix;
            input.m_impl.get().initialize(effectHash, "test", ramses_internal::EDataType::Matrix44F, semantics, 1u, 66u);
        }

        void checkInput(const EffectInput& input)
        {
            AttributeInput compInput;
            initializeInput(compInput);

            EXPECT_STREQ(compInput.getName(), input.getName());
            EXPECT_EQ(compInput.getDataType(), input.getDataType());
            EXPECT_EQ(compInput.m_impl.get().getSemantics(), input.m_impl.get().getSemantics());
            EXPECT_EQ(compInput.m_impl.get().getElementCount(), input.m_impl.get().getElementCount());
            EXPECT_EQ(compInput.m_impl.get().getInputIndex(), input.m_impl.get().getInputIndex());
            EXPECT_EQ(compInput.m_impl.get().getEffectHash(), input.m_impl.get().getEffectHash());
        }
    };

    TEST_F(AnEffectInput, UniformInputIsInitializedToDefaultUponCreation)
    {
        UniformInput input;
        EXPECT_STREQ("", input.getName());
        EXPECT_EQ(0u, input.getElementCount());
        EXPECT_EQ(ramses_internal::ResourceContentHash::Invalid(), input.m_impl.get().getEffectHash());
        EXPECT_FALSE(input.getDataType());
        EXPECT_EQ(ramses_internal::EFixedSemantics::Invalid, input.m_impl.get().getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::Invalid, input.getSemantics());
        EXPECT_EQ(static_cast<uint32_t>(-1), input.m_impl.get().getInputIndex());
        EXPECT_FALSE(input.isValid());
    }

    TEST_F(AnEffectInput, AttributeInputIsInitializedToDefaultUponCreation)
    {
        AttributeInput input;
        EXPECT_STREQ("", input.getName());
        EXPECT_EQ(ramses_internal::ResourceContentHash::Invalid(), input.m_impl.get().getEffectHash());
        EXPECT_FALSE(input.getDataType());
        EXPECT_EQ(ramses_internal::EFixedSemantics::Invalid, input.m_impl.get().getSemantics());
        EXPECT_EQ(static_cast<uint32_t>(-1), input.m_impl.get().getInputIndex());
        EXPECT_FALSE(input.isValid());
    }

    TEST_F(AnEffectInput, UniformInputIsInitializedToGivenValues)
    {
        const ramses_internal::ResourceContentHash effectHash(1u, 0);
        const std::string inputName("test");
        const ramses_internal::EDataType dataType = ramses_internal::EDataType::Int32;
        const ramses_internal::EFixedSemantics semantics = ramses_internal::EFixedSemantics::ModelMatrix;
        const uint32_t elementCount = 9u;
        const uint32_t index = 66u;

        UniformInput input;
        input.m_impl.get().initialize(effectHash, inputName, dataType, semantics, elementCount, index);

        EXPECT_STREQ(inputName.c_str(), input.getName());
        EXPECT_EQ(elementCount, input.getElementCount());
        EXPECT_EQ(EDataType::Int32, *input.getDataType());
        EXPECT_EQ(effectHash, input.m_impl.get().getEffectHash());
        EXPECT_EQ(dataType, input.m_impl.get().getInternalDataType());
        EXPECT_EQ(semantics, input.m_impl.get().getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::ModelMatrix, input.getSemantics());
        EXPECT_EQ(index, input.m_impl.get().getInputIndex());
        EXPECT_TRUE(input.isValid());
    }

    TEST_F(AnEffectInput, AttributeInputIsInitializedToGivenValues)
    {
        const ramses_internal::ResourceContentHash effectHash(1u, 0);
        const std::string inputName("test");
        const ramses_internal::EDataType dataType = ramses_internal::EDataType::Vector2Buffer;
        const ramses_internal::EFixedSemantics semantics = ramses_internal::EFixedSemantics::TextPositionsAttribute;
        const uint32_t index = 66u;

        AttributeInput input;
        input.m_impl.get().initialize(effectHash, inputName, dataType, semantics, 1u, index);

        EXPECT_STREQ(inputName.c_str(), input.getName());
        EXPECT_EQ(EDataType::Vector2F, *input.getDataType());
        EXPECT_EQ(effectHash, input.m_impl.get().getEffectHash());
        EXPECT_EQ(dataType, input.m_impl.get().getInternalDataType());
        EXPECT_EQ(semantics, input.m_impl.get().getSemantics());
        EXPECT_EQ(index, input.m_impl.get().getInputIndex());
        EXPECT_TRUE(input.isValid());
    }

    TEST_F(AnEffectInput, CanBeSerialized)
    {
        const ramses_internal::ResourceContentHash effectHash(1u, 0);
        const std::string inputName("test");
        const ramses_internal::EDataType dataType = ramses_internal::EDataType::Vector2Buffer;
        const ramses_internal::EFixedSemantics semantics = ramses_internal::EFixedSemantics::TextTextureCoordinatesAttribute;
        const uint32_t index = 66u;

        EffectInputImpl input;
        input.initialize(effectHash, inputName, dataType, semantics, 1u, index);

        const std::string fileName("someTemporaryFile.ram");

        ramses_internal::File outputFile(fileName);
        ramses_internal::BinaryFileOutputStream outputStream(outputFile);
        assert(outputFile.isOpen());

        input.serialize(outputStream);

        outputFile.close();

        ramses_internal::File inputFile(fileName);
        ramses_internal::BinaryFileInputStream inputStream(inputFile);
        assert(inputFile.isOpen());

        EffectInputImpl inputRead;

        inputRead.deserialize(inputStream);

        inputFile.close();

        EXPECT_EQ(inputName, inputRead.getName());
        EXPECT_EQ(input.getDataType(), inputRead.getDataType());
        EXPECT_EQ(effectHash, inputRead.getEffectHash());
        EXPECT_EQ(dataType, inputRead.getInternalDataType());
        EXPECT_EQ(semantics, inputRead.getSemantics());
        EXPECT_EQ(index, inputRead.getInputIndex());
        EXPECT_TRUE(inputRead.isValid());
    }

    TEST_F(AnEffectInput, CanBeComparedForEquality)
    {
        const ramses_internal::ResourceContentHash effectHash(1u, 0);
        const std::string inputName("test");
        const ramses_internal::EDataType dataType = ramses_internal::EDataType::Vector2Buffer;
        const ramses_internal::EFixedSemantics semantics = ramses_internal::EFixedSemantics::TextPositionsAttribute;
        const uint32_t index = 66u;

        EffectInputImpl input1;
        input1.initialize(effectHash, inputName, dataType, semantics, 1u, index);

        {
            EffectInputImpl input2;
            input2.initialize(effectHash, inputName, dataType, semantics, 1u, index);
            EXPECT_TRUE(input1 == input2);
            EXPECT_FALSE(input1 != input2);
        }

        {
            EffectInputImpl input2;
            input2.initialize(ramses_internal::ResourceContentHash(2u, 0), inputName, dataType, semantics, 1u, index);
            EXPECT_FALSE(input1 == input2);
            EXPECT_TRUE(input1 != input2);
        }

        {
            EffectInputImpl input2;
            input2.initialize(effectHash, "test2", dataType, semantics, 1u, index);
            EXPECT_FALSE(input1 == input2);
            EXPECT_TRUE(input1 != input2);
        }

        {
            EffectInputImpl input2;
            input2.initialize(effectHash, inputName, ramses_internal::EDataType::Vector3Buffer, semantics, 1u, index);
            EXPECT_FALSE(input1 == input2);
            EXPECT_TRUE(input1 != input2);
        }

        {
            EffectInputImpl input2;
            input2.initialize(effectHash, inputName, dataType, ramses_internal::EFixedSemantics::TextTextureCoordinatesAttribute, 1u, index);
            EXPECT_FALSE(input1 == input2);
            EXPECT_TRUE(input1 != input2);
        }

        {
            EffectInputImpl input2;
            input2.initialize(effectHash, inputName, dataType, semantics, 2u, index);
            EXPECT_FALSE(input1 == input2);
            EXPECT_TRUE(input1 != input2);
        }

        {
            EffectInputImpl input2;
            input2.initialize(effectHash, inputName, dataType, semantics, 1u, 67u);
            EXPECT_FALSE(input1 == input2);
            EXPECT_TRUE(input1 != input2);
        }
    }

    TEST_F(AnEffectInput, ReturnsCorrectDataType)
    {
        const ramses_internal::ResourceContentHash effectHash(1u, 0);
        const std::string inputName("test");
        const ramses_internal::EFixedSemantics semantics = ramses_internal::EFixedSemantics::ModelMatrix;
        const uint32_t index = 66u;
        UniformInput input;

        input.m_impl.get().initialize(effectHash, inputName, ramses_internal::EDataType::UInt16, semantics, 1u, index);
        EXPECT_EQ(*input.getDataType(), EDataType::UInt16);

        input.m_impl.get().initialize(effectHash, inputName, ramses_internal::EDataType::UInt32, semantics, 1u, index);
        EXPECT_EQ(*input.getDataType(), EDataType::UInt32);


        input.m_impl.get().initialize(effectHash, inputName, ramses_internal::EDataType::Int32, semantics, 1u, index);
        EXPECT_EQ(*input.getDataType(), EDataType::Int32);

        input.m_impl.get().initialize(effectHash, inputName, ramses_internal::EDataType::Vector2I, semantics, 1u, index);
        EXPECT_EQ(*input.getDataType(), EDataType::Vector2I);

        input.m_impl.get().initialize(effectHash, inputName, ramses_internal::EDataType::Vector3I, semantics, 1u, index);
        EXPECT_EQ(*input.getDataType(), EDataType::Vector3I);

        input.m_impl.get().initialize(effectHash, inputName, ramses_internal::EDataType::Vector4I, semantics, 1u, index);
        EXPECT_EQ(*input.getDataType(), EDataType::Vector4I);


        input.m_impl.get().initialize(effectHash, inputName, ramses_internal::EDataType::Float, semantics, 1u, index);
        EXPECT_EQ(*input.getDataType(), EDataType::Float);

        input.m_impl.get().initialize(effectHash, inputName, ramses_internal::EDataType::Vector2F, semantics, 1u, index);
        EXPECT_EQ(*input.getDataType(), EDataType::Vector2F);

        input.m_impl.get().initialize(effectHash, inputName, ramses_internal::EDataType::Vector3F, semantics, 1u, index);
        EXPECT_EQ(*input.getDataType(), EDataType::Vector3F);

        input.m_impl.get().initialize(effectHash, inputName, ramses_internal::EDataType::Vector4F, semantics, 1u, index);
        EXPECT_EQ(*input.getDataType(), EDataType::Vector4F);


        input.m_impl.get().initialize(effectHash, inputName, ramses_internal::EDataType::Matrix22F, semantics, 1u, index);
        EXPECT_EQ(*input.getDataType(), EDataType::Matrix22F);

        input.m_impl.get().initialize(effectHash, inputName, ramses_internal::EDataType::Matrix33F, semantics, 1u, index);
        EXPECT_EQ(*input.getDataType(), EDataType::Matrix33F);

        input.m_impl.get().initialize(effectHash, inputName, ramses_internal::EDataType::Matrix44F, semantics, 1u, index);
        EXPECT_EQ(*input.getDataType(), EDataType::Matrix44F);


        input.m_impl.get().initialize(effectHash, inputName, ramses_internal::EDataType::TextureSampler2D, semantics, 1u, index);
        EXPECT_EQ(*input.getDataType(), EDataType::TextureSampler2D);
        input.m_impl.get().initialize(effectHash, inputName, ramses_internal::EDataType::TextureSampler2DMS, semantics, 1u, index);
        EXPECT_EQ(*input.getDataType(), EDataType::TextureSampler2DMS);
        input.m_impl.get().initialize(effectHash, inputName, ramses_internal::EDataType::TextureSampler3D, semantics, 1u, index);
        EXPECT_EQ(*input.getDataType(), EDataType::TextureSampler3D);
        input.m_impl.get().initialize(effectHash, inputName, ramses_internal::EDataType::TextureSamplerCube, semantics, 1u, index);
        EXPECT_EQ(*input.getDataType(), EDataType::TextureSamplerCube);
        input.m_impl.get().initialize(effectHash, inputName, ramses_internal::EDataType::TextureSamplerExternal, semantics, 1u, index);
        EXPECT_EQ(*input.getDataType(), EDataType::TextureSamplerExternal);
    }

    TEST_F(AnEffectInput, CanBeCopyAndMoveConstructed_Uniform)
    {
        UniformInput input;
        initializeInput(input);

        UniformInput inputCopy{ input };
        checkInput(inputCopy);

        UniformInput inputMove{ std::move(input) };
        checkInput(inputMove);
    }

    TEST_F(AnEffectInput, CanBeCopyAndMoveAssigned_Uniform)
    {
        UniformInput input;
        initializeInput(input);

        UniformInput inputCopy;
        inputCopy = input;
        checkInput(inputCopy);

        UniformInput inputMove;
        inputMove = std::move(input);
        checkInput(inputMove);
    }

    TEST_F(AnEffectInput, CanBeSelfAssigned_Uniform)
    {
        UniformInput input;
        initializeInput(input);

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
        input = input;
        checkInput(input);
        input = std::move(input);
        // NOLINTNEXTLINE(bugprone-use-after-move)
        checkInput(input);
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    }

    TEST_F(AnEffectInput, CanBeCopyAndMoveConstructed_Attribute)
    {
        AttributeInput input;
        initializeInput(input);

        AttributeInput inputCopy{ input };
        checkInput(inputCopy);

        AttributeInput inputMove{ std::move(input) };
        checkInput(inputMove);
    }

    TEST_F(AnEffectInput, CanBeCopyAndMoveAssigned_Attribute)
    {
        AttributeInput input;
        initializeInput(input);

        AttributeInput inputCopy;
        inputCopy = input;
        checkInput(inputCopy);

        AttributeInput inputMove;
        inputMove = std::move(input);
        checkInput(inputMove);
    }

    TEST_F(AnEffectInput, CanBeSelfAssigned_Attribute)
    {
        AttributeInput input;
        initializeInput(input);

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
        input = input;
        checkInput(input);
        input = std::move(input);
        // NOLINTNEXTLINE(bugprone-use-after-move)
        checkInput(input);
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    }
}
