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
#include "FeatureLevelTestValues.h"

using namespace testing;

namespace ramses::internal
{
    class AnEffectInput : public TestWithSharedEffectPerFeatureLevel
    {
    protected:
        static void CompareInput(const EffectInput& input1, const EffectInput& input2)
        {
            EXPECT_STREQ(input1.getName(), input2.getName());
            EXPECT_EQ(input1.getDataType(), input2.getDataType());
            EXPECT_EQ(input1.impl().getSemantics(), input2.impl().getSemantics());
            EXPECT_EQ(input1.impl().getElementCount(), input2.impl().getElementCount());
            EXPECT_EQ(input1.impl().getInputIndex(), input2.impl().getInputIndex());
            EXPECT_EQ(input1.impl().getEffectHash(), input2.impl().getEffectHash());
            EXPECT_EQ(input1.impl().getUniformBufferBinding(), input2.impl().getUniformBufferBinding());
            EXPECT_EQ(input1.impl().getUniformBufferFieldOffset(), input2.impl().getUniformBufferFieldOffset());
            EXPECT_EQ(input1.impl().getUniformBufferElementSize(), input2.impl().getUniformBufferElementSize());
        }

        static constexpr std::array testUniformIndexValues
        {
            // 4=vec3, 24u=UB, 25=UB.mat44, 28u=Sampler
            4u, 24u, 25u, 28u
        };
    };

    RAMSES_INSTANTIATE_FEATURELEVEL_TEST_SUITE(AnEffectInput);

    TEST_P(AnEffectInput, InitializedToDefaultUponCreation)
    {
        EffectInputImpl input;
        EXPECT_EQ("", input.getName());
        EXPECT_EQ(std::numeric_limits<uint32_t>::max(), input.getElementCount());
        EXPECT_EQ(ramses::internal::ResourceContentHash::Invalid(), input.getEffectHash());
        EXPECT_EQ(ramses::internal::EDataType::Invalid, input.getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::Invalid, input.getSemantics());
        EXPECT_EQ(static_cast<uint32_t>(-1), input.getInputIndex());
        EXPECT_FALSE(input.getUniformBufferBinding().isValid());
        EXPECT_FALSE(input.getUniformBufferFieldOffset().isValid());
        EXPECT_FALSE(input.getUniformBufferElementSize().isValid());

    }

