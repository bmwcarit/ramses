//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EffectImpl.h"

#include "TestEffectCreator.h"
#include "ClientTestUtils.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/AttributeInput.h"
#include "EffectInputImpl.h"
#include "gtest/gtest.h"

using namespace testing;

namespace ramses
{
    class AnEffect : public ::testing::Test
    {
    public:
        static void SetUpTestCase()
        {
            sharedTestState = std::make_unique<TestEffectCreator>();
        }

        static void TearDownTestCase()
        {
            sharedTestState = nullptr;
        }

        void SetUp() override
        {
            EXPECT_TRUE(sharedTestState != nullptr);
        }

        static std::unique_ptr<TestEffectCreator> sharedTestState;
    };

    std::unique_ptr<TestEffectCreator> AnEffect::sharedTestState;

    class AnEffectWithSemantics : public AnEffect
    {
    public:
        static void SetUpTestCase()
        {
            sharedTestState = std::make_unique<TestEffectCreator>(true);
        }
    };

    TEST_F(AnEffect, hasProperNumberOfInputs)
    {
        const Effect* effect = sharedTestState->effect;
        EXPECT_EQ(26u, effect->getUniformInputCount());
        EXPECT_EQ(4u, effect->getAttributeInputCount());
    }

    TEST_F(AnEffect, hasNoGeometryShaderWhenNotCreatedWithSuch)
    {
        EXPECT_FALSE(Effect::hasGeometryShader(*sharedTestState->effect));
    }

    TEST_F(AnEffect, reportsErrorWhenAskedForGeometryInputType_ButNoGeometryShaderProvided)
    {
        EDrawMode mode;
        EXPECT_NE(StatusOK, Effect::getGeometryShaderInputType(*sharedTestState->effect, mode));
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
        EXPECT_NE(StatusOK, effect->findUniformInput(ramses::EEffectUniformSemantic::NormalMatrix, input));
        EXPECT_FALSE(input.isValid());
    }

    TEST_F(AnEffect, reportsErrorWhenAskingForNonExistingAttributeSemantic)
    {
        const Effect* effect = sharedTestState->effect;
        AttributeInput input;
        EXPECT_NE(StatusOK, effect->findAttributeInput(ramses::EEffectAttributeSemantic::Invalid, input));
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
        EXPECT_EQ(ramses_internal::EDataType::Vector3F, input.impl.getDataType());
        EXPECT_EQ(ramses_internal::EFixedSemantics::Invalid, input.impl.getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::Invalid, input.getSemantics());
        EXPECT_EQ(5u, input.impl.getInputIndex());
        EXPECT_TRUE(input.isValid());
    }

    TEST_F(AnEffectWithSemantics, findsUniformInputBySemantic)
    {
        const Effect* effect = sharedTestState->effect;
        UniformInput input;
        EXPECT_EQ(StatusOK, effect->findUniformInput(ramses::EEffectUniformSemantic::ModelViewMatrix, input));

        EXPECT_STREQ("matrix44fInput", input.getName());
        EXPECT_EQ(1u, input.getElementCount());
        EXPECT_EQ(effect->impl.getLowlevelResourceHash(), input.impl.getEffectHash());
        EXPECT_EQ(EEffectInputDataType_Matrix44F, input.getDataType());
        EXPECT_EQ(ramses_internal::EDataType::Matrix44F, input.impl.getDataType());
        EXPECT_EQ(ramses_internal::EFixedSemantics::ModelViewMatrix, input.impl.getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::ModelViewMatrix, input.getSemantics());
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
        EXPECT_EQ(ramses_internal::EDataType::Vector2Buffer, input.impl.getDataType());
        EXPECT_EQ(ramses_internal::EFixedSemantics::Invalid, input.impl.getSemantics());
        EXPECT_EQ(EEffectAttributeSemantic::Invalid, input.getSemantics());
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
        EXPECT_EQ(ramses_internal::EDataType::Vector2Buffer, input.impl.getDataType());
        EXPECT_EQ(ramses_internal::EFixedSemantics::TextPositionsAttribute, input.impl.getSemantics());
        EXPECT_EQ(EEffectAttributeSemantic::TextPositions, input.getSemantics());
        EXPECT_EQ(1u, input.impl.getInputIndex());
        EXPECT_TRUE(input.isValid());
    }

