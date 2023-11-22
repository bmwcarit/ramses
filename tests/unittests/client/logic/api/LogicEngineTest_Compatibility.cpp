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
#include "PropertyLinkTestUtils.h"

#include "ramses/client/logic/Property.h"
#include "ramses/client/logic/RenderBufferBinding.h"

#include "ramses/client/EffectDescription.h"
#include "ramses/client/Effect.h"
#include "ramses/client/Scene.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/UniformInput.h"
#include "ramses/framework/RamsesVersion.h"

#include "internal/logic/ApiObjects.h"
#include "internal/logic/FileUtils.h"

#include "internal/logic/flatbuffers/generated/LogicEngineGen.h"
#include "fmt/format.h"

#include <fstream>

namespace ramses::internal
{
    // These tests will always break on incompatible file format changes.
    class ALogicEngine_Binary_Compatibility : public ::testing::Test
    {
    protected:
        static void checkBaseContents(LogicEngine& logicEngine, ramses::Scene& ramsesScene)
        {
            ASSERT_NE(nullptr, logicEngine.findObject<LuaModule>("nestedModuleMath"));
            ASSERT_NE(nullptr, logicEngine.findObject<LuaModule>("moduleMath"));
            ASSERT_NE(nullptr, logicEngine.findObject<LuaModule>("moduleTypes"));
            const auto script1 = logicEngine.findObject<LuaScript>("script1");
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
            const auto script2 = logicEngine.findObject<LuaScript>("script2");
            ASSERT_NE(nullptr, script2);
            EXPECT_NE(nullptr, script2->getInputs()->getChild("floatInput"));
            EXPECT_NE(nullptr, script2->getOutputs()->getChild("cameraViewport")->getChild("offsetX"));
            EXPECT_NE(nullptr, script2->getOutputs()->getChild("cameraViewport")->getChild("offsetY"));
            EXPECT_NE(nullptr, script2->getOutputs()->getChild("cameraViewport")->getChild("width"));
            EXPECT_NE(nullptr, script2->getOutputs()->getChild("cameraViewport")->getChild("height"));
            EXPECT_NE(nullptr, script2->getOutputs()->getChild("floatUniform"));
            const auto animNode = logicEngine.findObject<AnimationNode>("animNode");
            ASSERT_NE(nullptr, animNode);
            EXPECT_EQ(1u, animNode->getInputs()->getChildCount());
            ASSERT_EQ(2u, animNode->getOutputs()->getChildCount());
            EXPECT_NE(nullptr, animNode->getOutputs()->getChild("channel"));
            ASSERT_NE(nullptr, logicEngine.findObject<AnimationNode>("animNodeWithDataProperties"));
            EXPECT_EQ(2u, logicEngine.findObject<AnimationNode>("animNodeWithDataProperties")->getInputs()->getChildCount());
            EXPECT_NE(nullptr, logicEngine.findObject<TimerNode>("timerNode"));

            const auto nodeBinding = logicEngine.findObject<NodeBinding>("nodebinding");
            ASSERT_NE(nullptr, nodeBinding);
            EXPECT_NE(nullptr, nodeBinding->getInputs()->getChild("enabled"));
            EXPECT_TRUE(logicEngine.isLinked(*nodeBinding));
            const auto cameraBinding = logicEngine.findObject<CameraBinding>("camerabinding");
            ASSERT_NE(nullptr, cameraBinding);
            const auto appearanceBinding = logicEngine.findObject<AppearanceBinding>("appearancebinding");
            ASSERT_NE(nullptr, appearanceBinding);
            EXPECT_NE(nullptr, logicEngine.findObject<DataArray>("dataarray"));

            std::vector<PropertyLink> expectedLinks;

            const auto intf = logicEngine.findObject<LuaInterface>("intf");
            ASSERT_NE(nullptr, intf);
            intf->getInputs()->getChild("struct")->getChild("floatInput")->set<float>(42.5f);
            expectedLinks.push_back({ intf->getOutputs()->getChild("struct")->getChild("floatInput"), script1->getInputs()->getChild("floatInput"), false });
            expectedLinks.push_back({ script1->getOutputs()->getChild("boolOutput"), nodeBinding->getInputs()->getChild("enabled"), false });
            expectedLinks.push_back({ script1->getOutputs()->getChild("floatOutput"), script2->getInputs()->getChild("floatInput"), false });
            expectedLinks.push_back({ script1->getOutputs()->getChild("nodeTranslation"), nodeBinding->getInputs()->getChild("translation"), false });
            expectedLinks.push_back({ script2->getOutputs()->getChild("cameraViewport")->getChild("offsetX"), cameraBinding->getInputs()->getChild("viewport")->getChild("offsetX"), false });
            expectedLinks.push_back({ script2->getOutputs()->getChild("cameraViewport")->getChild("offsetY"), cameraBinding->getInputs()->getChild("viewport")->getChild("offsetY"), false });
            expectedLinks.push_back({ script2->getOutputs()->getChild("cameraViewport")->getChild("width"), cameraBinding->getInputs()->getChild("viewport")->getChild("width"), false });
            expectedLinks.push_back({ script2->getOutputs()->getChild("cameraViewport")->getChild("height"), cameraBinding->getInputs()->getChild("viewport")->getChild("height"), false });
            expectedLinks.push_back({ script2->getOutputs()->getChild("floatUniform"), appearanceBinding->getInputs()->getChild("floatUniform"), false });
            expectedLinks.push_back({ animNode->getOutputs()->getChild("channel"), appearanceBinding->getInputs()->getChild("animatedFloatUniform"), false });

            const auto triLogicIntf = logicEngine.findObject<LuaInterface>("Interface_CameraCrane");
            ASSERT_TRUE(triLogicIntf);
            const auto triLogicScript = logicEngine.findObject<LuaScript>("CameraCrane");
            ASSERT_TRUE(triLogicScript);
            const auto triCamNode = logicEngine.findObject<NodeBinding>("triangleCamNodeBinding");
            ASSERT_TRUE(triCamNode);
            const auto triCamBinding = logicEngine.findObject<CameraBinding>("triangleCamBinding");
            ASSERT_TRUE(triCamBinding);
            expectedLinks.push_back({ triLogicIntf->getOutputs()->getChild("CraneGimbal")->getChild("Yaw"), triLogicScript->getInputs()->getChild("yaw"), false });
            expectedLinks.push_back({ triLogicIntf->getOutputs()->getChild("CraneGimbal")->getChild("Pitch"), triLogicScript->getInputs()->getChild("pitch"), false });
            expectedLinks.push_back({ triLogicIntf->getOutputs()->getChild("Viewport")->getChild("Width"), triCamBinding->getInputs()->getChild("viewport")->getChild("width"), false });
            expectedLinks.push_back({ triLogicIntf->getOutputs()->getChild("Viewport")->getChild("Height"), triCamBinding->getInputs()->getChild("viewport")->getChild("height"), false });
            expectedLinks.push_back({ triLogicScript->getOutputs()->getChild("translation"), triCamNode->getInputs()->getChild("translation"), false });

            PropertyLinkTestUtils::ExpectLinks(logicEngine, expectedLinks);

            EXPECT_TRUE(logicEngine.update());

            // Values on Ramses are updated according to expectations
            vec3f translation;
            auto  node = ramsesScene.findObject<ramses::Node>("test node");
            auto camera = ramsesScene.findObject<ramses::OrthographicCamera>("test camera");
            node->getTranslation(translation);
            EXPECT_EQ(translation, vec3f(42.5f, 2.f, 3.f));
            // test that linked value from script propagated to ramses scene
            EXPECT_EQ(ramses::EVisibilityMode::Off, node->getVisibility());

            EXPECT_EQ(camera->getViewportX(), 45);
            EXPECT_EQ(camera->getViewportY(), 47);
            EXPECT_EQ(camera->getViewportWidth(), 143u);
            EXPECT_EQ(camera->getViewportHeight(), 243u);

            // Animation node is linked and can be animated
            EXPECT_FLOAT_EQ(2.f, *animNode->getOutputs()->getChild("duration")->get<float>());
            animNode->getInputs()->getChild("progress")->set(0.75f);
            EXPECT_TRUE(logicEngine.update());

            auto appearance = ramsesScene.findObject<ramses::Appearance>("test appearance");
            const auto uniform = appearance->getEffect().getUniformInput(1);
            ASSERT_TRUE(uniform.has_value());
            float floatValue = 0.f;
            appearance->getInputValue(*uniform, floatValue);
            EXPECT_FLOAT_EQ(1.5f, floatValue);

            EXPECT_EQ(957, *logicEngine.findObject<LuaScript>("script2")->getOutputs()->getChild("nestedModulesResult")->get<int32_t>());

            EXPECT_TRUE(logicEngine.findObject<RenderPassBinding>("renderpassbinding"));
            EXPECT_TRUE(logicEngine.findObject<AnchorPoint>("anchorpoint"));

            const auto cameraBindingPersp = logicEngine.findObject<CameraBinding>("camerabindingPersp");
            const auto cameraBindingPerspWithFrustumPlanes = logicEngine.findObject<CameraBinding>("camerabindingPerspWithFrustumPlanes");
            ASSERT_TRUE(cameraBindingPersp && cameraBindingPerspWithFrustumPlanes);
            EXPECT_EQ(4u, cameraBindingPersp->getInputs()->getChild("frustum")->getChildCount());
            EXPECT_EQ(6u, cameraBindingPerspWithFrustumPlanes->getInputs()->getChild("frustum")->getChildCount());

            EXPECT_TRUE(logicEngine.findObject<RenderGroupBinding>("rendergroupbinding"));
            EXPECT_TRUE(logicEngine.findObject<SkinBinding>("skin"));
            const auto dataArray = logicEngine.findObject<DataArray>("dataarrayOfArrays");
            ASSERT_TRUE(dataArray);
            EXPECT_EQ(EPropertyType::Array, dataArray->getDataType());
            EXPECT_EQ(2u, dataArray->getNumElements());
            const auto data = dataArray->getData<std::vector<float>>();
            ASSERT_TRUE(data);
            const std::vector<std::vector<float>> expectedData{ { 1.f, 2.f, 3.f, 4.f, 5.f }, { 6.f, 7.f, 8.f, 9.f, 10.f } };
            EXPECT_EQ(expectedData, *data);
            EXPECT_TRUE(logicEngine.findObject<MeshNodeBinding>("meshnodebinding"));
            auto rbBinding = logicEngine.findObject<RenderBufferBinding>("renderBufferBinding");
            ASSERT_TRUE(rbBinding);
            EXPECT_EQ(ramsesScene.findObject<ramses::RenderBuffer>("renderBuffer"), &rbBinding->getRenderBuffer());
        }

