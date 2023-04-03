//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"

#include "RamsesTestUtils.h"
#include "WithTempDirectory.h"
#include "FeatureLevelTestValues.h"
#include "PropertyLinkTestUtils.h"

#include "ramses-logic/RamsesLogicVersion.h"
#include "ramses-logic/Property.h"

#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-framework-api/RamsesVersion.h"

#include "internals/ApiObjects.h"
#include "internals/FileUtils.h"

#include "generated/LogicEngineGen.h"
#include "fmt/format.h"

#include <fstream>

namespace rlogic::internal
{
    class ALogicEngine_Compatibility : public ALogicEngineBase, public ::testing::TestWithParam<EFeatureLevel>
    {
    public:
        ALogicEngine_Compatibility() : ALogicEngineBase{ GetParam() }
        {
        }

    protected:
        static const char* GetFileIdentifier(EFeatureLevel featureLevel = GetParam())
        {
            return featureLevel == EFeatureLevel_01 ? rlogic_serialization::LogicEngineIdentifier() : "rl02";
        }

        void createFlatLogicEngineData(
            ramses::RamsesVersion ramsesVersion,
            rlogic::RamsesLogicVersion logicVersion,
            const char* fileId = GetFileIdentifier(),
            EFeatureLevel featureLevel = GetParam())
        {
            ApiObjects emptyApiObjects{ featureLevel };

            auto logicEngine = rlogic_serialization::CreateLogicEngine(
                m_fbBuilder,
                rlogic_serialization::CreateVersion(m_fbBuilder,
                    ramsesVersion.major, ramsesVersion.minor, ramsesVersion.patch, m_fbBuilder.CreateString(ramsesVersion.string)),
                rlogic_serialization::CreateVersion(m_fbBuilder,
                    logicVersion.major, logicVersion.minor, logicVersion.patch, m_fbBuilder.CreateString(logicVersion.string)),
                ApiObjects::Serialize(emptyApiObjects, m_fbBuilder, ELuaSavingMode::ByteCodeOnly),
                0,
                featureLevel
            );

            m_fbBuilder.Finish(logicEngine, fileId);
        }

        static ramses::RamsesVersion FakeRamsesVersion()
        {
            ramses::RamsesVersion version{
                "10.20.900-suffix",
                10,
                20,
                900
            };
            return version;
        }

        flatbuffers::FlatBufferBuilder m_fbBuilder;
        WithTempDirectory m_tempDir;
    };

    INSTANTIATE_TEST_SUITE_P(
        ALogicEngine_CompatibilityTests,
        ALogicEngine_Compatibility,
        GetFeatureLevelTestValues());

    TEST_P(ALogicEngine_Compatibility, CreatesLogicEngineWithFeatureLevel)
    {
        LogicEngine logicEngine{ GetParam() };
        EXPECT_EQ(GetParam(), logicEngine.getFeatureLevel());
    }

    TEST_P(ALogicEngine_Compatibility, FallsBackToFeatureLevel01IfUnknownFeatureLevelRequested)
    {
        LogicEngine logicEngine{ static_cast<EFeatureLevel>(999) };
        EXPECT_EQ(EFeatureLevel_01, logicEngine.getFeatureLevel());
    }

    TEST_P(ALogicEngine_Compatibility, ProducesErrorIfDeserilizedFromFileReferencingIncompatibleRamsesVersion)
    {
        createFlatLogicEngineData(FakeRamsesVersion(), GetRamsesLogicVersion());

        ASSERT_TRUE(FileUtils::SaveBinary("wrong_ramses_version.bin", m_fbBuilder.GetBufferPointer(), m_fbBuilder.GetSize()));

        EXPECT_FALSE(m_logicEngine.loadFromFile("wrong_ramses_version.bin"));
        auto errors = m_logicEngine.getErrors();
        ASSERT_EQ(1u, errors.size());
        EXPECT_THAT(errors[0].message, ::testing::HasSubstr("Version mismatch while loading file 'wrong_ramses_version.bin' (size: "));
        EXPECT_THAT(errors[0].message, ::testing::HasSubstr(fmt::format("Expected Ramses version {}.x.x but found 10.20.900-suffix", ramses::GetRamsesVersion().major)));

        //Also test with buffer version of the API
        EXPECT_FALSE(m_logicEngine.loadFromBuffer(m_fbBuilder.GetBufferPointer(), m_fbBuilder.GetSize()));
        errors = m_logicEngine.getErrors();
        ASSERT_EQ(1u, errors.size());
        EXPECT_THAT(errors[0].message, ::testing::HasSubstr("Version mismatch while loading data buffer"));
        EXPECT_THAT(errors[0].message, ::testing::HasSubstr(fmt::format("Expected Ramses version {}.x.x but found 10.20.900-suffix", ramses::GetRamsesVersion().major)));
    }

