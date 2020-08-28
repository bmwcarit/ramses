//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "glslEffectBlock/GlslEffect.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "Resource/EffectResource.h"
#include "gmock/gmock.h"
#include <memory>

using namespace ramses_internal;

class AGlslEffect : public ::testing::Test
{
public:
    AGlslEffect()
    {
    }

    const String basicVertexShader = R"SHADER(
            #version 320 es
            void main(void)
            {
                gl_Position = vec4(0.0);
            }
            )SHADER";
    const String basicFragmentShader = R"SHADER(
            #version 320 es
            out lowp vec4 colorOut;
            void main(void)
            {
                colorOut = vec4(0.0);
            })SHADER";
    const String basicGeometryShader = R"SHADER(
            #version 320 es
            layout(points) in;
            layout(points, max_vertices = 1) out;
            void main() {
                gl_Position = vec4(0.0);
                EmitVertex();
            }
            )SHADER";
    const std::vector<String> emptyCompilerDefines;
    const HashMap<String, EFixedSemantics> emptySemanticInputs;

protected:
    static void VerifyUniformInputExists(const EffectResource& effect, const char* uniformName)
    {
        const DataFieldHandle effecthandle = effect.getUniformDataFieldHandleByName(String(uniformName));
        EXPECT_TRUE(effecthandle.isValid());
    };
};

TEST_F(AGlslEffect, canParseBasicShaders)
{
    GlslEffect ge(basicVertexShader, basicFragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
    ASSERT_TRUE(res);

    EXPECT_EQ(0u, res->getUniformInputs().size());
    EXPECT_EQ(0u, res->getAttributeInputs().size());
    EXPECT_EQ(String(), res->getName());
}

TEST_F(AGlslEffect, canParseBasicShaders_WithGeometryShader)
{
    GlslEffect ge(basicVertexShader, basicFragmentShader, basicGeometryShader, emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
    ASSERT_TRUE(res);

    EXPECT_EQ(0u, res->getUniformInputs().size());
    EXPECT_EQ(0u, res->getAttributeInputs().size());
    EXPECT_EQ(String(), res->getName());
}

TEST_F(AGlslEffect, usesPassedName)
{
    GlslEffect ge(basicVertexShader, basicFragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "someName");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
    ASSERT_TRUE(res);

    EXPECT_EQ(String("someName"), res->getName());
}

TEST_F(AGlslEffect, rejectsEmptyVertexShader)
{
    GlslEffect ge("", basicFragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
    EXPECT_FALSE(res);
}

TEST_F(AGlslEffect, rejectsBrokenVertexShader)
{
    GlslEffect ge("foo", basicFragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
    EXPECT_FALSE(res);
}

TEST_F(AGlslEffect, rejectsEmptyFragmentShader)
{
    GlslEffect ge(basicVertexShader, "", "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
    EXPECT_FALSE(res);
}

TEST_F(AGlslEffect, rejectsBrokenFragmentShader)
{
    GlslEffect ge(basicVertexShader, "bar", "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
    EXPECT_FALSE(res);
}

TEST_F(AGlslEffect, acceptsEmptyGeometryShader)
{
    GlslEffect ge(basicVertexShader, basicFragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
    EXPECT_TRUE(res);
}

TEST_F(AGlslEffect, rejectsBrokenGeometryShader)
{
    GlslEffect ge(basicVertexShader, basicFragmentShader, "bar", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
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
    std::vector<String> compilerDefines;
    compilerDefines.push_back("DEFINE_ZERO vec4(0.0)");
    compilerDefines.push_back("DEFINE_ONE vec4(1.0)");
    GlslEffect ge(vertexShader, fragmentShader, "", compilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
    ASSERT_TRUE(res);

    EXPECT_TRUE(String(res->getVertexShader()).find(compilerDefines[0]) != -1);
    EXPECT_TRUE(String(res->getVertexShader()).find(compilerDefines[1]) != -1);
    EXPECT_TRUE(String(res->getFragmentShader()).find(compilerDefines[0]) != -1);
    EXPECT_TRUE(String(res->getFragmentShader()).find(compilerDefines[1]) != -1);
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
    std::vector<String> compilerDefines;
    compilerDefines.push_back("FIRST_DEFINE foo");
    compilerDefines.push_back("OTHER_DEFINE bar");
    GlslEffect ge(vertexShader, fragmentShader, "", compilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
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

    GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));

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

    GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));

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

    const String geometryShader = R"SHADER(
            #version 310 es
            #extension GL_EXT_geometry_shader : enable
            layout(points) in;
            layout(points, max_vertices = 1) out;
            void main() {
                gl_Position = vec4(0.0);
                EmitVertex();
            }
            )SHADER";

    GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));

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

    GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));

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

    GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));

    EXPECT_FALSE(res);
}