    TEST_P(AnEffectInput, UniformInputIsInitializedToGivenValues)
    {
        const Effect* effect = m_sharedTestState.effect;
        UniformInput input{ *effect->findUniformInput("texture2dInput") };

        EXPECT_STREQ("texture2dInput", input.getName());
        EXPECT_EQ(1u, input.getElementCount());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), input.impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::TextureSampler2D, input.getDataType());
        EXPECT_EQ(ramses::internal::EDataType::TextureSampler2D, input.impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::TextTexture, input.impl().getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::TextTexture, input.getSemantics());
        EXPECT_EQ((GetParam() < EFeatureLevel_02 ? 25u : 38u), input.impl().getInputIndex());
        EXPECT_FALSE(input.impl().getUniformBufferBinding().isValid());
        EXPECT_FALSE(input.impl().getUniformBufferFieldOffset().isValid());
        EXPECT_FALSE(input.impl().getUniformBufferElementSize().isValid());
    }

    TEST_P(AnEffectInput, AttributeInputIsInitializedToGivenValues)
    {
        const Effect* effect = m_sharedTestState.effect;
        AttributeInput input{ *effect->getAttributeInput(1u) };

        EXPECT_STREQ("vec2fArrayInput", input.getName());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), input.impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::Vector2F, input.getDataType());
        EXPECT_EQ(ramses::internal::EDataType::Vector2Buffer, input.impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::TextPositionsAttribute, input.impl().getSemantics());
        EXPECT_EQ(EEffectAttributeSemantic::TextPositions, input.getSemantics());
        EXPECT_EQ(1u, input.impl().getInputIndex());
        EXPECT_FALSE(input.impl().getUniformBufferBinding().isValid());
        EXPECT_FALSE(input.impl().getUniformBufferFieldOffset().isValid());
        EXPECT_FALSE(input.impl().getUniformBufferElementSize().isValid());
    }

    TEST_P(AnEffectInput, ReturnsCorrectDataType)
    {
        const ramses::internal::ResourceContentHash effectHash(1u, 0);
        const std::string inputName("test");
        const ramses::internal::EFixedSemantics semantics = ramses::internal::EFixedSemantics::ModelMatrix;
        const size_t index = 66u;
        EffectInputImpl input;

        input.initialize(effectHash, EffectInputInformation{ inputName, 1u, ramses::internal::EDataType::UInt16, semantics }, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::UInt16);

        input.initialize(effectHash, EffectInputInformation{ inputName, 1u, ramses::internal::EDataType::UInt32, semantics }, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::UInt32);

        input.initialize(effectHash, EffectInputInformation{ inputName, 1u, ramses::internal::EDataType::Bool, semantics }, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Bool);

        input.initialize(effectHash, EffectInputInformation{ inputName, 1u, ramses::internal::EDataType::Int32, semantics }, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Int32);

        input.initialize(effectHash, EffectInputInformation{ inputName, 1u, ramses::internal::EDataType::Vector2I, semantics }, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Vector2I);

        input.initialize(effectHash, EffectInputInformation{ inputName, 1u, ramses::internal::EDataType::Vector3I, semantics }, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Vector3I);

        input.initialize(effectHash, EffectInputInformation{ inputName, 1u, ramses::internal::EDataType::Vector4I, semantics }, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Vector4I);


        input.initialize(effectHash, EffectInputInformation{ inputName, 1u, ramses::internal::EDataType::Float, semantics }, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Float);

        input.initialize(effectHash, EffectInputInformation{ inputName, 1u, ramses::internal::EDataType::Vector2F, semantics }, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Vector2F);

        input.initialize(effectHash, EffectInputInformation{ inputName, 1u, ramses::internal::EDataType::Vector3F, semantics }, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Vector3F);

        input.initialize(effectHash, EffectInputInformation{ inputName, 1u, ramses::internal::EDataType::Vector4F, semantics }, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Vector4F);


        input.initialize(effectHash, EffectInputInformation{ inputName, 1u, ramses::internal::EDataType::Matrix22F, semantics }, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Matrix22F);

        input.initialize(effectHash, EffectInputInformation{ inputName, 1u, ramses::internal::EDataType::Matrix33F, semantics }, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Matrix33F);

        input.initialize(effectHash, EffectInputInformation{ inputName, 1u, ramses::internal::EDataType::Matrix44F, semantics }, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::Matrix44F);


        input.initialize(effectHash, EffectInputInformation{ inputName, 1u, ramses::internal::EDataType::TextureSampler2D, semantics }, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::TextureSampler2D);
        input.initialize(effectHash, EffectInputInformation{ inputName, 1u, ramses::internal::EDataType::TextureSampler2DMS, semantics }, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::TextureSampler2DMS);
        input.initialize(effectHash, EffectInputInformation{ inputName, 1u, ramses::internal::EDataType::TextureSampler3D, semantics }, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::TextureSampler3D);
        input.initialize(effectHash, EffectInputInformation{ inputName, 1u, ramses::internal::EDataType::TextureSamplerCube, semantics }, index);
        EXPECT_EQ(input.getDataType(), ramses::EDataType::TextureSamplerCube);
        input.initialize(effectHash, EffectInputInformation{ inputName, 1u, ramses::internal::EDataType::TextureSamplerExternal, semantics }, index);

        EXPECT_EQ(input.getDataType(), ramses::EDataType::TextureSamplerExternal);
    }

    TEST_P(AnEffectInput, CanBeCopyAndMoveConstructed_Uniform)
    {
        for (const auto uniformIdx : testUniformIndexValues)
        {
            const auto creference = m_sharedTestState.effect->getUniformInput(uniformIdx);
            UniformInput inputCopy{ *creference };
            CompareInput(inputCopy, *creference);

            auto reference = m_sharedTestState.effect->getUniformInput(uniformIdx);
            UniformInput inputMove{ std::move(*reference) };
            CompareInput(inputMove, *creference);
        }
    }

    TEST_P(AnEffectInput, CanBeCopyAndMoveAssigned_Uniform)
    {
        for (const auto uniformIdx : testUniformIndexValues)
        {
            const auto creference = m_sharedTestState.effect->getUniformInput(uniformIdx);
            UniformInput inputCopy{ *m_sharedTestState.effect->getUniformInput(5) };
            inputCopy = *creference;
            CompareInput(inputCopy, *creference);

            auto reference = m_sharedTestState.effect->getUniformInput(uniformIdx);
            UniformInput inputMove{ std::move(*m_sharedTestState.effect->getUniformInput(5)) };
            inputMove = std::move(*reference);
            CompareInput(inputMove, *creference);
        }
    }

    TEST_P(AnEffectInput, CanBeSelfAssigned_Uniform)
    {
        for (const auto uniformIdx : testUniformIndexValues)
        {
            const auto   reference = m_sharedTestState.effect->getUniformInput(uniformIdx);
            UniformInput input{ *reference };

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

    TEST_P(AnEffectInput, CanBeCopyAndMoveConstructed_Attribute)
    {
        const auto creference = m_sharedTestState.effect->getAttributeInput(1u);
        AttributeInput inputCopy{*creference};
        CompareInput(inputCopy, *creference);

        auto reference = m_sharedTestState.effect->getAttributeInput(1u);
        AttributeInput inputMove{std::move(*reference)};
        CompareInput(inputMove, *creference);
    }

    TEST_P(AnEffectInput, CanBeCopyAndMoveAssigned_Attribute)
    {
        const auto creference = m_sharedTestState.effect->getAttributeInput(1u);
        AttributeInput inputCopy{*m_sharedTestState.effect->getAttributeInput(2u)};
        inputCopy = *creference;
        CompareInput(inputCopy, *creference);

        auto reference = m_sharedTestState.effect->getAttributeInput(1u);
        AttributeInput inputMove{*m_sharedTestState.effect->getAttributeInput(2u)};
        inputMove = std::move(*reference);
        CompareInput(inputMove, *creference);
    }

    TEST_P(AnEffectInput, CanBeSelfAssigned_Attribute)
    {
        const auto     reference = m_sharedTestState.effect->getAttributeInput(1u);
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