    TEST_P(ALogicEngine_Compatibility, ProducesErrorIfDeserilizedFromDifferentTypeOfFile)
    {
        const char* badFileIdentifier = "xyWW";
        createFlatLogicEngineData(ramses::GetRamsesVersion(), GetRamsesLogicVersion(), badFileIdentifier);

        ASSERT_TRUE(FileUtils::SaveBinary("temp.bin", m_fbBuilder.GetBufferPointer(), m_fbBuilder.GetSize()));

        EXPECT_FALSE(m_logicEngine.loadFromFile("temp.bin"));
        auto errors = m_logicEngine.getErrors();
        ASSERT_EQ(1u, errors.size());
        EXPECT_THAT(errors[0].message, ::testing::HasSubstr(fmt::format("Tried loading a binary data which doesn't store Ramses Logic content! Expected file bytes 4-5 to be 'rl', but found 'xy' instead")));
    }

    TEST_P(ALogicEngine_Compatibility, ProducesErrorIfDeserilizedFromIncompatibleFileVersion)
    {
        // Format was changed
        const char* versionFromFuture = "rl99";
        createFlatLogicEngineData(ramses::GetRamsesVersion(), GetRamsesLogicVersion(), versionFromFuture);

        ASSERT_TRUE(FileUtils::SaveBinary("temp.bin", m_fbBuilder.GetBufferPointer(), m_fbBuilder.GetSize()));

        EXPECT_FALSE(m_logicEngine.loadFromFile("temp.bin"));
        auto errors = m_logicEngine.getErrors();
        ASSERT_EQ(1u, errors.size());
        if (GetParam() == EFeatureLevel_01)
        {
            EXPECT_THAT(errors[0].message, ::testing::HasSubstr(fmt::format("Version mismatch while loading binary data! Expected version '01', but found '99'")));
        }
        else
        {
            EXPECT_THAT(errors[0].message, ::testing::HasSubstr(fmt::format("Version mismatch while loading binary data! Expected version '02', but found '99'")));
        }
    }

    TEST_P(ALogicEngine_Compatibility, CanDeserializeSameFeatureLevelVersion)
    {
        const EFeatureLevel featureLevel = GetParam();
        createFlatLogicEngineData(ramses::GetRamsesVersion(), GetRamsesLogicVersion(), GetFileIdentifier(), featureLevel);
        ASSERT_TRUE(FileUtils::SaveBinary("temp.bin", m_fbBuilder.GetBufferPointer(), m_fbBuilder.GetSize()));

        LogicEngine logicEngine(featureLevel);
        EXPECT_TRUE(logicEngine.loadFromFile("temp.bin"));
        EXPECT_TRUE(logicEngine.getErrors().empty());
    }

    TEST_P(ALogicEngine_Compatibility, EarlyErrorIfDeserializedFromIncompatibleFeatureLevelVersionInFileIdentifier)
    {
        // this test is not parametrized
        if (GetParam() != EFeatureLevel_01)
            GTEST_SKIP();

        // mismatch in file identifier can happen only if either file or engine are feature level 01,
        // from 02 onwards feature level is checked using standard flatbuffer entry, tested in tests below
        const std::vector<std::pair<EFeatureLevel, EFeatureLevel>> combinations{
            { EFeatureLevel_01, EFeatureLevel_02 },
            { EFeatureLevel_01, EFeatureLevel_03 },
            { EFeatureLevel_01, EFeatureLevel_04 },
            { EFeatureLevel_01, EFeatureLevel_05 },
            { EFeatureLevel_02, EFeatureLevel_01 },
            { EFeatureLevel_03, EFeatureLevel_01 },
            { EFeatureLevel_04, EFeatureLevel_01 },
            { EFeatureLevel_05, EFeatureLevel_01 }
        };

        for (const auto& comb : combinations)
        {
            const EFeatureLevel fileFeatureLevel = comb.first;
            const EFeatureLevel engineFeatureLevel = comb.second;

            createFlatLogicEngineData(ramses::GetRamsesVersion(), GetRamsesLogicVersion(), GetFileIdentifier(fileFeatureLevel), fileFeatureLevel);
            ASSERT_TRUE(FileUtils::SaveBinary("temp.bin", m_fbBuilder.GetBufferPointer(), m_fbBuilder.GetSize()));

            LogicEngine logicEngine{ engineFeatureLevel };
            EXPECT_FALSE(logicEngine.loadFromFile("temp.bin"));
            const auto& errors = logicEngine.getErrors();
            ASSERT_EQ(1u, errors.size());
            EXPECT_THAT(errors[0].message, ::testing::HasSubstr(fmt::format("file 'temp.bin' (size:")));
            EXPECT_THAT(errors[0].message, ::testing::HasSubstr(fmt::format("Feature level mismatch! ")));
            if (fileFeatureLevel == EFeatureLevel_01)
            {
                EXPECT_THAT(errors[0].message, ::testing::HasSubstr(fmt::format("Loaded file with feature level 1 but LogicEngine was instantiated with feature level {}", engineFeatureLevel)));
            }
            else
            {
                EXPECT_THAT(errors[0].message, ::testing::HasSubstr(fmt::format("Loaded file with feature level >=2 but LogicEngine was instantiated with feature level {}", engineFeatureLevel)));
            }
        }
    }

