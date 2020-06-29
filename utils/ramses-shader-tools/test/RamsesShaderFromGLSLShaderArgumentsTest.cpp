//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesShaderFromGLSLShaderArguments.h"
#include "gtest/gtest.h"

TEST(ARamsesShaderFromGLSLShaderArguments, canLoadCorrectlyWhenAllParametersAreProvidedWithShortName)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-ov", "res/ramses-shader-tools-out.vertexshader", "-of", "res/ramses-shader-tools-out.fragmentshader", "-ot", "client", "-oe", "res/ramses-shader-tools-out.hash", "-on", "theName", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_TRUE(arguments.loadArguments(argc, argv));

    EXPECT_TRUE(arguments.getInVertexShader() == ramses_internal::String("res/ramses-shader-tools-test.vertexshader"));
    EXPECT_TRUE(arguments.getInFragmentShader() == ramses_internal::String("res/ramses-shader-tools-test.fragmentshader"));

    EXPECT_TRUE(arguments.getOutVertexShader() == ramses_internal::String("res/ramses-shader-tools-out.vertexshader"));
    EXPECT_TRUE(arguments.getOutFragmentShader() == ramses_internal::String("res/ramses-shader-tools-out.fragmentshader"));
    EXPECT_EQ("theName", arguments.getOutEffectName());
}

