//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesTestUtils.h"
#include "WithTempDirectory.h"
#include "PropertyLinkTestUtils.h"
#include "FeatureLevelTestValues.h"

#include "ramses/client/EffectDescription.h"
#include "ramses/client/Effect.h"
#include "ramses/client/Scene.h"
#include "ramses/client/OrthographicCamera.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/logic/DataArray.h"
#include "ramses/client/logic/LuaInterface.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/logic/AnimationNode.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/RenderBufferBinding.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/UniformInput.h"

namespace ramses::internal
{
    // These tests will always break on incompatible file format changes.
    class AFeatureLevelCompatibility : public ::testing::TestWithParam<ramses::EFeatureLevel>
    {
    protected:
        void checkBaseContents()
        {
            auto logicEngine = m_scene->findObject<LogicEngine>("testAssetLogic");
            ASSERT_TRUE(logicEngine);
            EXPECT_TRUE(logicEngine->update());

            ASSERT_NE(nullptr, logicEngine->findObject<LuaModule>("nestedModuleMath"));
            ASSERT_NE(nullptr, logicEngine->findObject<LuaModule>("moduleMath"));
            ASSERT_NE(nullptr, logicEngine->findObject<LuaModule>("moduleTypes"));
            const auto script1 = logicEngine->findObject<LuaScript>("script1");
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
            const auto script2 = logicEngine->findObject<LuaScript>("script2");
            ASSERT_NE(nullptr, script2);
            EXPECT_NE(nullptr, script2->getInputs()->getChild("floatInput"));
            EXPECT_NE(nullptr, script2->getOutputs()->getChild("cameraViewport")->getChild("offsetX"));
            EXPECT_NE(nullptr, script2->getOutputs()->getChild("cameraViewport")->getChild("offsetY"));
            EXPECT_NE(nullptr, script2->getOutputs()->getChild("cameraViewport")->getChild("width"));
            EXPECT_NE(nullptr, script2->getOutputs()->getChild("cameraViewport")->getChild("height"));
            EXPECT_NE(nullptr, script2->getOutputs()->getChild("floatUniform"));
            const auto animNode = logicEngine->findObject<AnimationNode>("animNode");
            ASSERT_NE(nullptr, animNode);
            EXPECT_EQ(1u, animNode->getInputs()->getChildCount());
            ASSERT_EQ(2u, animNode->getOutputs()->getChildCount());
            EXPECT_NE(nullptr, animNode->getOutputs()->getChild("channel"));
            ASSERT_NE(nullptr, logicEngine->findObject<AnimationNode>("animNodeWithDataProperties"));
            EXPECT_EQ(2u, logicEngine->findObject<AnimationNode>("animNodeWithDataProperties")->getInputs()->getChildCount());
            EXPECT_NE(nullptr, logicEngine->findObject<TimerNode>("timerNode"));

            const auto nodeBinding = logicEngine->findObject<NodeBinding>("nodebinding");
            ASSERT_NE(nullptr, nodeBinding);
            EXPECT_NE(nullptr, nodeBinding->getInputs()->getChild("enabled"));
            EXPECT_TRUE(logicEngine->isLinked(*nodeBinding));
            const auto cameraBinding = logicEngine->findObject<CameraBinding>("camerabinding");
            ASSERT_NE(nullptr, cameraBinding);
            const auto appearanceBinding = logicEngine->findObject<AppearanceBinding>("appearancebinding");
            ASSERT_NE(nullptr, appearanceBinding);
            EXPECT_NE(nullptr, logicEngine->findObject<DataArray>("dataarray"));

            std::vector<PropertyLink> expectedLinks;

            const auto intf = logicEngine->findObject<LuaInterface>("intf");
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

            const auto triLogicIntf = logicEngine->findObject<LuaInterface>("Interface_CameraCrane");
            ASSERT_TRUE(triLogicIntf);
            const auto triLogicScript = logicEngine->findObject<LuaScript>("CameraCrane");
            ASSERT_TRUE(triLogicScript);
            const auto triCamNode = logicEngine->findObject<NodeBinding>("triangleCamNodeBinding");
            ASSERT_TRUE(triCamNode);
            const auto triCamBinding = logicEngine->findObject<CameraBinding>("triangleCamBinding");
            ASSERT_TRUE(triCamBinding);
            expectedLinks.push_back({ triLogicIntf->getOutputs()->getChild("CraneGimbal")->getChild("Yaw"), triLogicScript->getInputs()->getChild("yaw"), false });
            expectedLinks.push_back({ triLogicIntf->getOutputs()->getChild("CraneGimbal")->getChild("Pitch"), triLogicScript->getInputs()->getChild("pitch"), false });
            expectedLinks.push_back({ triLogicIntf->getOutputs()->getChild("Viewport")->getChild("Width"), triCamBinding->getInputs()->getChild("viewport")->getChild("width"), false });
            expectedLinks.push_back({ triLogicIntf->getOutputs()->getChild("Viewport")->getChild("Height"), triCamBinding->getInputs()->getChild("viewport")->getChild("height"), false });
            expectedLinks.push_back({ triLogicScript->getOutputs()->getChild("translation"), triCamNode->getInputs()->getChild("translation"), false });

            PropertyLinkTestUtils::ExpectLinks(*logicEngine, expectedLinks);

            EXPECT_TRUE(logicEngine->update());

            // Values on Ramses are updated according to expectations
            vec3f translation;
            auto  node = m_scene->findObject<ramses::Node>("test node");
            auto camera = m_scene->findObject<ramses::OrthographicCamera>("test camera");
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
            EXPECT_TRUE(logicEngine->update());

            auto appearance = m_scene->findObject<ramses::Appearance>("test appearance");
            const auto uniform = appearance->getEffect().getUniformInput(1);
            ASSERT_TRUE(uniform.has_value());
            float floatValue = 0.f;
            appearance->getInputValue(*uniform, floatValue);
            EXPECT_FLOAT_EQ(1.5f, floatValue);

            EXPECT_EQ(957, *logicEngine->findObject<LuaScript>("script2")->getOutputs()->getChild("nestedModulesResult")->get<int32_t>());

            EXPECT_TRUE(logicEngine->findObject<RenderPassBinding>("renderpassbinding"));
            EXPECT_TRUE(logicEngine->findObject<AnchorPoint>("anchorpoint"));

            const auto cameraBindingPersp = logicEngine->findObject<CameraBinding>("camerabindingPersp");
            const auto cameraBindingPerspWithFrustumPlanes = logicEngine->findObject<CameraBinding>("camerabindingPerspWithFrustumPlanes");
            ASSERT_TRUE(cameraBindingPersp && cameraBindingPerspWithFrustumPlanes);
            EXPECT_EQ(4u, cameraBindingPersp->getInputs()->getChild("frustum")->getChildCount());
            EXPECT_EQ(6u, cameraBindingPerspWithFrustumPlanes->getInputs()->getChild("frustum")->getChildCount());

            EXPECT_TRUE(logicEngine->findObject<RenderGroupBinding>("rendergroupbinding"));
            EXPECT_TRUE(logicEngine->findObject<SkinBinding>("skin"));
            const auto dataArray = logicEngine->findObject<DataArray>("dataarrayOfArrays");
            ASSERT_TRUE(dataArray);
            EXPECT_EQ(EPropertyType::Array, dataArray->getDataType());
            EXPECT_EQ(2u, dataArray->getNumElements());
            const auto data = dataArray->getData<std::vector<float>>();
            ASSERT_TRUE(data);
            const std::vector<std::vector<float>> expectedData{ { 1.f, 2.f, 3.f, 4.f, 5.f }, { 6.f, 7.f, 8.f, 9.f, 10.f } };
            EXPECT_EQ(expectedData, *data);
            EXPECT_TRUE(logicEngine->findObject<MeshNodeBinding>("meshnodebinding"));
            auto rbBinding = logicEngine->findObject<RenderBufferBinding>("renderBufferBinding");
            ASSERT_TRUE(rbBinding);
            EXPECT_EQ(m_scene->findObject<ramses::RenderBuffer>("renderBuffer"), &rbBinding->getRenderBuffer());
        }

