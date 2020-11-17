//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ShaderConverter.h"
#include "RamsesShaderFromGLSLShaderArguments.h"
#include "gtest/gtest.h"
#include "Utils/File.h"
#include "FileUtils.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/AttributeInput.h"
#include "EffectImpl.h"
#include "Collections/StringOutputStream.h"

namespace
{
    const char* OUTPUT_VERTEX_SHADER = "res/ramses-shader-tools-out.vertexshader";
    const char* OUTPUT_FRAGMENT_SHADER = "res/ramses-shader-tools-out.fragmentshader";
    const char* OUTPUT_HASH = "res/ramses-shader-tools-out.hash";
}

class AShaderConverter : public testing::Test
{
protected:
    AShaderConverter()
        : framework()
        , ramsesClient(*framework.createClient("shader-tool-test client"))
        , scene(*ramsesClient.createScene(ramses::sceneId_t{ 0xf00 }))
    {
        cleanupOutputFiles();
    }

    ~AShaderConverter()
    {
        cleanupOutputFiles();
    }

    void checkExistanceOfOutputFiles(bool exists)
    {
        EXPECT_TRUE(exists == FileUtils::FileExists(OUTPUT_VERTEX_SHADER));
        EXPECT_TRUE(exists == FileUtils::FileExists(OUTPUT_FRAGMENT_SHADER));
        EXPECT_TRUE(exists == FileUtils::FileExists(OUTPUT_HASH));
    }

    ramses::Effect* createEffectFromOutputFiles()
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShaderFromFile(OUTPUT_VERTEX_SHADER);
        effectDesc.setFragmentShaderFromFile(OUTPUT_FRAGMENT_SHADER);
        return scene.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "shadertool from output effect");
    }

    void cleanupOutputFiles()
    {
        FileUtils::RemoveFileIfExist(OUTPUT_VERTEX_SHADER);
        FileUtils::RemoveFileIfExist(OUTPUT_FRAGMENT_SHADER);
        FileUtils::RemoveFileIfExist(OUTPUT_HASH);

        EXPECT_FALSE(FileUtils::FileExists(OUTPUT_VERTEX_SHADER));
        EXPECT_FALSE(FileUtils::FileExists(OUTPUT_FRAGMENT_SHADER));
        EXPECT_FALSE(FileUtils::FileExists(OUTPUT_HASH));
    }

    ramses::UniformInput uniformInput;
    ramses::AttributeInput attributeInput;

    ramses::RamsesFramework framework;
    ramses::RamsesClient& ramsesClient;
    ramses::Scene& scene;
};