TEST_F(AGlslEffect, canParseShaderInputs)
{
    const char* vertexShader = R"SHADER(
            #version 320 es
            precision highp float;
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
            uniform sampler2D uniformSampler;
            uniform vec4 uniformVec;
            out vec4 colorOut;
            void main(void)
            {
                colorOut = vec4(0.0);
            })SHADER";
    const String geometryShader = R"SHADER(
            #version 320 es
            precision highp float;
            layout(points) in;
            layout(points, max_vertices = 1) out;
            out vec4 g_colorOut;
            uniform float uniformGeomFloat;
            uniform vec4 uniformGeomVec;
            uniform sampler2D uniformGeomSampler;
            void main() {
                gl_Position = uniformGeomVec + vec4(uniformGeomFloat);
                g_colorOut = texture(uniformGeomSampler, vec2(0.0));
                EmitVertex();
            }
            )SHADER";

    HashMap<String, EFixedSemantics> semantics;
    semantics.put("uniformWithSemantic", EFixedSemantics_ModelViewProjectionMatrix);
    semantics.put("attributeWithSemantic", EFixedSemantics_CameraWorldPosition);

    GlslEffect ge(vertexShader, fragmentShader, geometryShader, emptyCompilerDefines, semantics, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
    ASSERT_TRUE(res);

    const EffectInputInformationVector& uniforms = res->getUniformInputs();
    const EffectInputInformationVector& attributes = res->getAttributeInputs();

    ASSERT_EQ(8u, uniforms.size());
    EXPECT_EQ(EffectInputInformation("uniformWithSemantic", 1, EDataType::Matrix44F, EFixedSemantics_ModelViewProjectionMatrix), uniforms[0]);
    EXPECT_EQ(EffectInputInformation("matrix3x3", 1, EDataType::Matrix33F, EFixedSemantics_Invalid), uniforms[1]);
    EXPECT_EQ(EffectInputInformation("matrix2x2", 1, EDataType::Matrix22F, EFixedSemantics_Invalid), uniforms[2]);
    EXPECT_EQ(EffectInputInformation("uniformSampler", 1, EDataType::TextureSampler2D, EFixedSemantics_Invalid), uniforms[3]);
    EXPECT_EQ(EffectInputInformation("uniformVec", 1, EDataType::Vector4F, EFixedSemantics_Invalid), uniforms[4]);
    EXPECT_EQ(EffectInputInformation("uniformGeomFloat", 1, EDataType::Float, EFixedSemantics_Invalid), uniforms[5]);
    EXPECT_EQ(EffectInputInformation("uniformGeomVec", 1, EDataType::Vector4F, EFixedSemantics_Invalid), uniforms[6]);
    EXPECT_EQ(EffectInputInformation("uniformGeomSampler", 1, EDataType::TextureSampler2D, EFixedSemantics_Invalid), uniforms[7]);

    ASSERT_EQ(2u, attributes.size());
    EXPECT_EQ(EffectInputInformation("attributeWithSemantic", 1, EDataType::Vector3Buffer, EFixedSemantics_CameraWorldPosition), attributes[0]);
    EXPECT_EQ(EffectInputInformation("attributeFloat", 1, EDataType::FloatBuffer, EFixedSemantics_Invalid), attributes[1]);
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
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = vec4(0.0);\n"
        "}\n";
    GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));

    ASSERT_TRUE(res);

    EXPECT_EQ(0u, res->getAttributeInputs().size());

    const EffectInputInformationVector& uniforms = res->getUniformInputs();

    ASSERT_EQ(2u, uniforms.size());
    EXPECT_EQ(EffectInputInformation("s2d", 1, EDataType::TextureSampler2D, EFixedSemantics_Invalid), uniforms[0]);
    EXPECT_EQ(EffectInputInformation("sc", 1, EDataType::TextureSamplerCube, EFixedSemantics_Invalid), uniforms[1]);
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
        "out vec4 color;\n"
        "void main(void)\n"
        "{\n"
        "    color = vec4(0.0);\n"
        "}\n";
    GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));

    ASSERT_TRUE(res);

    EXPECT_EQ(0u, res->getAttributeInputs().size());

    const EffectInputInformationVector& uniforms = res->getUniformInputs();

    ASSERT_EQ(3u, uniforms.size());
    EXPECT_EQ(EffectInputInformation("s2d", 1, EDataType::TextureSampler2D, EFixedSemantics_Invalid), uniforms[0]);
    EXPECT_EQ(EffectInputInformation("s3d", 1, EDataType::TextureSampler3D, EFixedSemantics_Invalid), uniforms[1]);
    EXPECT_EQ(EffectInputInformation("sc", 1, EDataType::TextureSamplerCube, EFixedSemantics_Invalid), uniforms[2]);
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
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = vec4(0.0);\n"
        "}\n";
    GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));

    ASSERT_TRUE(res);

    EXPECT_EQ(0u, res->getAttributeInputs().size());

    const EffectInputInformationVector& uniforms = res->getUniformInputs();

    ASSERT_EQ(3u, uniforms.size());
    EXPECT_EQ(EffectInputInformation("v", 4, EDataType::Vector3F, EFixedSemantics_Invalid), uniforms[0]);
    EXPECT_EQ(EffectInputInformation("m", 2, EDataType::Matrix44F, EFixedSemantics_Invalid), uniforms[1]);
    EXPECT_EQ(EffectInputInformation("i", 2, EDataType::Int32, EFixedSemantics_Invalid), uniforms[2]);
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

    HashMap<String, EFixedSemantics> semantics;
    semantics.put("uniformWithWrongSemantic", EFixedSemantics_ModelViewProjectionMatrix);

    GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, semantics, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
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
    GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
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
    GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
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
    GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
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
    GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
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
    GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
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
    GlslEffectTestRunnable()
        : createdSuccessfully(false)
    {
    }

    virtual void run()
    {
        const std::vector<ramses_internal::String> defs;
        const ramses_internal::HashMap<ramses_internal::String, ramses_internal::EFixedSemantics> sems;
        const char* v = "void main(){gl_Position=vec4(0);}";
        const char* f = "void main(){gl_FragColor=vec4(0);}";

        GlslEffect eff(v, f, "", defs, sems, "myname");
        std::unique_ptr<EffectResource> resource(eff.createEffectResource(ResourceCacheFlag(0u)));
        createdSuccessfully = static_cast<bool>(resource);
    }

    Bool createdSuccessfully;
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
    const String basicVertexShader_v100 = R"SHADER(
            void main(void)
            {
                gl_Position = vec4(0.0);
            }
            )SHADER";
    const String basicFragmentShader_v100 = R"SHADER(
            void main(void)
            {
                gl_FragColor = vec4(0.0);
            })SHADER";

    GlslEffect ge(basicVertexShader_v100, basicFragmentShader_v100, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
    ASSERT_TRUE(res);
    EXPECT_EQ(100u, ge.getShadingLanguageVersion());
}

