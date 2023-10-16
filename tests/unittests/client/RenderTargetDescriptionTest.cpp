//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/RenderTargetDescription.h"
#include "impl/RenderBufferImpl.h"
#include "impl/RenderTargetDescriptionImpl.h"
#include "ClientTestUtils.h"

using namespace testing;

namespace ramses::internal
{
    class RenderTargetDescriptionTest : public LocalTestClientWithScene, public testing::Test
    {
    public:
    protected:
        RenderTargetDescription rtDesc;
        std::string errorMsg;
    };

    TEST_F(RenderTargetDescriptionTest, initialState)
    {
        EXPECT_TRUE(rtDesc.impl().getRenderBuffers().empty());
    }

    TEST_F(RenderTargetDescriptionTest, validatesAsErrorIfEmpty)
    {
        ValidationReport report;
        rtDesc.validate(report);
        EXPECT_TRUE(report.hasError());
    }

    TEST_F(RenderTargetDescriptionTest, canAddRenderBuffer)
    {
        const ramses::RenderBuffer& colorRb = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        EXPECT_TRUE(rtDesc.addRenderBuffer(colorRb, &errorMsg));
        EXPECT_TRUE(errorMsg.empty());

        ASSERT_EQ(1u, rtDesc.impl().getRenderBuffers().size());
        EXPECT_EQ(colorRb.impl().getRenderBufferHandle(), rtDesc.impl().getRenderBuffers()[0]);

        ValidationReport report;
        rtDesc.validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(RenderTargetDescriptionTest, canAddMultipleColorRenderBuffers)
    {
        const ramses::RenderBuffer& colorRb1 = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        const ramses::RenderBuffer& colorRb2 = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        EXPECT_TRUE(rtDesc.addRenderBuffer(colorRb1));
        EXPECT_TRUE(rtDesc.addRenderBuffer(colorRb2));

        ASSERT_EQ(2u, rtDesc.impl().getRenderBuffers().size());
        EXPECT_EQ(colorRb1.impl().getRenderBufferHandle(), rtDesc.impl().getRenderBuffers()[0]);
        EXPECT_EQ(colorRb2.impl().getRenderBufferHandle(), rtDesc.impl().getRenderBuffers()[1]);

        ValidationReport report;
        rtDesc.validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(RenderTargetDescriptionTest, canAddMultipleColorRenderBuffersWithDepthBuffer)
    {
        const ramses::RenderBuffer& colorRb1 = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        const ramses::RenderBuffer& colorRb2 = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        const ramses::RenderBuffer& depthRb = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::Depth32, ERenderBufferAccessMode::ReadWrite);
        EXPECT_TRUE(rtDesc.addRenderBuffer(colorRb1));
        EXPECT_TRUE(rtDesc.addRenderBuffer(colorRb2));
        EXPECT_TRUE(rtDesc.addRenderBuffer(depthRb));

        ASSERT_EQ(3u, rtDesc.impl().getRenderBuffers().size());
        EXPECT_EQ(colorRb1.impl().getRenderBufferHandle(), rtDesc.impl().getRenderBuffers()[0]);
        EXPECT_EQ(colorRb2.impl().getRenderBufferHandle(), rtDesc.impl().getRenderBuffers()[1]);
        EXPECT_EQ(depthRb.impl().getRenderBufferHandle(), rtDesc.impl().getRenderBuffers()[2]);

        ValidationReport report;
        rtDesc.validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(RenderTargetDescriptionTest, clearsErrorMessageIfSucceeded)
    {
        errorMsg = "foo";
        const ramses::RenderBuffer& colorRb = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        EXPECT_TRUE(rtDesc.addRenderBuffer(colorRb, &errorMsg));
        EXPECT_TRUE(errorMsg.empty());
    }

    TEST_F(RenderTargetDescriptionTest, failsToAddSameRenderBufferTwice)
    {
        const ramses::RenderBuffer& colorRb = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        EXPECT_TRUE(rtDesc.addRenderBuffer(colorRb, &errorMsg));
        EXPECT_TRUE(errorMsg.empty());
        EXPECT_FALSE(rtDesc.addRenderBuffer(colorRb, &errorMsg));
        EXPECT_EQ(errorMsg, "RenderTargetDescription::addRenderBuffer failed: trying to add a render buffer that is already contained!");

        ASSERT_EQ(1u, rtDesc.impl().getRenderBuffers().size());
        EXPECT_EQ(colorRb.impl().getRenderBufferHandle(), rtDesc.impl().getRenderBuffers()[0]);
    }

    TEST_F(RenderTargetDescriptionTest, failsToAddRenderBufferFromAnotherScene)
    {
        const ramses::RenderBuffer& colorRb = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        EXPECT_TRUE(rtDesc.addRenderBuffer(colorRb, &errorMsg));
        EXPECT_TRUE(errorMsg.empty());

        ramses::Scene& otherScene = *client.createScene(sceneId_t(666u));
        otherScene.createRenderBuffer(1u, 1u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite); // create another dummy RB so the internal handle is not 1
        const ramses::RenderBuffer& otherRb = *otherScene.createRenderBuffer(1u, 1u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        EXPECT_FALSE(rtDesc.addRenderBuffer(otherRb, &errorMsg));
        EXPECT_EQ(errorMsg, "RenderTargetDescription::addRenderBuffer failed: all render buffers must be from the same scene!");

        ASSERT_EQ(1u, rtDesc.impl().getRenderBuffers().size());
        EXPECT_EQ(colorRb.impl().getRenderBufferHandle(), rtDesc.impl().getRenderBuffers()[0]);
    }

    TEST_F(RenderTargetDescriptionTest, failsToAddRenderBufferWithDifferentResolution)
    {
        const ramses::RenderBuffer& colorRb = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        EXPECT_TRUE(rtDesc.addRenderBuffer(colorRb, &errorMsg));
        EXPECT_TRUE(errorMsg.empty());

        const ramses::RenderBuffer& otherRb = *m_scene.createRenderBuffer(1u, 2u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        EXPECT_FALSE(rtDesc.addRenderBuffer(otherRb, &errorMsg));
        EXPECT_EQ(errorMsg, "RenderTargetDescription::addRenderBuffer failed: all render buffers must have the same resolution!");

        ASSERT_EQ(1u, rtDesc.impl().getRenderBuffers().size());
        EXPECT_EQ(colorRb.impl().getRenderBufferHandle(), rtDesc.impl().getRenderBuffers()[0]);
    }

    TEST_F(RenderTargetDescriptionTest, failsToAddMoreThanOneDepthBuffer)
    {
        const ramses::RenderBuffer& colorRb = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        const ramses::RenderBuffer& depthRb1 = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::Depth32, ERenderBufferAccessMode::ReadWrite);
        const ramses::RenderBuffer& depthRb2 = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::Depth24_Stencil8, ERenderBufferAccessMode::ReadWrite);
        EXPECT_TRUE(rtDesc.addRenderBuffer(colorRb, &errorMsg));
        EXPECT_TRUE(errorMsg.empty());
        EXPECT_TRUE(rtDesc.addRenderBuffer(depthRb1, &errorMsg));
        EXPECT_TRUE(errorMsg.empty());
        EXPECT_FALSE(rtDesc.addRenderBuffer(depthRb2, &errorMsg));
        EXPECT_EQ(errorMsg, "RenderTargetDescription::addRenderBuffer failed: cannot add more than one depth/stencil buffer!");

        ASSERT_EQ(2u, rtDesc.impl().getRenderBuffers().size());
        EXPECT_EQ(colorRb.impl().getRenderBufferHandle(), rtDesc.impl().getRenderBuffers()[0]);
        EXPECT_EQ(depthRb1.impl().getRenderBufferHandle(), rtDesc.impl().getRenderBuffers()[1]);
    }

    TEST_F(RenderTargetDescriptionTest, failsToValidateAfterAddedRenderBufferDestroyed)
    {
        ramses::RenderBuffer& colorRb = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        EXPECT_TRUE(rtDesc.addRenderBuffer(colorRb));
        ValidationReport report;
        rtDesc.validate(report);
        EXPECT_FALSE(report.hasIssue());

        m_scene.destroy(colorRb);
        rtDesc.validate(report);
        EXPECT_TRUE(report.hasError());
    }

    TEST_F(RenderTargetDescriptionTest, canAddRenderBuffersWithMsaaSampleCountNotZero)
    {
        const ramses::RenderBuffer& colorRb = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::WriteOnly, 4u);
        const ramses::RenderBuffer& depthRb = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::Depth16, ERenderBufferAccessMode::WriteOnly, 4u);
        EXPECT_TRUE(rtDesc.addRenderBuffer(colorRb));
        EXPECT_TRUE(rtDesc.addRenderBuffer(depthRb));

        ASSERT_EQ(2u, rtDesc.impl().getRenderBuffers().size());
        EXPECT_EQ(colorRb.impl().getRenderBufferHandle(), rtDesc.impl().getRenderBuffers()[0]);
        EXPECT_EQ(depthRb.impl().getRenderBufferHandle(), rtDesc.impl().getRenderBuffers()[1]);
    }

