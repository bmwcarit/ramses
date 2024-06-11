//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/glslEffectBlock/GlslParser.h"
#include "gmock/gmock.h"
#include <string>

namespace ramses::internal
{
    class AGlslParser : public ::testing::Test
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
                out lowp vec4 colorOut;
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

        static std::unique_ptr<GlslParser>
        MakeParser(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader = std::string(), const std::vector<std::string>& defines ={})
        {
            return std::make_unique<GlslParser>(vertexShader, fragmentShader, geometryShader, defines);
        }
    };

    TEST_F(AGlslParser, canParseBasicShaders)
    {
        auto parser = MakeParser(basicVertexShader, basicFragmentShader, basicGeometryShader);
        ASSERT_TRUE(parser);

        EXPECT_TRUE(parser->valid());
        EXPECT_EQ("", parser->getErrors());
        EXPECT_TRUE(parser->generateWarnings().empty());
    }

    TEST_F(AGlslParser, rejectsEmptyVertexShader)
    {
        auto parser = MakeParser("", basicFragmentShader, "");
        EXPECT_FALSE(parser->valid());
        EXPECT_THAT(parser->getErrors(), testing::HasSubstr("Linker Error"));
        EXPECT_TRUE(parser->generateWarnings().empty());
    }

    TEST_F(AGlslParser, rejectsBrokenVertexShader)
    {
        auto parser = MakeParser("foo", basicFragmentShader, "");
        EXPECT_FALSE(parser->valid());
        EXPECT_THAT(parser->getErrors(), testing::HasSubstr("Parsing Error"));
        EXPECT_TRUE(parser->generateWarnings().empty());
    }

    TEST_F(AGlslParser, rejectsEmptyFragmentShader)
    {
        auto parser = MakeParser(basicVertexShader, "", "");
        EXPECT_FALSE(parser->valid());
        EXPECT_THAT(parser->getErrors(), testing::HasSubstr("Linker Error"));
        EXPECT_TRUE(parser->generateWarnings().empty());
    }

    TEST_F(AGlslParser, rejectsBrokenFragmentShader)
    {
        auto parser = MakeParser(basicVertexShader, "bar", "");
        EXPECT_FALSE(parser->valid());
        EXPECT_THAT(parser->getErrors(), testing::HasSubstr("Parsing Error"));
        EXPECT_TRUE(parser->generateWarnings().empty());
    }

    TEST_F(AGlslParser, acceptsEmptyGeometryShader)
    {
        auto parser = MakeParser(basicVertexShader, basicFragmentShader, "");
        EXPECT_TRUE(parser->valid());
        EXPECT_EQ("", parser->getErrors());
        EXPECT_TRUE(parser->generateWarnings().empty());
    }

    TEST_F(AGlslParser, rejectsBrokenGeometryShader)
    {
        auto parser = MakeParser(basicVertexShader, basicFragmentShader, "bar");
        EXPECT_FALSE(parser->valid());
        EXPECT_THAT(parser->getErrors(), testing::HasSubstr("Parsing Error"));
        EXPECT_TRUE(parser->generateWarnings().empty());
    }

    TEST_F(AGlslParser, usesProvidedDefines)
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

        auto parser = MakeParser(vertexShader, fragmentShader, "", compilerDefines);
        EXPECT_TRUE(parser->valid());
        EXPECT_EQ("", parser->getErrors());
        EXPECT_TRUE(parser->generateWarnings().empty());
    }

    TEST_F(AGlslParser, rejectsMissingNewlines)
    {
        const auto vertexShader = "#version 320 es void main(void) { gl_Position = vec4(0.0); }";
        auto parser = MakeParser(vertexShader, basicFragmentShader, "");
        EXPECT_FALSE(parser->valid());
        EXPECT_THAT(parser->getErrors(), testing::HasSubstr("Shader contains #version without newline"));
        EXPECT_TRUE(parser->generateWarnings().empty());
    }

    TEST_F(AGlslParser, warnUnusedVertexUniform)
    {
        const auto vertexShader = R"SHADER(
                #version 320 es
                uniform vec2 foo;
                void main(void)
                {
                    gl_Position = vec4(0.0);
                }
                )SHADER";
        auto parser = MakeParser(vertexShader, basicFragmentShader);
        EXPECT_TRUE(parser->valid());
        EXPECT_EQ("", parser->getErrors());
        auto warnings = parser->generateWarnings();
        EXPECT_EQ(1u, warnings.size());
        EXPECT_EQ(warnings[0].stage, EShaderStage::Vertex);
        EXPECT_EQ(warnings[0].category, EShaderWarningCategory::UnusedUniform);
        EXPECT_THAT(warnings[0].msg, testing::StartsWith("Unused [uniform]: 'foo'"));
    }

    TEST_F(AGlslParser, warnUnusedVertexInput)
    {
        const auto vertexShader = R"SHADER(
                #version 320 es
                in vec2 foo;
                void main(void)
                {
                    gl_Position = vec4(0.0);
                }
                )SHADER";
        auto parser = MakeParser(vertexShader, basicFragmentShader);
        EXPECT_TRUE(parser->valid());
        EXPECT_EQ("", parser->getErrors());
        auto warnings = parser->generateWarnings();
        EXPECT_EQ(1u, warnings.size());
        EXPECT_EQ(warnings[0].stage, EShaderStage::Vertex);
        EXPECT_EQ(warnings[0].category, EShaderWarningCategory::UnusedVarying);
        EXPECT_THAT(warnings[0].msg, testing::StartsWith("Unused [in]: 'foo'"));
    }

    TEST_F(AGlslParser, warnUnusedVertexOutput)
    {
        const auto vertexShader = R"SHADER(
                #version 320 es
                out vec2 foo;
                void main(void)
                {
                    gl_Position = vec4(0.0);
                }
                )SHADER";
        auto parser = MakeParser(vertexShader, basicFragmentShader);
        EXPECT_TRUE(parser->valid());
        EXPECT_EQ("", parser->getErrors());
        auto warnings = parser->generateWarnings();
        EXPECT_EQ(2u, warnings.size());
        EXPECT_EQ(warnings[0].stage, EShaderStage::Vertex);
        EXPECT_EQ(warnings[0].category, EShaderWarningCategory::UnusedVarying);
        EXPECT_THAT(warnings[0].msg, testing::StartsWith("Unused [out]: 'foo'"));
        EXPECT_EQ(warnings[1].category, EShaderWarningCategory::InterfaceMismatch);
        EXPECT_THAT(warnings[1].msg, testing::StartsWith("Vertex shader output 'foo' is not input in fragment shader"));
    }

    TEST_F(AGlslParser, warnUnusedVertexOutputInFragmentShader)
    {
        const auto vertexShader = R"SHADER(
                #version 320 es
                out vec2 foo;
                void main(void)
                {
                    foo = vec2(1.0);
                    gl_Position = vec4(0.0);
                }
                )SHADER";
        auto parser = MakeParser(vertexShader, basicFragmentShader);
        EXPECT_TRUE(parser->valid());
        EXPECT_EQ("", parser->getErrors());
        auto warnings = parser->generateWarnings();
        EXPECT_EQ(1u, warnings.size());
        EXPECT_EQ(warnings[0].stage, EShaderStage::Vertex);
        EXPECT_EQ(warnings[0].category, EShaderWarningCategory::InterfaceMismatch);
        EXPECT_THAT(warnings[0].msg, testing::StartsWith("Vertex shader output 'foo' is not input in fragment shader"));
    }

    TEST_F(AGlslParser, warnUnusedFragmentUniform)
    {
        const auto fragmentShader = R"SHADER(
                #version 320 es
                out lowp vec4 colorOut;
                uniform highp float foo;
                void main(void)
                {
                    colorOut = vec4(0.0);
                })SHADER";

        auto parser = MakeParser(basicVertexShader, fragmentShader);
        EXPECT_TRUE(parser->valid());
        EXPECT_EQ("", parser->getErrors());
        auto warnings = parser->generateWarnings();
        EXPECT_EQ(1u, warnings.size());
        EXPECT_EQ(warnings[0].stage, EShaderStage::Fragment);
        EXPECT_EQ(warnings[0].category, EShaderWarningCategory::UnusedUniform);
        EXPECT_THAT(warnings[0].msg, testing::StartsWith("Unused [uniform]: 'foo'"));
    }

    TEST_F(AGlslParser, warnUnusedFragmentInput)
    {
        const auto fragmentShader = R"SHADER(
                #version 320 es
                out lowp vec4 colorOut;
                in lowp float foo;
                void main(void)
                {
                    colorOut = vec4(0.0);
                })SHADER";

        auto parser = MakeParser(basicVertexShader, fragmentShader);
        EXPECT_TRUE(parser->valid());
        EXPECT_EQ("", parser->getErrors());
        auto warnings = parser->generateWarnings();
        EXPECT_EQ(2u, warnings.size());
        EXPECT_EQ(warnings[0].stage, EShaderStage::Fragment);
        EXPECT_EQ(warnings[0].category, EShaderWarningCategory::UnusedVarying);
        EXPECT_THAT(warnings[0].msg, testing::StartsWith("Unused [in]: 'foo'"));
        EXPECT_EQ(warnings[1].category, EShaderWarningCategory::InterfaceMismatch);
        EXPECT_THAT(warnings[1].msg, testing::StartsWith("Fragment shader input 'foo' is not output in vertex shader"));
    }

    TEST_F(AGlslParser, warnUnusedFragmentInputInVertexShader)
    {
        const auto fragmentShader = R"SHADER(
                #version 320 es
                out lowp vec4 colorOut;
                in lowp float foo;
                void main(void)
                {
                    colorOut = vec4(0.0);
                    colorOut.a = foo;
                })SHADER";

        auto parser = MakeParser(basicVertexShader, fragmentShader);
        EXPECT_TRUE(parser->valid());
        EXPECT_EQ("", parser->getErrors());
        auto warnings = parser->generateWarnings();
        EXPECT_EQ(1u, warnings.size());
        EXPECT_EQ(warnings[0].stage, EShaderStage::Fragment);
        EXPECT_EQ(warnings[0].category, EShaderWarningCategory::InterfaceMismatch);
        EXPECT_THAT(warnings[0].msg, testing::StartsWith("Fragment shader input 'foo' is not output in vertex shader"));
    }

    TEST_F(AGlslParser, noWarningForExplicitLayoutLocation)
    {
        const auto vertexShader = R"SHADER(
                #version 320 es
                layout(location = 1) out vec2 foo1;
                layout(location = 0) out float foo0;
                void main(void)
                {
                    foo0 = 0.0;
                    foo1 = vec2(1.0);
                    gl_Position = vec4(0.0);
                }
                )SHADER";

        const auto fragmentShader = R"SHADER(
                #version 320 es
                out lowp vec4 colorOut;
                layout(location = 1) in highp vec2 bar;
                layout(location = 0) in highp float bar0;
                void main(void)
                {
                    colorOut = vec4(0.0);
                    colorOut.a = bar.x;
                    colorOut.r = bar0;
                })SHADER";

        auto parser = MakeParser(vertexShader, fragmentShader);
        EXPECT_TRUE(parser->valid());
        EXPECT_EQ("", parser->getErrors());
        auto warnings = parser->generateWarnings();
        EXPECT_EQ(0u, warnings.size());
    }

    TEST_F(AGlslParser, doesNotWarnVertexLocationNotFoundButNameMatches)
    {
        const auto vertexShader = R"SHADER(
                #version 320 es
                layout(location = 1) out vec2 foo1;
                out float foo0;
                void main(void)
                {
                    foo0 = 0.0;
                    foo1 = vec2(1.0);
                    gl_Position = vec4(0.0);
                }
                )SHADER";

        const auto fragmentShader = R"SHADER(
                #version 320 es
                out lowp vec4 colorOut;
                layout(location = 1) in highp vec2 bar;
                layout(location = 0) in highp float foo0;
                void main(void)
                {
                    colorOut = vec4(0.0);
                    colorOut.a = bar.x;
                    colorOut.r = foo0;
                })SHADER";

        auto parser = MakeParser(vertexShader, fragmentShader);
        EXPECT_TRUE(parser->valid());
        EXPECT_EQ("", parser->getErrors());
        auto warnings = parser->generateWarnings();
        EXPECT_EQ(0u, warnings.size());
    }

    TEST_F(AGlslParser, warnFragmentLocationNotFoundButNameMatches)
    {
        const auto vertexShader = R"SHADER(
                #version 320 es
                layout(location = 1) out vec2 foo1;
                layout(location = 0) out float foo0;
                void main(void)
                {
                    foo0 = 0.0;
                    foo1 = vec2(1.0);
                    gl_Position = vec4(0.0);
                }
                )SHADER";

        const auto fragmentShader = R"SHADER(
                #version 320 es
                out lowp vec4 colorOut;
                layout(location = 1) in highp vec2 bar;
                in highp float foo0;
                void main(void)
                {
                    colorOut = vec4(0.0);
                    colorOut.a = bar.x;
                    colorOut.r = foo0;
                })SHADER";

        auto parser = MakeParser(vertexShader, fragmentShader);
        EXPECT_TRUE(parser->valid());
        EXPECT_EQ("", parser->getErrors());
        auto warnings = parser->generateWarnings();
        EXPECT_EQ(1u, warnings.size());
        EXPECT_EQ(warnings[0].stage, EShaderStage::Vertex);
        EXPECT_EQ(warnings[0].category, EShaderWarningCategory::InterfaceMismatch);
        EXPECT_EQ(warnings[0].msg, "Vertex shader output 'layout(location = 0) foo0' is not input in fragment shader");
    }

    TEST_F(AGlslParser, failsParsingIfTypesMismatch)
    {
        const auto vertexShader = R"SHADER(
                #version 320 es
                out vec2 foo0;
                void main(void)
                {
                    foo0 = vec2(0.0);
                    gl_Position = vec4(0.0);
                }
                )SHADER";

        const auto fragmentShader = R"SHADER(
                #version 320 es
                out lowp vec4 colorOut;
                in highp float foo0;
                void main(void)
                {
                    colorOut = vec4(0.0);
                    colorOut.r = foo0;
                })SHADER";

        auto parser = MakeParser(vertexShader, fragmentShader);
        EXPECT_FALSE(parser->valid());
        EXPECT_THAT(parser->getErrors(), ::testing::HasSubstr("ERROR: Linking vertex and fragment stages: Types must match"));
        auto warnings = parser->generateWarnings();
        EXPECT_EQ(0u, warnings.size());
    }

    TEST_F(AGlslParser, warnPrecisionMismatch)
    {
        const auto vertexShader = R"SHADER(
                #version 320 es
                out lowp float foo0;
                void main(void)
                {
                    foo0 = 0.0;
                    gl_Position = vec4(0.0);
                }
                )SHADER";

        const auto fragmentShader = R"SHADER(
                #version 320 es
                out lowp vec4 colorOut;
                in highp float foo0;
                void main(void)
                {
                    colorOut = vec4(0.0);
                    colorOut.r = foo0;
                })SHADER";

        auto parser = MakeParser(vertexShader, fragmentShader);
        EXPECT_TRUE(parser->valid());
        ASSERT_EQ("", parser->getErrors());
        auto warnings = parser->generateWarnings();
        EXPECT_EQ(1u, warnings.size());
        ASSERT_LE(1u, warnings.size());
        EXPECT_EQ(warnings[0].stage, EShaderStage::Vertex);
        EXPECT_EQ(warnings[0].category, EShaderWarningCategory::PrecisionMismatch);
        EXPECT_EQ(warnings[0].msg, "Precision mismatch: 'foo0'. (Vertex: smooth out lowp float, Fragment: smooth in highp float)");
    }

    TEST_F(AGlslParser, warnUniformPrecisionMismatch)
    {
        const auto vertexShader = R"SHADER(
                #version 320 es
                precision mediump float;
                out float foo0;
                uniform vec2 u_bar;
                void main(void)
                {
                    foo0 = u_bar.x;
                    gl_Position = vec4(0.0);
                }
                )SHADER";

        const auto fragmentShader = R"SHADER(
                #version 320 es
                precision highp float;
                out vec4 colorOut;
                in float foo0;
                uniform vec2 u_bar;
                void main(void)
                {
                    colorOut = vec4(u_bar.y);
                    colorOut.r = foo0;
                })SHADER";

        auto parser = MakeParser(vertexShader, fragmentShader);
        EXPECT_FALSE(parser->getProgram());
        EXPECT_THAT(parser->getErrors(), ::testing::HasSubstr("ERROR: Linking unknown stage and fragment stages: Precision qualifiers must match"));
        auto warnings = parser->generateWarnings();
        EXPECT_EQ(0u, warnings.size());
    }

    TEST_F(AGlslParser, shadersFromParts)
    {
        const auto vertexShader = R"SHADER(#version 320 es
                out lowp float foo0;
                void main(void)
                {
                    foo0 = 0.0;
                    gl_Position = vec4(0.0);
                }
                )SHADER";

        const auto fragmentShader = R"SHADER(#version 320 es
                out lowp vec4 colorOut;
                in highp float foo0;
                void main(void)
                {
                    colorOut = vec4(0.0);
                    colorOut.r = foo0;
                })SHADER";

        auto parser = MakeParser(vertexShader, fragmentShader);
        EXPECT_TRUE(parser->valid());
        EXPECT_EQ(vertexShader, parser->getVertexShader());
        EXPECT_EQ(fragmentShader, parser->getFragmentShader());
        EXPECT_EQ("", parser->getGeometryShader());
    }
}
