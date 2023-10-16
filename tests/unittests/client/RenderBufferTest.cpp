//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ramses/client/RenderBuffer.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/BlitPass.h"
#include "ramses/client/RenderTargetDescription.h"
#include "ramses/client/OrthographicCamera.h"
#include "impl/RenderBufferImpl.h"
#include "ClientTestUtils.h"

using namespace testing;

namespace ramses::internal
{
    class RenderBufferTest : public LocalTestClientWithScene, public testing::Test
    {
    protected:
        void useInRenderPass(const ramses::RenderBuffer& rb)
        {
            RenderTargetDescription rtDesc;
            rtDesc.addRenderBuffer(rb);
            ramses::RenderTarget* rt = this->m_scene.createRenderTarget(rtDesc);
            ramses::RenderPass* rp = this->m_scene.createRenderPass();
            OrthographicCamera* orthoCam = this->m_scene.createOrthographicCamera("camera");
            orthoCam->setFrustum(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f);
            orthoCam->setViewport(0, 0, 100, 200);
            rp->setCamera(*orthoCam);
            rp->setRenderTarget(rt);
        }

        void useInBlitPassAsSource(const ramses::RenderBuffer& rb)
        {
            const ramses::RenderBuffer* dummyRb = m_scene.createRenderBuffer(400u, 400u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
            this->m_scene.createBlitPass(rb, *dummyRb);
        }

        void useInBlitPassAsDestination(const ramses::RenderBuffer& rb)
        {
            const ramses::RenderBuffer* dummyRb = m_scene.createRenderBuffer(400u, 400u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
            this->m_scene.createBlitPass(*dummyRb, rb);
        }
    };

    TEST_F(RenderBufferTest, canCreateRenderBuffer)
    {
        const ramses::RenderBuffer* renderBuffer = m_scene.createRenderBuffer(600u, 400u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::WriteOnly, 4u, "RenderBuffer");
        ASSERT_TRUE(renderBuffer != nullptr);
        EXPECT_EQ(600u, renderBuffer->getWidth());
        EXPECT_EQ(400u, renderBuffer->getHeight());
        EXPECT_EQ(ERenderBufferFormat::RGBA8, renderBuffer->getBufferFormat());
        EXPECT_EQ(ERenderBufferAccessMode::WriteOnly, renderBuffer->getAccessMode());
        EXPECT_EQ(4u, renderBuffer->getSampleCount());
        const ramses::internal::RenderBufferHandle rbHandle = renderBuffer->impl().getRenderBufferHandle();
        EXPECT_TRUE(m_internalScene.isRenderBufferAllocated(rbHandle));
    }

    TEST_F(RenderBufferTest, failsToCreateRenderBufferWithZeroWidthOrHeight)
    {
        ramses::RenderBuffer* rbWithZeroWidth = m_scene.createRenderBuffer(0u, 400u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        EXPECT_EQ(nullptr, rbWithZeroWidth);

        ramses::RenderBuffer* rbWithZeroHeight = m_scene.createRenderBuffer(400u, 0u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        EXPECT_EQ(nullptr, rbWithZeroHeight);
    }

    TEST_F(RenderBufferTest, canCreateReadWriteMSAARenderBuffer)
    {
        const ramses::RenderBuffer* renderBuffer = m_scene.createRenderBuffer(600u, 400u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite, 4u, "RenderBuffer");
        ASSERT_TRUE(renderBuffer != nullptr);
        EXPECT_EQ(600u, renderBuffer->getWidth());
        EXPECT_EQ(400u, renderBuffer->getHeight());
        EXPECT_EQ(ERenderBufferFormat::RGBA8, renderBuffer->getBufferFormat());
        EXPECT_EQ(ERenderBufferAccessMode::ReadWrite, renderBuffer->getAccessMode());
        EXPECT_EQ(4u, renderBuffer->getSampleCount());
        const ramses::internal::RenderBufferHandle rbHandle = renderBuffer->impl().getRenderBufferHandle();
        EXPECT_TRUE(m_internalScene.isRenderBufferAllocated(rbHandle));
    }

    TEST_F(RenderBufferTest, reportsWarningIfNotUsedInAnyRenderPassNorBlitPass)
    {
        const ramses::RenderBuffer* renderBuffer = m_scene.createRenderBuffer(400u, 400u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        ASSERT_TRUE(renderBuffer != nullptr);
        ValidationReport report;
        renderBuffer->validate(report);
        EXPECT_TRUE(report.hasIssue());
    }

    TEST_F(RenderBufferTest, reportsWarningIfUsedInRenderPassButNotReferencedByAnySamplerNorUsedAsBlitPassSource)
    {
        const ramses::RenderBuffer* renderBuffer = m_scene.createRenderBuffer(400u, 400u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        ASSERT_TRUE(renderBuffer != nullptr);
        useInRenderPass(*renderBuffer);
        ValidationReport report;
        renderBuffer->validate(report);
        EXPECT_TRUE(report.hasIssue());
    }

    TEST_F(RenderBufferTest, reportsWarningIfReferencedBySamplerButNotUsedInAnyRenderPassNorUsedAsBlitPassDestination)
    {
        const ramses::RenderBuffer* renderBuffer = m_scene.createRenderBuffer(400u, 400u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        ASSERT_TRUE(renderBuffer != nullptr);
        m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Nearest, *renderBuffer);
        ValidationReport report;
        renderBuffer->validate(report);
        EXPECT_TRUE(report.hasIssue());
    }

    TEST_F(RenderBufferTest, validatesWhenUsedInRenderPassAndReferencedBySampler)
    {
        const ramses::RenderBuffer* renderBuffer = m_scene.createRenderBuffer(400u, 400u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        ASSERT_TRUE(renderBuffer != nullptr);
        useInRenderPass(*renderBuffer);
        m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Nearest, *renderBuffer);
        ValidationReport report;
        renderBuffer->validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(RenderBufferTest, validatesWhenUsedAsBlitPassDestinationAndReferencedBySampler)
    {
        const ramses::RenderBuffer* renderBuffer = m_scene.createRenderBuffer(400u, 400u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        ASSERT_TRUE(renderBuffer != nullptr);
        useInBlitPassAsDestination(*renderBuffer);
        m_scene.createTextureSampler(ETextureAddressMode::Clamp, ETextureAddressMode::Clamp, ETextureSamplingMethod::Nearest, ETextureSamplingMethod::Nearest, *renderBuffer);
        ValidationReport report;
        renderBuffer->validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(RenderBufferTest, validatesWhenNOTReferencedBySamplerButUsedAsBlitPassSource)
    {
        const ramses::RenderBuffer* renderBuffer = m_scene.createRenderBuffer(400u, 400u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        ASSERT_TRUE(renderBuffer != nullptr);
        useInRenderPass(*renderBuffer);
        useInBlitPassAsSource(*renderBuffer);
        ValidationReport report;
        renderBuffer->validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(RenderBufferTest, doesNotReportsErrorIfUsedInRenderPassButNotReferencedByAnySampler_DepthBufferType)
    {
        const ramses::RenderBuffer* renderBuffer = m_scene.createRenderBuffer(400u, 400u, ERenderBufferFormat::Depth24, ERenderBufferAccessMode::ReadWrite);
        ASSERT_TRUE(renderBuffer != nullptr);
        useInRenderPass(*renderBuffer);
        ValidationReport report;
        renderBuffer->validate(report);
        EXPECT_FALSE(report.hasIssue());
    }
}
