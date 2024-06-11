//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/SceneGraph/SceneAPI/IScene.h"
#include "gmock/gmock.h"
#include "impl/SceneFactory.h"
#include "internal/SceneGraph/Scene/ClientScene.h"

namespace ramses::internal
{
    using namespace testing;

    class ASceneFactory : public Test
    {
    protected:
        SceneFactory factory;
    };

    TEST_F(ASceneFactory, createsAndDeletesScene)
    {
        IScene* scene = factory.createScene(SceneInfo(), EFeatureLevel_Latest);
        ASSERT_TRUE(nullptr != scene);
        auto sceneOwnPtr = factory.releaseScene(scene->getSceneId());
        EXPECT_EQ(scene, sceneOwnPtr.get());
    }

    TEST_F(ASceneFactory, cannotCreateTwoScenesWithTheSameId)
    {
        IScene* scene = factory.createScene(SceneInfo(), EFeatureLevel_Latest);
        ASSERT_TRUE(nullptr != scene);
        EXPECT_TRUE(nullptr == factory.createScene(SceneInfo{ scene->getSceneId() }, EFeatureLevel_Latest));
    }

    TEST_F(ASceneFactory, createsSceneWithProvidedOptions)
    {
        const SceneSizeInformation sizeInfo(1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u, 12u, 13u, 14u, 15u, 16u, 17u, 18u, 19u);
        const SceneId sceneId(456u);
        const SceneInfo sceneInfo{ sceneId, "sceneName" };
        auto* scene = static_cast<Scene*>(factory.createScene(sceneInfo, EFeatureLevel_Latest));
        scene->preallocateSceneSize(sizeInfo);
        ASSERT_TRUE(scene != nullptr);
        EXPECT_EQ(std::string("sceneName"), scene->getName());
        EXPECT_EQ(sizeInfo, scene->getSceneSizeInformation());
        EXPECT_EQ(sceneId, scene->getSceneId());
    }
}
