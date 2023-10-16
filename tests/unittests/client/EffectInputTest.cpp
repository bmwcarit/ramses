//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "TestEffectCreator.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/AttributeInput.h"
#include "impl/EffectImpl.h"
#include "impl/EffectInputImpl.h"
#include "internal/SceneGraph/SceneAPI/IScene.h"
#include "internal/Core/Utils/File.h"
#include "internal/Core/Utils/BinaryFileOutputStream.h"
#include "internal/Core/Utils/BinaryFileInputStream.h"

using namespace testing;

namespace ramses::internal
{
    class AnEffectInput : public ::testing::Test
    {
    public:
        static void SetUpTestSuite()
        {
            sharedTestState = std::make_unique<TestEffectCreator>(true);
        }

        static void TearDownTestSuite()
        {
            sharedTestState = nullptr;
        }

        void SetUp() override
        {
            EXPECT_TRUE(sharedTestState != nullptr);
        }

    protected:
        static void CompareInput(const EffectInput& input1, const EffectInput& input2)
        {
            EXPECT_STREQ(input1.getName(), input2.getName());
            EXPECT_EQ(input1.getDataType(), input2.getDataType());
            EXPECT_EQ(input1.impl().getSemantics(), input2.impl().getSemantics());
            EXPECT_EQ(input1.impl().getElementCount(), input2.impl().getElementCount());
            EXPECT_EQ(input1.impl().getInputIndex(), input2.impl().getInputIndex());
            EXPECT_EQ(input1.impl().getEffectHash(), input2.impl().getEffectHash());
        }

        static std::unique_ptr<TestEffectCreator> sharedTestState;
    };

    std::unique_ptr<TestEffectCreator> AnEffectInput::sharedTestState;

    TEST_F(AnEffectInput, InitializedToDefaultUponCreation)
    {
        EffectInputImpl input;
        EXPECT_EQ("", input.getName());
        EXPECT_EQ(0u, input.getElementCount());
        EXPECT_EQ(ramses::internal::ResourceContentHash::Invalid(), input.getEffectHash());
        EXPECT_EQ(ramses::internal::EDataType::Invalid, input.getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::Invalid, input.getSemantics());
        EXPECT_EQ(static_cast<uint32_t>(-1), input.getInputIndex());
    }