    TEST_P(ALogicEngine_Compatibility, ProducesErrorIfDeserializedFromIncompatibleFeatureLevelVersion)
    {
        // this test is not parametrized
        if (GetParam() != EFeatureLevel_01)
            GTEST_SKIP();

        // test all mismatching combinations
        for (EFeatureLevel fileFeatureLevel : AllFeatureLevels)
        {
            for (EFeatureLevel engineFeatureLevel : AllFeatureLevels)
            {
                if (fileFeatureLevel == engineFeatureLevel)
                    continue;

                // note - use file identifier matching engine otherwise load fails already when checking file identifier (tested above)
                createFlatLogicEngineData(ramses::GetRamsesVersion(), GetRamsesLogicVersion(), GetFileIdentifier(engineFeatureLevel), fileFeatureLevel);
                ASSERT_TRUE(FileUtils::SaveBinary("temp.bin", m_fbBuilder.GetBufferPointer(), m_fbBuilder.GetSize()));

                LogicEngine logicEngine{ engineFeatureLevel };
                EXPECT_FALSE(logicEngine.loadFromFile("temp.bin"));
                const auto& errors = logicEngine.getErrors();
                ASSERT_EQ(1u, errors.size());
                EXPECT_THAT(errors[0].message, ::testing::HasSubstr(fmt::format("Feature level mismatch while loading file 'temp.bin' (size:")));
                EXPECT_THAT(errors[0].message, ::testing::HasSubstr(fmt::format("Loaded file with feature level {} but LogicEngine was instantiated with feature level {}", fileFeatureLevel, engineFeatureLevel)));
            }
        }
    }

    TEST_P(ALogicEngine_Compatibility, CanParseFeatureLevelFromFile)
    {
        const EFeatureLevel featureLevel = GetParam();
        createFlatLogicEngineData(ramses::GetRamsesVersion(), GetRamsesLogicVersion(), GetFileIdentifier(), featureLevel);
        ASSERT_TRUE(FileUtils::SaveBinary("temp.bin", m_fbBuilder.GetBufferPointer(), m_fbBuilder.GetSize()));

        EFeatureLevel detectedFeatureLevel = EFeatureLevel_01;
        ASSERT_TRUE(LogicEngine::GetFeatureLevelFromFile("temp.bin", detectedFeatureLevel));
        EXPECT_EQ(featureLevel, detectedFeatureLevel);
    }

    TEST_P(ALogicEngine_Compatibility, FailsToParseFeatureLevelFromNotExistingFile)
    {
        EFeatureLevel detectedFeatureLevel = EFeatureLevel_01;
        EXPECT_FALSE(LogicEngine::GetFeatureLevelFromFile("doesntexist", detectedFeatureLevel));
    }

    TEST_P(ALogicEngine_Compatibility, FailsToParseFeatureLevelFromCorruptedFile)
    {
        const std::string invalidData{"invaliddata"};
        ASSERT_TRUE(FileUtils::SaveBinary("temp.bin", invalidData.data(), invalidData.size()));

        EFeatureLevel detectedFeatureLevel = EFeatureLevel_01;
        EXPECT_FALSE(LogicEngine::GetFeatureLevelFromFile("temp.bin", detectedFeatureLevel));
    }

