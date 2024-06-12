//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/RendererLib/RendererScenes.h"
#include "internal/RendererLib/RendererEventCollector.h"

#include <string>

using namespace testing;
namespace ramses::internal
{
    class ARendererScenes : public ::testing::Test
    {
    public:
        ARendererScenes()
            : rendererScenes(eventCollector)
        {
        }

    protected:
        RendererEventCollector eventCollector;
        RendererScenes rendererScenes;
    };

    TEST_F(ARendererScenes, isEmptyInitially)
    {
        EXPECT_EQ(rendererScenes.begin(), rendererScenes.end());
        EXPECT_EQ(0u, rendererScenes.size());
    }

    TEST_F(ARendererScenes, isEmptyInitiallyConst)
    {
        const RendererScenes rendererScenesConst(eventCollector);
        EXPECT_EQ(rendererScenesConst.begin(), rendererScenesConst.end());
        EXPECT_EQ(0u, rendererScenesConst.size());
    }

    TEST_F(ARendererScenes, createsSceneAndStagingInfo)
    {
        const std::string sceneName("bla");
        const SceneId sceneID(12u);
        SceneInfo sceneInfo{ sceneID, sceneName };
        IScene& createdScene = rendererScenes.createScene(sceneInfo);

        SceneSizeInformation sceneSizeInfo(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19);
        createdScene.preallocateSceneSize(sceneSizeInfo);

        EXPECT_EQ(1u, rendererScenes.size());
        EXPECT_NE(rendererScenes.begin(), rendererScenes.end());
        EXPECT_TRUE(rendererScenes.hasScene(sceneID));
        EXPECT_EQ(&createdScene, &rendererScenes.getScene(sceneID));
        EXPECT_EQ(sceneSizeInfo, createdScene.getSceneSizeInformation());
        EXPECT_EQ(sceneName, createdScene.getName());
        EXPECT_EQ(sceneID, createdScene.getSceneId());
        rendererScenes.getStagingInfo(sceneID);
    }

    TEST_F(ARendererScenes, destroysSceneAndStagingInfo)
    {
        const SceneId sceneID(12u);
        rendererScenes.createScene(SceneInfo{ sceneID });

        rendererScenes.destroyScene(sceneID);

        EXPECT_EQ(0u, rendererScenes.size());
        EXPECT_EQ(rendererScenes.begin(), rendererScenes.end());
        EXPECT_FALSE(rendererScenes.hasScene(sceneID));
    }

    TEST_F(ARendererScenes, canIterateOverScenes)
    {
        const SceneId sceneID1(12u);
        rendererScenes.createScene(SceneInfo{ sceneID1 });

        const SceneId sceneID2(13u);
        rendererScenes.createScene(SceneInfo{ sceneID2 });

        const SceneId sceneID3(14u);
        rendererScenes.createScene(SceneInfo{ sceneID3 });

        EXPECT_EQ(3u, rendererScenes.size());
        EXPECT_NE(rendererScenes.begin(), rendererScenes.end());
        EXPECT_TRUE(rendererScenes.hasScene(sceneID1));
        EXPECT_TRUE(rendererScenes.hasScene(sceneID2));
        EXPECT_TRUE(rendererScenes.hasScene(sceneID3));

        uint32_t count = 0u;
        for(auto rendScene : rendererScenes)
        {
            EXPECT_TRUE(sceneID1 == rendScene.key || sceneID2 == rendScene.key || sceneID3 == rendScene.key);
            EXPECT_TRUE(nullptr != rendScene.value.scene);
            EXPECT_TRUE(nullptr != rendScene.value.stagingInfo);
            ++count;
        }
        EXPECT_EQ(3u, count);
    }
}
