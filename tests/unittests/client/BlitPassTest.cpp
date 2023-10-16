//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ClientTestUtils.h"
#include "ramses/client/BlitPass.h"
#include "ramses/client/RenderBuffer.h"
#include "impl/SceneImpl.h"
#include "impl/BlitPassImpl.h"
#include "impl/RenderBufferImpl.h"
#include "ramses/client/ramses-utils.h"
#include "internal/SceneGraph/Scene/ClientScene.h"

using namespace testing;

namespace ramses::internal
{
    class ABlitPass : public LocalTestClientWithScene, public testing::Test
    {
    protected:
        ABlitPass()
            : sourceRenderBuffer(*createRenderBuffer())
            , destinationRenderBuffer(*createRenderBuffer())
            , blitpass(*m_scene.createBlitPass(sourceRenderBuffer, destinationRenderBuffer, "BlitPass"))
            , blitpassHandle(blitpass.impl().getBlitPassHandle())
        {
        }

        ramses::RenderBuffer* createRenderBuffer()
        {
            ramses::RenderBuffer* renderBuffer = m_scene.createRenderBuffer(16u, 12u, ERenderBufferFormat::RGBA8, ERenderBufferAccessMode::ReadWrite);
            return renderBuffer;
        }

        ramses::RenderBuffer& sourceRenderBuffer;
        ramses::RenderBuffer& destinationRenderBuffer;
        ramses::BlitPass& blitpass;
        const ramses::internal::BlitPassHandle blitpassHandle;
    };

    TEST_F(ABlitPass, CanGetSourceAndDestinationBuffers)
    {
        const ramses::RenderBuffer& srcRB = blitpass.getSourceRenderBuffer();
        EXPECT_EQ(srcRB.impl().getRenderBufferHandle(), sourceRenderBuffer.impl().getRenderBufferHandle());

        const ramses::RenderBuffer& dstRB = blitpass.getDestinationRenderBuffer();
        EXPECT_EQ(dstRB.impl().getRenderBufferHandle(), destinationRenderBuffer.impl().getRenderBufferHandle());
    }

    TEST_F(ABlitPass, BlittingRegionIsSetToCompleteBufferByDefault)
    {
        const ramses::internal::PixelRectangle& sourceRegion = m_scene.impl().getIScene().getBlitPass(blitpassHandle).sourceRegion;
        EXPECT_EQ(0u, sourceRegion.x);
        EXPECT_EQ(0u, sourceRegion.y);
        EXPECT_EQ(static_cast<int32_t>(sourceRenderBuffer.getWidth()), sourceRegion.width);
        EXPECT_EQ(static_cast<int32_t>(sourceRenderBuffer.getHeight()), sourceRegion.height);

        const ramses::internal::PixelRectangle& destinationRegion = m_scene.impl().getIScene().getBlitPass(blitpassHandle).destinationRegion;
        EXPECT_EQ(0u, destinationRegion.x);
        EXPECT_EQ(0u, destinationRegion.y);
        EXPECT_EQ(static_cast<int32_t>(destinationRenderBuffer.getWidth()), destinationRegion.width);
        EXPECT_EQ(static_cast<int32_t>(destinationRenderBuffer.getHeight()), destinationRegion.height);

        {
            //client HL api
            uint32_t sourceXOut = std::numeric_limits<uint32_t>::max();
            uint32_t sourceYOut = std::numeric_limits<uint32_t>::max();
            uint32_t destinationXOut = std::numeric_limits<uint32_t>::max();
            uint32_t destinationYOut = std::numeric_limits<uint32_t>::max();
            uint32_t widthOut = std::numeric_limits<uint32_t>::max();
            uint32_t heightOut = std::numeric_limits<uint32_t>::max();
            blitpass.getBlittingRegion(sourceXOut, sourceYOut, destinationXOut, destinationYOut, widthOut, heightOut);
            EXPECT_EQ(0u, sourceXOut);
            EXPECT_EQ(0u, sourceYOut);
            EXPECT_EQ(0u, destinationXOut);
            EXPECT_EQ(0u, destinationYOut);
            EXPECT_EQ(sourceRenderBuffer.getWidth(), widthOut);
            EXPECT_EQ(sourceRenderBuffer.getHeight(), heightOut);
        }
    }

