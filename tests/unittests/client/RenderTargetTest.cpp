//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ramses/client/RenderBuffer.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/RenderTargetDescription.h"
#include "impl/RenderTargetImpl.h"
#include "impl/RenderBufferImpl.h"
#include "ClientTestUtils.h"

using namespace testing;

namespace ramses::internal
{
    class ARenderTarget : public LocalTestClientWithScene, public testing::Test
    {
    };

    TEST_F(ARenderTarget, canBeCreated)
    {
        const ramses::RenderBuffer& rb = *m_scene.createRenderBuffer(16u, 8u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
        RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(rb);

        const ramses::RenderTarget* renderTarget = m_scene.createRenderTarget(rtDesc, "ramses::RenderTarget");
        ASSERT_TRUE(renderTarget != nullptr);
        EXPECT_EQ(16u, renderTarget->getWidth());
        EXPECT_EQ(8u, renderTarget->getHeight());

        const RenderTargetHandle rtHandle = renderTarget->impl().getRenderTargetHandle();
        EXPECT_TRUE(m_internalScene.isRenderTargetAllocated(rtHandle));
        ASSERT_EQ(1u, m_internalScene.getRenderTargetRenderBufferCount(rtHandle));
        EXPECT_EQ(rb.impl().getRenderBufferHandle(), m_internalScene.getRenderTargetRenderBuffer(rtHandle, 0u));

        ValidationReport report;
        renderTarget->validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(ARenderTarget, failsToCreateUsingInvalidRenderTargetDescription)
    {
        RenderTargetDescription invalidRtDesc; // no buffers

        const ramses::RenderTarget* renderTarget = m_scene.createRenderTarget(invalidRtDesc, "RenderTarget");
        EXPECT_TRUE(renderTarget == nullptr);
    }

    TEST_F(ARenderTarget, validatesWithErrorIfMismatchingRenderBufferProperties)
    {
        ramses::RenderBuffer& rb1 = *m_scene.createRenderBuffer(16u, 8u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite, 2u);
        ramses::RenderBuffer& rb2 = *m_scene.createRenderBuffer(16u, 8u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite, 2u);
        RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(rb1);
        rtDesc.addRenderBuffer(rb2);

        const ramses::RenderTarget* renderTarget = m_scene.createRenderTarget(rtDesc, "rt");
        ASSERT_TRUE(renderTarget != nullptr);

        ValidationReport report;
        renderTarget->validate(report);
        EXPECT_FALSE(report.hasIssue());

        // change rb1 properties
        EXPECT_TRUE(rb1.impl().setProperties(1u, 8u, 2u));
        report.clear();
        renderTarget->validate(report);
        ASSERT_TRUE(report.hasError());
        EXPECT_EQ(renderTarget, report.getIssues().front().object);
        EXPECT_EQ(EIssueType::Error, report.getIssues().front().type);
        EXPECT_EQ("RenderTarget uses render buffers with mismatching resolution and/or sample count. This RenderTarget cannot be used and attempt to flush scene will fail.",
            report.getIssues().front().message);

        EXPECT_TRUE(rb1.impl().setProperties(16u, 1u, 2u));
        report.clear();
        renderTarget->validate(report);
        EXPECT_TRUE(report.hasError());

        EXPECT_TRUE(rb1.impl().setProperties(16u, 8u, 1u));
        report.clear();
        renderTarget->validate(report);
        EXPECT_TRUE(report.hasError());

        // reset to valid properties
        EXPECT_TRUE(rb1.impl().setProperties(16u, 8u, 2u));
        report.clear();
        renderTarget->validate(report);
        EXPECT_FALSE(report.hasError());

        // change rb2 properties
        EXPECT_TRUE(rb2.impl().setProperties(1u, 8u, 2u));
        report.clear();
        renderTarget->validate(report);
        EXPECT_TRUE(report.hasError());

        EXPECT_TRUE(rb2.impl().setProperties(16u, 1u, 2u));
        report.clear();
        renderTarget->validate(report);
        EXPECT_TRUE(report.hasError());

        EXPECT_TRUE(rb2.impl().setProperties(16u, 8u, 1u));
        report.clear();
        renderTarget->validate(report);
        EXPECT_TRUE(report.hasError());

        // reset to valid properties
        EXPECT_TRUE(rb2.impl().setProperties(16u, 8u, 2u));
        report.clear();
        renderTarget->validate(report);
        EXPECT_FALSE(report.hasError());
    }
}
