//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "SceneTest.h"

using namespace testing;

namespace ramses_internal
{
    TYPED_TEST_SUITE(AScene, SceneTypes);

    TYPED_TEST(AScene, CreateMultipleTargetsAndCheckTheirValidity)
    {
        EXPECT_EQ(0u, this->m_scene.getRenderTargetCount());

        RenderTargetHandle targetHandle1 = this->m_scene.allocateRenderTarget();

        EXPECT_NE(RenderTargetHandle::Invalid(), targetHandle1);
        EXPECT_TRUE(this->m_scene.isRenderTargetAllocated(targetHandle1));

        RenderTargetHandle targetHandle2 = this->m_scene.allocateRenderTarget();

        EXPECT_NE(RenderTargetHandle::Invalid(), targetHandle2);
        EXPECT_TRUE(this->m_scene.isRenderTargetAllocated(targetHandle2));
    }

    TYPED_TEST(AScene, DeleteRenderTargetsAndCheckScene)
    {
        RenderTargetHandle targetHandle = this->m_scene.allocateRenderTarget();

        EXPECT_NE(RenderTargetHandle::Invalid(), targetHandle);
        EXPECT_TRUE(this->m_scene.isRenderTargetAllocated(targetHandle));

        this->m_scene.releaseRenderTarget( targetHandle );

        EXPECT_FALSE(this->m_scene.isRenderTargetAllocated(targetHandle));
    }

    TYPED_TEST(AScene, ContainsRenderTargetInSceneKnowsExactlyAboutAllCreatedRenderTargets)
    {
        RenderTargetHandle targetHandle = this->m_scene.allocateRenderTarget();

        EXPECT_NE(RenderTargetHandle::Invalid(), targetHandle);
        EXPECT_TRUE(this->m_scene.isRenderTargetAllocated(targetHandle));
        EXPECT_FALSE(this->m_scene.isRenderTargetAllocated(targetHandle+10u));
        EXPECT_FALSE(this->m_scene.isRenderTargetAllocated(RenderTargetHandle::Invalid()));
    }

    TYPED_TEST(AScene, canAddRenderBufferToRenderTarget)
    {
        const RenderTargetHandle targetHandle = this->m_scene.allocateRenderTarget();
        EXPECT_EQ(0u, this->m_scene.getRenderTargetRenderBufferCount(targetHandle));

        const RenderBufferHandle bufferHandle(44u);
        this->m_scene.addRenderTargetRenderBuffer(targetHandle, bufferHandle);

        EXPECT_EQ(1u, this->m_scene.getRenderTargetRenderBufferCount(targetHandle));
        EXPECT_EQ(bufferHandle, this->m_scene.getRenderTargetRenderBuffer(targetHandle, 0u));
    }
}
