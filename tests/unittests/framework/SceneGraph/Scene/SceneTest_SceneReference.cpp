//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneTest.h"

using namespace testing;

namespace ramses::internal
{
    TYPED_TEST_SUITE(AScene, SceneTypes);

    TYPED_TEST(AScene, ContainsZeroTotalSceneReferencesUponCreation)
    {
        EXPECT_EQ(0u, this->m_scene.getSceneReferenceCount());
    }

    TYPED_TEST(AScene, AllocatesSceneReference)
    {
        constexpr SceneId sceneId{ 123 };
        constexpr SceneReferenceHandle requestedHandle{ 6 };

        const auto handle = this->m_scene.allocateSceneReference(sceneId, requestedHandle);
        EXPECT_EQ(handle, requestedHandle);
        EXPECT_EQ(7u, this->m_scene.getSceneReferenceCount());
        EXPECT_TRUE(this->m_scene.isSceneReferenceAllocated(handle));
        EXPECT_FALSE(this->m_scene.isSceneReferenceAllocated(SceneReferenceHandle{ 0 }));

        const SceneReference& sr = this->m_scene.getSceneReference(handle);
        EXPECT_EQ(sceneId, sr.sceneId);
        EXPECT_EQ(RendererSceneState::Available, sr.requestedState);
        EXPECT_EQ(0, sr.renderOrder);
        EXPECT_FALSE(sr.flushNotifications);
    }

    TYPED_TEST(AScene, ReleasesSceneReference)
    {
        constexpr SceneId sceneId{ 123 };
        const auto handle = this->m_scene.allocateSceneReference(sceneId, {});

        this->m_scene.releaseSceneReference(handle);

        EXPECT_FALSE(this->m_scene.isSceneReferenceAllocated(handle));
    }

    TYPED_TEST(AScene, RequestsSceneReferenceState)
    {
        constexpr SceneId sceneId{ 123 };

        const auto handle = this->m_scene.allocateSceneReference(sceneId, {});
        const SceneReference& sr = this->m_scene.getSceneReference(handle);
        this->m_scene.requestSceneReferenceState(handle, RendererSceneState::Ready);

        EXPECT_EQ(RendererSceneState::Ready, sr.requestedState);
    }

    TYPED_TEST(AScene, SetsSceneReferenceRenderOrder)
    {
        constexpr SceneId sceneId{ 123 };

        const auto handle = this->m_scene.allocateSceneReference(sceneId, {});
        const SceneReference& sr = this->m_scene.getSceneReference(handle);
        this->m_scene.setSceneReferenceRenderOrder(handle, -8);

        EXPECT_EQ(-8, sr.renderOrder);
    }

    TYPED_TEST(AScene, RequestsSceneReferenceFlushNotifications)
    {
        constexpr SceneId sceneId{ 123 };

        const auto handle = this->m_scene.allocateSceneReference(sceneId, {});
        const SceneReference& sr = this->m_scene.getSceneReference(handle);

        this->m_scene.requestSceneReferenceFlushNotifications(handle, true);
        EXPECT_TRUE(sr.flushNotifications);

        this->m_scene.requestSceneReferenceFlushNotifications(handle, false);
        EXPECT_FALSE(sr.flushNotifications);
    }
}
