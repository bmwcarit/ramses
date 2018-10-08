//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ramses-client-api/RenderBuffer.h"
#include "ramses-client-api/RenderTarget.h"
#include "ramses-client-api/RenderTargetDescription.h"
#include "RenderTargetImpl.h"
#include "RenderBufferImpl.h"
#include "ClientTestUtils.h"

using namespace testing;

namespace ramses
{
    class ARenderTarget : public LocalTestClientWithScene, public testing::Test
    {
    };

    TEST_F(ARenderTarget, canBeCreated)
    {
        const RenderBuffer& rb = *m_scene.createRenderBuffer(16u, 8u, ERenderBufferType_Color, ERenderBufferFormat_RGBA8, ERenderBufferAccessMode_ReadWrite);
        RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(rb);

        const RenderTarget* renderTarget = m_scene.createRenderTarget(rtDesc, "RenderTarget");
        ASSERT_TRUE(renderTarget != NULL);
        EXPECT_EQ(16u, renderTarget->getWidth());
        EXPECT_EQ(8u, renderTarget->getHeight());

        const ramses_internal::RenderTargetHandle rtHandle = renderTarget->impl.getRenderTargetHandle();
        EXPECT_TRUE(m_internalScene.isRenderTargetAllocated(rtHandle));
        ASSERT_EQ(1u, m_internalScene.getRenderTargetRenderBufferCount(rtHandle));
        EXPECT_EQ(rb.impl.getRenderBufferHandle(), m_internalScene.getRenderTargetRenderBuffer(rtHandle, 0u));
    }

    TEST_F(ARenderTarget, failsToCreateUsingInvalidRenderTargetDescription)
    {
        RenderTargetDescription invalidRtDesc; // no buffers

        const RenderTarget* renderTarget = m_scene.createRenderTarget(invalidRtDesc, "RenderTarget");
        EXPECT_TRUE(renderTarget == NULL);
    }
}
