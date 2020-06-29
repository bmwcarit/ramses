//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesEffectFromGLSLShaderArguments.h"
#include "gtest/gtest.h"

TEST(ARamsesEffectFromGLSLShaderArguments, canLoadCorrectlyWhenAllParametersAreProvidedWithShortName)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-or", "res/ramses-shader-tools-out.res", "-on", "outputEffectName", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesEffectFromGLSLShaderArguments arguments;
    EXPECT_TRUE(arguments.loadArguments(argc, argv));

    EXPECT_TRUE(arguments.getInVertexShader() == ramses_internal::String("res/ramses-shader-tools-test.vertexshader"));
    EXPECT_TRUE(arguments.getInFragmentShader() == ramses_internal::String("res/ramses-shader-tools-test.fragmentshader"));
    EXPECT_TRUE(arguments.getOutEffectName() == ramses_internal::String("outputEffectName"));
    EXPECT_TRUE(arguments.getOutResourceFile() == ramses_internal::String("res/ramses-shader-tools-out.res"));
    EXPECT_EQ(ramses_internal::String(), arguments.getOutEffectIDFile());
}

TEST(ARamsesEffectFromGLSLShaderArguments, canLoadCorrectlyWhenAllParametersAreProvidedWithFullName)
{
    const char* argv[] = { "program.exe", "--in-vertex-shader", "res/ramses-shader-tools-test.vertexshader", "--in-fragment-shader", "res/ramses-shader-tools-test.fragmentshader", "--in-config", "res/ramses-shader-tools-test.config", "--out-resource-file", "res/ramses-shader-tools-out.res", "--out-effect-name", "outputEffectName", nullptr};
    int argc = sizeof(argv) / sizeof(const char*) - 1;

    RamsesEffectFromGLSLShaderArguments arguments;
    EXPECT_TRUE(arguments.loadArguments(argc, argv));

    EXPECT_TRUE(arguments.getInVertexShader() == ramses_internal::String("res/ramses-shader-tools-test.vertexshader"));
    EXPECT_TRUE(arguments.getInFragmentShader() == ramses_internal::String("res/ramses-shader-tools-test.fragmentshader"));
    EXPECT_TRUE(arguments.getOutEffectName() == ramses_internal::String("outputEffectName"));
    EXPECT_TRUE(arguments.getOutResourceFile() == ramses_internal::String("res/ramses-shader-tools-out.res"));
    EXPECT_EQ(ramses_internal::String(), arguments.getOutEffectIDFile());
}

TEST(ARamsesEffectFromGLSLShaderArguments, canGenerateEffectNameCorrectlyWhenEffectNameIsNotProvided)
{
    {
        const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-or", "res/ramses-shader-tools-out.res", nullptr };
        int argc = sizeof(argv) / sizeof(char*) - 1;

        RamsesEffectFromGLSLShaderArguments arguments;
        EXPECT_TRUE(arguments.loadArguments(argc, argv));
        EXPECT_TRUE(arguments.getOutEffectName() == ramses_internal::String("ramses-shader-tools-test.vertexshader_ramses-shader-tools-test.fragmentshader"));
    }
    {
        const char* argv[] = { "program.exe", "--in-vertex-shader", "res/ramses-shader-tools-test.vertexshader", "--in-fragment-shader", "res/ramses-shader-tools-test.fragmentshader", "--in-config", "res/ramses-shader-tools-test.config", "--out-resource-file", "res/ramses-shader-tools-out.res", nullptr };
        int argc = sizeof(argv) / sizeof(const char*) - 1;

        RamsesEffectFromGLSLShaderArguments arguments;
        EXPECT_TRUE(arguments.loadArguments(argc, argv));
        EXPECT_TRUE(arguments.getOutEffectName() == ramses_internal::String("ramses-shader-tools-test.vertexshader_ramses-shader-tools-test.fragmentshader"));
    }
}