    TEST_F(ABlitPass, CanSetAndGetBlittingRegions)
    {
        const uint32_t sourceX(1u);
        const uint32_t sourceY(2u);
        const uint32_t destinationX(3u);
        const uint32_t destinationY(4u);
        const int32_t width(5u);
        const int32_t height(6u);
        EXPECT_TRUE(blitpass.setBlittingRegion(sourceX, sourceY, destinationX, destinationY, width, height));

        const ramses::internal::PixelRectangle& sourceRegion = m_scene.impl().getIScene().getBlitPass(blitpassHandle).sourceRegion;
        EXPECT_EQ(sourceX, sourceRegion.x);
        EXPECT_EQ(sourceY, sourceRegion.y);
        EXPECT_EQ(width, sourceRegion.width);
        EXPECT_EQ(height, sourceRegion.height);

        const ramses::internal::PixelRectangle& destinationRegion = m_scene.impl().getIScene().getBlitPass(blitpassHandle).destinationRegion;
        EXPECT_EQ(destinationX, destinationRegion.x);
        EXPECT_EQ(destinationY, destinationRegion.y);
        EXPECT_EQ(width, destinationRegion.width);
        EXPECT_EQ(height, destinationRegion.height);

        {
            //client HL api
            uint32_t sourceXOut = std::numeric_limits<uint32_t>::max();
            uint32_t sourceYOut = std::numeric_limits<uint32_t>::max();
            uint32_t destinationXOut = std::numeric_limits<uint32_t>::max();
            uint32_t destinationYOut = std::numeric_limits<uint32_t>::max();
            uint32_t widthOut = std::numeric_limits<uint32_t>::max();
            uint32_t heightOut = std::numeric_limits<uint32_t>::max();
            blitpass.getBlittingRegion(sourceXOut, sourceYOut, destinationXOut, destinationYOut, widthOut, heightOut);
            EXPECT_EQ(sourceX, sourceXOut);
            EXPECT_EQ(sourceY, sourceYOut);
            EXPECT_EQ(destinationX, destinationXOut);
            EXPECT_EQ(destinationY, destinationYOut);
            EXPECT_EQ(static_cast<uint32_t>(width), widthOut);
            EXPECT_EQ(static_cast<uint32_t>(height), heightOut);
        }
    }

    TEST_F(ABlitPass, CannotSetInvalidBlittingRegion)
    {
        EXPECT_FALSE(blitpass.setBlittingRegion(0u, 0u, 0u, 0u, sourceRenderBuffer.getWidth() + 1, 10u));
        EXPECT_FALSE(blitpass.setBlittingRegion(0u, 0u, 0u, 0u, 10u, sourceRenderBuffer.getHeight() + 1u));
        EXPECT_FALSE(blitpass.setBlittingRegion(sourceRenderBuffer.getWidth(), 0u, 0u, 0u, 10, 10u));
        EXPECT_FALSE(blitpass.setBlittingRegion(0, sourceRenderBuffer.getHeight(), 0u, 0u, 10, 10u));
        EXPECT_FALSE(blitpass.setBlittingRegion(0u, 0u, sourceRenderBuffer.getWidth(), 0u, 10, 10u));
        EXPECT_FALSE(blitpass.setBlittingRegion(0u, 0u, 0, sourceRenderBuffer.getHeight(), 10, 10u));

        const ramses::internal::PixelRectangle& sourceRegion = m_scene.impl().getIScene().getBlitPass(blitpassHandle).sourceRegion;
        EXPECT_EQ(0u, sourceRegion.x);
        EXPECT_EQ(0u, sourceRegion.y);
        EXPECT_EQ(static_cast<int32_t>(sourceRenderBuffer.getWidth()), sourceRegion.width);
        EXPECT_EQ(static_cast<int32_t>(sourceRenderBuffer.getHeight()), sourceRegion.height);

        const ramses::internal::PixelRectangle& destinationRegion = m_scene.impl().getIScene().getBlitPass(blitpassHandle).destinationRegion;
        EXPECT_EQ(0u, destinationRegion.x);
        EXPECT_EQ(0u, destinationRegion.y);
        EXPECT_EQ(static_cast<int32_t>(destinationRenderBuffer.getWidth()), destinationRegion.width);
        EXPECT_EQ(static_cast<int32_t>(destinationRenderBuffer.getHeight()), destinationRegion.height);
    }

    TEST_F(ABlitPass, canValidate)
    {
        ValidationReport report;
        blitpass.validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(ABlitPass, failsValidationIfSourceRenderBufferGetsDestroyed)
    {
        m_scene.destroy(sourceRenderBuffer);

        ValidationReport report;
        blitpass.validate(report);
        EXPECT_TRUE(report.hasError());
    }

    TEST_F(ABlitPass, failsValidationIfDestinationRenderBufferGetsDestroyed)
    {
        m_scene.destroy(destinationRenderBuffer);

        ValidationReport report;
        blitpass.validate(report);
        EXPECT_TRUE(report.hasError());
    }
}