        void checkUniformBufferInput(bool exists)
        {
            const auto& triangleEffect = m_scene->findObject<ramses::Appearance>("triangle appearance")->getEffect();
            auto uboInput = triangleEffect.findUniformInput("colorBlock");
            ASSERT_EQ(exists, uboInput.has_value());
            if (exists)
            {
                EXPECT_EQ(EDataType::UniformBuffer, uboInput->getDataType());
            }

            auto uboSemanticInput = triangleEffect.findUniformInput("modelCameraBlock");
            ASSERT_EQ(exists, uboSemanticInput.has_value());
            if (exists)
            {
                EXPECT_EQ(EDataType::UniformBuffer, uboSemanticInput->getDataType());
                EXPECT_EQ(EEffectUniformSemantic::ModelCameraBlock, uboSemanticInput->getSemantics());
            }
        }

        void expectFeatureLevel02Content()
        {
            const auto appearanceBindingLo = m_scene->findObject<ramses::LogicObject>("triangle appearance binding");
            ASSERT_TRUE(appearanceBindingLo);
            const auto appearanceBinding = appearanceBindingLo->as<ramses::AppearanceBinding>();
            ASSERT_TRUE(appearanceBinding);
            EXPECT_EQ(1.f, appearanceBinding->getInputs()->getChild("colorBlock.color[0].c1")->get<float>());
            EXPECT_EQ(0.1f, appearanceBinding->getInputs()->getChild("colorBlock.color[0].c2")->get<float>());
            EXPECT_EQ(0.2f, appearanceBinding->getInputs()->getChild("colorBlock.color[1].c1")->get<float>());
            EXPECT_EQ(1.f, appearanceBinding->getInputs()->getChild("colorBlock.color[1].c2")->get<float>());

            checkUniformBufferInput(true);
        }