TEST(ARamsesEffectFromGLSLShaderArguments, reportsErrorWhenInputVertexShaderIsNotAvailable)
{
    const char* argv[] = { "program.exe", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-or", "res/ramses-shader-tools-out.res", nullptr };
    int argc = sizeof(argv) / sizeof(const char*) - 1;

    RamsesEffectFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesEffectFromGLSLShaderArguments, reportsErrorWhenInputFragmentShaderIsNotAvailable)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-ic", "res/ramses-shader-tools-test.config", "-or", "res/ramses-shader-tools-out.res", nullptr };
    int argc = sizeof(argv) / sizeof(const char*) - 1;

    RamsesEffectFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesEffectFromGLSLShaderArguments, reportsErrorWhenInputEffectConfigIsNotAvailable)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-or", "res/ramses-shader-tools-out.res", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesEffectFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesEffectFromGLSLShaderArguments, reportsErrorWhenOutputResourceFileIsNotAvailable)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesEffectFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesEffectFromGLSLShaderArguments, reportsErrorWhenInputVertexShaderDoesNotExist)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-nonexist.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-or", "res/ramses-shader-tools-out.res", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesEffectFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesEffectFromGLSLShaderArguments, reportsErrorWhenInputFragmentShaderDoesNotExist)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-nonexist.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-or", "res/ramses-shader-tools-out.res", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesEffectFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesEffectFromGLSLShaderArguments, reportsErrorWhenInputEffectConfigDoesNotExist)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-nonexist.config", "-or", "res/ramses-shader-tools-out.res", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesEffectFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesEffectFromGLSLShaderArguments, reportsErrorWheInputEffectConfigFileIsNotValid)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-invalid-attribute-semantic-invalid-keyword.config", "-or", "res/ramses-shader-tools-out.res", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesEffectFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesEffectFromGLSLShaderArguments, canParseEffectIdShortName)
{
    const char* argv[] = { "program.exe",
        "-iv", "res/ramses-shader-tools-test.vertexshader",
        "-if", "res/ramses-shader-tools-test.fragmentshader",
        "-ic", "res/ramses-shader-tools-test.config",
        "-or", "res/ramses-shader-tools-out.res",
        "-on", "outputEffectName",
        "-oe", "outputEffectId",
        "-ot", "client", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesEffectFromGLSLShaderArguments arguments;
    EXPECT_TRUE(arguments.loadArguments(argc, argv));

    EXPECT_EQ("outputEffectId", arguments.getOutEffectIDFile());
    EXPECT_EQ(EEffectIdType_Client, arguments.getOutEffectIDType());
}

TEST(ARamsesEffectFromGLSLShaderArguments, canParseEffectIdLongName)
{
    const char* argv[] = { "program.exe",
        "-iv", "res/ramses-shader-tools-test.vertexshader",
        "-if", "res/ramses-shader-tools-test.fragmentshader",
        "-ic", "res/ramses-shader-tools-test.config",
        "-or", "res/ramses-shader-tools-out.res",
        "-on", "outputEffectName",
        "--out-effect-id", "outputEffectId",
        "--out-effect-id-type", "client", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesEffectFromGLSLShaderArguments arguments;
    EXPECT_TRUE(arguments.loadArguments(argc, argv));

    EXPECT_EQ("outputEffectId", arguments.getOutEffectIDFile());
    EXPECT_EQ(EEffectIdType_Client, arguments.getOutEffectIDType());
}

TEST(ARamsesEffectFromGLSLShaderArguments, canParseEffectIdTypeRenderer)
{
    const char* argv[] = { "program.exe",
        "-iv", "res/ramses-shader-tools-test.vertexshader",
        "-if", "res/ramses-shader-tools-test.fragmentshader",
        "-ic", "res/ramses-shader-tools-test.config",
        "-or", "res/ramses-shader-tools-out.res",
        "-on", "outputEffectName",
        "-oe", "outputEffectId",
        "-ot", "renderer", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesEffectFromGLSLShaderArguments arguments;
    EXPECT_TRUE(arguments.loadArguments(argc, argv));

    EXPECT_EQ("outputEffectId", arguments.getOutEffectIDFile());
    EXPECT_EQ(EEffectIdType_Renderer, arguments.getOutEffectIDType());
}

TEST(ARamsesEffectFromGLSLShaderArguments, effectIdNameParseFailsWhenIdTypeMissing)
{
    const char* argv[] = { "program.exe",
        "-iv", "res/ramses-shader-tools-test.vertexshader",
        "-if", "res/ramses-shader-tools-test.fragmentshader",
        "-ic", "res/ramses-shader-tools-test.config",
        "-or", "res/ramses-shader-tools-out.res",
        "-on", "outputEffectName",
        "-oe", "outputEffectId", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesEffectFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}

TEST(ARamsesEffectFromGLSLShaderArguments, effectIdTypeParseFailsForUnknownType)
{
    const char* argv[] = { "program.exe",
        "-iv", "res/ramses-shader-tools-test.vertexshader",
        "-if", "res/ramses-shader-tools-test.fragmentshader",
        "-ic", "res/ramses-shader-tools-test.config",
        "-or", "res/ramses-shader-tools-out.res",
        "-on", "outputEffectName",
        "-oe", "outputEffectId",
        "-ot", "foobar", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesEffectFromGLSLShaderArguments arguments;
    EXPECT_FALSE(arguments.loadArguments(argc, argv));
}
