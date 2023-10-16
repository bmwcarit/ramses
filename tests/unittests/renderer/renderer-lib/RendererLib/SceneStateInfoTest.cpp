//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "internal/RendererLib/SceneStateInfo.h"

using namespace testing;

namespace ramses::internal
{
    class ASceneStateInfo : public testing::Test
    {
    protected:
        void checkSceneIsUnknown(const SceneId scene) const
        {
            EXPECT_FALSE(isSceneKnown(scene));
        }

        void checkSceneIsKnown(const SceneId scene) const
        {
            EXPECT_TRUE(isSceneKnown(scene));
        }

        void checkNumberOfKnownScenes(uint32_t expectedNumber) const
        {
            SceneIdVector knownScenes;
            sceneStateInfo.getKnownSceneIds(knownScenes);
            EXPECT_EQ(expectedNumber, knownScenes.size());
        }

        SceneId sceneId;
        SceneStateInfo sceneStateInfo;

    private:
        [[nodiscard]] bool isSceneKnown(const SceneId  scene) const
        {
            SceneIdVector knownScenes;
            sceneStateInfo.getKnownSceneIds(knownScenes);
            return contains_c(knownScenes, scene);
        }

    };

    TEST_F(ASceneStateInfo, IsInitializedCorrectly)
    {
        checkNumberOfKnownScenes(0u);
        EXPECT_FALSE(sceneStateInfo.getScenePublicationMode(sceneId).has_value());
    }

    TEST_F(ASceneStateInfo, CanAddScene)
    {

        sceneStateInfo.addScene(sceneId, EScenePublicationMode::LocalOnly);
        EXPECT_TRUE(sceneStateInfo.hasScene(sceneId));
        EXPECT_EQ(EScenePublicationMode::LocalOnly, *sceneStateInfo.getScenePublicationMode(sceneId));
        checkNumberOfKnownScenes(1u);
        checkSceneIsKnown(sceneId);
    }

    TEST_F(ASceneStateInfo, CanRemoveScene)
    {
        sceneStateInfo.addScene(sceneId, EScenePublicationMode::LocalOnly);
        checkNumberOfKnownScenes(1u);
        checkSceneIsKnown(sceneId);

        sceneStateInfo.removeScene(sceneId);
        EXPECT_FALSE(sceneStateInfo.hasScene(sceneId));
        checkNumberOfKnownScenes(0u);
        checkSceneIsUnknown(sceneId);
    }

    TEST_F(ASceneStateInfo, CanAddAndRemoveMultipleScenes)
    {
        SceneId sceneId2(5u);
        sceneStateInfo.addScene(sceneId, EScenePublicationMode::LocalOnly);
        checkNumberOfKnownScenes(1u);
        checkSceneIsKnown(sceneId);
        checkSceneIsUnknown(sceneId2);

        sceneStateInfo.addScene(sceneId2, EScenePublicationMode::LocalAndRemote);
        checkNumberOfKnownScenes(2u);
        checkSceneIsKnown(sceneId);
        checkSceneIsKnown(sceneId2);

        EXPECT_EQ(EScenePublicationMode::LocalOnly, *sceneStateInfo.getScenePublicationMode(sceneId));
        EXPECT_EQ(EScenePublicationMode::LocalAndRemote, *sceneStateInfo.getScenePublicationMode(sceneId2));

        sceneStateInfo.removeScene(sceneId);
        EXPECT_FALSE(sceneStateInfo.hasScene(sceneId));
        checkNumberOfKnownScenes(1u);
        checkSceneIsUnknown(sceneId);
        checkSceneIsKnown(sceneId2);

        sceneStateInfo.removeScene(sceneId2);
        EXPECT_FALSE(sceneStateInfo.hasScene(sceneId2));
        checkNumberOfKnownScenes(0u);
        checkSceneIsUnknown(sceneId);
        checkSceneIsUnknown(sceneId2);
    }

    TEST_F(ASceneStateInfo, CanGetSceneState)
    {
        sceneStateInfo.addScene(sceneId, EScenePublicationMode::LocalAndRemote);
        EXPECT_EQ(ESceneState::Published, sceneStateInfo.getSceneState(sceneId));
    }

    TEST_F(ASceneStateInfo, CanSetSceneState)
    {
        sceneStateInfo.addScene(sceneId, EScenePublicationMode::LocalAndRemote);
        sceneStateInfo.setSceneState(sceneId, ESceneState::Subscribed);
        EXPECT_EQ(ESceneState::Subscribed, sceneStateInfo.getSceneState(sceneId));
    }
}
