//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EffectResourceCreator.h"
#include "gtest/gtest.h"

#include "FileUtils.h"
#include "RamsesEffectFromGLSLShaderArguments.h"

#include "ramses-client-api/Effect.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/ResourceFileDescription.h"
#include "ramses-framework-api/RamsesFramework.h"

#include "EffectImpl.h"
#include "EffectInputImpl.h"
#include "RamsesClientImpl.h"
#include "RamsesObjectTypeUtils.h"
#include "Utils/GtestHelper.h"

namespace
{
    const char* OUTPUT_RESOURCE_FILE = "res/ramses-shader-tools-out.effectresource";
    const char* OUTPUT_EFFECT_NAME = "testOutputEffectName";
    const char* OUTPUT_EFFECT_ID_NAME = "res/ramses-shader-tools-out.effecthash";
}

class AnEffectResourceCreator : public testing::Test
{
protected:
    AnEffectResourceCreator()
    : framework()
    , ramsesClient(*framework.createClient("ramses client"))
    {
        cleanupOutputFile();
    }

    void checkExistanceOfOutputFile(bool exists)
    {
        EXPECT_TRUE(exists == FileUtils::FileExists(OUTPUT_RESOURCE_FILE));
    }

    void loadEffectFromFile(const char* filePath, ramses::Effect*& effectRead)
    {
        ramses::ResourceFileDescription resourceFileDesc(filePath);
        ASSERT_TRUE(ramses::StatusOK == ramsesClient.loadResources(resourceFileDesc));

        const ramses::RamsesObjectVector resources = ramsesClient.impl.getListOfResourceObjects();
        ASSERT_EQ(1u, resources.size());

        ramses::RamsesObject* ramsesObject = resources[0];
        ASSERT_TRUE(nullptr != ramsesObject);

        effectRead = &ramses::RamsesObjectTypeUtils::ConvertTo<ramses::Effect>(*ramsesObject);
        ASSERT_TRUE(nullptr != effectRead);
    }

    ramses::RamsesFramework framework;
    ramses::RamsesClient& ramsesClient;

private:
    void cleanupOutputFile()
    {
        FileUtils::RemoveFileIfExist(OUTPUT_RESOURCE_FILE);
        EXPECT_FALSE(FileUtils::FileExists(OUTPUT_RESOURCE_FILE));
        FileUtils::RemoveFileIfExist(OUTPUT_EFFECT_ID_NAME);
        EXPECT_FALSE(FileUtils::FileExists(OUTPUT_EFFECT_ID_NAME));
    }
};

TEST_F(AnEffectResourceCreator, canCreateEffectResourceFromGlslShaders)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-or", OUTPUT_RESOURCE_FILE, "-on", "testOutputEffectName", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesEffectFromGLSLShaderArguments arguments;
    EXPECT_TRUE(arguments.loadArguments(argc, argv));
    EXPECT_TRUE(EffectResourceCreator::Create(arguments));
    checkExistanceOfOutputFile(true);
    EXPECT_FALSE(FileUtils::FileExists(OUTPUT_EFFECT_ID_NAME));

    ramses::Effect* effectRead = nullptr;
    loadEffectFromFile(OUTPUT_RESOURCE_FILE, effectRead);
    ASSERT_TRUE(effectRead != nullptr);
    EXPECT_STREQ(effectRead->getName(), OUTPUT_EFFECT_NAME);

    ramses::EffectInputImpl matrix44fInput;
    EXPECT_TRUE(ramses::StatusOK == effectRead->impl.findUniformInput("matrix44fInput", matrix44fInput));
    EXPECT_TRUE(ramses::EEffectUniformSemantic_ModelViewMatrix == matrix44fInput.getUniformSemantics());

    ramses::EffectInputImpl texture2dInput;
    EXPECT_TRUE(ramses::StatusOK == effectRead->impl.findUniformInput("texture2dInput", texture2dInput));
    EXPECT_TRUE(ramses::EEffectUniformSemantic_TextTexture == texture2dInput.getUniformSemantics());

    ramses::EffectInputImpl vec2fArrayInput;
    EXPECT_TRUE(ramses::StatusOK == effectRead->impl.findAttributeInput("vec2fArrayInput", vec2fArrayInput));
    EXPECT_TRUE(ramses::EEffectAttributeSemantic_TextPositions == vec2fArrayInput.getAttributeSemantics());
}