TEST(ARamsesShaderFromGLSLShaderArguments, canLoadCorrectlyWhenAllParametersAreProvidedWithFullName)
{
    const char* argv[] = { "program.exe", "--in-vertex-shader", "res/ramses-shader-tools-test.vertexshader", "--in-fragment-shader", "res/ramses-shader-tools-test.fragmentshader", "--in-config", "res/ramses-shader-tools-test.config", "--out-vertex-shader", "res/ramses-shader-tools-out.vertexshader", "--out-fragment-shader", "res/ramses-shader-tools-out.fragmentshader", "--out-effect-id-type", "client", "--out-effect-id", "res/ramses-shader-tools-out.hash", "--out-effect-name", "theName", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_TRUE(arguments.loadArguments(argc, argv));

    EXPECT_TRUE(arguments.getInVertexShader() == ramses_internal::String("res/ramses-shader-tools-test.vertexshader"));
    EXPECT_TRUE(arguments.getInFragmentShader() == ramses_internal::String("res/ramses-shader-tools-test.fragmentshader"));

    EXPECT_TRUE(arguments.getOutVertexShader() == ramses_internal::String("res/ramses-shader-tools-out.vertexshader"));
    EXPECT_TRUE(arguments.getOutFragmentShader() == ramses_internal::String("res/ramses-shader-tools-out.fragmentshader"));
    EXPECT_EQ("theName", arguments.getOutEffectName());
}

TEST(ARamsesShaderFromGLSLShaderArguments, reportsErrorWhenInputVertexShaderIsNotAvailable)
{
    const char* argv[] = { "program.exe", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-ov", "res/ramses-shader-tools-out.vertexshader", "-of", "res/ramses-shader-tools-out.fragmentshader", "-ot", "client", "-oe", "res/ramses-shader-tools-out.hash", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesShaderFromGLSLShaderArguments, reportsErrorWhenInputFragmentShaderIsNotAvailable)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-ic", "res/ramses-shader-tools-test.config", "-ov", "res/ramses-shader-tools-out.vertexshader", "-of", "res/ramses-shader-tools-out.fragmentshader", "-ot", "client", "-oe", "res/ramses-shader-tools-out.hash", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesShaderFromGLSLShaderArguments, reportsErrorWhenInputEffectConfigIsNotAvailable)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ov", "res/ramses-shader-tools-out.vertexshader", "-of", "res/ramses-shader-tools-out.fragmentshader", "-ot", "client", "-oe", "res/ramses-shader-tools-out.hash", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesShaderFromGLSLShaderArguments, reportsErrorWhenOutputVertexShaderIsNotAvailable)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-of", "res/ramses-shader-tools-out.fragmentshader", "-ot", "client", "-oe", "res/ramses-shader-tools-out.hash", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesShaderFromGLSLShaderArguments, reportsErrorWhenOutputFragmentShaderIsNotAvailable)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-ov", "res/ramses-shader-tools-out.vertexshader", "-ot", "client", "-oe", "res/ramses-shader-tools-out.hash", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesShaderFromGLSLShaderArguments, reportsErrorWhenOutputHashIsNotAvailable)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-ov", "res/ramses-shader-tools-out.vertexshader", "-ot", "client", "-of", "res/ramses-shader-tools-out.fragmentshader", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesShaderFromGLSLShaderArguments, reportsErrorWhenInputVertexShaderDoesNotExist)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-nonexist.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-ov", "res/ramses-shader-tools-out.vertexshader", "-of", "res/ramses-shader-tools-out.fragmentshader", "-ot", "client", "-oe", "res/ramses-shader-tools-out.hash", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesShaderFromGLSLShaderArguments, reportsErrorWhenInputFragmentShaderDoesNotExist)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-nonexist.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-ov", "res/ramses-shader-tools-out.vertexshader", "-of", "res/ramses-shader-tools-out.fragmentshader", "-ot", "client", "-oe", "res/ramses-shader-tools-out.hash", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesShaderFromGLSLShaderArguments, reportsErrorWhenInputEffectConfigDoesNotExist)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-nonexist.config", "-ov", "res/ramses-shader-tools-out.vertexshader", "-of", "res/ramses-shader-tools-out.fragmentshader", "-ot", "client", "-oe", "res/ramses-shader-tools-out.hash", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesShaderFromGLSLShaderArguments, reportsErrorWheInputEffectConfigFileIsNotValid)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-invalid-attribute-semantic-invalid-keyword.config", "-ov", "res/ramses-shader-tools-out.vertexshader", "-ot", "client", "-of", "res/ramses-shader-tools-out.fragmentshader", "-oe", "res/ramses-shader-tools-out.hash", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesShaderFromGLSLShaderArguments, reportsErrorWhenEffectIdTypeMissing)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-ov", "res/ramses-shader-tools-out.vertexshader", "-of", "res/ramses-shader-tools-out.fragmentshader", "-oe", "res/ramses-shader-tools-out.hash", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesShaderFromGLSLShaderArguments, reportsErrorWhenEffectIdTypeUnknown)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-ov", "res/ramses-shader-tools-out.vertexshader", "-of", "res/ramses-shader-tools-out.fragmentshader", "-ot", "foobar", "-oe", "res/ramses-shader-tools-out.hash", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesShaderFromGLSLShaderArguments, supportsNecessaryEffectIdTypes)
{
    {
        const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-ov", "res/ramses-shader-tools-out.vertexshader", "-of", "res/ramses-shader-tools-out.fragmentshader", "-ot", "client", "-oe", "res/ramses-shader-tools-out.hash", nullptr };
        int argc = sizeof(argv) / sizeof(char*) - 1;

        RamsesShaderFromGLSLShaderArguments arguments;
        ASSERT_TRUE(arguments.loadArguments(argc, argv));
        EXPECT_EQ(EEffectIdType_Client, arguments.getOutEffectIDType());
    }
    {
        const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-ov", "res/ramses-shader-tools-out.vertexshader", "-of", "res/ramses-shader-tools-out.fragmentshader", "-ot", "renderer", "-oe", "res/ramses-shader-tools-out.hash", nullptr };
        int argc = sizeof(argv) / sizeof(char*) - 1;

        RamsesShaderFromGLSLShaderArguments arguments;
        ASSERT_TRUE(arguments.loadArguments(argc, argv));
        EXPECT_EQ(EEffectIdType_Renderer, arguments.getOutEffectIDType());
    }
}

TEST(ARamsesShaderFromGLSLShaderArguments, hasDefaultShaderName)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-ov", "res/ramses-shader-tools-out.vertexshader", "-of", "res/ramses-shader-tools-out.fragmentshader", "-ot", "client", "-oe", "res/ramses-shader-tools-out.hash", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_TRUE(arguments.loadArguments(argc, argv));
    EXPECT_EQ("glsl shader", arguments.getOutEffectName());
}
