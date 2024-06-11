//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/glslEffectBlock/GlslEffect.h"
#include "internal/PlatformAbstraction/PlatformThread.h"
#include "internal/SceneGraph/Resource/EffectResource.h"
#include "gmock/gmock.h"

#include <memory>
#include <string_view>
#include <string>

namespace ramses::internal
{
    class AGlslEffect : public ::testing::Test
    {
    public:
        const std::string basicVertexShader = R"SHADER(
                #version 320 es
                void main(void)
                {
                    gl_Position = vec4(0.0);
                }
                )SHADER";
        const std::string basicFragmentShader = R"SHADER(
                #version 320 es
                layout(location=0) out lowp vec4 colorOut;
                void main(void)
                {
                    colorOut = vec4(0.0);
                })SHADER";
        const std::string basicFragmentShader310 = R"SHADER(
                #version 310 es
                layout(location=0) out lowp vec4 colorOut;
                void main(void)
                {
                    colorOut = vec4(0.0);
                })SHADER";
        const std::string basicGeometryShader = R"SHADER(
                #version 320 es
                layout(points) in;
                layout(points, max_vertices = 1) out;
                void main() {
                    gl_Position = vec4(0.0);
                    EmitVertex();
                }
                )SHADER";
        const std::vector<std::string> emptyCompilerDefines{};
        const SemanticsMap emptySemanticInputs{};

    protected:
        static void VerifyUniformInputExists(const EffectResource& effect, std::string_view uniformName)
        {
            const DataFieldHandle effecthandle = effect.getUniformDataFieldHandleByName(std::string(uniformName));
            EXPECT_TRUE(effecthandle.isValid());
        };

        static void CheckSPIRVShaderSanity(const uint32_t* spirvShader)
        {
            //check alignment
            auto spirvShaderAddress = reinterpret_cast<uintptr_t>(spirvShader);
            EXPECT_EQ(0u, spirvShaderAddress % sizeof(uint32_t));

            // simply sanity check on contents of SPIRV data
            constexpr uint32_t spirvMagicNumber = 0x07230203;
            EXPECT_EQ(spirvMagicNumber, spirvShader[0]);
        }
    };

    TEST_F(AGlslEffect, canParseBasicShaders)
    {
        GlslEffect ge(basicVertexShader, basicFragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        EXPECT_EQ(0u, res->getUniformInputs().size());
        EXPECT_EQ(0u, res->getAttributeInputs().size());
        EXPECT_FALSE(res->getGeometryShaderInputType().has_value());
        EXPECT_EQ(std::string(), res->getName());
    }

    TEST_F(AGlslEffect, canParseBasicShaders_WithGeometryShader)
    {
        GlslEffect ge(basicVertexShader, basicFragmentShader, basicGeometryShader, emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        EXPECT_EQ(0u, res->getUniformInputs().size());
        EXPECT_EQ(0u, res->getAttributeInputs().size());
        EXPECT_EQ(EDrawMode::Points, res->getGeometryShaderInputType());
        EXPECT_EQ(std::string(), res->getName());
    }

    TEST_F(AGlslEffect, canParseGeometryShaderWithTriangles)
    {
        const std::string geometryShaderTriangles = R"SHADER(
                #version 320 es
                layout(triangles) in;
                layout(points, max_vertices = 1) out;
                void main() {
                    gl_Position = vec4(0.0);
                }
                )SHADER";

        GlslEffect ge(basicVertexShader, basicFragmentShader, geometryShaderTriangles, emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        EXPECT_EQ(0u, res->getUniformInputs().size());
        EXPECT_EQ(0u, res->getAttributeInputs().size());
        EXPECT_EQ(EDrawMode::Triangles, res->getGeometryShaderInputType());
        EXPECT_EQ(std::string(), res->getName());
    }

    TEST_F(AGlslEffect, usesPassedName)
    {
        GlslEffect ge(basicVertexShader, basicFragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "someName");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        EXPECT_EQ(std::string("someName"), res->getName());
    }

    TEST_F(AGlslEffect, rejectsEmptyVertexShader)
    {
        GlslEffect ge("", basicFragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        EXPECT_FALSE(res);
    }

    TEST_F(AGlslEffect, rejectsBrokenVertexShader)
    {
        GlslEffect ge("foo", basicFragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        EXPECT_FALSE(res);
    }

    TEST_F(AGlslEffect, rejectsEmptyFragmentShader)
    {
        GlslEffect ge(basicVertexShader, "", "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        EXPECT_FALSE(res);
    }

    TEST_F(AGlslEffect, rejectsBrokenFragmentShader)
    {
        GlslEffect ge(basicVertexShader, "bar", "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        EXPECT_FALSE(res);
    }

    TEST_F(AGlslEffect, acceptsEmptyGeometryShader)
    {
        GlslEffect ge(basicVertexShader, basicFragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        EXPECT_TRUE(res);
    }

    TEST_F(AGlslEffect, rejectsBrokenGeometryShader)
    {
        GlslEffect ge(basicVertexShader, basicFragmentShader, "bar", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        EXPECT_FALSE(res);
    }

    TEST_F(AGlslEffect, usesProvidedDefines)
    {
        const char* vertexShader =
            "void main(void)\n"
            "{\n"
            "    gl_Position = DEFINE_ZERO;\n"
            "}\n";
        const char* fragmentShader =
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = DEFINE_ONE;\n"
            "}\n";
        std::vector<std::string> compilerDefines;
        compilerDefines.emplace_back("DEFINE_ZERO vec4(0.0)");
        compilerDefines.emplace_back("DEFINE_ONE vec4(1.0)");
        GlslEffect ge(vertexShader, fragmentShader, "", compilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        EXPECT_THAT(res->getVertexShader(), ::testing::HasSubstr(compilerDefines[0]));
        EXPECT_THAT(res->getVertexShader(), ::testing::HasSubstr(compilerDefines[1]));
        EXPECT_THAT(res->getFragmentShader(), ::testing::HasSubstr(compilerDefines[0]));
        EXPECT_THAT(res->getFragmentShader(), ::testing::HasSubstr(compilerDefines[1]));
    }

    TEST_F(AGlslEffect, generatedShaderWithDefaultVersionAndDefinesEmbedded)
    {
        // this test will break if anything is changed in shader generation (even if no semantic change like adding newline!)
        const char* vertexShader =
            "void main(void)\n"
            "{\n"
            "    gl_Position = vec4(0.0);\n"
            "}\n";
        const char* fragmentShader =
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = vec4(0.0);\n"
            "}\n";
        std::vector<std::string> compilerDefines;
        compilerDefines.emplace_back("FIRST_DEFINE foo");
        compilerDefines.emplace_back("OTHER_DEFINE bar");
        GlslEffect ge(vertexShader, fragmentShader, "", compilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        const char* expectedVertexShader =
            "#version 100\n"
            "#define FIRST_DEFINE foo\n"
            "#define OTHER_DEFINE bar\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = vec4(0.0);\n"
            "}\n";
        const char* expectedFragmentShader =
            "#version 100\n"
            "#define FIRST_DEFINE foo\n"
            "#define OTHER_DEFINE bar\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = vec4(0.0);\n"
            "}\n";

        EXPECT_STREQ(expectedVertexShader, res->getVertexShader());
        EXPECT_STREQ(expectedFragmentShader, res->getFragmentShader());
    }

    TEST_F(AGlslEffect, acceptsGLSLESShaders_Version300es)
    {
        const char* vertexShader = R"SHADER(
                #version 300 es
                in lowp vec3 a_position;
                out lowp vec3 v_position;
                void main(void)
                {
                    v_position = a_position;
                    gl_Position = vec4(a_position, 1.0);
                })SHADER";
        const char* fragmentShader = R"SHADER(
                #version 300 es
                in lowp vec3 v_position;
                out lowp vec4 color;
                void main(void)
                {
                    color = vec4(v_position, 1.0);
                })SHADER";

        GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));

        EXPECT_TRUE(res);
    }

    TEST_F(AGlslEffect, acceptsGLSLESShaders_Version310es)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                in lowp vec3 a_position;
                out lowp vec3 v_position;
                void main(void)
                {
                    v_position = a_position;
                    gl_Position = vec4(a_position, 1.0);
                })SHADER";
        const char* fragmentShader = R"SHADER(
                #version 310 es
                in lowp vec3 v_position;
                out lowp vec4 color;
                void main(void)
                {
                    color = vec4(v_position, 1.0);
                })SHADER";

        GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));

        EXPECT_TRUE(res);
    }

    TEST_F(AGlslEffect, acceptsGLSLESShaders_Version310esWithGeometryShaderExtension)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                in lowp vec3 a_position;
                out lowp vec3 v_position;
                void main(void)
                {
                    v_position = a_position;
                    gl_Position = vec4(a_position, 1.0);
                })SHADER";
        const char* fragmentShader = R"SHADER(
                #version 310 es
                in lowp vec3 v_position;
                out lowp vec4 color;
                void main(void)
                {
                    color = vec4(v_position, 1.0);
                })SHADER";

        const std::string geometryShader = R"SHADER(
                #version 310 es
                #extension GL_EXT_geometry_shader : enable
                layout(points) in;
                layout(points, max_vertices = 1) out;
                void main() {
                    gl_Position = vec4(0.0);
                    EmitVertex();
                }
                )SHADER";

        GlslEffect ge(vertexShader, fragmentShader, geometryShader, emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));

        EXPECT_EQ(EDrawMode::Points, res->getGeometryShaderInputType());

        EXPECT_TRUE(res);
    }

    TEST_F(AGlslEffect, doesNotAcceptMixedES2VertexAndES3FragmentShaders)
    {
        const char* vertexShader =
            "#version 100\n"
            "attribute lowp vec3 a_position;\n"
            "varying lowp vec3 v_position;\n"
            "void main(void)\n"
            "{\n"
            "    v_position = a_position;\n"
            "    gl_Position = vec4(a_position, 1.0);\n"
            "}\n";
        const char* fragmentShader =
            "#version 300 es\n"
            "in lowp vec3 v_position;\n"
            "out lowp vec4 color;\n"
            "void main(void)\n"
            "{\n"
            "    color = vec4(v_position, 1.0);\n"
            "}\n";

        GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));

        EXPECT_FALSE(res);
    }

    TEST_F(AGlslEffect, doesNotAcceptMixedES3VertexAndES2FragmentShaders)
    {
        const char* vertexShader =
            "#version 300 es\n"
            "in lowp vec3 a_position;\n"
            "out lowp vec3 v_position;\n"
            "void main(void)\n"
            "{\n"
            "    v_position = a_position;\n"
            "    gl_Position = vec4(a_position, 1.0);\n"
            "}\n";
        const char* fragmentShader =
            "#version 100\n"
            "varying lowp vec3 v_position;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = vec4(v_position, 1.0);\n"
            "}\n";

        GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));

        EXPECT_FALSE(res);
    }

    TEST_F(AGlslEffect, canParseShaderInputs)
    {
        const char* vertexShader = R"SHADER(
                #version 320 es
                precision highp float;
                uniform bool uniformBool1;
                uniform mat4 uniformWithSemantic;
                uniform mat3 matrix3x3;
                uniform mat2 matrix2x2;
                in vec3 attributeWithSemantic;
                in float attributeFloat;

                void main(void)
                {
                    gl_Position = vec4(0.0);
                })SHADER";
        const char* fragmentShader = R"SHADER(
                #version 320 es
                precision highp float;
                uniform bool uniformBool2;
                uniform sampler2D uniformSampler;
                uniform vec4 uniformVec;
                out vec4 colorOut;
                void main(void)
                {
                    colorOut = vec4(0.0);
                })SHADER";
        const std::string geometryShader = R"SHADER(
                #version 320 es
                precision highp float;
                layout(points) in;
                layout(points, max_vertices = 1) out;
                out vec4 g_colorOut;
                uniform bool uniformBool3;
                uniform float uniformGeomFloat;
                uniform vec4 uniformGeomVec;
                uniform sampler2D uniformGeomSampler;
                void main() {
                    gl_Position = uniformGeomVec + vec4(uniformGeomFloat);
                    g_colorOut = texture(uniformGeomSampler, vec2(0.0));
                    EmitVertex();
                }
                )SHADER";

        const SemanticsMap semantics{
            {"uniformWithSemantic", EFixedSemantics::ModelViewProjectionMatrix},
            {"attributeWithSemantic", EFixedSemantics::CameraWorldPosition}
        };

        GlslEffect ge(vertexShader, fragmentShader, geometryShader, emptyCompilerDefines, semantics, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        const EffectInputInformationVector& uniforms = res->getUniformInputs();
        const EffectInputInformationVector& attributes = res->getAttributeInputs();

        ASSERT_EQ(11u, uniforms.size());
        EXPECT_EQ(EffectInputInformation("uniformBool1", 1, ramses::internal::EDataType::Bool, EFixedSemantics::Invalid), uniforms[0]);
        EXPECT_EQ(EffectInputInformation("uniformWithSemantic", 1, ramses::internal::EDataType::Matrix44F, EFixedSemantics::ModelViewProjectionMatrix), uniforms[1]);
        EXPECT_EQ(EffectInputInformation("matrix3x3", 1, ramses::internal::EDataType::Matrix33F, EFixedSemantics::Invalid), uniforms[2]);
        EXPECT_EQ(EffectInputInformation("matrix2x2", 1, ramses::internal::EDataType::Matrix22F, EFixedSemantics::Invalid), uniforms[3]);
        EXPECT_EQ(EffectInputInformation("uniformBool2", 1, ramses::internal::EDataType::Bool, EFixedSemantics::Invalid), uniforms[4]);
        EXPECT_EQ(EffectInputInformation("uniformSampler", 1, ramses::internal::EDataType::TextureSampler2D, EFixedSemantics::Invalid), uniforms[5]);
        EXPECT_EQ(EffectInputInformation("uniformVec", 1, ramses::internal::EDataType::Vector4F, EFixedSemantics::Invalid), uniforms[6]);
        EXPECT_EQ(EffectInputInformation("uniformBool3", 1, ramses::internal::EDataType::Bool, EFixedSemantics::Invalid), uniforms[7]);
        EXPECT_EQ(EffectInputInformation("uniformGeomFloat", 1, ramses::internal::EDataType::Float, EFixedSemantics::Invalid), uniforms[8]);
        EXPECT_EQ(EffectInputInformation("uniformGeomVec", 1, ramses::internal::EDataType::Vector4F, EFixedSemantics::Invalid), uniforms[9]);
        EXPECT_EQ(EffectInputInformation("uniformGeomSampler", 1, ramses::internal::EDataType::TextureSampler2D, EFixedSemantics::Invalid), uniforms[10]);

        ASSERT_EQ(2u, attributes.size());
        EXPECT_EQ(EffectInputInformation("attributeWithSemantic", 1, ramses::internal::EDataType::Vector3Buffer, EFixedSemantics::CameraWorldPosition), attributes[0]);
        EXPECT_EQ(EffectInputInformation("attributeFloat", 1, ramses::internal::EDataType::FloatBuffer, EFixedSemantics::Invalid), attributes[1]);
    }

    TEST_F(AGlslEffect, canParseShaderInputs_UBOs)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 uboMat1;
                    float uboFloat1[10];
                    mat3 uboMat2;
                } uniformBuffer1;

                layout(std140,binding=2) uniform uniformBuffer_t2
                {
                    mat4 uboMat1;
                    float uboFloat1;
                    mat3 uboMat2;
                } uniformBuffer2;

                void main(void)
                {
                    gl_Position = uniformBuffer1.uboMat1 * uniformBuffer2.uboMat1 * vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        const EffectInputInformationVector& uniforms = res->getUniformInputs();

        ASSERT_EQ(8u, uniforms.size());
        EXPECT_EQ(EffectInputInformation("uniformBuffer1", 1, ramses::internal::EDataType::UniformBuffer, EFixedSemantics::Invalid,     UniformBufferBinding{ 1u},  UniformBufferElementSize{ 272u },  UniformBufferFieldOffset{}),       uniforms[0]);
        EXPECT_EQ(EffectInputInformation("uniformBuffer1.uboMat1", 1, ramses::internal::EDataType::Matrix44F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 64u },   UniformBufferFieldOffset{ 0u }),   uniforms[1]);
        EXPECT_EQ(EffectInputInformation("uniformBuffer1.uboFloat1", 10, ramses::internal::EDataType::Float, EFixedSemantics::Invalid,  UniformBufferBinding{ 1u},  UniformBufferElementSize{ 16u },   UniformBufferFieldOffset{ 64u}),   uniforms[2]);
        EXPECT_EQ(EffectInputInformation("uniformBuffer1.uboMat2", 1, ramses::internal::EDataType::Matrix33F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 48u },   UniformBufferFieldOffset{ 224u }), uniforms[3]);

        EXPECT_EQ(EffectInputInformation("uniformBuffer2", 1, ramses::internal::EDataType::UniformBuffer, EFixedSemantics::Invalid,     UniformBufferBinding{ 2u},  UniformBufferElementSize{ 128 },    UniformBufferFieldOffset{}),        uniforms[4]);
        EXPECT_EQ(EffectInputInformation("uniformBuffer2.uboMat1", 1, ramses::internal::EDataType::Matrix44F, EFixedSemantics::Invalid, UniformBufferBinding{ 2u},  UniformBufferElementSize{ 64u },    UniformBufferFieldOffset{ 0u }),    uniforms[5]);
        EXPECT_EQ(EffectInputInformation("uniformBuffer2.uboFloat1", 1, ramses::internal::EDataType::Float, EFixedSemantics::Invalid,   UniformBufferBinding{ 2u},  UniformBufferElementSize{ 4u },     UniformBufferFieldOffset{ 64u }),   uniforms[6]);
        EXPECT_EQ(EffectInputInformation("uniformBuffer2.uboMat2", 1, ramses::internal::EDataType::Matrix33F, EFixedSemantics::Invalid, UniformBufferBinding{ 2u},  UniformBufferElementSize{ 48u },    UniformBufferFieldOffset{ 80u }),   uniforms[7]);
    }

    TEST_F(AGlslEffect, canParseShaderInputs_AnonymousUBOs)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 ubo1Mat1;
                    float ubo1Float1[10];
                    mat3 ubo1Mat2;
                };

                layout(std140,binding=2) uniform uniformBuffer_t2
                {
                    mat4 ubo2Mat1;
                    float ubo2Float1;
                    mat3 ubo2Mat2;
                };

                void main(void)
                {
                    gl_Position = ubo1Mat1 * ubo2Mat1 * vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        const EffectInputInformationVector& uniforms = res->getUniformInputs();

        ASSERT_EQ(8u, uniforms.size());
        //The "name" assigned to anonymous UBOs by glslang is (wrong and) irrelevant, it's checked here in the test for simplification
        EXPECT_EQ(EffectInputInformation("anon@ubo_binding=1", 1, ramses::internal::EDataType::UniformBuffer, EFixedSemantics::Invalid, UniformBufferBinding{ 1u},  UniformBufferElementSize{ 272u },  UniformBufferFieldOffset{}),       uniforms[0]);
        EXPECT_EQ(EffectInputInformation("ubo1Mat1", 1, ramses::internal::EDataType::Matrix44F, EFixedSemantics::Invalid,   UniformBufferBinding{ 1u }, UniformBufferElementSize{ 64u },   UniformBufferFieldOffset{ 0u }),   uniforms[1]);
        EXPECT_EQ(EffectInputInformation("ubo1Float1", 10, ramses::internal::EDataType::Float, EFixedSemantics::Invalid,    UniformBufferBinding{ 1u},  UniformBufferElementSize{ 16u },   UniformBufferFieldOffset{ 64u}),   uniforms[2]);
        EXPECT_EQ(EffectInputInformation("ubo1Mat2", 1, ramses::internal::EDataType::Matrix33F, EFixedSemantics::Invalid,   UniformBufferBinding{ 1u }, UniformBufferElementSize{ 48u },   UniformBufferFieldOffset{ 224u }), uniforms[3]);

        EXPECT_EQ(EffectInputInformation("anon@ubo_binding=2", 1, ramses::internal::EDataType::UniformBuffer, EFixedSemantics::Invalid, UniformBufferBinding{ 2u},  UniformBufferElementSize{ 128 },    UniformBufferFieldOffset{}),        uniforms[4]);
        EXPECT_EQ(EffectInputInformation("ubo2Mat1", 1, ramses::internal::EDataType::Matrix44F, EFixedSemantics::Invalid,   UniformBufferBinding{ 2u},  UniformBufferElementSize{ 64u },    UniformBufferFieldOffset{ 0u }),    uniforms[5]);
        EXPECT_EQ(EffectInputInformation("ubo2Float1", 1, ramses::internal::EDataType::Float, EFixedSemantics::Invalid,     UniformBufferBinding{ 2u},  UniformBufferElementSize{ 4u },     UniformBufferFieldOffset{ 64u }),   uniforms[6]);
        EXPECT_EQ(EffectInputInformation("ubo2Mat2", 1, ramses::internal::EDataType::Matrix33F, EFixedSemantics::Invalid,   UniformBufferBinding{ 2u},  UniformBufferElementSize{ 48u },    UniformBufferFieldOffset{ 80u }),   uniforms[7]);
    }

    TEST_F(AGlslEffect, canParseShaderInputs_UBORedefinedInOtherStage_Anonymous)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 ubo1Mat1;
                    float ubo1Float1[10];
                    mat3 ubo1Mat2;
                };

                void main(void)
                {
                    gl_Position = ubo1Mat1 * vec4(0.0);
                })SHADER";

        const char* fragmentShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 ubo1Mat1;
                    float ubo1Float1[10];
                    mat3 ubo1Mat2;
                };
                out vec4 colorOut;
                void main(void)
                {
                    colorOut = ubo1Mat1 *vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, fragmentShader, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_02));
        ASSERT_TRUE(res);

        const EffectInputInformationVector& uniforms = res->getUniformInputs();

        ASSERT_EQ(4u, uniforms.size());
        //The "name" assigned to anonymous UBOs by glslang is (wrong and) irrelevant, it's checked here in the test for simplification
        EXPECT_EQ(EffectInputInformation("anon@ubo_binding=1", 1, ramses::internal::EDataType::UniformBuffer, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 272u }, UniformBufferFieldOffset{}), uniforms[0]);
        EXPECT_EQ(EffectInputInformation("ubo1Mat1", 1, ramses::internal::EDataType::Matrix44F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{ 0u }), uniforms[1]);
        EXPECT_EQ(EffectInputInformation("ubo1Float1", 10, ramses::internal::EDataType::Float, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 16u }, UniformBufferFieldOffset{ 64u }), uniforms[2]);
        EXPECT_EQ(EffectInputInformation("ubo1Mat2", 1, ramses::internal::EDataType::Matrix33F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 48u }, UniformBufferFieldOffset{ 224u }), uniforms[3]);
    }

    TEST_F(AGlslEffect, canParseShaderInputs_UBORedefinedInOtherStage_Anonymous_2)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 ubo1Mat1;
                    float ubo1Float1[10];
                    mat3 ubo1Mat2;
                };

                void main(void)
                {
                    gl_Position = ubo1Mat1 * vec4(0.0);
                })SHADER";

        const char* fragmentShader = R"SHADER(
                #version 310 es
                precision highp float;

                layout(std140,binding=2) uniform uniformBuffer_t2
                {
                    mat4 ubo2Mat1;
                };

                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 ubo1Mat1;
                    float ubo1Float1[10];
                    mat3 ubo1Mat2;
                };
                out vec4 colorOut;
                void main(void)
                {
                    colorOut = ubo2Mat1* ubo1Mat1 *vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, fragmentShader, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_02));
        ASSERT_TRUE(res);

        const EffectInputInformationVector& uniforms = res->getUniformInputs();

        ASSERT_EQ(6u, uniforms.size());
        //The "name" assigned to anonymous UBOs by glslang is (wrong and) irrelevant, it's checked here in the test for simplification
        EXPECT_EQ(EffectInputInformation("anon@ubo_binding=2", 1, ramses::internal::EDataType::UniformBuffer, EFixedSemantics::Invalid, UniformBufferBinding{ 2u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{}), uniforms[0]);
        EXPECT_EQ(EffectInputInformation("ubo2Mat1", 1, ramses::internal::EDataType::Matrix44F, EFixedSemantics::Invalid, UniformBufferBinding{ 2u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{ 0u }), uniforms[1]);
        EXPECT_EQ(EffectInputInformation("anon@ubo_binding=1", 1, ramses::internal::EDataType::UniformBuffer, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 272u }, UniformBufferFieldOffset{}), uniforms[2]);
        EXPECT_EQ(EffectInputInformation("ubo1Mat1", 1, ramses::internal::EDataType::Matrix44F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{ 0u }), uniforms[3]);
        EXPECT_EQ(EffectInputInformation("ubo1Float1", 10, ramses::internal::EDataType::Float, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 16u }, UniformBufferFieldOffset{ 64u }), uniforms[4]);
        EXPECT_EQ(EffectInputInformation("ubo1Mat2", 1, ramses::internal::EDataType::Matrix33F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 48u }, UniformBufferFieldOffset{ 224u }), uniforms[5]);
    }

    TEST_F(AGlslEffect, canParseShaderInputs_SeveralAnonymousUBOs)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t1
                {
                    mat4 uboMat1;
                };

                layout(std140,binding=2) uniform uniformBuffer_t2
                {
                    mat4 uboMat2;
                };

                void main(void)
                {
                    gl_Position = uboMat1 * vec4(0.0);
                })SHADER";

        const char* fragmentShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=3) uniform uniformBuffer_t3
                {
                    mat4 uboMat3;
                };

                layout(std140,binding=4) uniform uniformBuffer_t4
                {
                    mat4 uboMat4;
                };
                out vec4 colorOut;
                void main(void)
                {
                    colorOut = uboMat4 *vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, fragmentShader, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_02));
        ASSERT_TRUE(res);

        const EffectInputInformationVector& uniforms = res->getUniformInputs();
        ASSERT_EQ(8u, uniforms.size());
        EXPECT_EQ(EffectInputInformation("anon@ubo_binding=1", 1, ramses::internal::EDataType::UniformBuffer, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{}), uniforms[0]);
        EXPECT_EQ(EffectInputInformation("uboMat1", 1, ramses::internal::EDataType::Matrix44F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{ 0u }), uniforms[1]);
        EXPECT_EQ(EffectInputInformation("anon@ubo_binding=2", 1, ramses::internal::EDataType::UniformBuffer, EFixedSemantics::Invalid, UniformBufferBinding{ 2u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{}), uniforms[2]);
        EXPECT_EQ(EffectInputInformation("uboMat2", 1, ramses::internal::EDataType::Matrix44F, EFixedSemantics::Invalid, UniformBufferBinding{ 2u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{ 0u }), uniforms[3]);
        EXPECT_EQ(EffectInputInformation("anon@ubo_binding=3", 1, ramses::internal::EDataType::UniformBuffer, EFixedSemantics::Invalid, UniformBufferBinding{ 3u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{}), uniforms[4]);
        EXPECT_EQ(EffectInputInformation("uboMat3", 1, ramses::internal::EDataType::Matrix44F, EFixedSemantics::Invalid, UniformBufferBinding{ 3u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{ 0u }), uniforms[5]);
        EXPECT_EQ(EffectInputInformation("anon@ubo_binding=4", 1, ramses::internal::EDataType::UniformBuffer, EFixedSemantics::Invalid, UniformBufferBinding{ 4u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{}), uniforms[6]);
        EXPECT_EQ(EffectInputInformation("uboMat4", 1, ramses::internal::EDataType::Matrix44F, EFixedSemantics::Invalid, UniformBufferBinding{ 4u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{ 0u }), uniforms[7]);
    }

    TEST_F(AGlslEffect, failsCreatingEffectIfDifferentUBOsUseSameBinding_DifferentTypeName_DifferentStages)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t1
                {
                    mat4 ubo1Mat1;
                    float ubo1Float1[10];
                    mat3 ubo1Mat2;
                } ub1;

                void main(void)
                {
                    gl_Position = ub1.ubo1Mat1 * vec4(0.0);
                })SHADER";

        const char* fragmentShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t2
                {
                    mat4 ubo1Mat1;
                    float ubo1Float1[10];
                    mat3 ubo1Mat2;
                } ub1;
                out vec4 colorOut;
                void main(void)
                {
                    colorOut = ub1.ubo1Mat1 *vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, fragmentShader, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_02));
        ASSERT_FALSE(res);

        EXPECT_THAT(ge.getEffectErrorMessages(),
            ::testing::HasSubstr("ub1: uniform with same name but different data type declared in multiple stages"));
    }

    TEST_F(AGlslEffect, failsCreatingEffectIfDifferentUBOsUseSameBinding_DifferentTypeName_SameStage)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t1
                {
                    mat4 ubo1Mat1;
                    float ubo1Float1[10];
                    mat3 ubo1Mat2;
                } ub1;

                layout(std140,binding=1) uniform uniformBuffer_t2
                {
                    mat4 ubo1Mat1;
                    float ubo1Float1[10];
                    mat3 ubo1Mat2;
                } ub2;

                void main(void)
                {
                    gl_Position = ub1.ubo1Mat1 * ub2.ubo1Mat1 * vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_02));
        ASSERT_FALSE(res);

        EXPECT_THAT(ge.getEffectErrorMessages(),
            ::testing::HasSubstr("ub1: several uniform buffers with same binding but different definition at binding: 1"));
    }

    TEST_F(AGlslEffect, canParseShaderInputs_UBOsWithStruct)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                struct TheStruct
                {
                    float ubFloat1;
                    float ubFloat2;
                    mat3 ubMat3;
                    float ubFloat3;
                };
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    TheStruct ubStruct1;
                    TheStruct ubStruct2;
                } ub1;

                void main(void)
                {
                    gl_Position = ub1.ubStruct1.ubFloat1 * vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        const EffectInputInformationVector& uniforms = res->getUniformInputs();

        ASSERT_EQ(9u, uniforms.size());
        EXPECT_EQ(EffectInputInformation("ub1", 1, ramses::internal::EDataType::UniformBuffer, EFixedSemantics::Invalid,                UniformBufferBinding{ 1u }, UniformBufferElementSize{ 160u },   UniformBufferFieldOffset{}),    uniforms[0]);

        EXPECT_EQ(EffectInputInformation("ub1.ubStruct1.ubFloat1", 1, ramses::internal::EDataType::Float, EFixedSemantics::Invalid,     UniformBufferBinding{ 1u }, UniformBufferElementSize{ 4u },     UniformBufferFieldOffset{ 0u }),    uniforms[1]);
        EXPECT_EQ(EffectInputInformation("ub1.ubStruct1.ubFloat2", 1, ramses::internal::EDataType::Float, EFixedSemantics::Invalid,     UniformBufferBinding{ 1u }, UniformBufferElementSize{ 4u },     UniformBufferFieldOffset{ 4u }),    uniforms[2]);
        EXPECT_EQ(EffectInputInformation("ub1.ubStruct1.ubMat3", 1, ramses::internal::EDataType::Matrix33F, EFixedSemantics::Invalid,   UniformBufferBinding{ 1u }, UniformBufferElementSize{ 48u },    UniformBufferFieldOffset{ 16u }),   uniforms[3]);
        EXPECT_EQ(EffectInputInformation("ub1.ubStruct1.ubFloat3", 1, ramses::internal::EDataType::Float, EFixedSemantics::Invalid,     UniformBufferBinding{ 1u }, UniformBufferElementSize{ 4u },     UniformBufferFieldOffset{ 64u }),   uniforms[4]);

        EXPECT_EQ(EffectInputInformation("ub1.ubStruct2.ubFloat1", 1, ramses::internal::EDataType::Float, EFixedSemantics::Invalid,     UniformBufferBinding{ 1u }, UniformBufferElementSize{ 4u },     UniformBufferFieldOffset{ 80u + 0u }),    uniforms[5]);
        EXPECT_EQ(EffectInputInformation("ub1.ubStruct2.ubFloat2", 1, ramses::internal::EDataType::Float, EFixedSemantics::Invalid,     UniformBufferBinding{ 1u }, UniformBufferElementSize{ 4u },     UniformBufferFieldOffset{ 80u + 4u }),    uniforms[6]);
        EXPECT_EQ(EffectInputInformation("ub1.ubStruct2.ubMat3", 1, ramses::internal::EDataType::Matrix33F, EFixedSemantics::Invalid,   UniformBufferBinding{ 1u }, UniformBufferElementSize{ 48u },    UniformBufferFieldOffset{ 80u + 16u }),   uniforms[7]);
        EXPECT_EQ(EffectInputInformation("ub1.ubStruct2.ubFloat3", 1, ramses::internal::EDataType::Float, EFixedSemantics::Invalid,     UniformBufferBinding{ 1u }, UniformBufferElementSize{ 4u },     UniformBufferFieldOffset{ 80u + 64u }),   uniforms[8]);
    }

    TEST_F(AGlslEffect, canParseShaderInputs_UBOsWithStructOfStructs)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                struct Fish
                {
                    float plankton1;
                    mat3 plankton2;
                };

                struct Shark
                {
                    Fish fish1;
                    Fish fish2;
                };

                layout(std140,binding=1) uniform orcaUbo_t
                {
                    Shark shark1;
                    Shark shark2;
                } orcaUbo;

                void main(void)
                {
                    gl_Position = orcaUbo.shark1.fish2.plankton1 * vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        const EffectInputInformationVector& uniforms = res->getUniformInputs();

        ASSERT_EQ(9u, uniforms.size());
        EXPECT_EQ(EffectInputInformation("orcaUbo", 1, ramses::internal::EDataType::UniformBuffer, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 256u }, UniformBufferFieldOffset{}), uniforms[0]);

        EXPECT_EQ(EffectInputInformation("orcaUbo.shark1.fish1.plankton1", 1, ramses::internal::EDataType::Float, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }      , UniformBufferElementSize{ 4u }    , UniformBufferFieldOffset{ 0u }), uniforms[1]);
        EXPECT_EQ(EffectInputInformation("orcaUbo.shark1.fish1.plankton2", 1, ramses::internal::EDataType::Matrix33F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }  , UniformBufferElementSize{ 48u }   , UniformBufferFieldOffset{ 16u }), uniforms[2]);
        EXPECT_EQ(EffectInputInformation("orcaUbo.shark1.fish2.plankton1", 1, ramses::internal::EDataType::Float, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }      , UniformBufferElementSize{ 4u }    , UniformBufferFieldOffset{ 64u }), uniforms[3]);
        EXPECT_EQ(EffectInputInformation("orcaUbo.shark1.fish2.plankton2", 1, ramses::internal::EDataType::Matrix33F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }  , UniformBufferElementSize{ 48u }   , UniformBufferFieldOffset{ 80u }), uniforms[4]);
        EXPECT_EQ(EffectInputInformation("orcaUbo.shark2.fish1.plankton1", 1, ramses::internal::EDataType::Float, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }      , UniformBufferElementSize{ 4u }    , UniformBufferFieldOffset{ 128u }), uniforms[5]);
        EXPECT_EQ(EffectInputInformation("orcaUbo.shark2.fish1.plankton2", 1, ramses::internal::EDataType::Matrix33F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }  , UniformBufferElementSize{ 48u }   , UniformBufferFieldOffset{ 144u }), uniforms[6]);
        EXPECT_EQ(EffectInputInformation("orcaUbo.shark2.fish2.plankton1", 1, ramses::internal::EDataType::Float, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }      , UniformBufferElementSize{ 4u }    , UniformBufferFieldOffset{ 192u }), uniforms[7]);
        EXPECT_EQ(EffectInputInformation("orcaUbo.shark2.fish2.plankton2", 1, ramses::internal::EDataType::Matrix33F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }  , UniformBufferElementSize{ 48u }   , UniformBufferFieldOffset{ 208u }), uniforms[8]);
    }

    TEST_F(AGlslEffect, canParseShaderInputs_UBOsWithStrucstOfStructArrays)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                struct Fish
                {
                    float plankton1[2];
                    mat3 plankton2;
                };

                struct Shark
                {
                    Fish fish[2];
                };

                layout(std140,binding=1) uniform orcaUbo_t
                {
                    Shark shark[2];
                } orcaUbo;

                void main(void)
                {
                    gl_Position = orcaUbo.shark[0].fish[1].plankton1[0] * vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        const EffectInputInformationVector& uniforms = res->getUniformInputs();

        ASSERT_EQ(9u, uniforms.size());
        EXPECT_EQ(EffectInputInformation("orcaUbo", 1, ramses::internal::EDataType::UniformBuffer, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 320u }, UniformBufferFieldOffset{}), uniforms[0]);

        EXPECT_EQ(EffectInputInformation("orcaUbo.shark[0].fish[0].plankton1", 2, ramses::internal::EDataType::Float, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 16u }, UniformBufferFieldOffset{ 0u }), uniforms[1]);
        EXPECT_EQ(EffectInputInformation("orcaUbo.shark[0].fish[0].plankton2", 1, ramses::internal::EDataType::Matrix33F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 48u }, UniformBufferFieldOffset{ 32u }), uniforms[2]);
        EXPECT_EQ(EffectInputInformation("orcaUbo.shark[0].fish[1].plankton1", 2, ramses::internal::EDataType::Float, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 16u }, UniformBufferFieldOffset{ 80u }), uniforms[3]);
        EXPECT_EQ(EffectInputInformation("orcaUbo.shark[0].fish[1].plankton2", 1, ramses::internal::EDataType::Matrix33F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 48u }, UniformBufferFieldOffset{ 112u }), uniforms[4]);
        EXPECT_EQ(EffectInputInformation("orcaUbo.shark[1].fish[0].plankton1", 2, ramses::internal::EDataType::Float, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 16u }, UniformBufferFieldOffset{ 160u }), uniforms[5]);
        EXPECT_EQ(EffectInputInformation("orcaUbo.shark[1].fish[0].plankton2", 1, ramses::internal::EDataType::Matrix33F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 48u }, UniformBufferFieldOffset{ 192u }), uniforms[6]);
        EXPECT_EQ(EffectInputInformation("orcaUbo.shark[1].fish[1].plankton1", 2, ramses::internal::EDataType::Float, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 16u }, UniformBufferFieldOffset{ 240u }), uniforms[7]);
        EXPECT_EQ(EffectInputInformation("orcaUbo.shark[1].fish[1].plankton2", 1, ramses::internal::EDataType::Matrix33F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 48u }, UniformBufferFieldOffset{ 272u }), uniforms[8]);
    }

    TEST_F(AGlslEffect, canParseShaderInputs_UBOsWithStructArray)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                struct TheStruct
                {
                    float ubFloat1;
                    float ubFloat2;
                    mat3 ubMat3;
                    float ubFloat3;
                };
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    TheStruct ubStruct[2];
                } ub1;

                void main(void)
                {
                    gl_Position = ub1.ubStruct[0].ubFloat1 * vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        const EffectInputInformationVector& uniforms = res->getUniformInputs();

        ASSERT_EQ(9u, uniforms.size());

        EXPECT_EQ(EffectInputInformation("ub1", 1, ramses::internal::EDataType::UniformBuffer, EFixedSemantics::Invalid,                UniformBufferBinding{ 1u }, UniformBufferElementSize{ 160u },   UniformBufferFieldOffset{}),    uniforms[0]);

        EXPECT_EQ(EffectInputInformation("ub1.ubStruct[0].ubFloat1", 1, ramses::internal::EDataType::Float, EFixedSemantics::Invalid,     UniformBufferBinding{ 1u }, UniformBufferElementSize{ 4u },     UniformBufferFieldOffset{ 0u }),    uniforms[1]);
        EXPECT_EQ(EffectInputInformation("ub1.ubStruct[0].ubFloat2", 1, ramses::internal::EDataType::Float, EFixedSemantics::Invalid,     UniformBufferBinding{ 1u }, UniformBufferElementSize{ 4u },     UniformBufferFieldOffset{ 4u }),    uniforms[2]);
        EXPECT_EQ(EffectInputInformation("ub1.ubStruct[0].ubMat3", 1, ramses::internal::EDataType::Matrix33F, EFixedSemantics::Invalid,   UniformBufferBinding{ 1u }, UniformBufferElementSize{ 48u },    UniformBufferFieldOffset{ 16u }),   uniforms[3]);
        EXPECT_EQ(EffectInputInformation("ub1.ubStruct[0].ubFloat3", 1, ramses::internal::EDataType::Float, EFixedSemantics::Invalid,     UniformBufferBinding{ 1u }, UniformBufferElementSize{ 4u },     UniformBufferFieldOffset{ 64u }),   uniforms[4]);

        EXPECT_EQ(EffectInputInformation("ub1.ubStruct[1].ubFloat1", 1, ramses::internal::EDataType::Float, EFixedSemantics::Invalid,     UniformBufferBinding{ 1u }, UniformBufferElementSize{ 4u },     UniformBufferFieldOffset{ 80u + 0u }),    uniforms[5]);
        EXPECT_EQ(EffectInputInformation("ub1.ubStruct[1].ubFloat2", 1, ramses::internal::EDataType::Float, EFixedSemantics::Invalid,     UniformBufferBinding{ 1u }, UniformBufferElementSize{ 4u },     UniformBufferFieldOffset{ 80u + 4u }),    uniforms[6]);
        EXPECT_EQ(EffectInputInformation("ub1.ubStruct[1].ubMat3", 1, ramses::internal::EDataType::Matrix33F, EFixedSemantics::Invalid,   UniformBufferBinding{ 1u }, UniformBufferElementSize{ 48u },    UniformBufferFieldOffset{ 80u + 16u }),   uniforms[7]);
        EXPECT_EQ(EffectInputInformation("ub1.ubStruct[1].ubFloat3", 1, ramses::internal::EDataType::Float, EFixedSemantics::Invalid,     UniformBufferBinding{ 1u }, UniformBufferElementSize{ 4u },     UniformBufferFieldOffset{ 80u + 64u }),   uniforms[8]);
    }

    TEST_F(AGlslEffect, failsCreatingEffectIfUBOLayoutNotSetToStd140)
    {
        const char* vertexShader = R"SHADER(
        #version 310 es
        precision highp float;
        layout(binding=1) uniform uniformBuffer_t
        {
            mat4 uboMat1;
            float uboFloat1;
            mat3 uboMat2;
        } uniformBuffer1;

        void main(void)
        {
            gl_Position = uniformBuffer1.uboMat1 * vec4(0.0);
        })SHADER";
        const char* fragmentShader = R"SHADER(
        #version 310 es
        precision highp float;
        out vec4 colorOut;
        void main(void)
        {
            colorOut = vec4(0.0);
        })SHADER";

        GlslEffect ge(vertexShader, fragmentShader, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_FALSE(res);
        EXPECT_THAT(ge.getEffectErrorMessages(),
        ::testing::HasSubstr("Failed creating effect input for uniform block uniformBuffer1 of type uniformBuffer_t. Layout must be explicitly set to std140"));
    }

    TEST_F(AGlslEffect, failsCreatingEffectIfUBOIsArray)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;

                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 uboMat1;
                    float uboFloat1;
                    mat3 uboMat2;
                } uniformBuffer[2];

                void main(void)
                {
                    gl_Position = vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_FALSE(res);
        EXPECT_THAT(ge.getEffectErrorMessages(),
            ::testing::HasSubstr("Failed creating effect input for uniform block uniformBuffer[] of type uniformBuffer_t. Uniform block arrays are not supported"));
    }

    TEST_F(AGlslEffect, failsCreatingEffectIfDifferentUBOsUseSameBinding_InDifferentStages)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 uboMat1;
                    float uboFloat1[10];
                    mat3 uboMat2;
                } uniformBuffer1;

                void main(void)
                {
                    gl_Position = uniformBuffer1.uboMat1 * vec4(0.0);
                })SHADER";

        const char* fragmentShader = R"SHADER(
                #version 310 es
                precision highp float;

                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 uboMat1;
                    float uboFloat1[10];
                    mat3 uboMat2;

                    mat3 uboMat3; // add a field to the UBO
                } uniformBuffer1;

                out vec4 colorOut;
                void main(void)
                {
                    colorOut = vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, fragmentShader, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_02));
        EXPECT_FALSE(res);

        EXPECT_THAT(ge.getEffectErrorMessages(),
            ::testing::HasSubstr("[GLSL Compiler] Shader Program Linker Error:\n"
                                "ERROR: Linking unknown stage and fragment stages: fragment block member has no corresponding member in unknown stage block:\n"
                                "    fragment stage: Block: uniformBuffer_t, Member: uboMat3\n"
                                "    unknown stage stage: Block: uniformBuffer_t, Member: n/a"));
    }

    TEST_F(AGlslEffect, failsCreatingEffectIfDifferentUBOsUseSameBinding_InDifferentStages_AnonymousUBOs)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 uboMat1;
                    float uboFloat1[10];
                    mat3 uboMat2;
                };

                void main(void)
                {
                    gl_Position = uboMat1 * vec4(0.0);
                })SHADER";

        const char* fragmentShader = R"SHADER(
                #version 310 es
                precision highp float;

                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 uboMat1;
                    float uboFloat1[10];
                    mat3 uboMat2;

                    mat3 uboMat3; // add a field to the UBO
                };

                out vec4 colorOut;
                void main(void)
                {
                    colorOut = vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, fragmentShader, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_02));
        EXPECT_FALSE(res);

        EXPECT_THAT(ge.getEffectErrorMessages(),
            ::testing::HasSubstr("[GLSL Compiler] Shader Program Linker Error:\n"
                                "ERROR: Linking unknown stage and fragment stages: fragment block member has no corresponding member in unknown stage block:\n"
                                "    fragment stage: Block: uniformBuffer_t, Member: uboMat3\n"
                                "    unknown stage stage: Block: uniformBuffer_t, Member: n/a"));
    }

    TEST_F(AGlslEffect, failsCreatingEffectIfDifferentUBOsUseSameBinding_InSameStage)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 uboMat1;
                    float uboFloat1[10];
                    mat3 uboMat2;
                } uniformBuffer1;

                layout(std140,binding=1) uniform uniformBuffer_t2
                {
                    mat4 uboMat1;
                    float uboFloat1;
                    mat3 uboMat2;
                } uniformBuffer2;

                void main(void)
                {
                    gl_Position = uniformBuffer1.uboMat1 * uniformBuffer2.uboMat1 * vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_02));
        EXPECT_FALSE(res);

        EXPECT_THAT(ge.getEffectErrorMessages(),
            ::testing::HasSubstr("uniformBuffer1: several uniform buffers with same binding but different definition at binding: 1"));
    }

    TEST_F(AGlslEffect, failsCreatingEffectIfDifferentUBOsUseSameBinding_InSameStage_AnonymousUBOs)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 uboMat1;
                    float uboFloat1[10];
                    mat3 uboMat2;
                };

                layout(std140,binding=1) uniform uniformBuffer_t2
                {
                    mat4 ubo2Mat1;
                    float ubo2Float1;
                    mat3 ubo2Mat2;
                };

                void main(void)
                {
                    gl_Position = uboMat1 * ubo2Mat1 * vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_02));
        EXPECT_FALSE(res);

        EXPECT_THAT(ge.getEffectErrorMessages(),
            ::testing::HasSubstr("anon@ubo_binding=1: several uniform buffers with same binding but different definition at binding: 1"));
    }

    TEST_F(AGlslEffect, failsCreatingEffectIfDifferentUBOsUseSameBinding_NameIsDifferent)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 uboMat1;
                    float uboFloat1[10];
                    mat3 uboMat2;
                } uniformBuffer1;

                layout(std140,binding=1) uniform uniformBuffer_t2
                {
                    mat4 uboMat1;
                    float uboFloat1[10];
                    mat3 uboMat2;
                } uniformBuffer2;

                void main(void)
                {
                    gl_Position = uniformBuffer1.uboMat1 * uniformBuffer2.uboMat1 * vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_02));
        EXPECT_FALSE(res);

        EXPECT_THAT(ge.getEffectErrorMessages(),
            ::testing::HasSubstr("uniformBuffer1: several uniform buffers with same binding but different definition at binding: 1"));
    }

    TEST_F(AGlslEffect, failsCreatingEffectIfDifferentUBOsUseSameName_InDifferentStages)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 uboMat1;
                    float uboFloat1[10];
                    mat3 uboMat2;
                } sameUboName;

                void main(void)
                {
                    gl_Position = sameUboName.uboMat1 * vec4(0.0);
                })SHADER";

        const char* fragmentShader = R"SHADER(
                #version 310 es
                precision highp float;

                layout(std140,binding=2) uniform uniformBuffer_t
                {
                    mat4 uboMat1;
                    float uboFloat1[10];
                    mat3 uboMat2;
                } sameUboName;

                out vec4 colorOut;
                void main(void)
                {
                    colorOut = vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, fragmentShader, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_02));
        EXPECT_FALSE(res);

        EXPECT_THAT(ge.getEffectErrorMessages(),
            ::testing::HasSubstr("[GLSL Compiler] Shader Program Linker Error:\n"
                                "ERROR: Linking unknown stage and fragment stages: Layout binding qualifier must match:\n"
                                "    unknown stage stage: Block: uniformBuffer_t Instance: sameUboName: \"layout( binding=1 column_major std140) uniform\"\n"
                                "    fragment stage: Block: uniformBuffer_t Instance: sameUboName: \"layout( binding=2 column_major std140) uniform"));
    }

    TEST_F(AGlslEffect, failsCreatingEffectIfUBOsAndUniformUseSameName_InSameStage)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 uboMat1;
                    float uboFloat1[10];
                    mat3 uboMat2;
                } sameUboName;

                uniform float sameUboName;

                void main(void)
                {
                    gl_Position = sameUboName.uboMat1 * vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_02));
        EXPECT_FALSE(res);

        EXPECT_THAT(ge.getEffectErrorMessages(),
            ::testing::HasSubstr("vertex shader Shader Parsing Error:\nERROR: 2:9: 'sameUboName' : redefinition"));
    }

    TEST_F(AGlslEffect, failsCreatingEffectIfUBOsAndUniformUseSameName_InDifferentStages)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 uboMat1;
                    float uboFloat1[10];
                    mat3 uboMat2;
                } sameUboName;

                void main(void)
                {
                    gl_Position = sameUboName.uboMat1 * vec4(0.0);
                })SHADER";

        const char* fragmentShader = R"SHADER(
                #version 310 es
                precision highp float;

                uniform float sameUboName;

                out vec4 colorOut;
                void main(void)
                {
                    colorOut = vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, fragmentShader, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_02));
        EXPECT_FALSE(res);

        EXPECT_THAT(ge.getEffectErrorMessages(),
            ::testing::HasSubstr("[GLSL Compiler] Shader Program Linker Error:\n"
                                "ERROR: Linking unknown stage and fragment stages: Types must match:\n"
                                "ERROR: Linking unknown stage and fragment stages: Precision qualifiers must match:\n"
                                "ERROR: Linking unknown stage and fragment stages: Layout matrix qualifier must match:\n"
                                "ERROR: Linking unknown stage and fragment stages: Layout packing qualifier must match:\n"
                                "    unknown stage stage: \"layout( binding=1 column_major std140) uniform {layout( column_major std140 offset=0) uniform highp mat4x4 uboMat1, layout( column_major std140 offset=64) uniform highp float uboFloat1[10], layout( column_major std140 offset=224) uniform highp mat3x3 uboMat2} sameUboName\"\n"
                                "    fragment stage: \" uniform highp float sameUboName"));
    }

    TEST_F(AGlslEffect, failsCreatingEffectIfDifferentUBOsUseSameBinding_UbosWithSameSize)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t1
                {
                    mat4 uboMat1;
                    mat4 uboMat2;
                } uniformBuffer1;

                void main(void)
                {
                    gl_Position = uniformBuffer1.uboMat1 * vec4(0.0);
                })SHADER";

        const char* fragmentShader = R"SHADER(
                #version 310 es
                precision highp float;

                layout(std140,binding=1) uniform uniformBuffer_t2
                {
                    mat4 uboMat1;
                    vec4 v1;
                    vec4 v2;
                    vec4 v3;
                    vec4 v4;
                } uniformBuffer1;

                out vec4 colorOut;
                void main(void)
                {
                    colorOut = uniformBuffer1.v1 + vec4(0.0);
                })SHADER";

        GlslEffect ge(vertexShader, fragmentShader, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_02));
        EXPECT_FALSE(res);

        EXPECT_THAT(ge.getEffectErrorMessages(),
            ::testing::HasSubstr("uniformBuffer1: uniform with same name but different data type declared in multiple stages"));
    }

    TEST_F(AGlslEffect, canParseShaderInputs_CanAssignSemanticToUbo_UsingUniformBufferBinding)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 modelMat;
                } uniformBuffer1;

                void main(void)
                {
                })SHADER";

        const SemanticsMap semanticsMap{ {UniformBufferBinding{ 1u }, EFixedSemantics::ModelBlock} };
        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, semanticsMap, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        const EffectInputInformationVector& uniforms = res->getUniformInputs();

        ASSERT_EQ(2u, uniforms.size());
        EXPECT_EQ(EffectInputInformation("uniformBuffer1", 1, ramses::internal::EDataType::UniformBuffer, EFixedSemantics::ModelBlock, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{}), uniforms[0]);
        EXPECT_EQ(EffectInputInformation("uniformBuffer1.modelMat", 1, ramses::internal::EDataType::Matrix44F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{ 0u }), uniforms[1]);
    }

    TEST_F(AGlslEffect, canParseShaderInputs_CanAssignSemanticToUBO_ModelBlock)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 modelMat;
                } uniformBuffer1;

                void main(void)
                {
                })SHADER";

        const SemanticsMap semanticsMap{ {"uniformBuffer1", EFixedSemantics::ModelBlock} };
        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, semanticsMap, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        const EffectInputInformationVector& uniforms = res->getUniformInputs();

        ASSERT_EQ(2u, uniforms.size());
        EXPECT_EQ(EffectInputInformation("uniformBuffer1", 1, ramses::internal::EDataType::UniformBuffer, EFixedSemantics::ModelBlock, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{}), uniforms[0]);
        EXPECT_EQ(EffectInputInformation("uniformBuffer1.modelMat", 1, ramses::internal::EDataType::Matrix44F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{ 0u }), uniforms[1]);
    }

    TEST_F(AGlslEffect, failsCreatingEffectIfUboSemanticInvalid_WrongTypeInModelBlock)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat3 modelMat;
                } uniformBuffer1;

                void main(void)
                {
                })SHADER";

        const SemanticsMap semanticsMap{ {"uniformBuffer1", EFixedSemantics::ModelBlock} };
        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, semanticsMap, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_FALSE(res);
        EXPECT_THAT(ge.getEffectErrorMessages(),
            ::testing::HasSubstr("uniformBuffer1: is a uniform buffer that does not have correct format for semantic :EFixedSemantics::ModelBlock"));
    }

    TEST_F(AGlslEffect, failsCreatingEffectIfUboSemanticInvalid_WrongFormatInModelBlock)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 modelMat;
                    mat4 someOtherMatrixIDecidedLooksCoolHere;
                } uniformBuffer1;

                void main(void)
                {
                })SHADER";

        const SemanticsMap semanticsMap{ {"uniformBuffer1", EFixedSemantics::ModelBlock} };
        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, semanticsMap, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_FALSE(res);
        EXPECT_THAT(ge.getEffectErrorMessages(),
            ::testing::HasSubstr("uniformBuffer1: is a uniform buffer that does not have correct format for semantic :EFixedSemantics::ModelBlock"));
    }

    TEST_F(AGlslEffect, failsCreatingEffectIfUboSemanticInvalid_SettingModelBlockOnNonUbo)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;

                uniform mat4 modelMat;
                void main(void)
                {
                })SHADER";

        const SemanticsMap semanticsMap{ {"modelMat", EFixedSemantics::ModelBlock} };
        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, semanticsMap, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_FALSE(res);
        EXPECT_THAT(ge.getEffectErrorMessages(),
            ::testing::HasSubstr("modelMat: input type DATATYPE_MATRIX44F not compatible with semantic EFixedSemantics::ModelBlock"));
    }

    TEST_F(AGlslEffect, canParseShaderInputs_CanAssignSemanticToUBO_CameraBlock)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 projMat;
                    mat4 viewMat;
                    vec3 cameraPos;
                } uniformBuffer1;

                void main(void)
                {
                })SHADER";

        const SemanticsMap semanticsMap{ {"uniformBuffer1", EFixedSemantics::CameraBlock} };
        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, semanticsMap, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_02));
        ASSERT_TRUE(res);

        const EffectInputInformationVector& uniforms = res->getUniformInputs();

        ASSERT_EQ(4u, uniforms.size());
        EXPECT_EQ(EffectInputInformation("uniformBuffer1", 1, ramses::internal::EDataType::UniformBuffer, EFixedSemantics::CameraBlock, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 140u }, UniformBufferFieldOffset{}), uniforms[0]);
        EXPECT_EQ(EffectInputInformation("uniformBuffer1.projMat", 1, ramses::internal::EDataType::Matrix44F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{ 0u }), uniforms[1]);
        EXPECT_EQ(EffectInputInformation("uniformBuffer1.viewMat", 1, ramses::internal::EDataType::Matrix44F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{ 64u }), uniforms[2]);
        EXPECT_EQ(EffectInputInformation("uniformBuffer1.cameraPos", 1, ramses::internal::EDataType::Vector3F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 12u }, UniformBufferFieldOffset{ 128u }), uniforms[3]);
    }

    TEST_F(AGlslEffect, failsCreatingEffectIfUboSemanticInvalid_WrongTypeInCameraBlock)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat3 projMat; //wrong type
                    mat4 viewMat;
                    vec3 cameraPos;
                } uniformBuffer1;

                void main(void)
                {
                })SHADER";

        const SemanticsMap semanticsMap{ {"uniformBuffer1", EFixedSemantics::CameraBlock} };
        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, semanticsMap, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_02));
        ASSERT_FALSE(res);
        EXPECT_THAT(ge.getEffectErrorMessages(),
            ::testing::HasSubstr("uniformBuffer1: is a uniform buffer that does not have correct format for semantic :EFixedSemantics::CameraBlock"));
    }

    TEST_F(AGlslEffect, failsCreatingEffectIfUboSemanticInvalid_WrongFormatInCameraBlock)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 modelMat;
                    vec3 posOutOfPos;
                    mat4 someOtherMatrixIDecidedLooksCoolHere;
                } uniformBuffer1;

                void main(void)
                {
                })SHADER";

        const SemanticsMap semanticsMap{ {"uniformBuffer1", EFixedSemantics::CameraBlock} };
        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, semanticsMap, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_02));
        ASSERT_FALSE(res);
        EXPECT_THAT(ge.getEffectErrorMessages(),
            ::testing::HasSubstr("uniformBuffer1: is a uniform buffer that does not have correct format for semantic :EFixedSemantics::CameraBlock"));
    }

    TEST_F(AGlslEffect, canParseShaderInputs_CanAssignSemanticToUBO_ModelCameraBlock)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 mvpMat;
                    mat4 mvMat;
                    mat4 normalMat;
                } uniformBuffer1;

                void main(void)
                {
                })SHADER";

        const SemanticsMap semanticsMap{ {"uniformBuffer1", EFixedSemantics::ModelCameraBlock} };
        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, semanticsMap, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_02));
        ASSERT_TRUE(res);

        const EffectInputInformationVector& uniforms = res->getUniformInputs();

        ASSERT_EQ(4u, uniforms.size());
        EXPECT_EQ(EffectInputInformation("uniformBuffer1", 1, ramses::internal::EDataType::UniformBuffer, EFixedSemantics::ModelCameraBlock, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 192u }, UniformBufferFieldOffset{}), uniforms[0]);
        EXPECT_EQ(EffectInputInformation("uniformBuffer1.mvpMat", 1, ramses::internal::EDataType::Matrix44F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{ 0u }), uniforms[1]);
        EXPECT_EQ(EffectInputInformation("uniformBuffer1.mvMat", 1, ramses::internal::EDataType::Matrix44F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{ 64u }), uniforms[2]);
        EXPECT_EQ(EffectInputInformation("uniformBuffer1.normalMat", 1, ramses::internal::EDataType::Matrix44F, EFixedSemantics::Invalid, UniformBufferBinding{ 1u }, UniformBufferElementSize{ 64u }, UniformBufferFieldOffset{ 128u }), uniforms[3]);
    }

    TEST_F(AGlslEffect, failsCreatingEffectIfUboSemanticInvalid_WrongTypeInModelCameraBlock)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat3 mvpMat; // wrong type
                    mat4 mvMat;
                    mat4 normalMat;
                } uniformBuffer1;

                void main(void)
                {
                })SHADER";

        const SemanticsMap semanticsMap{ {"uniformBuffer1", EFixedSemantics::ModelCameraBlock} };
        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, semanticsMap, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_02));
        ASSERT_FALSE(res);
        EXPECT_THAT(ge.getEffectErrorMessages(),
            ::testing::HasSubstr("uniformBuffer1: is a uniform buffer that does not have correct format for semantic :EFixedSemantics::ModelCameraBlock"));
    }

    TEST_F(AGlslEffect, failsCreatingEffectIfUboSemanticInvalid_WrongFormatInModelCameraBlock)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 mvpMat;
                    mat4 mvMat;
                    mat4 normalMat;
                    int wrongElement;
                } uniformBuffer1;

                void main(void)
                {
                })SHADER";

        const SemanticsMap semanticsMap{ {"uniformBuffer1", EFixedSemantics::ModelCameraBlock} };
        GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, semanticsMap, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_02));
        ASSERT_FALSE(res);
        EXPECT_THAT(ge.getEffectErrorMessages(),
            ::testing::HasSubstr("uniformBuffer1: is a uniform buffer that does not have correct format for semantic :EFixedSemantics::ModelCameraBlock"));
    }

    TEST_F(AGlslEffect, canParseSamplerInputsGLSLES2)
    {
        const char* vertexShader =
            "void main(void)\n"
            "{\n"
            "    gl_Position = vec4(0.0);\n"
            "}\n";
        const char* fragmentShader =
            "precision highp float;\n"
            "uniform sampler2D s2d;\n"
            "uniform samplerCube sc;\n"
            "#extension GL_OES_EGL_image_external : require\n"
            "uniform samplerExternalOES texExternal;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = vec4(0.0);\n"
            "}\n";
        GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));

        ASSERT_TRUE(res);

        EXPECT_EQ(0u, res->getAttributeInputs().size());

        const EffectInputInformationVector& uniforms = res->getUniformInputs();

        ASSERT_EQ(3u, uniforms.size());
        EXPECT_EQ(EffectInputInformation("s2d", 1, ramses::internal::EDataType::TextureSampler2D, EFixedSemantics::Invalid), uniforms[0]);
        EXPECT_EQ(EffectInputInformation("sc", 1, ramses::internal::EDataType::TextureSamplerCube, EFixedSemantics::Invalid), uniforms[1]);
        EXPECT_EQ(EffectInputInformation("texExternal", 1, ramses::internal::EDataType::TextureSamplerExternal, EFixedSemantics::Invalid), uniforms[2]);
    }

    TEST_F(AGlslEffect, canParseSamplerInputsGLSLES3)
    {
        const char* vertexShader =
            "#version 300 es\n"
            "precision highp float;"
            "void main(void)\n"
            "{\n"
            "    gl_Position = vec4(0.0);\n"
            "}\n";
        const char* fragmentShader =
            "#version 300 es\n"
            "precision highp float;"
            "uniform sampler2D s2d;\n"
            "uniform highp sampler3D s3d;\n"
            "uniform samplerCube sc;\n"
            "#extension GL_OES_EGL_image_external_essl3 : require\n"
            "uniform samplerExternalOES texExternal;\n"

            "out vec4 color;\n"
            "void main(void)\n"
            "{\n"
            "    color = vec4(0.0);\n"
            "}\n";
        GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));

        ASSERT_TRUE(res);

        EXPECT_EQ(0u, res->getAttributeInputs().size());

        const EffectInputInformationVector& uniforms = res->getUniformInputs();

        ASSERT_EQ(4u, uniforms.size());
        EXPECT_EQ(EffectInputInformation("s2d", 1, ramses::internal::EDataType::TextureSampler2D, EFixedSemantics::Invalid), uniforms[0]);
        EXPECT_EQ(EffectInputInformation("s3d", 1, ramses::internal::EDataType::TextureSampler3D, EFixedSemantics::Invalid), uniforms[1]);
        EXPECT_EQ(EffectInputInformation("sc", 1, ramses::internal::EDataType::TextureSamplerCube, EFixedSemantics::Invalid), uniforms[2]);
        EXPECT_EQ(EffectInputInformation("texExternal", 1, ramses::internal::EDataType::TextureSamplerExternal, EFixedSemantics::Invalid), uniforms[3]);
    }

    TEST_F(AGlslEffect, canParseSamplerInputsGLSLES31)
    {
        const char* vertexShader =
            "#version 310 es\n"
            "precision highp float;"
            "void main(void)\n"
            "{\n"
            "    gl_Position = vec4(0.0);\n"
            "}\n";
        const char* fragmentShader =
            "#version 310 es\n"
            "precision highp float;"
            "uniform sampler2D s2d;\n"
            "uniform highp sampler2DMS s2dMS;"
            "uniform highp sampler3D s3d;\n"
            "uniform samplerCube sc;\n"
            "#extension GL_OES_EGL_image_external_essl3 : require\n"
            "uniform samplerExternalOES texExternal;\n"
            "out vec4 color;\n"
            "void main(void)\n"
            "{\n"
            "    color = vec4(0.0);\n"
            "}\n";
        GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));

        ASSERT_TRUE(res);

        EXPECT_EQ(0u, res->getAttributeInputs().size());

        const EffectInputInformationVector& uniforms = res->getUniformInputs();

        ASSERT_EQ(5u, uniforms.size());
        EXPECT_EQ(EffectInputInformation("s2d", 1, ramses::internal::EDataType::TextureSampler2D, EFixedSemantics::Invalid), uniforms[0]);
        EXPECT_EQ(EffectInputInformation("s2dMS", 1, ramses::internal::EDataType::TextureSampler2DMS, EFixedSemantics::Invalid), uniforms[1]);
        EXPECT_EQ(EffectInputInformation("s3d", 1, ramses::internal::EDataType::TextureSampler3D, EFixedSemantics::Invalid), uniforms[2]);
        EXPECT_EQ(EffectInputInformation("sc", 1, ramses::internal::EDataType::TextureSamplerCube, EFixedSemantics::Invalid), uniforms[3]);
        EXPECT_EQ(EffectInputInformation("texExternal", 1, ramses::internal::EDataType::TextureSamplerExternal, EFixedSemantics::Invalid), uniforms[4]);
    }


    TEST_F(AGlslEffect, canParseArrayInputs)
    {
        const char* vertexShader =
            "void main(void)\n"
            "{\n"
            "    gl_Position = vec4(0.0);\n"
            "}\n";
        const char* fragmentShader =
            "precision highp float;\n"
            "uniform vec3 v[4];\n"
            "uniform mat4 m[2];\n"
            "uniform int i[2];\n"
            "uniform bool b[3];\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = vec4(0.0);\n"
            "}\n";
        GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));

        ASSERT_TRUE(res);

        EXPECT_EQ(0u, res->getAttributeInputs().size());

        const EffectInputInformationVector& uniforms = res->getUniformInputs();

        ASSERT_EQ(4u, uniforms.size());
        EXPECT_EQ(EffectInputInformation("v", 4, ramses::internal::EDataType::Vector3F, EFixedSemantics::Invalid), uniforms[0]);
        EXPECT_EQ(EffectInputInformation("m", 2, ramses::internal::EDataType::Matrix44F, EFixedSemantics::Invalid), uniforms[1]);
        EXPECT_EQ(EffectInputInformation("i", 2, ramses::internal::EDataType::Int32, EFixedSemantics::Invalid), uniforms[2]);
        EXPECT_EQ(EffectInputInformation("b", 3, ramses::internal::EDataType::Bool, EFixedSemantics::Invalid), uniforms[3]);
    }

    TEST_F(AGlslEffect, failsWithWrongSemanticForType)
    {
        const char* vertexShader =
            "precision highp float;\n"
            "uniform vec3 uniformWithWrongSemantic;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = vec4(0.0);\n"
            "}\n";
        const char* fragmentShader =
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = vec4(0.0);\n"
            "}\n";

        const SemanticsMap semantics{ {"uniformWithWrongSemantic", EFixedSemantics::ModelViewProjectionMatrix} };

        GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, semantics, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        EXPECT_FALSE(res);
    }

    TEST_F(AGlslEffect, ignoresNormalGlobalsAndVaryings)
    {
        const char* vertexShader =
            "precision highp float;\n"
            "mat4 foobar;\n"
            "const float fl = float(0.1);\n"
            "varying vec3 v3;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = foobar * vec4(v3, fl);\n"
            "}\n";
        const char* fragmentShader =
            "precision highp float;\n"
            "varying vec3 v3;\n"
            "bool b;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = vec4(0.0);\n"
            "}\n";
        GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        EXPECT_EQ(0u, res->getUniformInputs().size());
        EXPECT_EQ(0u, res->getAttributeInputs().size());
    }

    TEST_F(AGlslEffect, canParseStructUniform)
    {
        const char* vertexShader =
            "precision highp float;\n"
            "struct S {\n"
            "  vec3 a;\n"
            "  vec3 b;\n"
            "};\n"
            "uniform S s;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = vec4(0.0);\n"
            "}\n";
        const char* fragmentShader =
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = vec4(0.0);\n"
            "}\n";
        GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        EXPECT_EQ(2u, res->getUniformInputs().size());
        VerifyUniformInputExists(*res, "s.a");
        VerifyUniformInputExists(*res, "s.b");
    }

    TEST_F(AGlslEffect, canParseArrayOfStructUniform)
    {
        const char* vertexShader =
            "precision highp float;\n"
            "struct S {\n"
            "  vec3 a;\n"
            "  vec3 b;\n"
            "};\n"
            "uniform S s[4];\n" // Array declaration
            "void main(void)\n"
            "{\n"
            "    gl_Position = vec4(0.0);\n"
            "}\n";
        const char* fragmentShader =
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = vec4(0.0);\n"
            "}\n";
        GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        EXPECT_EQ(8u, res->getUniformInputs().size());
        VerifyUniformInputExists(*res, "s[0].a");
        VerifyUniformInputExists(*res, "s[0].b");

        VerifyUniformInputExists(*res, "s[1].a");
        VerifyUniformInputExists(*res, "s[1].b");

        VerifyUniformInputExists(*res, "s[2].a");
        VerifyUniformInputExists(*res, "s[2].b");

        VerifyUniformInputExists(*res, "s[3].a");
        VerifyUniformInputExists(*res, "s[3].b");
    }

    TEST_F(AGlslEffect, canParseNestedStructUniform)
    {
        const char* vertexShader =
            "precision highp float;\n"
            "struct simpleStruct {\n"
            "  vec3 a;\n"
            "  vec3 b;\n"
            "};\n"
            "struct nestedStruct {\n" // Include previous struct as a member
            "  vec3 c;\n"
            "  simpleStruct d;\n"
            "};\n"
            "uniform nestedStruct ns;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = vec4(0.0);\n"
            "}\n";
        const char* fragmentShader =
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = vec4(0.0);\n"
            "}\n";
        GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        EXPECT_EQ(3u, res->getUniformInputs().size());
        VerifyUniformInputExists(*res, "ns.c");
        VerifyUniformInputExists(*res, "ns.d.a");
        VerifyUniformInputExists(*res, "ns.d.b");
    }

    TEST_F(AGlslEffect, canParseArrayOfNestedStructUniforms)
    {
        const char* vertexShader =
            "precision highp float;\n"
            "struct simpleStruct {\n"
            "  vec3 a;\n"
            "  vec3 b;\n"
            "};\n"
            "struct nestedStruct {\n" // Include previous struct as a member
            "  simpleStruct c;\n"
            "};\n"
            "struct doubleNestedStruct {\n" // Add another level; this time as an array
            "  nestedStruct d[2];\n"
            "};\n"
            "uniform doubleNestedStruct ans[5];\n" // Array of those guys
            "void main(void)\n"
            "{\n"
            "    gl_Position = vec4(0.0);\n"
            "}\n";
        const char* fragmentShader =
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = vec4(0.0);\n"
            "}\n";
        GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        EXPECT_EQ(20u, res->getUniformInputs().size());

        VerifyUniformInputExists(*res, "ans[0].d[0].c.a");
        VerifyUniformInputExists(*res, "ans[0].d[0].c.b");
        VerifyUniformInputExists(*res, "ans[0].d[1].c.a");
        VerifyUniformInputExists(*res, "ans[0].d[1].c.b");

        VerifyUniformInputExists(*res, "ans[1].d[0].c.a");
        VerifyUniformInputExists(*res, "ans[1].d[0].c.b");
        VerifyUniformInputExists(*res, "ans[1].d[1].c.a");
        VerifyUniformInputExists(*res, "ans[1].d[1].c.b");

        VerifyUniformInputExists(*res, "ans[2].d[0].c.a");
        VerifyUniformInputExists(*res, "ans[2].d[0].c.b");
        VerifyUniformInputExists(*res, "ans[2].d[1].c.a");
        VerifyUniformInputExists(*res, "ans[2].d[1].c.b");

        VerifyUniformInputExists(*res, "ans[3].d[0].c.a");
        VerifyUniformInputExists(*res, "ans[3].d[0].c.b");
        VerifyUniformInputExists(*res, "ans[3].d[1].c.a");
        VerifyUniformInputExists(*res, "ans[3].d[1].c.b");

        VerifyUniformInputExists(*res, "ans[4].d[0].c.a");
        VerifyUniformInputExists(*res, "ans[4].d[0].c.b");
        VerifyUniformInputExists(*res, "ans[4].d[1].c.a");
        VerifyUniformInputExists(*res, "ans[4].d[1].c.b");
    }

    class GlslEffectTestRunnable : public Runnable
    {
    public:
        void run() override
        {
            const std::vector<std::string> defs;
            const SemanticsMap sems;
            const char* v = "void main(){gl_Position=vec4(0);}";
            const char* f = "void main(){gl_FragColor=vec4(0);}";

            GlslEffect eff(v, f, "", defs, sems, ERenderBackendCompatibility::OpenGL, "myname");
            std::unique_ptr<EffectResource> resource(eff.createEffectResource(EFeatureLevel_Latest));
            createdSuccessfully = static_cast<bool>(resource);
        }

        bool createdSuccessfully{false};
    };

    TEST_F(AGlslEffect, UseGlslEffectFromDifferentThread)
    {
        PlatformThread th("GlslEffectThrd");
        GlslEffectTestRunnable r;
        th.start(r);
        th.join();
        EXPECT_TRUE(r.createdSuccessfully);
    }

    TEST_F(AGlslEffect, UseGlslEffectFromMainThread)
    {
        GlslEffectTestRunnable r;
        r.run();
        EXPECT_TRUE(r.createdSuccessfully);
    }

    TEST_F(AGlslEffect, hasDefaultShaderVersion100)
    {
        const std::string basicVertexShader_v100 = R"SHADER(
                void main(void)
                {
                    gl_Position = vec4(0.0);
                }
                )SHADER";
        const std::string basicFragmentShader_v100 = R"SHADER(
                void main(void)
                {
                    gl_FragColor = vec4(0.0);
                })SHADER";

        GlslEffect ge(basicVertexShader_v100, basicFragmentShader_v100, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);
        EXPECT_EQ(100u, ge.getShadingLanguageVersion());
    }

    TEST_F(AGlslEffect, isAbleToParseShadersWithVersionString)
    {
        GlslEffect ge(basicVertexShader, basicFragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);
    }

    TEST_F(AGlslEffect, acceptsReturnStatementsEveryWhere)
    {
        const char* vertexShader = R"SHADER(
            #version 320 es
            precision highp float;
            out float attr;
            void fun() {
                if (attr < 1.0) return;
                return;
            }
            void main(void)
            {
                if (attr > 1.0) return;
                gl_Position = vec4(0.0);
                return;
            })SHADER";

        GlslEffect ge(vertexShader, basicFragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        EXPECT_TRUE(res);
    }

    TEST_F(AGlslEffect, acceptsLoopsInShaders)
    {
        const char* vertexShader = R"SHADER(
            #version 320 es
            precision highp float;
            uniform int value;
            void main(void)
            {
                int i = 0;
                for (i = 0; i < value; ++i) continue;
                while (value < 10) --i;
                do { break; } while (false);
                gl_Position = vec4(0.0);
            })SHADER";

        GlslEffect ge(vertexShader, basicFragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        EXPECT_TRUE(res);
    }

    TEST_F(AGlslEffect, supportsExternalTextureExtension)
    {
        const char* vertexShader =
            "#version 100\n"
            "precision highp float;\n"
            "attribute vec2 texCoords;\n"
            "varying vec2 texCoord;\n"
            "void main(void)\n"
            "{\n"
            "    texCoord = texCoords;\n"
            "    gl_Position = vec4(0.0);\n"
            "}\n";
        const char* fragmentShader =
            "#version 100\n"
            "precision highp float;\n"
            "#extension GL_OES_EGL_image_external : require\n"
            "uniform samplerExternalOES tex;\n"
            "varying vec2 texCoord;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = texture2D(tex, texCoord);\n"
            "}\n";

        GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        EXPECT_TRUE(res);
    }

    TEST_F(AGlslEffect, doesNotSupportFloatTextureExtension)
    {
        const char* vertexShader =
            "#version 100\n"
            "precision highp float;\n"
            "attribute vec2 texCoords;\n"
            "varying vec2 texCoord;\n"
            "void main(void)\n"
            "{\n"
            "    texCoord = texCoords;\n"
            "    gl_Position = vec4(0.0);\n"
            "}\n";
        const char* fragmentShader =
            "#version 100\n"
            "precision highp float;\n"
            "#extension GL_OES_texture_float : require\n"
            "uniform sampler2D tex;\n"
            "varying vec2 texCoord;\n"
            "void main(void)\n"
            "{\n"
            "    float color = texture2D(tex, texCoord);\n"
            "    gl_FragColor = vec4(vec3(color*255), 1.0);\n"
            "}\n";

        GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        EXPECT_FALSE(res);
    }

    TEST_F(AGlslEffect, doesNotSupport3DTextureExtension)
    {
        const char* vertexShader =
            "#version 100\n"
            "precision highp float;\n"
            "attribute vec3 texCoords;\n"
            "varying vec3 texCoord;\n"
            "void main(void)\n"
            "{\n"
            "    texCoord = texCoords;\n"
            "    gl_Position = vec4(0.0);\n"
            "}\n";
        const char* fragmentShader =
            "#version 100\n"
            "precision highp float;\n"
            "#extension GL_OES_texture_3D : require\n"
            "uniform mediump sampler3D tex;\n"
            "varying vec3 texCoord;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = texture3D(tex, texCoord);\n"
            "}\n";

        GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        EXPECT_FALSE(res);
    }

    TEST_F(AGlslEffect, treatsUniformDeclaredInBothStagesWithSameNameAsSingleUniform)
    {
        const char* vertexShader =
            "uniform lowp vec4 dummy;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = dummy;\n"
            "}\n";
        const char* fragmentShader =
            "uniform lowp vec4 dummy;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = dummy;\n"
            "}\n";

        GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        const std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);

        EXPECT_EQ(1u, res->getUniformInputs().size());
    }

    TEST_F(AGlslEffect, rejectsEffectWithUniformDeclaredInBothStagesWithSameNameButDifferentDataType)
    {
        const char* vertexShader =
            "uniform lowp vec4 dummy;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = dummy;\n"
            "}\n";
        const char* fragmentShader =
            "uniform lowp float dummy;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = vec4(0.0, 0.0, 0.0, dummy);\n"
            "}\n";

        GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        const std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        EXPECT_FALSE(res);
    }

    // This is a bug in glslang, or maybe wrong configuration in ramses. In gles 100, active should be allowed
    TEST_F(AGlslEffect, doesNotAcceptActiveAsKeywordInVersion100)
    {
        const std::string basicVertexShader_v100 = R"SHADER(
                attribute float active;
                void main(void)
                {
                    gl_Position = vec4(0.0);
                }
                )SHADER";
        const std::string basicFragmentShader_v100 = R"SHADER(
                void main(void)
                {
                    gl_FragColor = vec4(0.0);
                })SHADER";
        GlslEffect ge(basicVertexShader_v100, basicFragmentShader_v100, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        EXPECT_FALSE(res);
    }

    TEST_F(AGlslEffect, canRetrieveGLSLErrorMessage)
    {
        const char* vertexShader = R"SHADER(
                                    #version 320 es
                                    out float inp;
                                    void main(void)
                                    {
                                        gl_Position = vec4(0.0)
                                    })SHADER";
        GlslEffect ge(vertexShader, basicFragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::OpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_FALSE(res);
        using namespace ::testing;
        EXPECT_THAT(ge.getEffectErrorMessages(),
                    AnyOf(Eq("[GLSL Compiler] vertex shader Shader Parsing Error:\n"
                            "ERROR: 2:5: '' :  syntax error\n"
                            "ERROR: 1 compilation errors.  No code generated.\n\n\n"),
                        Eq("[GLSL Compiler] vertex shader Shader Parsing Error:\n"
                            "ERROR: 2:5: '' :  syntax error, unexpected RIGHT_BRACE, expecting COMMA or SEMICOLON\n"
                            "ERROR: 1 compilation errors.  No code generated.\n\n\n")));
    }

    TEST_F(AGlslEffect, failsToCreateEffectIfUsingUBOAndLowFeatureLevel)
    {
        const char* vertexShader = R"SHADER(
                #version 310 es
                precision highp float;
                layout(std140,binding=1) uniform uniformBuffer_t
                {
                    mat4 ubo1Mat1;
                    float ubo1Float1[10];
                    mat3 ubo1Mat2;
                };

                void main(void)
                {
                    gl_Position = ubo1Mat1 * vec4(0.0);
                })SHADER";

        {
            GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
            EXPECT_FALSE(ge.createEffectResource(EFeatureLevel_01));
            EXPECT_EQ(ge.getEffectErrorMessages(), "Uniform buffer objects are supported only with feature level 02 or higher");
        }

        {
            GlslEffect ge(vertexShader, basicFragmentShader310, {}, emptyCompilerDefines, {}, ERenderBackendCompatibility::OpenGL, "");
            EXPECT_TRUE(ge.createEffectResource(EFeatureLevel_02));
            EXPECT_TRUE(ge.getEffectErrorMessages().empty());
        }
    }

    TEST_F(AGlslEffect, canCreateEffectWithVulkanAndOpenGLCompatibility)
    {
        GlslEffect ge(basicVertexShader, basicFragmentShader, "", emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::VulkanAndOpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);
        EXPECT_NE(0u, res->getVertexShaderSPIRVSize());
        EXPECT_NE(0u, res->getFragmentShaderSPIRVSize());
        EXPECT_EQ(0u, res->getGeometryShaderSPIRVSize()); // no geometry shader
        EXPECT_NE(nullptr, res->getVertexShaderSPIRV());
        EXPECT_NE(nullptr, res->getFragmentShaderSPIRV());

        CheckSPIRVShaderSanity(res->getVertexShaderSPIRV());
        CheckSPIRVShaderSanity(res->getFragmentShaderSPIRV());
    }

    TEST_F(AGlslEffect, canCreateEffectWithVulkanAndOpenGLCompatibility_WithGeometryShader)
    {
        GlslEffect ge(basicVertexShader, basicFragmentShader, basicGeometryShader, emptyCompilerDefines, emptySemanticInputs, ERenderBackendCompatibility::VulkanAndOpenGL, "");
        std::unique_ptr<EffectResource> res(ge.createEffectResource(EFeatureLevel_Latest));
        ASSERT_TRUE(res);
        EXPECT_NE(0u, res->getVertexShaderSPIRVSize());
        EXPECT_NE(0u, res->getFragmentShaderSPIRVSize());
        EXPECT_NE(0u, res->getGeometryShaderSPIRVSize());
        EXPECT_NE(nullptr, res->getVertexShaderSPIRV());
        EXPECT_NE(nullptr, res->getFragmentShaderSPIRV());
        EXPECT_NE(nullptr, res->getGeometryShaderSPIRV());

        CheckSPIRVShaderSanity(res->getVertexShaderSPIRV());
        CheckSPIRVShaderSanity(res->getFragmentShaderSPIRV());
        CheckSPIRVShaderSanity(res->getGeometryShaderSPIRV());
    }
}