    TEST_P(ALogicEngine_Compatibility, FailsToParseFeatureLevelFromValidFileButUnknownFeatureLevel)
    {
        createFlatLogicEngineData(ramses::GetRamsesVersion(), GetRamsesLogicVersion(), GetFileIdentifier(), static_cast<EFeatureLevel>(999));
        ASSERT_TRUE(FileUtils::SaveBinary("temp.bin", m_fbBuilder.GetBufferPointer(), m_fbBuilder.GetSize()));

        EFeatureLevel detectedFeatureLevel = EFeatureLevel_01;
        EXPECT_FALSE(LogicEngine::GetFeatureLevelFromFile("temp.bin", detectedFeatureLevel));
    }

    TEST_P(ALogicEngine_Compatibility, CanParseFeatureLevelFromBuffer)
    {
        const EFeatureLevel featureLevel = GetParam();
        createFlatLogicEngineData(ramses::GetRamsesVersion(), GetRamsesLogicVersion(), GetFileIdentifier(), featureLevel);
        EFeatureLevel detectedFeatureLevel = EFeatureLevel_01;
        ASSERT_TRUE(LogicEngine::GetFeatureLevelFromBuffer("temp.bin", m_fbBuilder.GetBufferPointer(), m_fbBuilder.GetSize(), detectedFeatureLevel));
        EXPECT_EQ(featureLevel, detectedFeatureLevel);
    }

