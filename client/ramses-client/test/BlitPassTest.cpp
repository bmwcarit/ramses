//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ClientTestUtils.h"
#include "ramses-client-api/BlitPass.h"
#include "ramses-client-api/RenderBuffer.h"
#include "SceneImpl.h"
#include "BlitPassImpl.h"
#include "RenderBufferImpl.h"
#include "ramses-utils.h"
#include "Scene/ClientScene.h"

using namespace testing;
using namespace ramses_internal;

namespace ramses
{
    class ABlitPass : public LocalTestClientWithScene, public testing::Test
    {
    protected:
        ABlitPass()
            : LocalTestClientWithScene()
            , sourceRenderBuffer(*createRenderBuffer())
            , destinationRenderBuffer(*createRenderBuffer())
            , blitpass(*m_scene.createBlitPass(sourceRenderBuffer, destinationRenderBuffer, "BlitPass"))
            , blitpassHandle(blitpass.impl.getBlitPassHandle())
        {
        }

        RenderBuffer* createRenderBuffer()
        {
            RenderBuffer* renderBuffer = m_scene.createRenderBuffer(16u, 12u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
            return renderBuffer;
        }

        RenderBuffer& sourceRenderBuffer;
        RenderBuffer& destinationRenderBuffer;
        BlitPass& blitpass;
        const ramses_internal::BlitPassHandle blitpassHandle;
    };

    TEST_F(ABlitPass, CanGetSourceAndDestinationBuffers)
    {
        const RenderBuffer& srcRB = blitpass.getSourceRenderBuffer();
        EXPECT_EQ(srcRB.impl.getRenderBufferHandle(), sourceRenderBuffer.impl.getRenderBufferHandle());

        const RenderBuffer& dstRB = blitpass.getDestinationRenderBuffer();
        EXPECT_EQ(dstRB.impl.getRenderBufferHandle(), destinationRenderBuffer.impl.getRenderBufferHandle());
    }

    TEST_F(ABlitPass, BlittingRegionIsSetToCompleteBufferByDefault)
    {
        const ramses_internal::PixelRectangle& sourceRegion = m_scene.impl.getIScene().getBlitPass(blitpassHandle).sourceRegion;
        EXPECT_EQ(0u, sourceRegion.x);
        EXPECT_EQ(0u, sourceRegion.y);
        EXPECT_EQ(static_cast<ramses_internal::Int32>(sourceRenderBuffer.getWidth()), sourceRegion.width);
        EXPECT_EQ(static_cast<ramses_internal::Int32>(sourceRenderBuffer.getHeight()), sourceRegion.height);

        const ramses_internal::PixelRectangle& destinationRegion = m_scene.impl.getIScene().getBlitPass(blitpassHandle).destinationRegion;
        EXPECT_EQ(0u, destinationRegion.x);
        EXPECT_EQ(0u, destinationRegion.y);
        EXPECT_EQ(static_cast<ramses_internal::Int32>(destinationRenderBuffer.getWidth()), destinationRegion.width);
        EXPECT_EQ(static_cast<ramses_internal::Int32>(destinationRenderBuffer.getHeight()), destinationRegion.height);

        {
            //client HL api
            uint32_t sourceXOut;
            uint32_t sourceYOut;
            uint32_t destinationXOut;
            uint32_t destinationYOut;
            uint32_t widthOut;
            uint32_t heightOut;
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
        EXPECT_EQ(StatusOK, blitpass.setBlittingRegion(sourceX, sourceY, destinationX, destinationY, width, height));

        const ramses_internal::PixelRectangle& sourceRegion = m_scene.impl.getIScene().getBlitPass(blitpassHandle).sourceRegion;
        EXPECT_EQ(sourceX, sourceRegion.x);
        EXPECT_EQ(sourceY, sourceRegion.y);
        EXPECT_EQ(width, sourceRegion.width);
        EXPECT_EQ(height, sourceRegion.height);

        const ramses_internal::PixelRectangle& destinationRegion = m_scene.impl.getIScene().getBlitPass(blitpassHandle).destinationRegion;
        EXPECT_EQ(destinationX, destinationRegion.x);
        EXPECT_EQ(destinationY, destinationRegion.y);
        EXPECT_EQ(width, destinationRegion.width);
        EXPECT_EQ(height, destinationRegion.height);

        {
            //client HL api
            uint32_t sourceXOut;
            uint32_t sourceYOut;
            uint32_t destinationXOut;
            uint32_t destinationYOut;
            uint32_t widthOut;
            uint32_t heightOut;
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
        EXPECT_NE(StatusOK, blitpass.setBlittingRegion(0u, 0u, 0u, 0u, sourceRenderBuffer.getWidth() + 1, 10u));
        EXPECT_NE(StatusOK, blitpass.setBlittingRegion(0u, 0u, 0u, 0u, 10u, sourceRenderBuffer.getHeight() + 1u));
        EXPECT_NE(StatusOK, blitpass.setBlittingRegion(sourceRenderBuffer.getWidth(), 0u, 0u, 0u, 10, 10u));
        EXPECT_NE(StatusOK, blitpass.setBlittingRegion(0, sourceRenderBuffer.getHeight(), 0u, 0u, 10, 10u));
        EXPECT_NE(StatusOK, blitpass.setBlittingRegion(0u, 0u, sourceRenderBuffer.getWidth(), 0u, 10, 10u));
        EXPECT_NE(StatusOK, blitpass.setBlittingRegion(0u, 0u, 0, sourceRenderBuffer.getHeight(), 10, 10u));

        const ramses_internal::PixelRectangle& sourceRegion = m_scene.impl.getIScene().getBlitPass(blitpassHandle).sourceRegion;
        EXPECT_EQ(0u, sourceRegion.x);
        EXPECT_EQ(0u, sourceRegion.y);
        EXPECT_EQ(static_cast<ramses_internal::Int32>(sourceRenderBuffer.getWidth()), sourceRegion.width);
        EXPECT_EQ(static_cast<ramses_internal::Int32>(sourceRenderBuffer.getHeight()), sourceRegion.height);

        const ramses_internal::PixelRectangle& destinationRegion = m_scene.impl.getIScene().getBlitPass(blitpassHandle).destinationRegion;
        EXPECT_EQ(0u, destinationRegion.x);
        EXPECT_EQ(0u, destinationRegion.y);
        EXPECT_EQ(static_cast<ramses_internal::Int32>(destinationRenderBuffer.getWidth()), destinationRegion.width);
        EXPECT_EQ(static_cast<ramses_internal::Int32>(destinationRenderBuffer.getHeight()), destinationRegion.height);
    }

    TEST_F(ABlitPass, canValidate)
    {
        EXPECT_EQ(StatusOK, blitpass.validate());
    }

    TEST_F(ABlitPass, failsValidationIfSourceRenderBufferGetsDestroyed)
    {
        m_scene.destroy(sourceRenderBuffer);

        EXPECT_NE(StatusOK, blitpass.validate());
    }

    TEST_F(ABlitPass, failsValidationIfDestinationRenderBufferGetsDestroyed)
    {
        m_scene.destroy(destinationRenderBuffer);

        EXPECT_NE(StatusOK, blitpass.validate());
    }
}
