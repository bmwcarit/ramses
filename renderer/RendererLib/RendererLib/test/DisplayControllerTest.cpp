//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DisplayControllerTestBase.h"

namespace ramses_internal
{
    using namespace testing;

    class ADisplayController : public DisplayControllerTestBase, public ::testing::Test
    {
    };

    TEST_F(ADisplayController, canBeCreatedAndDestroyed)
    {
        IDisplayController& displayController = createDisplayController();
        destroyDisplayController(displayController);
    }

    TEST_F(ADisplayController, createsFramebuffer)
    {
        IDisplayController& displayController = createDisplayController();
        destroyDisplayController(displayController);
    }

    TEST_F(ADisplayController, queriesSurfaceIfItCanRenderNewFrame)
    {
        IDisplayController& displayController = createDisplayController();

        InSequence seq;
        EXPECT_CALL(m_renderBackend, getWindow());
        EXPECT_CALL(m_renderBackend.windowMock, canRenderNewFrame()).WillOnce(Return(false));

        EXPECT_FALSE(displayController.canRenderNewFrame());

        destroyDisplayController(displayController);
    }

    TEST_F(ADisplayController, doesNotEnableContextOnBeginFrame)
    {
        IDisplayController& displayController = createDisplayController();
        EXPECT_CALL(m_renderBackend.contextMock, enable()).Times(0);
        destroyDisplayController(displayController);
    }

    TEST_F(ADisplayController, activatesBufferAndClearsOnClearBuffer_clearAll)
    {
        const glm::vec4 clearColor(0.1f, 0.2f, 0.3f, 0.4f);
        IDisplayController& displayController = createDisplayController();

        InSequence seq;
        EXPECT_CALL(m_renderBackend.deviceMock, activateRenderTarget(DeviceMock::FakeFrameBufferRenderTargetDeviceHandle));
        EXPECT_CALL(m_renderBackend.deviceMock, colorMask(true, true, true, true));
        EXPECT_CALL(m_renderBackend.deviceMock, clearColor(clearColor));
        EXPECT_CALL(m_renderBackend.deviceMock, depthWrite(EDepthWrite::Enabled));
        RenderState::ScissorRegion scissorRegion{};
        EXPECT_CALL(m_renderBackend.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion));
        EXPECT_CALL(m_renderBackend.deviceMock, clear(EClearFlags_All));

        displayController.clearBuffer(DeviceMock::FakeFrameBufferRenderTargetDeviceHandle, EClearFlags_All, clearColor);

        destroyDisplayController(displayController);
    }

    TEST_F(ADisplayController, activatesBufferAndClearsOnClearBuffer_clearColor)
    {
        const glm::vec4 clearColor(0.1f, 0.2f, 0.3f, 0.4f);
        IDisplayController& displayController = createDisplayController();

        InSequence seq;
        EXPECT_CALL(m_renderBackend.deviceMock, activateRenderTarget(DeviceMock::FakeFrameBufferRenderTargetDeviceHandle));
        EXPECT_CALL(m_renderBackend.deviceMock, colorMask(true, true, true, true));
        EXPECT_CALL(m_renderBackend.deviceMock, clearColor(clearColor));
        RenderState::ScissorRegion scissorRegion{};
        EXPECT_CALL(m_renderBackend.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion));
        EXPECT_CALL(m_renderBackend.deviceMock, clear(EClearFlags_Color));

        displayController.clearBuffer(DeviceMock::FakeFrameBufferRenderTargetDeviceHandle, EClearFlags_Color, clearColor);

        destroyDisplayController(displayController);
    }

    TEST_F(ADisplayController, doesNotActivatesBufferNorClearsOnClearBuffer_clearNone)
    {
        const glm::vec4 clearColor(0.1f, 0.2f, 0.3f, 0.4f);
        IDisplayController& displayController = createDisplayController();

        displayController.clearBuffer(DeviceMock::FakeFrameBufferRenderTargetDeviceHandle, EClearFlags_None, clearColor);

        destroyDisplayController(displayController);
    }

    TEST_F(ADisplayController, swapsBuffersAndCallsFrameRenderedOnEndFrame)
    {
        IDisplayController& displayController = createDisplayController();

        InSequence seq;
        EXPECT_CALL(m_renderBackend, getContext());
        EXPECT_CALL(m_renderBackend.contextMock, swapBuffers());
        EXPECT_CALL(m_renderBackend, getWindow());
        EXPECT_CALL(m_renderBackend.windowMock, frameRendered());
        EXPECT_CALL(m_renderBackend, getDevice());
        EXPECT_CALL(m_renderBackend.deviceMock, validateDeviceStatusHealthy());

        displayController.swapBuffers();

        destroyDisplayController(displayController);
    }

    TEST_F(ADisplayController, canHandleWindowEvents)
    {
        IDisplayController& displayController = createDisplayController();

        InSequence seq;
        EXPECT_CALL(m_renderBackend, getWindow());
        EXPECT_CALL(m_renderBackend.windowMock, handleEvents());

        displayController.handleWindowEvents();

        destroyDisplayController(displayController);
    }

    TEST_F(ADisplayController, readsPixelsFromFramebuffer)
    {
        IDisplayController& displayController = createDisplayController();

        const UInt32 x = 1u;
        const UInt32 y = 2u;
        const UInt32 width = WindowMock::FakeWidth - 2u;
        const UInt32 height = WindowMock::FakeHeight - 3u;

        InSequence seq;
        EXPECT_CALL(m_renderBackend.deviceMock, activateRenderTarget(DeviceMock::FakeFrameBufferRenderTargetDeviceHandle));
        EXPECT_CALL(m_renderBackend.deviceMock, readPixels(_, x, y, width, height));
        UInt8Vector pixels;
        displayController.readPixels(DeviceMock::FakeFrameBufferRenderTargetDeviceHandle, x, y, width, height, pixels);

        EXPECT_EQ(width * height * 4u, pixels.size());

        destroyDisplayController(displayController);
    }

    TEST_F(ADisplayController, readsPixelsFromOffscreenBuffer)
    {
        IDisplayController& displayController = createDisplayController();

        const UInt32 x = 1u;
        const UInt32 y = 2u;
        const UInt32 width = WindowMock::FakeWidth - 2u;
        const UInt32 height = WindowMock::FakeHeight - 3u;

        const DeviceResourceHandle obRenderTargetDeviceHandle{ 7799u };
        ASSERT_NE(obRenderTargetDeviceHandle, DeviceMock::FakeFrameBufferRenderTargetDeviceHandle);

        InSequence seq;
        EXPECT_CALL(m_renderBackend.deviceMock, activateRenderTarget(obRenderTargetDeviceHandle));
        EXPECT_CALL(m_renderBackend.deviceMock, readPixels(_, x, y, width, height));
        UInt8Vector pixels;
        displayController.readPixels(obRenderTargetDeviceHandle, x, y, width, height, pixels);

        EXPECT_EQ(width * height * 4u, pixels.size());

        destroyDisplayController(displayController);
    }
}
