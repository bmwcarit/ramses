//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "TestEffectCreator.h"
#include "ClientTestUtils.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/AttributeInput.h"
#include "EffectInputImpl.h"
#include "EffectImpl.h"

using namespace testing;

namespace ramses
{
    class AnEffect : public ::testing::Test
    {
    public:
        static void SetUpTestCase()
        {
            sharedTestState = new TestEffectCreator;
        }

        static void TearDownTestCase()
        {
            delete sharedTestState;
            sharedTestState = 0;
        }

        void SetUp()
        {
            EXPECT_TRUE(sharedTestState != NULL);
        }

        static TestEffectCreator* sharedTestState;
    };

    TestEffectCreator* AnEffect::sharedTestState = 0;

    class AnEffectWithSemantics : public AnEffect
    {
    public:
        static void SetUpTestCase()
        {
            sharedTestState = new TestEffectCreator(true);
        }
    };

    TEST_F(AnEffect, hasProperNumberOfInputs)
    {
        const Effect* effect = sharedTestState->effect;
        EXPECT_EQ(24u, effect->getUniformInputCount());
        EXPECT_EQ(4u, effect->getAttributeInputCount());
    }

    TEST_F(AnEffect, reportsErrorWhenAskingForNonExistingUniformInput)
    {
        const Effect* effect = sharedTestState->effect;
        UniformInput input;
        EXPECT_NE(StatusOK, effect->getUniformInput(99u, input));
        EXPECT_FALSE(input.isValid());
    }

    TEST_F(AnEffect, reportsErrorWhenAskingForNonExistingAttributeInput)
    {
        const Effect* effect = sharedTestState->effect;
        AttributeInput input;
        EXPECT_NE(StatusOK, effect->getAttributeInput(99u, input));
        EXPECT_FALSE(input.isValid());
    }

    TEST_F(AnEffect, reportsErrorWhenAskingForNonExistingUniformName)
    {
        const Effect* effect = sharedTestState->effect;
        UniformInput input;
        EXPECT_NE(StatusOK, effect->findUniformInput("xxx", input));
        EXPECT_FALSE(input.isValid());
    }

    TEST_F(AnEffect, reportsErrorWhenAskingForNonExistingAttributeName)
    {
        const Effect* effect = sharedTestState->effect;
        AttributeInput input;
        EXPECT_NE(StatusOK, effect->findAttributeInput("xxx", input));
        EXPECT_FALSE(input.isValid());
    }

    TEST_F(AnEffect, reportsErrorWhenAskingForNonExistingUniformSemantic)
    {
        const Effect* effect = sharedTestState->effect;
        UniformInput input;
        EXPECT_NE(StatusOK, effect->findUniformInput(ramses::EEffectUniformSemantic_NormalMatrix, input));
        EXPECT_FALSE(input.isValid());
    }

    TEST_F(AnEffect, reportsErrorWhenAskingForNonExistingAttributeSemantic)
    {
        const Effect* effect = sharedTestState->effect;
        AttributeInput input;
        EXPECT_NE(StatusOK, effect->findAttributeInput(ramses::EEffectAttributeSemantic_Invalid, input));
        EXPECT_FALSE(input.isValid());
    }

    TEST_F(AnEffect, getsUniformInputByIndex)
    {
        const Effect* effect = sharedTestState->effect;
        UniformInput input;
        EXPECT_EQ(StatusOK, effect->getUniformInput(5u, input));

        EXPECT_STREQ("vec3fInputArray", input.getName());
        EXPECT_EQ(3u, input.getElementCount());
        EXPECT_EQ(effect->impl.getLowlevelResourceHash(), input.impl.getEffectHash());
        EXPECT_EQ(EEffectInputDataType_Vector3F, input.getDataType());
        EXPECT_EQ(ramses_internal::EDataType_Vector3F, input.impl.getDataType());
        EXPECT_EQ(ramses_internal::EFixedSemantics_Invalid, input.impl.getSemantics());
        EXPECT_EQ(EEffectUniformSemantic_Invalid, input.getSemantics());
        EXPECT_EQ(5u, input.impl.getInputIndex());
        EXPECT_TRUE(input.isValid());
    }