    TEST_F(AnEffectWithSemantics, findsAttributeInputBySemantic)
    {
        const Effect* effect = sharedTestState->effect;
        AttributeInput input;
        EXPECT_EQ(StatusOK, effect->findAttributeInput(ramses::EEffectAttributeSemantic::TextPositions, input));

        EXPECT_STREQ("vec2fArrayInput", input.getName());
        EXPECT_EQ(effect->impl.getLowlevelResourceHash(), input.impl.getEffectHash());
        EXPECT_EQ(EEffectInputDataType_Vector2F, input.getDataType());
        EXPECT_EQ(ramses_internal::EDataType::Vector2Buffer, input.impl.getDataType());
        EXPECT_EQ(ramses_internal::EFixedSemantics::TextPositionsAttribute, input.impl.getSemantics());
        EXPECT_EQ(EEffectAttributeSemantic::TextPositions, input.getSemantics());
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
        EXPECT_EQ(ramses_internal::EDataType::Vector3F, input.impl.getDataType());
        EXPECT_EQ(ramses_internal::EFixedSemantics::Invalid, input.impl.getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::Invalid, input.getSemantics());
        EXPECT_EQ(5u, input.impl.getInputIndex());
        EXPECT_TRUE(input.isValid());
    }

    TEST_F(AnEffect, findsAllTextureTypesOfUniformInputByName)
    {
        const Effect* effect = sharedTestState->effect;
        UniformInput input2d;
        EXPECT_EQ(StatusOK,effect->findUniformInput("texture2dInput", input2d));
        UniformInput input3d;
        EXPECT_EQ(StatusOK, effect->findUniformInput("texture3dInput", input3d));
        UniformInput inputcube;
        EXPECT_EQ(StatusOK, effect->findUniformInput("textureCubeInput", inputcube));

        EXPECT_EQ(EEffectInputDataType_TextureSampler2D, input2d.getDataType());
        EXPECT_TRUE(input2d.isValid());
        EXPECT_EQ(EEffectInputDataType_TextureSampler3D, input3d.getDataType());
        EXPECT_TRUE(input3d.isValid());
        EXPECT_EQ(EEffectInputDataType_TextureSamplerCube, inputcube.getDataType());
        EXPECT_TRUE(inputcube.isValid());
    }

    TEST_F(AnEffect, findsAttributeInputByName)
    {
        const Effect* effect = sharedTestState->effect;
        AttributeInput input;
        EXPECT_EQ(StatusOK, effect->findAttributeInput("vec2fArrayInput", input));

        EXPECT_STREQ("vec2fArrayInput", input.getName());
        EXPECT_EQ(effect->impl.getLowlevelResourceHash(), input.impl.getEffectHash());
        EXPECT_EQ(EEffectInputDataType_Vector2F, input.getDataType());
        EXPECT_EQ(ramses_internal::EDataType::Vector2Buffer, input.impl.getDataType());
        EXPECT_EQ(ramses_internal::EFixedSemantics::Invalid, input.impl.getSemantics());
        EXPECT_EQ(EEffectAttributeSemantic::Invalid, input.getSemantics());
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
        EXPECT_EQ(ramses_internal::EDataType::Vector2Buffer, input.impl.getDataType());
        EXPECT_EQ(ramses_internal::EFixedSemantics::TextPositionsAttribute, input.impl.getSemantics());
        EXPECT_EQ(EEffectAttributeSemantic::TextPositions, input.getSemantics());
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
        EXPECT_EQ(EEffectInputDataType_TextureSampler2D, input.getDataType());
        EXPECT_EQ(ramses_internal::EDataType::TextureSampler2D, input.impl.getDataType());
        EXPECT_EQ(ramses_internal::EFixedSemantics::TextTexture, input.impl.getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::TextTexture, input.getSemantics());
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
        EXPECT_NE(static_cast<Effect*>(nullptr), sharedTestState->getScene().impl.createEffect(effectDesc, ResourceCacheFlag_DoNotCache, ""));

        effectDesc.setAttributeSemantic("a_position", EEffectAttributeSemantic::TextPositions);

        /// Can not create ...
        EXPECT_EQ(static_cast<Effect*>(nullptr), sharedTestState->getScene().impl.createEffect(effectDesc, ResourceCacheFlag_DoNotCache, ""));
    }