TEST_F(AnEffectResourceCreator, canCreateEffectWithCorrectNameWhenEffectNameIsNotProvided)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-or", OUTPUT_RESOURCE_FILE, nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesEffectFromGLSLShaderArguments arguments;
    EXPECT_TRUE(arguments.loadArguments(argc, argv));
    EXPECT_TRUE(EffectResourceCreator::Create(arguments));
    checkExistanceOfOutputFile(true);

    ramses::Effect* effectRead = nullptr;
    loadEffectFromFile(OUTPUT_RESOURCE_FILE, effectRead);
    ASSERT_NOT_NULL(effectRead);
    EXPECT_STREQ(effectRead->getName(), "ramses-shader-tools-test.vertexshader_ramses-shader-tools-test.fragmentshader");
}

TEST_F(AnEffectResourceCreator, reportsErrorWhenCreatingFromInvalidShader)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-invalid-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test_without_compiler_defines.config", "-or", OUTPUT_RESOURCE_FILE, nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesEffectFromGLSLShaderArguments arguments;
    EXPECT_TRUE(arguments.loadArguments(argc, argv));
    EXPECT_FALSE(EffectResourceCreator::Create(arguments));
    checkExistanceOfOutputFile(false);
}

TEST_F(AnEffectResourceCreator, canWriteEffectIdWithHighLevelResourceHash)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-or", OUTPUT_RESOURCE_FILE, "-on", "testOutputEffectName", "-oe", OUTPUT_EFFECT_ID_NAME, "-ot", "client", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesEffectFromGLSLShaderArguments arguments;
    EXPECT_TRUE(arguments.loadArguments(argc, argv));
    EXPECT_TRUE(EffectResourceCreator::Create(arguments));
    checkExistanceOfOutputFile(true);
    EXPECT_TRUE(FileUtils::FileExists(OUTPUT_EFFECT_ID_NAME));

    ramses::Effect* effectRead = nullptr;
    loadEffectFromFile(OUTPUT_RESOURCE_FILE, effectRead);

    ramses_internal::String hashFileContent;
    FileUtils::ReadFileContentsToString(OUTPUT_EFFECT_ID_NAME, hashFileContent);

    ramses_internal::StringOutputStream hlHashStream;
    const auto effectId = effectRead->impl.getResourceId();
    const ramses_internal::ResourceContentHash effectIdAsResourceHash = { effectId.lowPart, effectId.highPart };
    hlHashStream << effectIdAsResourceHash << "\n";

    EXPECT_EQ(ramses_internal::String(hlHashStream.c_str()), hashFileContent);
}

TEST_F(AnEffectResourceCreator, canWriteEffectIdWithLowLevelResourceHash)
{
    const char* argv[] = { "program.exe", "-iv", "res/ramses-shader-tools-test.vertexshader", "-if", "res/ramses-shader-tools-test.fragmentshader", "-ic", "res/ramses-shader-tools-test.config", "-or", OUTPUT_RESOURCE_FILE, "-on", "testOutputEffectName", "-oe", OUTPUT_EFFECT_ID_NAME, "-ot", "renderer", nullptr };
    int argc = sizeof(argv) / sizeof(char*) - 1;

    RamsesEffectFromGLSLShaderArguments arguments;
    EXPECT_TRUE(arguments.loadArguments(argc, argv));
    EXPECT_TRUE(EffectResourceCreator::Create(arguments));
    checkExistanceOfOutputFile(true);
    EXPECT_TRUE(FileUtils::FileExists(OUTPUT_EFFECT_ID_NAME));

    ramses::Effect* effectRead = nullptr;
    loadEffectFromFile(OUTPUT_RESOURCE_FILE, effectRead);

    ramses_internal::String hashFileContent;
    FileUtils::ReadFileContentsToString(OUTPUT_EFFECT_ID_NAME, hashFileContent);

    ramses_internal::StringOutputStream llHashStream;
    llHashStream << effectRead->impl.getLowlevelResourceHash() << "\n";

    EXPECT_EQ(ramses_internal::String(llHashStream.c_str()), hashFileContent);
}
