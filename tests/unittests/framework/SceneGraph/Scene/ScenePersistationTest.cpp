//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/SceneGraph/Scene/ScenePersistation.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "TestingScene.h"
#include "FeatureLevelTestValues.h"

using namespace testing;

namespace ramses::internal
{
    class AScenePersistation : public ::testing::TestWithParam<EFeatureLevel>
    {
    };

    RAMSES_INSTANTIATE_FEATURELEVEL_TEST_SUITE(AScenePersistation);

    TEST_P(AScenePersistation, canReadWrite)
    {
        ClientScene scene;
        NodeHandle parentNodeHandle = scene.allocateNode(0, {});
        NodeHandle childNodeHandle = scene.allocateNode(0, {});
        scene.addChildToNode(parentNodeHandle, childNodeHandle);
        ScenePersistation::WriteSceneToFile("testfile", scene, GetParam());

        Scene loadedScene;
        ScenePersistation::ReadSceneFromFile("testfile", loadedScene, GetParam(), nullptr);
        ASSERT_EQ(2u, loadedScene.getNodeCount());
        ASSERT_EQ(parentNodeHandle, loadedScene.getParent(childNodeHandle));
    }

    TEST_P(AScenePersistation, canReadWriteTestingScene)
    {
        ClientScene scene;
        TestingScene testingScene{ scene, GetParam() };
        ScenePersistation::WriteSceneToFile("testfile", scene, GetParam());

        Scene loadedScene;
        SceneActionCollection dummyCollection;
        ScenePersistation::ReadSceneFromFile("testfile", loadedScene, GetParam(), nullptr);
        testingScene.VerifyContent(loadedScene);
    }
}