    // These tests will always break on incompatible file format changes.
    class ALogicEngine_Binary_Compatibility : public ::testing::Test
    {
    protected:
        static void checkBaseContents(LogicEngine& logicEngine, ramses::Scene& ramsesScene)
        {
            ASSERT_NE(nullptr, logicEngine.findByName<LuaModule>("nestedModuleMath"));
            ASSERT_NE(nullptr, logicEngine.findByName<LuaModule>("moduleMath"));
            ASSERT_NE(nullptr, logicEngine.findByName<LuaModule>("moduleTypes"));
            const auto script1 = logicEngine.findByName<LuaScript>("script1");
            ASSERT_NE(nullptr, script1);
            EXPECT_NE(nullptr, script1->getInputs()->getChild("intInput"));
            EXPECT_NE(nullptr, script1->getInputs()->getChild("int64Input"));
            EXPECT_NE(nullptr, script1->getInputs()->getChild("vec2iInput"));
            EXPECT_NE(nullptr, script1->getInputs()->getChild("vec3iInput"));
            EXPECT_NE(nullptr, script1->getInputs()->getChild("vec4iInput"));
            EXPECT_NE(nullptr, script1->getInputs()->getChild("floatInput"));
            EXPECT_NE(nullptr, script1->getInputs()->getChild("vec2fInput"));
            EXPECT_NE(nullptr, script1->getInputs()->getChild("vec3fInput"));
            EXPECT_NE(nullptr, script1->getInputs()->getChild("vec4fInput"));
            EXPECT_NE(nullptr, script1->getInputs()->getChild("boolInput"));
            EXPECT_NE(nullptr, script1->getInputs()->getChild("stringInput"));
            EXPECT_NE(nullptr, script1->getInputs()->getChild("structInput"));
            EXPECT_NE(nullptr, script1->getInputs()->getChild("arrayInput"));
            EXPECT_NE(nullptr, script1->getOutputs()->getChild("floatOutput"));
            EXPECT_NE(nullptr, script1->getOutputs()->getChild("nodeTranslation"));
            EXPECT_NE(nullptr, script1->getInputs()->getChild("floatInput"));
            const auto script2 = logicEngine.findByName<LuaScript>("script2");
            ASSERT_NE(nullptr, script2);
            EXPECT_NE(nullptr, script2->getInputs()->getChild("floatInput"));
            EXPECT_NE(nullptr, script2->getOutputs()->getChild("cameraViewport")->getChild("offsetX"));
            EXPECT_NE(nullptr, script2->getOutputs()->getChild("cameraViewport")->getChild("offsetY"));
            EXPECT_NE(nullptr, script2->getOutputs()->getChild("cameraViewport")->getChild("width"));
            EXPECT_NE(nullptr, script2->getOutputs()->getChild("cameraViewport")->getChild("height"));
            EXPECT_NE(nullptr, script2->getOutputs()->getChild("floatUniform"));
            const auto animNode = logicEngine.findByName<AnimationNode>("animNode");
            ASSERT_NE(nullptr, animNode);
            EXPECT_EQ(1u, animNode->getInputs()->getChildCount());
            ASSERT_EQ(2u, animNode->getOutputs()->getChildCount());
            EXPECT_NE(nullptr, animNode->getOutputs()->getChild("channel"));
            ASSERT_NE(nullptr, logicEngine.findByName<AnimationNode>("animNodeWithDataProperties"));
            EXPECT_EQ(2u, logicEngine.findByName<AnimationNode>("animNodeWithDataProperties")->getInputs()->getChildCount());
            EXPECT_NE(nullptr, logicEngine.findByName<TimerNode>("timerNode"));

            const auto nodeBinding = logicEngine.findByName<RamsesNodeBinding>("nodebinding");
            ASSERT_NE(nullptr, nodeBinding);
            const auto cameraBinding = logicEngine.findByName<RamsesCameraBinding>("camerabinding");
            ASSERT_NE(nullptr, cameraBinding);
            const auto appearanceBinding = logicEngine.findByName<RamsesAppearanceBinding>("appearancebinding");
            ASSERT_NE(nullptr, appearanceBinding);
            EXPECT_NE(nullptr, logicEngine.findByName<DataArray>("dataarray"));

            std::vector<PropertyLink> expectedLinks;

            // VersionBreak There was a mess up with the test content, this is a workaround for it until next breaking change on logic engine
            if(logicEngine.getFeatureLevel() == EFeatureLevel_01)
            {
                script1->getInputs()->getChild("floatInput")->set<float>(42.5f);
            }
            else
            {
                const auto intf = logicEngine.findByName<LuaInterface>("intf");
                ASSERT_NE(nullptr, intf);
                intf->getInputs()->getChild("struct")->getChild("floatInput")->set<float>(42.5f);
                expectedLinks.push_back({ intf->getOutputs()->getChild("struct")->getChild("floatInput"), script1->getInputs()->getChild("floatInput"), false });
            }

            if (logicEngine.getFeatureLevel() >= EFeatureLevel_02)
            {
                expectedLinks.push_back({ script1->getOutputs()->getChild("boolOutput"), nodeBinding->getInputs()->getChild("enabled"), false });
            }

            expectedLinks.push_back({ script1->getOutputs()->getChild("floatOutput"), script2->getInputs()->getChild("floatInput"), false });
            expectedLinks.push_back({ script1->getOutputs()->getChild("nodeTranslation"), nodeBinding->getInputs()->getChild("translation"), false });
            expectedLinks.push_back({ script2->getOutputs()->getChild("cameraViewport")->getChild("offsetX"), cameraBinding->getInputs()->getChild("viewport")->getChild("offsetX"), false });
            expectedLinks.push_back({ script2->getOutputs()->getChild("cameraViewport")->getChild("offsetY"), cameraBinding->getInputs()->getChild("viewport")->getChild("offsetY"), false });
            expectedLinks.push_back({ script2->getOutputs()->getChild("cameraViewport")->getChild("width"), cameraBinding->getInputs()->getChild("viewport")->getChild("width"), false });
            expectedLinks.push_back({ script2->getOutputs()->getChild("cameraViewport")->getChild("height"), cameraBinding->getInputs()->getChild("viewport")->getChild("height"), false });
            expectedLinks.push_back({ script2->getOutputs()->getChild("floatUniform"), appearanceBinding->getInputs()->getChild("floatUniform"), false });
            expectedLinks.push_back({ animNode->getOutputs()->getChild("channel"), appearanceBinding->getInputs()->getChild("animatedFloatUniform"), false });

            PropertyLinkTestUtils::ExpectLinks(logicEngine, expectedLinks);

            EXPECT_TRUE(logicEngine.update());

            // Values on Ramses are updated according to expectations
            vec3f translation;
            auto node = ramses::RamsesUtils::TryConvert<ramses::Node>(*ramsesScene.findObjectByName("test node"));
            auto camera = ramses::RamsesUtils::TryConvert<ramses::OrthographicCamera>(*ramsesScene.findObjectByName("test camera"));
            node->getTranslation(translation[0], translation[1], translation[2]);
            EXPECT_THAT(translation, ::testing::ElementsAre(42.5f, 2.f, 3.f));

            EXPECT_EQ(camera->getViewportX(), 45);
            EXPECT_EQ(camera->getViewportY(), 47);
            EXPECT_EQ(camera->getViewportWidth(), 143u);
            EXPECT_EQ(camera->getViewportHeight(), 243u);

            // Animation node is linked and can be animated
            EXPECT_FLOAT_EQ(2.f, *animNode->getOutputs()->getChild("duration")->get<float>());
            animNode->getInputs()->getChild("progress")->set(0.75f);
            EXPECT_TRUE(logicEngine.update());

            ramses::UniformInput uniform;
            auto appearance = ramses::RamsesUtils::TryConvert<ramses::Appearance>(*ramsesScene.findObjectByName("test appearance"));
            appearance->getEffect().getUniformInput(1, uniform);
            float floatValue = 0.f;
            appearance->getInputValue(uniform, floatValue);
            EXPECT_FLOAT_EQ(1.5f, floatValue);

            EXPECT_EQ(957, *logicEngine.findByName<LuaScript>("script2")->getOutputs()->getChild("nestedModulesResult")->get<int32_t>());
        }