    TEST_F(AnEffectWithSemantics, findsUniformInputBySemantic)
    {
        const Effect* effect = sharedTestState->effect;
        UniformInput input;
        EXPECT_EQ(StatusOK, effect->findUniformInput(ramses::EEffectUniformSemantic_ModelViewMatrix, input));

        EXPECT_STREQ("matrix44fInput", input.getName());
        EXPECT_EQ(1u, input.getElementCount());
        EXPECT_EQ(effect->impl.getLowlevelResourceHash(), input.impl.getEffectHash());
        EXPECT_EQ(EEffectInputDataType_Matrix44F, input.getDataType());
        EXPECT_EQ(ramses_internal::EDataType_Matrix44F, input.impl.getDataType());
        EXPECT_EQ(ramses_internal::EFixedSemantics_ModelViewMatrix, input.impl.getSemantics());
        EXPECT_EQ(EEffectUniformSemantic_ModelViewMatrix, input.getSemantics());
        EXPECT_TRUE(input.isValid());
    }

    TEST_F(AnEffect, getsAttributeInputByIndex)
    {
        const Effect*  effect = sharedTestState->effect;
        AttributeInput input;
        EXPECT_EQ(StatusOK, effect->getAttributeInput(1u, input));

        EXPECT_STREQ("vec2fArrayInput", input.getName());
        EXPECT_EQ(effect->impl.getLowlevelResourceHash(), input.impl.getEffectHash());
        EXPECT_EQ(EEffectInputDataType_Vector2F, input.getDataType());
        EXPECT_EQ(ramses_internal::EDataType_Vector2Buffer, input.impl.getDataType());
        EXPECT_EQ(ramses_internal::EFixedSemantics_Invalid, input.impl.getSemantics());
        EXPECT_EQ(EEffectAttributeSemantic_Invalid, input.getSemantics());
        EXPECT_EQ(1u, input.impl.getInputIndex());
        EXPECT_TRUE(input.isValid());
    }

    TEST_F(AnEffectWithSemantics, getsAttributeInputByIndex)
    {
        const Effect* effect = sharedTestState->effect;
        AttributeInput input;
        EXPECT_EQ(StatusOK, effect->getAttributeInput(1u, input));

        EXPECT_STREQ("vec2fArrayInput", input.getName());
        EXPECT_EQ(effect->impl.getLowlevelResourceHash(), input.impl.getEffectHash());
        EXPECT_EQ(EEffectInputDataType_Vector2F, input.getDataType());
        EXPECT_EQ(ramses_internal::EDataType_Vector2Buffer, input.impl.getDataType());
        EXPECT_EQ(ramses_internal::EFixedSemantics_TextPositionsAttribute, input.impl.getSemantics());
        EXPECT_EQ(EEffectAttributeSemantic_TextPositions, input.getSemantics());
        EXPECT_EQ(1u, input.impl.getInputIndex());
        EXPECT_TRUE(input.isValid());
    }

    TEST_F(AnEffectWithSemantics, findsAttributeInputBySemantic)
    {
        const Effect* effect = sharedTestState->effect;
        AttributeInput input;
        EXPECT_EQ(StatusOK, effect->getAttributeInput(ramses::EEffectAttributeSemantic_TextPositions, input));

        EXPECT_STREQ("vec2fArrayInput", input.getName());
        EXPECT_EQ(effect->impl.getLowlevelResourceHash(), input.impl.getEffectHash());
        EXPECT_EQ(EEffectInputDataType_Vector2F, input.getDataType());
        EXPECT_EQ(ramses_internal::EDataType_Vector2Buffer, input.impl.getDataType());
        EXPECT_EQ(ramses_internal::EFixedSemantics_TextPositionsAttribute, input.impl.getSemantics());
        EXPECT_EQ(EEffectAttributeSemantic_TextPositions, input.getSemantics());
        EXPECT_TRUE(input.isValid());
    }

    TEST_F(AnEffect, findsUniformInputByName)
    {
        const Effect* effect = sharedTestState->effect;
        UniformInput input;
        EXPECT_EQ(StatusOK, effect->findUniformInput("vec3fInputArray", input));

        EXPECT_STREQ("vec3fInputArray", input.getName());
        EXPECT_EQ(3u, input.getElementCount());
        EXPECT_EQ(effect->impl.getLowlevelResourceHash(), input.impl.getEffectHash());
        EXPECT_EQ(EEffectInputDataType_Vector3F, input.getDataType());
        EXPECT_EQ(ramses_internal::EDataType_Vector3F, input.impl.getDataType());
        EXPECT_EQ(ramses_internal::EFixedSemantics_Invalid, input.impl.getSemantics());
        EXPECT_EQ(EEffectUniformSemantic_Invalid, input.getSemantics());
        EXPECT_EQ(5u, input.impl.getInputIndex());
        EXPECT_TRUE(input.isValid());
    }

