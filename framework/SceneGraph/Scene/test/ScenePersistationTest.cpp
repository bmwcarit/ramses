//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Scene/ScenePersistation.h"
#include "Scene/ClientScene.h"
#include "TestingScene.h"

using namespace testing;

namespace ramses_internal
{
    TEST(AScenePersistation, canReadWrite)
    {
        ClientScene scene;
        NodeHandle parentNodeHandle = scene.allocateNode();
        NodeHandle childNodeHandle = scene.allocateNode();
        scene.addChildToNode(parentNodeHandle, childNodeHandle);
        ScenePersistation::WriteSceneToFile("testfile", scene);

        Scene loadedScene;
        ScenePersistation::ReadSceneFromFile("testfile", loadedScene);
        ASSERT_EQ(2u, loadedScene.getNodeCount());
        ASSERT_EQ(parentNodeHandle, loadedScene.getParent(childNodeHandle));
    }

    TEST(AScenePersistation, canReadWriteMockScene)
    {
        TestingScene<ClientScene> scene;
        ScenePersistation::WriteSceneToFile("testfile", scene.getScene());

        Scene loadedScene;
        ScenePersistation::ReadSceneFromFile("testfile", loadedScene);
        scene.CheckEquivalentTo<IScene>(loadedScene);
    }
}
