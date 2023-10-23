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

using namespace testing;

namespace ramses::internal
{
    TEST(AScenePersistation, canReadWrite)
    {
        ClientScene scene;
        NodeHandle parentNodeHandle = scene.allocateNode(0, {});
        NodeHandle childNodeHandle = scene.allocateNode(0, {});
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
        SceneActionCollection dummyCollection;
        ScenePersistation::ReadSceneFromFile("testfile", loadedScene);
        scene.CheckEquivalentTo<IScene>(loadedScene);
    }
}