        static void expectFeatureLevel02Content(const LogicEngine& logicEngine, ramses::Scene& ramsesScene)
        {
            const auto nodeBinding = logicEngine.findByName<RamsesNodeBinding>("nodebinding");
            ASSERT_TRUE(nodeBinding);
            EXPECT_TRUE(logicEngine.isLinked(*nodeBinding));
            EXPECT_NE(nullptr, nodeBinding->getInputs()->getChild("enabled"));

            EXPECT_TRUE(logicEngine.findByName<RamsesRenderPassBinding>("renderpassbinding"));
            EXPECT_TRUE(logicEngine.findByName<AnchorPoint>("anchorpoint"));

            const auto cameraBindingPersp = logicEngine.findByName<RamsesCameraBinding>("camerabindingPersp");
            const auto cameraBindingPerspWithFrustumPlanes = logicEngine.findByName<RamsesCameraBinding>("camerabindingPerspWithFrustumPlanes");
            ASSERT_TRUE(cameraBindingPersp && cameraBindingPerspWithFrustumPlanes);
            EXPECT_EQ(4u, cameraBindingPersp->getInputs()->getChild("frustum")->getChildCount());
            EXPECT_EQ(6u, cameraBindingPerspWithFrustumPlanes->getInputs()->getChild("frustum")->getChildCount());

            // test that linked value from script propagated to ramses scene
            const auto node = ramses::RamsesUtils::TryConvert<ramses::Node>(*ramsesScene.findObjectByName("test node"));
            ASSERT_TRUE(node);
            EXPECT_EQ(ramses::EVisibilityMode::Off, node->getVisibility());
        }

        static void expectFeatureLevel02ContentNotPresent(const LogicEngine& logicEngine)
        {
            const auto nodeBinding = logicEngine.findByName<RamsesNodeBinding>("nodebinding");
            ASSERT_TRUE(nodeBinding);
            EXPECT_EQ(nullptr, nodeBinding->getInputs()->getChild("enabled"));
            EXPECT_FALSE(logicEngine.findByName<LogicObject>("renderpassbinding"));
            EXPECT_FALSE(logicEngine.findByName<LogicObject>("anchorpoint"));
            EXPECT_FALSE(logicEngine.findByName<LogicObject>("dataarrayOfArrays"));
        }

        static void expectFeatureLevel03Content(const LogicEngine& logicEngine)
        {
            EXPECT_TRUE(logicEngine.findByName<RamsesRenderGroupBinding>("rendergroupbinding"));
        }

        static void expectFeatureLevel03ContentNotPresent(const LogicEngine& logicEngine)
        {
            EXPECT_FALSE(logicEngine.findByName<LogicObject>("rendergroupbinding"));
            EXPECT_FALSE(logicEngine.findByName<LogicObject>("dataarrayOfArrays"));
        }

        static void expectFeatureLevel04Content(const LogicEngine& logicEngine)
        {
            EXPECT_TRUE(logicEngine.findByName<SkinBinding>("skin"));
            const auto dataArray = logicEngine.findByName<DataArray>("dataarrayOfArrays");
            ASSERT_TRUE(dataArray);
            EXPECT_EQ(EPropertyType::Array, dataArray->getDataType());
            EXPECT_EQ(2u, dataArray->getNumElements());
            const auto data = dataArray->getData<std::vector<float>>();
            ASSERT_TRUE(data);
            const std::vector<std::vector<float>> expectedData{ { 1.f, 2.f, 3.f, 4.f, 5.f }, { 6.f, 7.f, 8.f, 9.f, 10.f } };
            EXPECT_EQ(expectedData, *data);
        }

