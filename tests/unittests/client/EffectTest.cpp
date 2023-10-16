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
#include "gtest/gtest.h"

using namespace testing;

namespace ramses::internal
{
    class AnEffect : public ::testing::Test
    {
    public:
        static void SetUpTestSuite()
        {
            sharedTestState = std::make_unique<TestEffectCreator>();
        }

        static void TearDownTestSuite()
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
        static void SetUpTestSuite()
        {
            sharedTestState = std::make_unique<TestEffectCreator>(true);
        }
    };

    TEST_F(AnEffect, hasProperNumberOfInputs)
    {
        const Effect* effect = sharedTestState->effect;
        EXPECT_EQ(29u, effect->getUniformInputCount());
        EXPECT_EQ(4u, effect->getAttributeInputCount());
    }

    TEST_F(AnEffect, hasNoGeometryShaderWhenNotCreatedWithSuch)
    {
        EXPECT_FALSE(sharedTestState->effect->hasGeometryShader());
    }

    TEST_F(AnEffect, reportsErrorWhenAskedForGeometryInputType_ButNoGeometryShaderProvided)
    {
        EDrawMode mode;
        EXPECT_FALSE(sharedTestState->effect->getGeometryShaderInputType(mode));
    }

    TEST_F(AnEffect, reportsErrorWhenAskingForNonExistingUniformInput)
    {
        const Effect* effect = sharedTestState->effect;
        const std::optional<UniformInput> optInput = effect->getUniformInput(99u);
        EXPECT_FALSE(optInput.has_value());
    }

    TEST_F(AnEffect, reportsErrorWhenAskingForNonExistingAttributeInput)
    {
        const Effect* effect = sharedTestState->effect;
        const std::optional<AttributeInput> optInput = effect->getAttributeInput(99u);
        EXPECT_FALSE(optInput.has_value());
    }

    TEST_F(AnEffect, reportsErrorWhenAskingForNonExistingUniformName)
    {
        const Effect* effect = sharedTestState->effect;
        const std::optional<UniformInput> optInput = effect->findUniformInput("xxx");
        EXPECT_FALSE(optInput.has_value());
    }

    TEST_F(AnEffect, reportsErrorWhenAskingForNonExistingAttributeName)
    {
        const Effect* effect = sharedTestState->effect;
        const std::optional<AttributeInput> optInput = effect->findAttributeInput("xxx");
        EXPECT_FALSE(optInput.has_value());
    }

    TEST_F(AnEffect, reportsErrorWhenAskingForNonExistingUniformSemantic)
    {
        const Effect* effect = sharedTestState->effect;
        const std::optional<UniformInput> optInput = effect->findUniformInput(EEffectUniformSemantic::NormalMatrix);
        EXPECT_FALSE(optInput.has_value());
    }

    TEST_F(AnEffect, reportsErrorWhenAskingForNonExistingAttributeSemantic)
    {
        const Effect* effect = sharedTestState->effect;
        const std::optional<AttributeInput> optInput = effect->findAttributeInput(EEffectAttributeSemantic::Invalid);
        EXPECT_FALSE(optInput.has_value());
    }

    TEST_F(AnEffect, getsUniformInputByIndex)
    {
        const Effect* effect = sharedTestState->effect;
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
    }