TEST_F(AGlslEffect, isAbleToParseShadersWithVersionString)
{
    GlslEffect ge(basicVertexShader, basicFragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
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

    GlslEffect ge(vertexShader, basicFragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
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

    GlslEffect ge(vertexShader, basicFragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
    EXPECT_TRUE(res);
}

TEST_F(AGlslEffect, doesNotSupportExternalTextureExtension)
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

    GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
    EXPECT_FALSE(res);
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

    GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
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

    GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
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

    GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    const std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
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

    GlslEffect ge(vertexShader, fragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    const std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
    EXPECT_FALSE(res);
}

TEST_F(AGlslEffect, DISABLED_acceptsActiveAsKeyword)
{
    const String basicVertexShader_v100 = R"SHADER(
            attribute float active;
            void main(void)
            {
                gl_Position = vec4(0.0);
            }
            )SHADER";
    const String basicFragmentShader_v100 = R"SHADER(
            void main(void)
            {
                gl_FragColor = vec4(0.0);
            })SHADER";
    GlslEffect ge(basicVertexShader_v100, basicFragmentShader_v100, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
    EXPECT_TRUE(res);
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
    GlslEffect ge(vertexShader, basicFragmentShader, "", emptyCompilerDefines, emptySemanticInputs, "");
    std::unique_ptr<EffectResource> res(ge.createEffectResource(ResourceCacheFlag(0u)));
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