    TEST_F(AnEffect, findsAttributeInputByName)
    {
        const Effect* effect = sharedTestState->effect;
        AttributeInput input;
        EXPECT_EQ(StatusOK, effect->findAttributeInput("vec2fArrayInput", input));

        EXPECT_STREQ("vec2fArrayInput", input.getName());
        EXPECT_EQ(effect->impl.getLowlevelResourceHash(), input.impl.getEffectHash());
        EXPECT_EQ(EEffectInputDataType_Vector2F, input.getDataType());
        EXPECT_EQ(ramses_internal::EDataType_Vector2Buffer, input.impl.getDataType());
        EXPECT_EQ(ramses_internal::EFixedSemantics_Invalid, input.impl.getSemantics());
        EXPECT_EQ(EEffectAttributeSemantic_Invalid, input.getSemantics());
        EXPECT_EQ(1u, input.impl.getInputIndex());
        EXPECT_TRUE(input.isValid());
    }

    TEST_F(AnEffectWithSemantics, findsAttributeInputByName)
    {
        const Effect*  effect = sharedTestState->effect;
        AttributeInput input;
        EXPECT_EQ(StatusOK, effect->findAttributeInput("vec2fArrayInput", input));

        EXPECT_STREQ("vec2fArrayInput", input.getName());
        EXPECT_EQ(effect->impl.getLowlevelResourceHash(), input.impl.getEffectHash());
        EXPECT_EQ(EEffectInputDataType_Vector2F, input.getDataType());
        EXPECT_EQ(ramses_internal::EDataType_Vector2Buffer, input.impl.getDataType());
        EXPECT_EQ(ramses_internal::EFixedSemantics_TextPositionsAttribute, input.impl.getSemantics());
        EXPECT_EQ(EEffectAttributeSemantic_TextPositions, input.getSemantics());
        EXPECT_EQ(1u, input.impl.getInputIndex());
        EXPECT_TRUE(input.isValid());
    }

    TEST_F(AnEffectWithSemantics, findsTextureInputWithSemantics)
    {
        const Effect* effect = sharedTestState->effect;
        UniformInput input;
        EXPECT_EQ(StatusOK, effect->findUniformInput("texture2dInput", input));

        EXPECT_STREQ("texture2dInput", input.getName());
        EXPECT_EQ(effect->impl.getLowlevelResourceHash(), input.impl.getEffectHash());
        EXPECT_EQ(EEffectInputDataType_TextureSampler, input.getDataType());
        EXPECT_EQ(ramses_internal::EDataType_TextureSampler, input.impl.getDataType());
        EXPECT_EQ(ramses_internal::EFixedSemantics_TextTextureUniform, input.impl.getSemantics());
        EXPECT_EQ(EEffectUniformSemantic_TextTexture, input.getSemantics());
        EXPECT_EQ(22u, input.impl.getInputIndex());
        EXPECT_TRUE(input.isValid());
    }

    TEST_F(AnEffect, canNotCreateEffectWhenTextPositionsSemanticsHasWrongType)
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
        EXPECT_NE(static_cast<Effect*>(0), sharedTestState->getClient().impl.createEffect(effectDesc, ResourceCacheFlag_DoNotCache, ""));

        effectDesc.setAttributeSemantic("a_position", EEffectAttributeSemantic_TextPositions);

        /// Can not create ...
        EXPECT_EQ(static_cast<Effect*>(0), sharedTestState->getClient().impl.createEffect(effectDesc, ResourceCacheFlag_DoNotCache, ""));
    }

    TEST_F(AnEffect, canNotCreateEffectWhenTextTextureCoordinatesSemanticsHasWrongType)
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
        EXPECT_NE(static_cast<Effect*>(0), sharedTestState->getClient().impl.createEffect(effectDesc, ResourceCacheFlag_DoNotCache, ""));

        effectDesc.setAttributeSemantic("a_texcoord", EEffectAttributeSemantic_TextTextureCoordinates);

        /// Can not create ...
        EXPECT_EQ(static_cast<Effect*>(0), sharedTestState->getClient().impl.createEffect(effectDesc, ResourceCacheFlag_DoNotCache, ""));
    }

    TEST_F(AnEffect, canNotCreateEffectWhenTextTextureSemanticsHasWrongType)
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
        EXPECT_NE(static_cast<Effect*>(0), sharedTestState->getClient().impl.createEffect(effectDesc, ResourceCacheFlag_DoNotCache, ""));

        effectDesc.setUniformSemantic("u_texture", EEffectUniformSemantic_TextTexture);

        /// Can not create ...
        EXPECT_EQ(static_cast<Effect*>(0), sharedTestState->getClient().impl.createEffect(effectDesc, ResourceCacheFlag_DoNotCache, ""));
    }
}