        static void expectFeatureLevel02Content(const LogicEngine& /*logicEngine*/, ramses::Scene& /*ramsesScene*/)
        {
            // features added in future feature level expected to be present
        }

        static void expectFeatureLevel02ContentNotPresent(const LogicEngine& /*logicEngine*/)
        {
            // features added in future feature level expected NOT to be present in previous feature levels
        }

        static void checkContents(LogicEngine& logicEngine, ramses::Scene& scene)
        {
            const ramses::EFeatureLevel featureLevel = scene.getRamsesClient().getRamsesFramework().getFeatureLevel();

            // check for content expected to exist
            // higher feature level always contains content supported by lower level
            switch (featureLevel)
            {
            //case ramses::EFeatureLevel_02:
            //    expectFeatureLevel02Content(logicEngine, scene);
            //    [[fallthrough]];
            case ramses::EFeatureLevel_01:
                checkBaseContents(logicEngine, scene);
                break;
            }

            // check for content expected to not exist
            // lower feature level never contains content supported only in higher level
            switch (featureLevel)
            {
            case ramses::EFeatureLevel_01:
                expectFeatureLevel02ContentNotPresent(logicEngine);
            //    [[fallthrough]];
            //case EFeatureLevel_02:
            //    break;
            }
        }

        static void saveAndReloadAndCheckContents(ramses::Scene& scene)
        {
            WithTempDirectory tempDir;

            EXPECT_TRUE(scene.saveToFile("temp.ramses", {}));

            auto& client = scene.getRamsesClient();
            client.destroy(scene);
            auto otherScene = client.loadSceneFromFile("temp.ramses");
            auto logicEngineAnother = otherScene->findObject<LogicEngine>("testAssetLogic");
            EXPECT_TRUE(logicEngineAnother->update());

            checkContents(*logicEngineAnother, *otherScene);
            client.destroy(*otherScene);
        }

        ramses::Scene* loadRamsesScene(ramses::EFeatureLevel featureLevel)
        {
            switch(featureLevel)
            {
            case ramses::EFeatureLevel_01:
                return &m_ramses.loadSceneFromFile("res/testScene_01.ramses");
            }
            return nullptr;
        }

        RamsesTestSetup m_ramses;
    };

    TEST_F(ALogicEngine_Binary_Compatibility, CanLoadAndUpdateABinaryFileExportedWithLastCompatibleVersionOfEngine_FeatureLevel01)
    {
        ramses::Scene* scene = loadRamsesScene(ramses::EFeatureLevel_01);
        ASSERT_TRUE(scene);
        auto* logicEngine = scene->findObject<LogicEngine>("testAssetLogic");
        ASSERT_TRUE(logicEngine);
        EXPECT_TRUE(logicEngine->update());

        checkContents(*logicEngine, *scene);
        saveAndReloadAndCheckContents(*scene);
    }
}
