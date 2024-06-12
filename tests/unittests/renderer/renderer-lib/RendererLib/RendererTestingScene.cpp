//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "TestingScene.h"
#include "FeatureLevelTestValues.h"
#include "internal/RendererLib/RendererCachedScene.h"
#include "internal/RendererLib/RendererScenes.h"
#include "internal/RendererLib/RendererEventCollector.h"
#include "internal/RendererLib/SceneLinksManager.h"

using namespace testing;

namespace ramses::internal
{
    class ARendererTestingScene : public ::testing::TestWithParam<ramses::EFeatureLevel>
    {
    };

    RAMSES_INSTANTIATE_FEATURELEVEL_TEST_SUITE(ARendererTestingScene);

    TEST_P(ARendererTestingScene, generateAndCheckContent)
    {
        // renderer scenes are explicitly allocated, i.e. all pool sizes have to be preallocated,
        // here we get the sizes using regular scene
        SceneSizeInformation sceneSizeInfo;
        {
            Scene sceneForSizeInfo;
            TestingScene testingSceneForSizeInfo{ sceneForSizeInfo, EFeatureLevel_Latest };
            sceneSizeInfo = sceneForSizeInfo.getSceneSizeInformation();
        }

        RendererEventCollector dummyEventCollector;
        RendererScenes rendererScenes{ dummyEventCollector };
        auto& scene = rendererScenes.createScene(SceneInfo{ SceneId{ 123u} });
        scene.preallocateSceneSize(sceneSizeInfo);

        TestingScene testingScene{ scene, GetParam() };
        testingScene.VerifyContent(scene);
    }
}
