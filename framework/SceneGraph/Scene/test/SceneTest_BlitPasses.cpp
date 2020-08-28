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

    TYPED_TEST(AScene, BlitPassCreated)
    {
        EXPECT_EQ(0u, this->m_scene.getBlitPassCount());

        BlitPassHandle pass = this->m_scene.allocateBlitPass(RenderBufferHandle(1u), RenderBufferHandle(2u));

        EXPECT_EQ(1u, this->m_scene.getBlitPassCount());
        EXPECT_TRUE(this->m_scene.isBlitPassAllocated(pass));
    }

    TYPED_TEST(AScene, BlitPassReleased)
    {
        BlitPassHandle pass = this->m_scene.allocateBlitPass(RenderBufferHandle(1u), RenderBufferHandle(2u));
        this->m_scene.releaseBlitPass(pass);

        EXPECT_FALSE(this->m_scene.isBlitPassAllocated(pass));
    }

    TYPED_TEST(AScene, DoesNotContainBlitPassWhichWasNotCreated)
    {
        EXPECT_FALSE(this->m_scene.isBlitPassAllocated(BlitPassHandle(1)));
    }

    TYPED_TEST(AScene, BlitPassCanGetSourceAndDestinationRenderBuffers)
    {
        const RenderBufferHandle sourceRenderBuffer = this->m_scene.allocateRenderBuffer({ 1u, 2u, ERenderBufferType_DepthBuffer, ETextureFormat::Depth16, ERenderBufferAccessMode_ReadWrite, 0u });
        const RenderBufferHandle destinationRenderBuffer = this->m_scene.allocateRenderBuffer({ 1u, 2u, ERenderBufferType_DepthBuffer, ETextureFormat::Depth16, ERenderBufferAccessMode_ReadWrite, 0u });

        BlitPassHandle pass = this->m_scene.allocateBlitPass(sourceRenderBuffer, destinationRenderBuffer);
        EXPECT_EQ(sourceRenderBuffer, this->m_scene.getBlitPass(pass).sourceRenderBuffer);
        EXPECT_EQ(destinationRenderBuffer, this->m_scene.getBlitPass(pass).destinationRenderBuffer);
    }

    TYPED_TEST(AScene, BlitPassCanGetSourceAndDestinationRegions)
    {
        const PixelRectangle sourceRegion =
        {
            1u, 2u, 3u, 4u
        };

        const PixelRectangle destinationRegion =
        {
            5u, 6u, 7u, 8u
        };

        BlitPassHandle pass = this->m_scene.allocateBlitPass(RenderBufferHandle(1u), RenderBufferHandle(2u));
        this->m_scene.setBlitPassRegions(pass, sourceRegion, destinationRegion);
        const BlitPass& blitPass = this->m_scene.getBlitPass(pass);

        EXPECT_EQ(sourceRegion.x,      blitPass.sourceRegion.x);
        EXPECT_EQ(sourceRegion.y,      blitPass.sourceRegion.y);
        EXPECT_EQ(sourceRegion.width,  blitPass.sourceRegion.width);
        EXPECT_EQ(sourceRegion.height, blitPass.sourceRegion.height);

        EXPECT_EQ(destinationRegion.x,      blitPass.destinationRegion.x);
        EXPECT_EQ(destinationRegion.y,      blitPass.destinationRegion.y);
        EXPECT_EQ(destinationRegion.width,  blitPass.destinationRegion.width);
        EXPECT_EQ(destinationRegion.height, blitPass.destinationRegion.height);
    }

    TYPED_TEST(AScene, CanDisableBlitPass)
    {
        BlitPassHandle pass = this->m_scene.allocateBlitPass(RenderBufferHandle(1u), RenderBufferHandle(2u));
        EXPECT_TRUE(this->m_scene.getBlitPass(pass).isEnabled);

        this->m_scene.setBlitPassEnabled(pass, false);
        EXPECT_FALSE(this->m_scene.getBlitPass(pass).isEnabled);
    }

    TYPED_TEST(AScene, createsBlitPassWithOrderZeroAndSetsOrderTwice)
    {
        BlitPassHandle pass = this->m_scene.allocateBlitPass(RenderBufferHandle(1u), RenderBufferHandle(2u));
        EXPECT_EQ(0, this->m_scene.getBlitPass(pass).renderOrder);

        this->m_scene.setBlitPassRenderOrder(pass, 42);
        EXPECT_EQ(42, this->m_scene.getBlitPass(pass).renderOrder);

        this->m_scene.setBlitPassRenderOrder(pass, 0);
        EXPECT_EQ(0, this->m_scene.getBlitPass(pass).renderOrder);
    }
}