TEST_F(AShaderConverter, canConvertShaderFromGlslShaderWithCompilerDefines)
{
    const char* argv[] = {"program.exe",
        "-iv", "res/ramses-shader-tools-test.vertexshader",
        "-if", "res/ramses-shader-tools-test.fragmentshader",
        "-ic", "res/ramses-shader-tools-test.config",
        "-os", "gles2.0",
        "-ov", OUTPUT_VERTEX_SHADER,
        "-of", OUTPUT_FRAGMENT_SHADER,
        "-ot", "client",
        "-oe", OUTPUT_HASH, nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_TRUE(arguments.loadArguments(argc, argv));
    EXPECT_TRUE(ShaderConverter::Convert(arguments));
    checkExistanceOfOutputFiles(true);

    ramses::Effect* fromOutputEffect = createEffectFromOutputFiles();
    ASSERT_TRUE(fromOutputEffect != nullptr);

    EXPECT_EQ(ramses::StatusOK, fromOutputEffect->findUniformInput("matrix44fInput", uniformInput));
    EXPECT_EQ(ramses::StatusOK, fromOutputEffect->findUniformInput("texture2dInput", uniformInput));
    EXPECT_EQ(ramses::StatusOK, fromOutputEffect->findAttributeInput("vec4fArrayInput", attributeInput));
    EXPECT_EQ(ramses::StatusOK, fromOutputEffect->findAttributeInput("floatArrayInputDummy", attributeInput)); // enabled by define
}

TEST_F(AShaderConverter, canConvertShaderFromGlslShaderWithoutCompilerDefines)
{
    const char* argv[] = { "program.exe",
        "-iv", "res/ramses-shader-tools-test.vertexshader",
        "-if", "res/ramses-shader-tools-test.fragmentshader",
        "-ic", "res/ramses-shader-tools-test_without_compiler_defines.config",
        "-os", "gles2.0",
        "-ov", OUTPUT_VERTEX_SHADER,
        "-of", OUTPUT_FRAGMENT_SHADER,
        "-ot", "client",
        "-oe", OUTPUT_HASH, nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_TRUE(arguments.loadArguments(argc, argv));
    EXPECT_TRUE(ShaderConverter::Convert(arguments));
    checkExistanceOfOutputFiles(true);

    ramses::Effect* fromOutputEffect = createEffectFromOutputFiles();
    ASSERT_TRUE(fromOutputEffect != nullptr);

    EXPECT_EQ(ramses::StatusOK, fromOutputEffect->findAttributeInput("floatArrayInput", attributeInput));  // enabled by NOT providing define
}

TEST_F(AShaderConverter, reportsErrorWhenConvertingInvalidShader)
{
    const char* argv[] = { "program.exe",
        "-iv", "res/ramses-shader-tools-invalid-test.vertexshader",
        "-if", "res/ramses-shader-tools-test.fragmentshader",
        "-ic", "res/ramses-shader-tools-test_without_compiler_defines.config",
        "-os", "gles2.0",
        "-ov", OUTPUT_VERTEX_SHADER,
        "-of", OUTPUT_FRAGMENT_SHADER,
        "-ot", "client",
        "-oe", OUTPUT_HASH, nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_TRUE(arguments.loadArguments(argc, argv));
    EXPECT_FALSE(ShaderConverter::Convert(arguments));
    checkExistanceOfOutputFiles(false);
}

TEST_F(AShaderConverter, generateHashFileWithLowLevelHash)
{
    const char* argv[] = { "program.exe",
        "-iv", "res/ramses-shader-tools-test.vertexshader",
        "-if", "res/ramses-shader-tools-test.fragmentshader",
        "-ic", "res/ramses-shader-tools-test_without_compiler_defines.config",
        "-os", "gles2.0",
        "-ov", OUTPUT_VERTEX_SHADER,
        "-of", OUTPUT_FRAGMENT_SHADER,
        "-ot", "renderer",
        "-oe", OUTPUT_HASH, nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_TRUE(arguments.loadArguments(argc, argv));
    EXPECT_TRUE(ShaderConverter::Convert(arguments));
    checkExistanceOfOutputFiles(true);

    ramses_internal::String hashFileContent;
    FileUtils::ReadFileContentsToString(OUTPUT_HASH, hashFileContent);

    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-shader-tools-test.vertexshader");
    effectDesc.setFragmentShaderFromFile("res/ramses-shader-tools-test.fragmentshader");
    effectDesc.setUniformSemantic("matrix44fInput", ramses::EEffectUniformSemantic::ModelViewMatrix);
    effectDesc.setUniformSemantic("texture2dInput", ramses::EEffectUniformSemantic::TextTexture);
    effectDesc.setAttributeSemantic("vec2fArrayInput", ramses::EEffectAttributeSemantic::TextPositions);
    ramses::Effect* fromOutputEffect = scene.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
    ASSERT_TRUE(fromOutputEffect != nullptr);

    ramses_internal::ResourceContentHash llHash = fromOutputEffect->impl.getLowlevelResourceHash();
    EXPECT_EQ(fmt::format("0x{:016X}{:016X}\n", llHash.highPart, llHash.lowPart), hashFileContent.stdRef());
}

TEST_F(AShaderConverter, generateHashFileWithHighLevelHash)
{
    const char* argv[] = { "program.exe",
        "-iv", "res/ramses-shader-tools-test.vertexshader",
        "-if", "res/ramses-shader-tools-test.fragmentshader",
        "-ic", "res/ramses-shader-tools-test_without_compiler_defines.config",
        "-os", "gles2.0",
        "-ov", OUTPUT_VERTEX_SHADER,
        "-of", OUTPUT_FRAGMENT_SHADER,
        "-ot", "client",
        "-oe", OUTPUT_HASH, nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_TRUE(arguments.loadArguments(argc, argv));
    EXPECT_TRUE(ShaderConverter::Convert(arguments));
    checkExistanceOfOutputFiles(true);

    ramses_internal::String hashFileContent;
    FileUtils::ReadFileContentsToString(OUTPUT_HASH, hashFileContent);

    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-shader-tools-test.vertexshader");
    effectDesc.setFragmentShaderFromFile("res/ramses-shader-tools-test.fragmentshader");
    effectDesc.setUniformSemantic("matrix44fInput", ramses::EEffectUniformSemantic::ModelViewMatrix);
    effectDesc.setUniformSemantic("texture2dInput", ramses::EEffectUniformSemantic::TextTexture);
    effectDesc.setAttributeSemantic("vec2fArrayInput", ramses::EEffectAttributeSemantic::TextPositions);
    ramses::Effect* fromOutputEffect = scene.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
    ASSERT_TRUE(fromOutputEffect != nullptr);

    const auto effectId = fromOutputEffect->impl.getResourceId();
    EXPECT_EQ(fmt::format("0x{:016X}{:016X}\n", effectId.highPart, effectId.lowPart), hashFileContent.stdRef());
}

TEST_F(AShaderConverter, generateHashFileWithHighLevelHashWithCorrectEffectName)
{
    const char* argv[] = { "program.exe",
        "-iv", "res/ramses-shader-tools-test.vertexshader",
        "-if", "res/ramses-shader-tools-test.fragmentshader",
        "-ic", "res/ramses-shader-tools-test_without_compiler_defines.config",
        "-os", "gles2.0",
        "-ov", OUTPUT_VERTEX_SHADER,
        "-of", OUTPUT_FRAGMENT_SHADER,
        "-ot", "client",
        "-oe", OUTPUT_HASH,
        "-on", "otherName", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesShaderFromGLSLShaderArguments arguments;
    EXPECT_TRUE(arguments.loadArguments(argc, argv));
    EXPECT_TRUE(ShaderConverter::Convert(arguments));
    checkExistanceOfOutputFiles(true);

    ramses_internal::String hashFileContent;
    FileUtils::ReadFileContentsToString(OUTPUT_HASH, hashFileContent);

    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-shader-tools-test.vertexshader");
    effectDesc.setFragmentShaderFromFile("res/ramses-shader-tools-test.fragmentshader");
    effectDesc.setUniformSemantic("matrix44fInput", ramses::EEffectUniformSemantic::ModelViewMatrix);
    effectDesc.setUniformSemantic("texture2dInput", ramses::EEffectUniformSemantic::TextTexture);
    effectDesc.setAttributeSemantic("vec2fArrayInput", ramses::EEffectAttributeSemantic::TextPositions);
    ramses::Effect* fromOutputEffect = scene.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "otherName");
    ASSERT_TRUE(fromOutputEffect != nullptr);

    const auto effectId = fromOutputEffect->impl.getResourceId();
    EXPECT_EQ(fmt::format("0x{:016X}{:016X}\n", effectId.highPart, effectId.lowPart), hashFileContent.stdRef());
}