        static void expectFeatureLevel04ContentNotPresent(const LogicEngine& logicEngine)
        {
            EXPECT_FALSE(logicEngine.findByName<LogicObject>("skin"));
        }

        static void expectFeatureLevel05Content(const LogicEngine& logicEngine)
        {
            EXPECT_TRUE(logicEngine.findByName<RamsesMeshNodeBinding>("meshnodebinding"));
        }

        static void expectFeatureLevel05ContentNotPresent(const LogicEngine& logicEngine)
        {
            EXPECT_FALSE(logicEngine.findByName<LogicObject>("meshnodebinding"));
        }

        static void checkContents(LogicEngine& logicEngine, ramses::Scene& scene)
        {
            // check for content expected to exist
            // higher feature level always contains content supported by lower level
            switch (logicEngine.getFeatureLevel())
            {
            case EFeatureLevel_05:
                expectFeatureLevel05Content(logicEngine);
                [[fallthrough]];
            case EFeatureLevel_04:
                expectFeatureLevel04Content(logicEngine);
                [[fallthrough]];
            case EFeatureLevel_03:
                expectFeatureLevel03Content(logicEngine);
                [[fallthrough]];
            case EFeatureLevel_02:
                expectFeatureLevel02Content(logicEngine, scene);
                [[fallthrough]];
            case EFeatureLevel_01:
                checkBaseContents(logicEngine, scene);
                break;
            }

            // check for content expected to not exist
            // lower feature level never contains content supported only in higher level
            switch (logicEngine.getFeatureLevel())
            {
            case EFeatureLevel_01:
                expectFeatureLevel02ContentNotPresent(logicEngine);
                [[fallthrough]];
            case EFeatureLevel_02:
                expectFeatureLevel03ContentNotPresent(logicEngine);
                [[fallthrough]];
            case EFeatureLevel_03:
                expectFeatureLevel04ContentNotPresent(logicEngine);
                [[fallthrough]];
            case EFeatureLevel_04:
                expectFeatureLevel05ContentNotPresent(logicEngine);
                [[fallthrough]];
            case EFeatureLevel_05:
                break;
            }
        }

        static void saveAndReloadAndCheckContents(LogicEngine& logicEngine, ramses::Scene& scene)
        {
            WithTempDirectory tempDir;

            rlogic::SaveFileConfig noValidationConfig;
            noValidationConfig.setValidationEnabled(false);
            logicEngine.saveToFile("temp.rlogic", noValidationConfig);

            LogicEngine logicEngineAnother{ logicEngine.getFeatureLevel() };
            ASSERT_TRUE(logicEngineAnother.loadFromFile("temp.rlogic", &scene));
            EXPECT_TRUE(logicEngineAnother.update());

            checkContents(logicEngineAnother, scene);
        }

        ramses::Scene* loadRamsesScene(EFeatureLevel featureLevel) {
            switch(featureLevel)
            {
            case EFeatureLevel_01:
                return &m_ramses.loadSceneFromFile("res/unittests/testScene_01.ramses");
            case EFeatureLevel_02:
                return &m_ramses.loadSceneFromFile("res/unittests/testScene_02.ramses");
            case EFeatureLevel_03:
                return &m_ramses.loadSceneFromFile("res/unittests/testScene_03.ramses");
            case EFeatureLevel_04:
                return &m_ramses.loadSceneFromFile("res/unittests/testScene_04.ramses");
            case EFeatureLevel_05:
                return &m_ramses.loadSceneFromFile("res/unittests/testScene_05.ramses");
            }
            return nullptr;
        }

        RamsesTestSetup m_ramses;
    };

    // TODO vaclav - following tests use files generated in rlogic when using ramses27 as external lib, r27 did not have a feature level token
    // (introduced in r28) so these files are not possible to load anymore, they would have to be re-generated.
    // However feature levels in rlogic will be flattened so these tests will probably become obsolete - TBD as part of another story
    TEST_F(ALogicEngine_Binary_Compatibility, DISABLED_CanLoadAndUpdateABinaryFileExportedWithLastCompatibleVersionOfEngine_FeatureLevel01)
    {
        EFeatureLevel featureLevel = EFeatureLevel_02;
        EXPECT_TRUE(LogicEngine::GetFeatureLevelFromFile("res/unittests/testLogic_01.rlogic", featureLevel));
        EXPECT_EQ(EFeatureLevel_01, featureLevel);

        ramses::Scene* scene = loadRamsesScene(EFeatureLevel_01);
        ASSERT_TRUE(scene);
        LogicEngine logicEngine;
        ASSERT_TRUE(logicEngine.loadFromFile("res/unittests/testLogic_01.rlogic", scene));
        EXPECT_TRUE(logicEngine.update());

        checkContents(logicEngine, *scene);
        saveAndReloadAndCheckContents(logicEngine, *scene);
    }

