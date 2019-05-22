//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/BlitPass.h"
#include "ramses-client-api/RenderTargetDescription.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "RenderBufferImpl.h"
#include "ClientTestUtils.h"

using namespace testing;

namespace ramses
{
    class RenderBufferTest : public LocalTestClientWithScene, public testing::Test
    {
    protected:
        void useInRenderPass(const RenderBuffer& rb)
        {
            RenderTargetDescription rtDesc;
            rtDesc.addRenderBuffer(rb);
            RenderTarget* rt = this->m_scene.createRenderTarget(rtDesc);
            RenderPass* rp = this->m_scene.createRenderPass();
            OrthographicCamera* orthoCam = this->m_scene.createOrthographicCamera("camera");
            orthoCam->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
            orthoCam->setViewport(0, 0, 100, 200);
            rp->setCamera(*orthoCam);
            rp->setRenderTarget(rt);
        }

        void useInBlitPassAsSource(const RenderBuffer& rb)
        {
            const RenderBuffer* dummyRb = m_scene.createRenderBuffer(400u, 400u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
            this->m_scene.createBlitPass(rb, *dummyRb);
        }

        void useInBlitPassAsDestination(const RenderBuffer& rb)
        {
            const RenderBuffer* dummyRb = m_scene.createRenderBuffer(400u, 400u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
            this->m_scene.createBlitPass(*dummyRb, rb);
        }
    };

    TEST_F(RenderBufferTest, canCreateRenderBuffer)
    {
        const RenderBuffer* renderBuffer = m_scene.createRenderBuffer(600u, 400u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_WriteOnly, 4u, "RenderBuffer");
        ASSERT_TRUE(renderBuffer != NULL);
        EXPECT_EQ(600u, renderBuffer->getWidth());
        EXPECT_EQ(400u, renderBuffer->getHeight());
        EXPECT_EQ(ERenderBufferType_Color, renderBuffer->getBufferType());
        EXPECT_EQ(ERenderBufferFormat_RGBA8, renderBuffer->getBufferFormat());
        EXPECT_EQ(ERenderBufferAccessMode_WriteOnly, renderBuffer->getAccessMode());
        EXPECT_EQ(4u, renderBuffer->getSampleCount());
        const ramses_internal::RenderBufferHandle rbHandle = renderBuffer->impl.getRenderBufferHandle();
        EXPECT_TRUE(m_internalScene.isRenderBufferAllocated(rbHandle));
    }

    TEST_F(RenderBufferTest, failsToCreateRenderBufferWithZeroWidthOrHeight)
    {
        RenderBuffer* rbWithZeroWidth = m_scene.createRenderBuffer(0u, 400u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        EXPECT_EQ(NULL, rbWithZeroWidth);

        RenderBuffer* rbWithZeroHeight = m_scene.createRenderBuffer(400u, 0u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        EXPECT_EQ(NULL, rbWithZeroHeight);
    }

    TEST_F(RenderBufferTest, failsToCreateRenderBufferOfIncompatibleTypeAndFormat)
    {
        EXPECT_TRUE(NULL == m_scene.createRenderBuffer(1u, 1u, ERenderBufferType_Color, ERenderBufferFormat_Depth24, ERenderBufferAccessMode_ReadWrite));
        EXPECT_TRUE(NULL == m_scene.createRenderBuffer(1u, 1u, ERenderBufferType_Color, ERenderBufferFormat_Depth24_Stencil8, ERenderBufferAccessMode_ReadWrite));
        EXPECT_TRUE(NULL == m_scene.createRenderBuffer(1u, 1u, ERenderBufferType_Depth, ERenderBufferFormat_Depth24_Stencil8, ERenderBufferAccessMode_ReadWrite));
        EXPECT_TRUE(NULL == m_scene.createRenderBuffer(1u, 1u, ERenderBufferType_Depth, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite));
        EXPECT_TRUE(NULL == m_scene.createRenderBuffer(1u, 1u, ERenderBufferType_DepthStencil, ERenderBufferFormat_Depth24, ERenderBufferAccessMode_ReadWrite));
        EXPECT_TRUE(NULL == m_scene.createRenderBuffer(1u, 1u, ERenderBufferType_DepthStencil, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite));
    }

    TEST_F(RenderBufferTest, canNotCreateReadWriteMSAARenderBuffer)
    {
        const RenderBuffer* renderBuffer = m_scene.createRenderBuffer(400u, 400u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite, 4u);
        EXPECT_EQ(NULL, renderBuffer);
    }

    TEST_F(RenderBufferTest, reportsErrorIfNotUsedInAnyRenderPassNorBlitPass)
    {
        const RenderBuffer* renderBuffer = m_scene.createRenderBuffer(400u, 400u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(renderBuffer != nullptr);
        EXPECT_NE(StatusOK, renderBuffer->validate());
    }

    TEST_F(RenderBufferTest, reportsErrorIfUsedInRenderPassButNotReferencedByAnySamplerNorUsedAsBlitPassSource)
    {
        const RenderBuffer* renderBuffer = m_scene.createRenderBuffer(400u, 400u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(renderBuffer != nullptr);
        useInRenderPass(*renderBuffer);
        EXPECT_NE(StatusOK, renderBuffer->validate());
    }

    TEST_F(RenderBufferTest, reportsErrorIfReferencedBySamplerButNotUsedInAnyRenderPassNorUsedAsBlitPassDestination)
    {
        const RenderBuffer* renderBuffer = m_scene.createRenderBuffer(400u, 400u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(renderBuffer != nullptr);
        m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Nearest, *renderBuffer);
        EXPECT_NE(StatusOK, renderBuffer->validate());
    }

    TEST_F(RenderBufferTest, validatesWhenUsedInRenderPassAndReferencedBySampler)
    {
        const RenderBuffer* renderBuffer = m_scene.createRenderBuffer(400u, 400u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(renderBuffer != nullptr);
        useInRenderPass(*renderBuffer);
        m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Nearest, *renderBuffer);
        EXPECT_EQ(StatusOK, renderBuffer->validate());
    }

    TEST_F(RenderBufferTest, validatesWhenUsedAsBlitPassDestinationAndReferencedBySampler)
    {
        const RenderBuffer* renderBuffer = m_scene.createRenderBuffer(400u, 400u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(renderBuffer != nullptr);
        useInBlitPassAsDestination(*renderBuffer);
        m_scene.createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Nearest, ETextureSamplingMethod_Nearest, *renderBuffer);
        EXPECT_EQ(StatusOK, renderBuffer->validate());
    }

    TEST_F(RenderBufferTest, validatesWhenNOTReferencedBySamplerButUsedAsBlitPassSource)
    {
        const RenderBuffer* renderBuffer = m_scene.createRenderBuffer(400u, 400u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(renderBuffer != nullptr);
        useInRenderPass(*renderBuffer);
        useInBlitPassAsSource(*renderBuffer);
        EXPECT_EQ(StatusOK, renderBuffer->validate());
    }

    TEST_F(RenderBufferTest, doesNotReportsErrorIfUsedInRenderPassButNotReferencedByAnySampler_DepthBufferType)
    {
        const RenderBuffer* renderBuffer = m_scene.createRenderBuffer(400u, 400u, ERenderBufferType_Depth, ERenderBufferFormat_Depth24, ERenderBufferAccessMode_ReadWrite);
        ASSERT_TRUE(renderBuffer != nullptr);
        useInRenderPass(*renderBuffer);
        EXPECT_EQ(StatusOK, renderBuffer->validate());
    }
}