    TEST_F(AnEffectInput, UniformInputIsInitializedToGivenValues)
    {
        const Effect* effect = sharedTestState->effect;
        UniformInput input{ *effect->findUniformInput("texture2dInput") };

        EXPECT_STREQ("texture2dInput", input.getName());
        EXPECT_EQ(1u, input.getElementCount());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), input.impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::TextureSampler2D, input.getDataType());
        EXPECT_EQ(ramses::internal::EDataType::TextureSampler2D, input.impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::TextTexture, input.impl().getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::TextTexture, input.getSemantics());
        EXPECT_EQ(24u, input.impl().getInputIndex());
    }

    TEST_F(AnEffectInput, AttributeInputIsInitializedToGivenValues)
    {
        const Effect* effect = sharedTestState->effect;
        AttributeInput input{ *effect->getAttributeInput(1u) };

        EXPECT_STREQ("vec2fArrayInput", input.getName());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), input.impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::Vector2F, input.getDataType());
        EXPECT_EQ(ramses::internal::EDataType::Vector2Buffer, input.impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::TextPositionsAttribute, input.impl().getSemantics());
        EXPECT_EQ(EEffectAttributeSemantic::TextPositions, input.getSemantics());
        EXPECT_EQ(1u, input.impl().getInputIndex());
    }

    TEST_F(AnEffectInput, ReturnsCorrectDataType)
    {
        const ramses::internal::ResourceContentHash effectHash(1u, 0);
        const std::string inputName("test");
        const ramses::internal::EFixedSemantics semantics = ramses::internal::EFixedSemantics::ModelMatrix;
        const size_t index = 66u;
        EffectInputImpl input;

        input.initialize(effectHash, inputName, ramses::internal::EDataType::UInt16, semantics, 1u, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::UInt16);

        input.initialize(effectHash, inputName, ramses::internal::EDataType::UInt32, semantics, 1u, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::UInt32);

        input.initialize(effectHash, inputName, ramses::internal::EDataType::Bool, semantics, 1u, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Bool);

        input.initialize(effectHash, inputName, ramses::internal::EDataType::Int32, semantics, 1u, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Int32);

        input.initialize(effectHash, inputName, ramses::internal::EDataType::Vector2I, semantics, 1u, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Vector2I);

        input.initialize(effectHash, inputName, ramses::internal::EDataType::Vector3I, semantics, 1u, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Vector3I);

        input.initialize(effectHash, inputName, ramses::internal::EDataType::Vector4I, semantics, 1u, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Vector4I);


        input.initialize(effectHash, inputName, ramses::internal::EDataType::Float, semantics, 1u, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Float);

        input.initialize(effectHash, inputName, ramses::internal::EDataType::Vector2F, semantics, 1u, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Vector2F);

        input.initialize(effectHash, inputName, ramses::internal::EDataType::Vector3F, semantics, 1u, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Vector3F);

        input.initialize(effectHash, inputName, ramses::internal::EDataType::Vector4F, semantics, 1u, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Vector4F);


        input.initialize(effectHash, inputName, ramses::internal::EDataType::Matrix22F, semantics, 1u, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Matrix22F);

        input.initialize(effectHash, inputName, ramses::internal::EDataType::Matrix33F, semantics, 1u, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Matrix33F);

        input.initialize(effectHash, inputName, ramses::internal::EDataType::Matrix44F, semantics, 1u, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Matrix44F);


        input.initialize(effectHash, inputName, ramses::internal::EDataType::TextureSampler2D, semantics, 1u, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::TextureSampler2D);
        input.initialize(effectHash, inputName, ramses::internal::EDataType::TextureSampler2DMS, semantics, 1u, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::TextureSampler2DMS);
        input.initialize(effectHash, inputName, ramses::internal::EDataType::TextureSampler3D, semantics, 1u, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::TextureSampler3D);
        input.initialize(effectHash, inputName, ramses::internal::EDataType::TextureSamplerCube, semantics, 1u, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::TextureSamplerCube);
        input.initialize(effectHash, inputName, ramses::internal::EDataType::TextureSamplerExternal, semantics, 1u, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::TextureSamplerExternal);
    }

    TEST_F(AnEffectInput, CanBeCopyAndMoveConstructed_Uniform)
    {
        const auto creference = sharedTestState->effect->getUniformInput(5u);
        UniformInput inputCopy{*creference};
        CompareInput(inputCopy, *creference);

        auto reference = sharedTestState->effect->getUniformInput(5u);
        UniformInput inputMove{std::move(*reference)};
        CompareInput(inputMove, *creference);
    }

    TEST_F(AnEffectInput, CanBeCopyAndMoveAssigned_Uniform)
    {
        const auto creference = sharedTestState->effect->getUniformInput(4u);
        UniformInput inputCopy{*sharedTestState->effect->getUniformInput(5u)};
        inputCopy = *creference;
        CompareInput(inputCopy, *creference);

        auto reference = sharedTestState->effect->getUniformInput(4u);
        UniformInput inputMove{std::move(*sharedTestState->effect->getUniformInput(5u))};
        inputMove = std::move(*reference);
        CompareInput(inputMove, *creference);
    }

    TEST_F(AnEffectInput, CanBeSelfAssigned_Uniform)
    {
        const auto   reference = sharedTestState->effect->getUniformInput(5u);
        UniformInput input{*reference};

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
        input = input;
        CompareInput(input, *reference);

        input = std::move(input);
        // NOLINTNEXTLINE(bugprone-use-after-move)
        CompareInput(input, *reference);
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    }

    TEST_F(AnEffectInput, CanBeCopyAndMoveConstructed_Attribute)
    {
        const auto creference = sharedTestState->effect->getAttributeInput(1u);
        AttributeInput inputCopy{*creference};
        CompareInput(inputCopy, *creference);

        auto reference = sharedTestState->effect->getAttributeInput(1u);
        AttributeInput inputMove{std::move(*reference)};
        CompareInput(inputMove, *creference);
    }

    TEST_F(AnEffectInput, CanBeCopyAndMoveAssigned_Attribute)
    {
        const auto creference = sharedTestState->effect->getAttributeInput(1u);
        AttributeInput inputCopy{*sharedTestState->effect->getAttributeInput(2u)};
        inputCopy = *creference;
        CompareInput(inputCopy, *creference);

        auto reference = sharedTestState->effect->getAttributeInput(1u);
        AttributeInput inputMove{*sharedTestState->effect->getAttributeInput(2u)};
        inputMove = std::move(*reference);
        CompareInput(inputMove, *creference);
    }

    TEST_F(AnEffectInput, CanBeSelfAssigned_Attribute)
    {
        const auto     reference = sharedTestState->effect->getAttributeInput(1u);
        AttributeInput input{*reference};

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
        input = input;
        CompareInput(input, *reference);
        input = std::move(input);
        // NOLINTNEXTLINE(bugprone-use-after-move)
        CompareInput(input, *reference);
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    }
}
