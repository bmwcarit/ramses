//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/EffectImpl.h"

#include "TestEffectCreator.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/AttributeInput.h"
#include "impl/EffectInputImpl.h"
#include "FeatureLevelTestValues.h"
#include "gtest/gtest.h"

using namespace testing;

namespace ramses::internal
{
    class AnEffect : public TestWithSharedEffectPerFeatureLevel
    {
    };

    RAMSES_INSTANTIATE_FEATURELEVEL_TEST_SUITE(AnEffect);

    TEST_P(AnEffect, hasProperNumberOfInputs)
    {
        const Effect* effect = m_sharedTestState.effect;
        EXPECT_EQ((GetParam() < EFeatureLevel_02 ? 30u : 43u), effect->getUniformInputCount());
        EXPECT_EQ(4u, effect->getAttributeInputCount());
    }

    TEST_P(AnEffect, hasNoGeometryShaderWhenNotCreatedWithSuch)
    {
        EXPECT_FALSE(m_sharedTestState.effect->hasGeometryShader());
    }

    TEST_P(AnEffect, reportsErrorWhenAskedForGeometryInputType_ButNoGeometryShaderProvided)
    {
        EDrawMode mode;
        EXPECT_FALSE(m_sharedTestState.effect->getGeometryShaderInputType(mode));
    }

    TEST_P(AnEffect, reportsErrorWhenAskingForNonExistingUniformInput)
    {
        const Effect* effect = m_sharedTestState.effect;
        const std::optional<UniformInput> optInput = effect->getUniformInput(99u);
        EXPECT_FALSE(optInput.has_value());
    }

    TEST_P(AnEffect, reportsErrorWhenAskingForNonExistingAttributeInput)
    {
        const Effect* effect = m_sharedTestState.effect;
        const std::optional<AttributeInput> optInput = effect->getAttributeInput(99u);
        EXPECT_FALSE(optInput.has_value());
    }

    TEST_P(AnEffect, reportsErrorWhenAskingForNonExistingUniformName)
    {
        const Effect* effect = m_sharedTestState.effect;
        const std::optional<UniformInput> optInput = effect->findUniformInput("xxx");
        EXPECT_FALSE(optInput.has_value());
    }

    TEST_P(AnEffect, reportsErrorWhenAskingForNonExistingAttributeName)
    {
        const Effect* effect = m_sharedTestState.effect;
        const std::optional<AttributeInput> optInput = effect->findAttributeInput("xxx");
        EXPECT_FALSE(optInput.has_value());
    }

    TEST_P(AnEffect, reportsErrorWhenAskingForNonExistingUniformSemantic)
    {
        const Effect* effect = m_sharedTestState.effect;
        const std::optional<UniformInput> optInput = effect->findUniformInput(EEffectUniformSemantic::NormalMatrix);
        EXPECT_FALSE(optInput.has_value());
    }

    TEST_P(AnEffect, reportsErrorWhenAskingForNonExistingAttributeSemantic)
    {
        const Effect* effect = m_sharedTestState.effect;
        const std::optional<AttributeInput> optInput = effect->findAttributeInput(EEffectAttributeSemantic::Invalid);
        EXPECT_FALSE(optInput.has_value());
    }