    TEST_F(RenderTargetDescriptionTest, canNotAddRenderBuffersWithDifferentMsaaSampleCount)
    {
        const ramses::RenderBuffer& colorRb = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::WriteOnly, 3u);
        const ramses::RenderBuffer& depthRb = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::Depth24, ERenderBufferAccessMode::WriteOnly, 4u);
        EXPECT_TRUE(rtDesc.addRenderBuffer(colorRb, &errorMsg));
        EXPECT_TRUE(errorMsg.empty());
        EXPECT_FALSE(rtDesc.addRenderBuffer(depthRb, &errorMsg));
        EXPECT_EQ(errorMsg, "RenderTargetDescription::addRenderBuffer failed: all render buffers must have same MSAA sample count!");

        ASSERT_EQ(1u, rtDesc.impl().getRenderBuffers().size());
        EXPECT_EQ(colorRb.impl().getRenderBufferHandle(), rtDesc.impl().getRenderBuffers()[0]);
    }

    TEST_F(RenderTargetDescriptionTest, CanBeCopyAndMoveConstructed)
    {
        const ramses::RenderBuffer& colorRb = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        EXPECT_TRUE(rtDesc.addRenderBuffer(colorRb));

        RenderTargetDescription rtDescCopy{ rtDesc };
        EXPECT_THAT(rtDescCopy.impl().getRenderBuffers(), ElementsAre(colorRb.impl().getRenderBufferHandle()));

        RenderTargetDescription rtDescMove{ std::move(rtDesc) };
        EXPECT_THAT(rtDescMove.impl().getRenderBuffers(), ElementsAre(colorRb.impl().getRenderBufferHandle()));
    }

    TEST_F(RenderTargetDescriptionTest, CanBeCopyAndMoveAssigned)
    {
        const ramses::RenderBuffer& colorRb = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        EXPECT_TRUE(rtDesc.addRenderBuffer(colorRb));

        RenderTargetDescription rtDescCopy;
        rtDescCopy = rtDesc;
        EXPECT_THAT(rtDescCopy.impl().getRenderBuffers(), ElementsAre(colorRb.impl().getRenderBufferHandle()));

        RenderTargetDescription rtDescMove;
        rtDescMove = std::move(rtDesc);
        EXPECT_THAT(rtDescMove.impl().getRenderBuffers(), ElementsAre(colorRb.impl().getRenderBufferHandle()));
    }

    TEST_F(RenderTargetDescriptionTest, CanBeSelfAssigned)
    {
        const ramses::RenderBuffer& colorRb = *m_scene.createRenderBuffer(1u, 1u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        EXPECT_TRUE(rtDesc.addRenderBuffer(colorRb));

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
        rtDesc = rtDesc;
        EXPECT_THAT(rtDesc.impl().getRenderBuffers(), ElementsAre(colorRb.impl().getRenderBufferHandle()));
        rtDesc = std::move(rtDesc);
        // NOLINTNEXTLINE(bugprone-use-after-move)
        EXPECT_THAT(rtDesc.impl().getRenderBuffers(), ElementsAre(colorRb.impl().getRenderBufferHandle()));
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    }
}