    TEST_F(AnEffectWithSemantics, findsUniformInputBySemantic)
    {
        const Effect* effect = sharedTestState->effect;
        const std::optional<UniformInput> optInput = effect->findUniformInput(EEffectUniformSemantic::ModelViewMatrix);
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("matrix44fInput", optInput->getName());
        EXPECT_EQ(1u, optInput->getElementCount());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::Matrix44F, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::Matrix44F, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::ModelViewMatrix, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::ModelViewMatrix, optInput->getSemantics());
    }

    TEST_F(AnEffect, getsAttributeInputByIndex)
    {
        const Effect*  effect = sharedTestState->effect;
        const std::optional<AttributeInput> optInput = effect->getAttributeInput(1u);
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("vec2fArrayInput", optInput->getName());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::Vector2F, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::Vector2Buffer, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::Invalid, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectAttributeSemantic::Invalid, optInput->getSemantics());
        EXPECT_EQ(1u, optInput->impl().getInputIndex());
    }

    TEST_F(AnEffectWithSemantics, getsAttributeInputByIndex)
    {
        const Effect* effect = sharedTestState->effect;
        const std::optional<AttributeInput> optInput = effect->getAttributeInput(1u);
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("vec2fArrayInput", optInput->getName());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::Vector2F, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::Vector2Buffer, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::TextPositionsAttribute, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectAttributeSemantic::TextPositions, optInput->getSemantics());
        EXPECT_EQ(1u, optInput->impl().getInputIndex());
    }

    TEST_F(AnEffectWithSemantics, findsAttributeInputBySemantic)
    {
        const Effect* effect = sharedTestState->effect;
        const std::optional<AttributeInput> optInput = effect->findAttributeInput(EEffectAttributeSemantic::TextPositions);
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("vec2fArrayInput", optInput->getName());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::Vector2F, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::Vector2Buffer, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::TextPositionsAttribute, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectAttributeSemantic::TextPositions, optInput->getSemantics());
    }

    TEST_F(AnEffect, findsUniformInputByName)
    {
        const Effect* effect = sharedTestState->effect;
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
    }

    TEST_F(AnEffect, findsAllTextureTypesOfUniformInputByName)
    {
        const Effect* effect = sharedTestState->effect;
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

    TEST_F(AnEffect, findsAttributeInputByName)
    {
        const Effect* effect = sharedTestState->effect;
        const std::optional<AttributeInput> optInput = effect->findAttributeInput("vec2fArrayInput");
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("vec2fArrayInput", optInput->getName());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::Vector2F, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::Vector2Buffer, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::Invalid, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectAttributeSemantic::Invalid, optInput->getSemantics());
        EXPECT_EQ(1u, optInput->impl().getInputIndex());
    }

    TEST_F(AnEffectWithSemantics, findsAttributeInputByName)
    {
        const Effect*  effect = sharedTestState->effect;
        const std::optional<AttributeInput> optInput = effect->findAttributeInput("vec2fArrayInput");
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("vec2fArrayInput", optInput->getName());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::Vector2F, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::Vector2Buffer, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::TextPositionsAttribute, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectAttributeSemantic::TextPositions, optInput->getSemantics());
        EXPECT_EQ(1u, optInput->impl().getInputIndex());
    }

    TEST_F(AnEffectWithSemantics, findsTextureInputWithSemantics)
    {
        const Effect* effect = sharedTestState->effect;
        const std::optional<UniformInput> optInput = effect->findUniformInput("texture2dInput");
        ASSERT_TRUE(optInput.has_value());

        EXPECT_STREQ("texture2dInput", optInput->getName());
        EXPECT_EQ(effect->impl().getLowlevelResourceHash(), optInput->impl().getEffectHash());
        EXPECT_EQ(ramses::EDataType::TextureSampler2D, optInput->getDataType());
        EXPECT_EQ(ramses::internal::EDataType::TextureSampler2D, optInput->impl().getInternalDataType());
        EXPECT_EQ(ramses::internal::EFixedSemantics::TextTexture, optInput->impl().getSemantics());
        EXPECT_EQ(EEffectUniformSemantic::TextTexture, optInput->getSemantics());
        EXPECT_EQ(24u, optInput->impl().getInputIndex());
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
        EXPECT_NE(static_cast<Effect*>(nullptr), sharedTestState->getScene().impl().createEffect(effectDesc, ""));

        effectDesc.setAttributeSemantic("a_position", EEffectAttributeSemantic::TextPositions);

        /// Can not create ...
        EXPECT_EQ(static_cast<Effect*>(nullptr), sharedTestState->getScene().impl().createEffect(effectDesc, ""));
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
        const Effect* effect = sharedTestState->getScene().createEffect(effectDesc);
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
        const Effect* effect1 = sharedTestState->getScene().createEffect(effectDesc);
        EXPECT_EQ(nullptr, effect1);
        EXPECT_NE("", sharedTestState->getScene().getLastEffectErrorMessages());

        effectDesc.setVertexShader("#version 100\n"
                                   "attribute float inp;\n"
                                   "void main(void)\n"
                                   "{\n"
                                   "    gl_Position = vec4(0.0)\n;"
                                   "}\n");

        const Effect* effect2 = sharedTestState->getScene().createEffect(effectDesc);
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
        EXPECT_NE(static_cast<Effect*>(nullptr), sharedTestState->getScene().impl().createEffect(effectDesc, ""));

        effectDesc.setAttributeSemantic("a_texcoord", EEffectAttributeSemantic::TextTextureCoordinates);

        /// Can not create ...
        EXPECT_EQ(static_cast<Effect*>(nullptr), sharedTestState->getScene().impl().createEffect(effectDesc, ""));
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
        EXPECT_NE(static_cast<Effect*>(nullptr), sharedTestState->getScene().impl().createEffect(effectDesc, ""));

        effectDesc.setUniformSemantic("u_texture", EEffectUniformSemantic::TextTexture);

        /// Can not create ...
        EXPECT_EQ(static_cast<Effect*>(nullptr), sharedTestState->getScene().impl().createEffect(effectDesc, ""));
    }

    TEST_F(AnEffect, supportsBoolUniforms)
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

        const Effect* effect = sharedTestState->getScene().createEffect(effectDesc);

        EXPECT_EQ("", sharedTestState->getScene().getLastEffectErrorMessages());

        ASSERT_NE(nullptr, effect);

        const std::optional<UniformInput> optUniform = effect->findUniformInput("u_theBool");
        ASSERT_TRUE(optUniform.has_value());

        Appearance* appearance = sharedTestState->getScene().createAppearance(*effect);
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
        EXPECT_TRUE(effect->hasGeometryShader());
        EDrawMode geometryShaderInput;
        EXPECT_TRUE(effect->getGeometryShaderInputType(geometryShaderInput));
        EXPECT_EQ(geometryShaderInput, EDrawMode::Points);
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
        EXPECT_TRUE(effect->hasGeometryShader());
        EDrawMode geometryShaderInput;
        EXPECT_TRUE(effect->getGeometryShaderInputType(geometryShaderInput));
        EXPECT_EQ(geometryShaderInput, EDrawMode::Lines);
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
        EXPECT_TRUE(effect->getGeometryShaderInputType(geometryShaderInput));
        EXPECT_EQ(geometryShaderInput, EDrawMode::Triangles);
    }

    TEST_F(AnEffectWithGeometryShader, providesExpectedGeometryInputType_whenMultipleIdenticalEffectsCreated)
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

        Effect* effect1 = sharedTestState->getScene().createEffect(effectDesc);
        Effect* effect2 = sharedTestState->getScene().createEffect(effectDesc);
        ASSERT_NE(nullptr, effect1);
        ASSERT_NE(nullptr, effect2);
        EDrawMode geometryShaderInput;
        EXPECT_TRUE(effect1->getGeometryShaderInputType(geometryShaderInput));
        EXPECT_EQ(geometryShaderInput, EDrawMode::Triangles);
        EXPECT_TRUE(effect2->getGeometryShaderInputType(geometryShaderInput));
        EXPECT_EQ(geometryShaderInput, EDrawMode::Triangles);
    }

    TEST_F(AnEffect, sceneValiadationProducesShaderWarnings)
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

        const auto* effect = sharedTestState->getScene().createEffect(effectDesc);
        EXPECT_NE(nullptr, effect);

        ValidationReport report;
        sharedTestState->getScene().validate(report);
        EXPECT_TRUE(report.hasIssue());

        EXPECT_THAT(report.impl().toString(), HasSubstr("Precision mismatch: 'v_texcoord'. (Vertex: smooth out lowp 2-component vector of float, Fragment: smooth in highp 2-component vector of float)"));
        ValidationReport report2;
        sharedTestState->getScene().validate(report2);
        EXPECT_EQ(report.getIssues(), report2.getIssues());
    }
}