    TEST_F(ALogicEngine_Binary_Compatibility, DISABLED_CanLoadAndUpdateABinaryFileExportedWithLastCompatibleVersionOfEngine_FeatureLevel02)
    {
        EFeatureLevel featureLevel = EFeatureLevel_01;
        EXPECT_TRUE(LogicEngine::GetFeatureLevelFromFile("res/unittests/testLogic_02.rlogic", featureLevel));
        EXPECT_EQ(EFeatureLevel_02, featureLevel);

        ramses::Scene* scene = loadRamsesScene(EFeatureLevel_02);
        ASSERT_TRUE(scene);
        LogicEngine logicEngine{ EFeatureLevel_02 };
        ASSERT_TRUE(logicEngine.loadFromFile("res/unittests/testLogic_02.rlogic", scene));
        EXPECT_TRUE(logicEngine.update());

        checkContents(logicEngine, *scene);
        saveAndReloadAndCheckContents(logicEngine, *scene);
    }

    TEST_F(ALogicEngine_Binary_Compatibility, DISABLED_CanLoadAndUpdateABinaryFileExportedWithLastCompatibleVersionOfEngine_FeatureLevel03)
    {
        EFeatureLevel featureLevel = EFeatureLevel_01;
        EXPECT_TRUE(LogicEngine::GetFeatureLevelFromFile("res/unittests/testLogic_03.rlogic", featureLevel));
        EXPECT_EQ(EFeatureLevel_03, featureLevel);

        ramses::Scene* scene = loadRamsesScene(EFeatureLevel_03);
        ASSERT_TRUE(scene);
        LogicEngine logicEngine{ EFeatureLevel_03 };
        ASSERT_TRUE(logicEngine.loadFromFile("res/unittests/testLogic_03.rlogic", scene));
        EXPECT_TRUE(logicEngine.update());

        checkContents(logicEngine, *scene);
        saveAndReloadAndCheckContents(logicEngine, *scene);
    }

    TEST_F(ALogicEngine_Binary_Compatibility, DISABLED_CanLoadAndUpdateABinaryFileExportedWithLastCompatibleVersionOfEngine_FeatureLevel04)
    {
        EFeatureLevel featureLevel = EFeatureLevel_01;
        EXPECT_TRUE(LogicEngine::GetFeatureLevelFromFile("res/unittests/testLogic_04.rlogic", featureLevel));
        EXPECT_EQ(EFeatureLevel_04, featureLevel);

        ramses::Scene* scene = loadRamsesScene(EFeatureLevel_04);
        ASSERT_TRUE(scene);
        LogicEngine logicEngine{ EFeatureLevel_04 };
        ASSERT_TRUE(logicEngine.loadFromFile("res/unittests/testLogic_04.rlogic", scene));
        EXPECT_TRUE(logicEngine.update());

        checkContents(logicEngine, *scene);
        saveAndReloadAndCheckContents(logicEngine, *scene);
    }

    TEST_F(ALogicEngine_Binary_Compatibility, DISABLED_CanLoadAndUpdateABinaryFileExportedWithLastCompatibleVersionOfEngine_FeatureLevel05)
    {
        EFeatureLevel featureLevel = EFeatureLevel_01;
        EXPECT_TRUE(LogicEngine::GetFeatureLevelFromFile("res/unittests/testLogic_05.rlogic", featureLevel));
        EXPECT_EQ(EFeatureLevel_05, featureLevel);

        ramses::Scene* scene = loadRamsesScene(EFeatureLevel_05);
        ASSERT_TRUE(scene);
        LogicEngine logicEngine{ EFeatureLevel_05 };
        ASSERT_TRUE(logicEngine.loadFromFile("res/unittests/testLogic_05.rlogic", scene));
        EXPECT_TRUE(logicEngine.update());

        checkContents(logicEngine, *scene);
        saveAndReloadAndCheckContents(logicEngine, *scene);
    }
}