    TEST_F(AnEffect, canRetrieveGLSLErrorMessageFromClient)
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
        const Effect* effect = sharedTestState->getScene().createEffect(effectDesc, ResourceCacheFlag_DoNotCache);
        EXPECT_EQ(nullptr, effect);
        using namespace ::testing;
        EXPECT_THAT(sharedTestState->getScene().getLastEffectErrorMessages(),
                    AnyOf(Eq("[GLSL Compiler] vertex shader Shader Parsing Error:\n"
                             "ERROR: 2:5: '' :  syntax error\n"
                             "ERROR: 1 compilation errors.  No code generated.\n\n\n"),
                          Eq("[GLSL Compiler] vertex shader Shader Parsing Error:\n"
                             "ERROR: 2:5: '' :  syntax error, unexpected RIGHT_BRACE, expecting COMMA or SEMICOLON\n"
                             "ERROR: 1 compilation errors.  No code generated.\n\n\n")));
    }

    TEST_F(AnEffect, clientDeletesEffectErrorMessagesOfLastEffect)
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
        const Effect* effect1 = sharedTestState->getScene().createEffect(effectDesc, ResourceCacheFlag_DoNotCache);
        EXPECT_EQ(nullptr, effect1);
        EXPECT_NE("", sharedTestState->getScene().getLastEffectErrorMessages());

        effectDesc.setVertexShader("#version 100\n"
                                   "attribute float inp;\n"
                                   "void main(void)\n"
                                   "{\n"
                                   "    gl_Position = vec4(0.0)\n;"
                                   "}\n");

        const Effect* effect2 = sharedTestState->getScene().createEffect(effectDesc, ResourceCacheFlag_DoNotCache);
        EXPECT_NE(nullptr, effect2);
        EXPECT_EQ("", sharedTestState->getScene().getLastEffectErrorMessages());
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
        EXPECT_NE(static_cast<Effect*>(nullptr), sharedTestState->getScene().impl.createEffect(effectDesc, ResourceCacheFlag_DoNotCache, ""));

        effectDesc.setAttributeSemantic("a_texcoord", EEffectAttributeSemantic::TextTextureCoordinates);

        /// Can not create ...
        EXPECT_EQ(static_cast<Effect*>(nullptr), sharedTestState->getScene().impl.createEffect(effectDesc, ResourceCacheFlag_DoNotCache, ""));
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
        EXPECT_NE(static_cast<Effect*>(nullptr), sharedTestState->getScene().impl.createEffect(effectDesc, ResourceCacheFlag_DoNotCache, ""));

        effectDesc.setUniformSemantic("u_texture", EEffectUniformSemantic::TextTexture);

        /// Can not create ...
        EXPECT_EQ(static_cast<Effect*>(nullptr), sharedTestState->getScene().impl.createEffect(effectDesc, ResourceCacheFlag_DoNotCache, ""));
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

    TEST_F(AnEffectWithGeometryShader, providesExpectedGeometryInputType_Points)
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

        Effect* effect = sharedTestState->getScene().createEffect(effectDesc);

        ASSERT_NE(nullptr, effect);
        EDrawMode geometryShaderInput;
        EXPECT_EQ(StatusOK, Effect::getGeometryShaderInputType(*effect, geometryShaderInput));
        EXPECT_EQ(geometryShaderInput, EDrawMode::EDrawMode_Points);
    }

    TEST_F(AnEffectWithGeometryShader, providesExpectedGeometryInputType_Lines)
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

        Effect* effect = sharedTestState->getScene().createEffect(effectDesc);
        ASSERT_NE(nullptr, effect);
        EDrawMode geometryShaderInput;
        EXPECT_EQ(StatusOK, Effect::getGeometryShaderInputType(*effect, geometryShaderInput));
        EXPECT_EQ(geometryShaderInput, EDrawMode::EDrawMode_Lines);
    }

    TEST_F(AnEffectWithGeometryShader, providesExpectedGeometryInputType_Triangles)
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

        Effect* effect = sharedTestState->getScene().createEffect(effectDesc);
        ASSERT_NE(nullptr, effect);
        EDrawMode geometryShaderInput;
        EXPECT_EQ(StatusOK, Effect::getGeometryShaderInputType(*effect, geometryShaderInput));
        EXPECT_EQ(geometryShaderInput, EDrawMode::EDrawMode_Triangles);
    }
}