    TEST_P(AnEffect, getsUniformInputByIndex)
    {
        const Effect* effect = m_sharedTestState.effect;
        const std::optional<UniformInput> optInput = effect->getUniformInput(5u);
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("vec3fInputArray", optInput->getName());
        EXPECT_EQ(3u, optInput->getElementCount());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::Vector3F, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::Vector3F, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::Invalid, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::Invalid, optInput->getSemantics());
        EXPECT_EQ(5u, optInput->impl().getInputIndex());
        EXPECT_FALSE(optInput->impl().getUniformBufferBinding().isValid());
        EXPECT_FALSE(optInput->impl().getUniformBufferFieldOffset().isValid());
        EXPECT_FALSE(optInput->impl().getUniformBufferElementSize().isValid());
    }

    TEST_P(AnEffect, getsUniformInputByIndex_UniformBuffer)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        const Effect* effect = m_sharedTestState.effect;
        const std::optional<UniformInput> optInput = effect->getUniformInput(24u);
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("uniformBlock", optInput->getName());
        EXPECT_EQ(1u, optInput->getElementCount());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::UniformBuffer, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::UniformBuffer, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::Invalid, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::Invalid, optInput->getSemantics());
        EXPECT_EQ(24u, optInput->impl().getInputIndex());
        EXPECT_EQ(optInput->impl().getUniformBufferBinding().getValue(), 1u);
        EXPECT_FALSE(optInput->impl().getUniformBufferFieldOffset().isValid());
        EXPECT_EQ(optInput->impl().getUniformBufferElementSize().getValue(), 160u);
    }

    TEST_P(AnEffect, getsUniformInputByIndex_UniformBufferField)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        const Effect* effect = m_sharedTestState.effect;
        const std::optional<UniformInput> optInput = effect->getUniformInput(26u);
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("uniformBlock.ubFloat", optInput->getName());
        EXPECT_EQ(3u, optInput->getElementCount());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::Float, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::Float, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::Invalid, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::Invalid, optInput->getSemantics());
        EXPECT_EQ(26u, optInput->impl().getInputIndex());
        EXPECT_EQ(optInput->impl().getUniformBufferBinding().getValue(), 1u);
        EXPECT_EQ(optInput->impl().getUniformBufferFieldOffset().getValue(), 64u);
        EXPECT_EQ(optInput->impl().getUniformBufferElementSize().getValue(), 16u);
    }

    TEST_P(AnEffect, getsUniformInputByIndex_AnonymousUniformBuffer)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        const Effect* effect = m_sharedTestState.effect;
        const std::optional<UniformInput> optInput = effect->getUniformInput(28u);
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("anon@ubo_binding=2", optInput->getName());
        EXPECT_EQ(1u, optInput->getElementCount());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::UniformBuffer, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::UniformBuffer, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::Invalid, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::Invalid, optInput->getSemantics());
        EXPECT_EQ(28u, optInput->impl().getInputIndex());
        EXPECT_EQ(optInput->impl().getUniformBufferBinding().getValue(), 2u);
        EXPECT_FALSE(optInput->impl().getUniformBufferFieldOffset().isValid());
        EXPECT_EQ(optInput->impl().getUniformBufferElementSize().getValue(), 124u);
    }

    TEST_P(AnEffect, getsUniformInputByIndex_AnonymousUniformBufferField)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        const Effect* effect = m_sharedTestState.effect;
        const std::optional<UniformInput> optInput = effect->getUniformInput(30u);
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("ubMat33", optInput->getName());
        EXPECT_EQ(1u, optInput->getElementCount());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::Matrix33F, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::Matrix33F, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::Invalid, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::Invalid, optInput->getSemantics());
        EXPECT_EQ(30u, optInput->impl().getInputIndex());
        EXPECT_EQ(optInput->impl().getUniformBufferBinding().getValue(), 2u);
        EXPECT_EQ(optInput->impl().getUniformBufferFieldOffset().getValue(), 16u); // mat33 has vec4 alignment
        EXPECT_EQ(optInput->impl().getUniformBufferElementSize().getValue(), 48u);
    }

    TEST_P(AnEffect, findsUniformInputBySemantic)
    {
        const Effect* effect = m_sharedTestState.effect;
        const std::optional<UniformInput> optInput = effect->findUniformInput(EEffectUniformSemantic::ModelViewMatrix);
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("mvMatrix", optInput->getName());
        EXPECT_EQ(1u, optInput->getElementCount());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::Matrix44F, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::Matrix44F, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::ModelViewMatrix, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::ModelViewMatrix, optInput->getSemantics());
        EXPECT_FALSE(optInput->impl().getUniformBufferBinding().isValid());
        EXPECT_FALSE(optInput->impl().getUniformBufferFieldOffset().isValid());
        EXPECT_FALSE(optInput->impl().getUniformBufferElementSize().isValid());
    }

    TEST_P(AnEffect, findsUniformInputBySemantic_UniformBlock)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        const Effect* effect = m_sharedTestState.effect;
        const std::optional<UniformInput> optInput = effect->findUniformInput(EEffectUniformSemantic::ModelBlock);
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("ubWithSemantics", optInput->getName());
        EXPECT_EQ(1u, optInput->getElementCount());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::UniformBuffer, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::UniformBuffer, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::ModelBlock, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::ModelBlock, optInput->getSemantics());
        EXPECT_EQ(35u, optInput->impl().getInputIndex());
        EXPECT_EQ(3u, optInput->impl().getUniformBufferBinding().getValue());
        EXPECT_FALSE(optInput->impl().getUniformBufferFieldOffset().isValid());
        EXPECT_EQ(64u, optInput->impl().getUniformBufferElementSize().getValue());
    }

    TEST_P(AnEffect, getsAttributeInputByIndex)
    {
        const Effect* effect = m_sharedTestState.effect;
        const std::optional<AttributeInput> optInput = effect->getAttributeInput(1u);
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("vec2fArrayInput", optInput->getName());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::Vector2F, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::Vector2Buffer, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::TextPositionsAttribute, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectAttributeSemantic::TextPositions, optInput->getSemantics());
        EXPECT_EQ(1u, optInput->impl().getInputIndex());
        EXPECT_FALSE(optInput->impl().getUniformBufferBinding().isValid());
        EXPECT_FALSE(optInput->impl().getUniformBufferFieldOffset().isValid());
        EXPECT_FALSE(optInput->impl().getUniformBufferElementSize().isValid());
    }

    TEST_P(AnEffect, findsAttributeInputBySemantic)
    {
        const Effect* effect = m_sharedTestState.effect;
        const std::optional<AttributeInput> optInput = effect->findAttributeInput(EEffectAttributeSemantic::TextPositions);
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("vec2fArrayInput", optInput->getName());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::Vector2F, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::Vector2Buffer, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::TextPositionsAttribute, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectAttributeSemantic::TextPositions, optInput->getSemantics());
        EXPECT_FALSE(optInput->impl().getUniformBufferBinding().isValid());
        EXPECT_FALSE(optInput->impl().getUniformBufferFieldOffset().isValid());
        EXPECT_FALSE(optInput->impl().getUniformBufferElementSize().isValid());
    }

    TEST_P(AnEffect, findsUniformInputByName)
    {
        const Effect* effect = m_sharedTestState.effect;
        const std::optional<UniformInput> optInput = effect->findUniformInput("vec3fInputArray");
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("vec3fInputArray", optInput->getName());
        EXPECT_EQ(3u, optInput->getElementCount());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::Vector3F, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::Vector3F, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::Invalid, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::Invalid, optInput->getSemantics());
        EXPECT_EQ(5u, optInput->impl().getInputIndex());
        EXPECT_FALSE(optInput->impl().getUniformBufferBinding().isValid());
        EXPECT_FALSE(optInput->impl().getUniformBufferFieldOffset().isValid());
        EXPECT_FALSE(optInput->impl().getUniformBufferElementSize().isValid());
    }

    TEST_P(AnEffect, findsUniformInputByName_UniformBuffer)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        const Effect* effect = m_sharedTestState.effect;
        const std::optional<UniformInput> optInput = effect->findUniformInput("uniformBlock");
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("uniformBlock", optInput->getName());
        EXPECT_EQ(1u, optInput->getElementCount());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::UniformBuffer, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::UniformBuffer, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::Invalid, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::Invalid, optInput->getSemantics());
        EXPECT_EQ(24u, optInput->impl().getInputIndex());
        EXPECT_EQ(optInput->impl().getUniformBufferBinding().getValue(), 1u);
        EXPECT_FALSE(optInput->impl().getUniformBufferFieldOffset().isValid());
        EXPECT_EQ(optInput->impl().getUniformBufferElementSize().getValue(), 160u);
    }

    TEST_P(AnEffect, findsUniformInputByName_UniformBufferField)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        const Effect* effect = m_sharedTestState.effect;
        const std::optional<UniformInput> optInput = effect->findUniformInput("uniformBlock.ubMat33");
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("uniformBlock.ubMat33", optInput->getName());
        EXPECT_EQ(1u, optInput->getElementCount());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::Matrix33F, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::Matrix33F, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::Invalid, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::Invalid, optInput->getSemantics());
        EXPECT_EQ(27u, optInput->impl().getInputIndex());
        EXPECT_EQ(optInput->impl().getUniformBufferBinding().getValue(), 1u);
        EXPECT_EQ(optInput->impl().getUniformBufferFieldOffset().getValue(), 112u);
        EXPECT_EQ(optInput->impl().getUniformBufferElementSize().getValue(), 48u);
    }

    TEST_P(AnEffect, findsUniformInputByLayoutBinding)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        const Effect* effect = m_sharedTestState.effect;
        const std::optional<UniformInput> optInput = effect->findUniformInputAtBinding(1u);
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("uniformBlock", optInput->getName());
        EXPECT_EQ(1u, optInput->getElementCount());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::UniformBuffer, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::UniformBuffer, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::Invalid, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::Invalid, optInput->getSemantics());
        EXPECT_EQ(24u, optInput->impl().getInputIndex());
        EXPECT_EQ(optInput->getUniformBufferBinding(), 1u);
        EXPECT_FALSE(optInput->impl().getUniformBufferFieldOffset().isValid());
        EXPECT_EQ(optInput->impl().getUniformBufferElementSize().getValue(), 160u);
    }

    TEST_P(AnEffect, reportsErrorWhenAskingForNonExistingUniformBufferBinding)
    {
        const Effect* effect = m_sharedTestState.effect;
        const std::optional<UniformInput> optInput = effect->findUniformInputAtBinding(999u);
        EXPECT_FALSE(optInput.has_value());
    }

    TEST_P(AnEffect, findsAllTextureTypesOfUniformInputByName)
    {
        const Effect* effect = m_sharedTestState.effect;
        const std::optional<UniformInput> optInput2d              = effect->findUniformInput("texture2dInput");
        const std::optional<UniformInput> optInput3d              = effect->findUniformInput("texture3dInput");
        const std::optional<UniformInput> optInputcube            = effect->findUniformInput("textureCubeInput");
        const std::optional<UniformInput> optInputExternalTexture = effect->findUniformInput("textureExternalInput");

        ASSERT_TRUE(optInput2d.has_value());
        EXPECT_EQ(ramses::EDataType::TextureSampler2D, optInput2d->getDataType());
        ASSERT_TRUE(optInput3d.has_value());
        EXPECT_EQ(ramses::EDataType::TextureSampler3D, optInput3d->getDataType());
        ASSERT_TRUE(optInputcube.has_value());
        EXPECT_EQ(ramses::EDataType::TextureSamplerCube, optInputcube->getDataType());
        ASSERT_TRUE(optInputExternalTexture.has_value());
        EXPECT_EQ(ramses::EDataType::TextureSamplerExternal, optInputExternalTexture->getDataType());
    }

    TEST_P(AnEffect, findsAttributeInputByName)
    {
        const Effect*  effect = m_sharedTestState.effect;
        const std::optional<AttributeInput> optInput = effect->findAttributeInput("vec2fArrayInput");
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("vec2fArrayInput", optInput->getName());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::Vector2F, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::Vector2Buffer, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::TextPositionsAttribute, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectAttributeSemantic::TextPositions, optInput->getSemantics());
        EXPECT_EQ(1u, optInput->impl().getInputIndex());
        EXPECT_FALSE(optInput->impl().getUniformBufferBinding().isValid());
        EXPECT_FALSE(optInput->impl().getUniformBufferFieldOffset().isValid());
        EXPECT_FALSE(optInput->impl().getUniformBufferElementSize().isValid());
    }

    TEST_P(AnEffect, findsTextureInputWithSemantics)
    {
        const Effect* effect = m_sharedTestState.effect;
        const std::optional<UniformInput> optInput = effect->findUniformInput("texture2dInput");
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("texture2dInput", optInput->getName());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::TextureSampler2D, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::TextureSampler2D, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::TextTexture, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::TextTexture, optInput->getSemantics());
        EXPECT_EQ((GetParam() < EFeatureLevel_02 ? 25u : 38u), optInput->impl().getInputIndex());
        EXPECT_FALSE(optInput->impl().getUniformBufferBinding().isValid());
        EXPECT_FALSE(optInput->impl().getUniformBufferFieldOffset().isValid());
        EXPECT_FALSE(optInput->impl().getUniformBufferElementSize().isValid());
    }

    TEST_P(AnEffect, canNotCreateEffectWhenTextPositionsSemanticsHasWrongType)
    {
        EffectDescription effectDesc;
        effectDesc.setVertexShader(
            "precision highp float;"
            "attribute vec3 a_position;"
            "void main()"
            "{"
            "  gl_Position = vec4(1.0, 1.0, 1.0, 1.0);"
            "}");
        effectDesc.setFragmentShader(
            "precision highp float;"
            "void main(void)\n"
            "{"
            "  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);"
            "}");

        /// Can create ...
        EXPECT_NE(static_cast<Effect*>(nullptr), m_sharedTestState.getScene().createEffect(effectDesc, ""));

        effectDesc.setAttributeSemantic("a_position", EEffectAttributeSemantic::TextPositions);

        /// Can not create ...
        EXPECT_EQ(static_cast<Effect*>(nullptr), m_sharedTestState.getScene().createEffect(effectDesc, ""));
    }

    TEST_P(AnEffect, canNotCreateEffectWhenSettingSemanticOnUniformBufferField)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        EffectDescription effectDesc;
        effectDesc.setVertexShader(
            "#version 320 es\n"
            "precision highp float;"
            "in vec3 a_position;"
            "layout(std140,binding=1) uniform ub_t { mat4 ubMat4; } uniformBlock;"
            "void main()"
            "{"
            "  gl_Position = uniformBlock.ubMat4 * vec4(1.0);"
            "}");
        effectDesc.setFragmentShader(
            "#version 320 es\n"
            "precision highp float;"
            "out vec4 fragColor;"
            "void main(void)\n"
            "{"
            "  fragColor = vec4(1.0, 1.0, 1.0, 1.0);"
            "}");

        /// Can create ...
        EXPECT_NE(nullptr, m_sharedTestState.getScene().createEffect(effectDesc, ""));

        effectDesc.setUniformSemantic("uniformBlock.ubMat4", EEffectUniformSemantic::ModelViewProjectionMatrix);

        /// Can not create ...
        EXPECT_EQ(nullptr, m_sharedTestState.getScene().createEffect(effectDesc, ""));

        EXPECT_THAT(m_sharedTestState.getScene().getLastEffectErrorMessages(),
            ::testing::HasSubstr("uniformBlock.ubMat4: can not have semantic because it is declared in a uniform block"));
    }

    TEST_P(AnEffect, canNotCreateEffectWhenSettingSemanticOnUniformBuffer)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        EffectDescription effectDesc;
        effectDesc.setVertexShader(
            "#version 320 es\n"
            "precision highp float;"
            "in vec3 a_position;"
            "layout(std140,binding=1) uniform ub_t { float ubFloat; } uniformBlock;"
            "void main()"
            "{"
            "  gl_Position = vec4(uniformBlock.ubFloat, 1.0, 1.0, 1.0);"
            "}");
        effectDesc.setFragmentShader(
            "#version 320 es\n"
            "precision highp float;"
            "out vec4 fragColor;"
            "void main(void)\n"
            "{"
            "  fragColor = vec4(1.0, 1.0, 1.0, 1.0);"
            "}");

        /// Can create ...
        EXPECT_NE(nullptr, m_sharedTestState.getScene().createEffect(effectDesc, ""));

        effectDesc.setUniformSemantic("uniformBlock", EEffectUniformSemantic::ModelViewProjectionMatrix);

        /// Can not create ...
        EXPECT_EQ(nullptr, m_sharedTestState.getScene().createEffect(effectDesc, ""));

        EXPECT_THAT(m_sharedTestState.getScene().getLastEffectErrorMessages(),
            ::testing::HasSubstr("uniformBlock: input type DATATYPE_UNIFORMBUFFER not compatible with semantic EFixedSemantics::ModelViewProjectionMatrix"));
    }

    TEST_P(AnEffect, canRetrieveGLSLErrorMessageFromClient)
    {

        EffectDescription effectDesc;

        effectDesc.setVertexShader("#version 100\n"
                                   "attribute float inp;\n"
                                   "void main(void)\n"
                                   "{\n"
                                   "    gl_Position = vec4(0.0)\n"
                                   "}\n");
        effectDesc.setFragmentShader("precision highp float;"
                                     "void main(void)\n"
                                     "{"
                                     "  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);"
                                     "}");
        const Effect* effect = m_sharedTestState.getScene().createEffect(effectDesc);
        EXPECT_EQ(nullptr, effect);
        using namespace ::testing;
        EXPECT_THAT(m_sharedTestState.getScene().getLastEffectErrorMessages(),
                    AnyOf(Eq("[GLSL Compiler] vertex shader Shader Parsing Error:\n"
                             "ERROR: 2:5: '' :  syntax error\n"
                             "ERROR: 1 compilation errors.  No code generated.\n\n\n"),
                          Eq("[GLSL Compiler] vertex shader Shader Parsing Error:\n"
                             "ERROR: 2:5: '' :  syntax error, unexpected RIGHT_BRACE, expecting COMMA or SEMICOLON\n"
                             "ERROR: 1 compilation errors.  No code generated.\n\n\n")));
    }

    TEST_P(AnEffect, clientDeletesEffectErrorMessagesOfLastEffect)
    {
        EffectDescription effectDesc;

        effectDesc.setVertexShader("#version 100\n"
                                   "attribute float inp;\n"
                                   "void main(void)\n"
                                   "{\n"
                                   "    gl_Position = vec4(0.0)\n"
                                   "}\n");
        effectDesc.setFragmentShader("precision highp float;"
                                     "void main(void)\n"
                                     "{"
                                     "  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);"
                                     "}");
        const Effect* effect1 = m_sharedTestState.getScene().createEffect(effectDesc);
        EXPECT_EQ(nullptr, effect1);
        EXPECT_NE("", m_sharedTestState.getScene().getLastEffectErrorMessages());

        effectDesc.setVertexShader("#version 100\n"
                                   "attribute float inp;\n"
                                   "void main(void)\n"
                                   "{\n"
                                   "    gl_Position = vec4(0.0)\n;"
                                   "}\n");

        const Effect* effect2 = m_sharedTestState.getScene().createEffect(effectDesc);
        EXPECT_NE(nullptr, effect2);
        EXPECT_EQ("", m_sharedTestState.getScene().getLastEffectErrorMessages());
    }

    TEST_P(AnEffect, canNotCreateEffectWhenTextTextureCoordinatesSemanticsHasWrongType)
    {
        EffectDescription effectDesc;
        effectDesc.setVertexShader(
            "precision highp float;"
            "attribute vec3 a_texcoord;"
            "void main()"
            "{"
            "  gl_Position = vec4(1.0, 1.0, 1.0, 1.0);"
            "}");
        effectDesc.setFragmentShader(
            "precision highp float;"
            "void main(void)\n"
            "{"
            "  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);"
            "}");

        /// Can create ...
        EXPECT_NE(static_cast<Effect*>(nullptr), m_sharedTestState.getScene().createEffect(effectDesc, ""));

        effectDesc.setAttributeSemantic("a_texcoord", EEffectAttributeSemantic::TextTextureCoordinates);

        /// Can not create ...
        EXPECT_EQ(static_cast<Effect*>(nullptr), m_sharedTestState.getScene().createEffect(effectDesc, ""));
    }

    TEST_P(AnEffect, canNotCreateEffectWhenTextTextureSemanticsHasWrongType)
    {
        EffectDescription effectDesc;
        effectDesc.setVertexShader(
            "precision highp float;"
            "void main()"
            "{"
            "  gl_Position = vec4(1.0, 1.0, 1.0, 1.0);"
            "}");
        effectDesc.setFragmentShader(
            "precision highp float;"
            "uniform vec4 u_texture;"
            "void main(void)"
            "{"
            "  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);"
            "}");

        /// Can create ...
        EXPECT_NE(static_cast<Effect*>(nullptr), m_sharedTestState.getScene().createEffect(effectDesc, ""));

        effectDesc.setUniformSemantic("u_texture", EEffectUniformSemantic::TextTexture);

        /// Can not create ...
        EXPECT_EQ(static_cast<Effect*>(nullptr), m_sharedTestState.getScene().createEffect(effectDesc, ""));
    }

    TEST_P(AnEffect, supportsBoolUniforms)
    {
        const char* vertShader = R"SHADER(
            #version 320 es
            uniform bool u_theBool;
            void main(void)
            {
                if(u_theBool)
                {
                    gl_Position = vec4(0.0);
                }
                else
                {
                    gl_Position = vec4(1.0);
                }
            }
            )SHADER";

        const char* fragShader = R"SHADER(
            #version 320 es
            out lowp vec4 colorOut;
            void main(void)
            {
                colorOut = vec4(0.0);
            })SHADER";
        EffectDescription effectDesc;
        effectDesc.setVertexShader(vertShader);
        effectDesc.setFragmentShader(fragShader);

        const Effect* effect = m_sharedTestState.getScene().createEffect(effectDesc);

        EXPECT_EQ("", m_sharedTestState.getScene().getLastEffectErrorMessages());

        ASSERT_NE(nullptr, effect);

        const std::optional<UniformInput> optUniform = effect->findUniformInput("u_theBool");
        ASSERT_TRUE(optUniform.has_value());

        Appearance* appearance = m_sharedTestState.getScene().createAppearance(*effect);
        ASSERT_NE(nullptr, appearance);

        EXPECT_TRUE(appearance->setInputValue(*optUniform, true));
    }

    class AnEffectWithGeometryShader : public AnEffect
    {
    protected:
        const char* m_vertShader = R"SHADER(
            #version 320 es
            void main(void)
            {
                gl_Position = vec4(0.0);
            }
            )SHADER";

        const char* m_fragShader = R"SHADER(
            #version 320 es
            out lowp vec4 colorOut;
            void main(void)
            {
                colorOut = vec4(0.0);
            })SHADER";
    };

    RAMSES_INSTANTIATE_FEATURELEVEL_TEST_SUITE(AnEffectWithGeometryShader);

    TEST_P(AnEffectWithGeometryShader, providesExpectedGeometryInputType_Points)
    {
        EffectDescription effectDesc;
        effectDesc.setVertexShader(m_vertShader);
        effectDesc.setFragmentShader(m_fragShader);
        effectDesc.setGeometryShader(R"SHADER(
            #version 320 es
            layout(points) in;
            layout(points, max_vertices = 1) out;
            void main() {
                gl_Position = vec4(0.0);
                EmitVertex();
            }
            )SHADER");

        Effect* effect = m_sharedTestState.getScene().createEffect(effectDesc);

        ASSERT_NE(nullptr, effect);
        EXPECT_TRUE(effect->hasGeometryShader());
        EDrawMode geometryShaderInput;
        EXPECT_TRUE(effect->getGeometryShaderInputType(geometryShaderInput));
        EXPECT_EQ(geometryShaderInput, EDrawMode::Points);
    }

    TEST_P(AnEffectWithGeometryShader, providesExpectedGeometryInputType_Lines)
    {
        EffectDescription effectDesc;
        effectDesc.setVertexShader(m_vertShader);
        effectDesc.setFragmentShader(m_fragShader);
        effectDesc.setGeometryShader(R"SHADER(
            #version 320 es
            layout(lines) in;
            layout(points, max_vertices = 1) out;
            void main() {
                gl_Position = vec4(0.0);
                EmitVertex();
            }
            )SHADER");

        Effect* effect = m_sharedTestState.getScene().createEffect(effectDesc);
        ASSERT_NE(nullptr, effect);
        EXPECT_TRUE(effect->hasGeometryShader());
        EDrawMode geometryShaderInput;
        EXPECT_TRUE(effect->getGeometryShaderInputType(geometryShaderInput));
        EXPECT_EQ(geometryShaderInput, EDrawMode::Lines);
    }

    TEST_P(AnEffectWithGeometryShader, providesExpectedGeometryInputType_Triangles)
    {
        EffectDescription effectDesc;
        effectDesc.setVertexShader(m_vertShader);
        effectDesc.setFragmentShader(m_fragShader);
        effectDesc.setGeometryShader(R"SHADER(
            #version 320 es
            layout(triangles) in;
            layout(points, max_vertices = 1) out;
            void main() {
                gl_Position = vec4(0.0);
                EmitVertex();
            }
            )SHADER");

        Effect* effect = m_sharedTestState.getScene().createEffect(effectDesc);
        ASSERT_NE(nullptr, effect);
        EDrawMode geometryShaderInput;
        EXPECT_TRUE(effect->getGeometryShaderInputType(geometryShaderInput));
        EXPECT_EQ(geometryShaderInput, EDrawMode::Triangles);
    }

    TEST_P(AnEffectWithGeometryShader, providesExpectedGeometryInputType_whenMultipleIdenticalEffectsCreated)
    {
        EffectDescription effectDesc;
        effectDesc.setVertexShader(m_vertShader);
        effectDesc.setFragmentShader(m_fragShader);
        effectDesc.setGeometryShader(R"SHADER(
            #version 320 es
            layout(triangles) in;
            layout(points, max_vertices = 1) out;
            void main() {
                gl_Position = vec4(0.0);
                EmitVertex();
            }
            )SHADER");

        Effect* effect1 = m_sharedTestState.getScene().createEffect(effectDesc);
        Effect* effect2 = m_sharedTestState.getScene().createEffect(effectDesc);
        ASSERT_NE(nullptr, effect1);
        ASSERT_NE(nullptr, effect2);
        EDrawMode geometryShaderInput;
        EXPECT_TRUE(effect1->getGeometryShaderInputType(geometryShaderInput));
        EXPECT_EQ(geometryShaderInput, EDrawMode::Triangles);
        EXPECT_TRUE(effect2->getGeometryShaderInputType(geometryShaderInput));
        EXPECT_EQ(geometryShaderInput, EDrawMode::Triangles);
    }

    TEST_P(AnEffect, sceneValiadationProducesShaderWarnings)
    {
        EffectDescription effectDesc;

        effectDesc.setVertexShader(R"SHADER(
            #version 100

            uniform highp mat4 mvpMatrix;

            attribute vec3 a_position;
            attribute lowp vec2 a_texcoord;

            // precision mismatch: lowp != highp
            varying lowp vec2 v_texcoord;

            void main()
            {
                gl_Position = mvpMatrix * vec4(a_position, 1.0);
                v_texcoord = a_texcoord;
            }
            )SHADER");

        effectDesc.setFragmentShader(R"SHADER(
            #version 100

            uniform sampler2D textureSampler;

            // precision mismatch: highp != lowp
            varying highp vec2 v_texcoord;

            void main(void)
            {
                gl_FragColor = texture2D(textureSampler, v_texcoord);
            }
            )SHADER");

        const auto* effect = m_sharedTestState.getScene().createEffect(effectDesc);
        EXPECT_NE(nullptr, effect);

        ValidationReport report;
        m_sharedTestState.getScene().validate(report);
        EXPECT_TRUE(report.hasIssue());

        EXPECT_THAT(report.impl().toString(), HasSubstr("Precision mismatch: 'v_texcoord'. (Vertex: smooth out lowp 2-component vector of float, Fragment: smooth in highp 2-component vector of float)"));
        ValidationReport report2;
        m_sharedTestState.getScene().validate(report2);
        EXPECT_EQ(report.getIssues(), report2.getIssues());
    }
}