        void expectFeatureLevel02ContentNotPresent()
        {
            checkUniformBufferInput(false);
        }

        void checkContents()
        {
            // check for content expected to exist
            // higher feature level always contains content supported by lower level
            switch (GetParam())
            {
            case ramses::EFeatureLevel_02:
                expectFeatureLevel02Content();
                [[fallthrough]];
            case ramses::EFeatureLevel_01:
                checkBaseContents();
                break;
            }

            // check for content expected to not exist
            // lower feature level never contains content supported only in higher level
            switch (GetParam())
            {
            case ramses::EFeatureLevel_01:
                expectFeatureLevel02ContentNotPresent();
                [[fallthrough]];
            case EFeatureLevel_02:
                break;
            }
        }

        void saveAndReloadAndCheckContents()
        {
            EXPECT_TRUE(m_scene->saveToFile("temp.ramses", {}));
            m_ramses.getClient().destroy(*m_scene);

            m_scene = m_ramses.getClient().loadSceneFromFile("temp.ramses");
            ASSERT_TRUE(m_scene);

            checkContents();
        }

        void loadScene()
        {
            switch(GetParam())
            {
            case ramses::EFeatureLevel_01:
                m_scene = &m_ramses.loadSceneFromFile("../res/testScene_01.ramses");
                break;
            case ramses::EFeatureLevel_02:
                m_scene = &m_ramses.loadSceneFromFile("../res/testScene_02.ramses");
                break;
            default:
                assert(false);
                break;
            }
        }

        WithTempDirectory m_tempDir;
        RamsesTestSetup m_ramses{ GetParam() };
        Scene* m_scene = nullptr;
    };

    RAMSES_INSTANTIATE_FEATURELEVEL_TEST_SUITE(AFeatureLevelCompatibility)

    // disabled until UBO isolated within FL02
    TEST_P(AFeatureLevelCompatibility, CanLoadExportedBinaryAndVerifyContent)
    {
        loadScene();
        checkContents();
        saveAndReloadAndCheckContents();
    }
}
