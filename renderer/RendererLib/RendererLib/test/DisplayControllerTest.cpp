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
        EXPECT_CALL(m_renderBackend, getSurface());
        EXPECT_CALL(m_renderBackend.surfaceMock, canRenderNewFrame()).WillOnce(Return(false));

        EXPECT_FALSE(displayController.canRenderNewFrame());

        destroyDisplayController(displayController);
    }

    TEST_F(ADisplayController, enablesContextOnBeginFrame)
    {
        IDisplayController& displayController = createDisplayController();

        InSequence seq;
        EXPECT_CALL(m_renderBackend, getSurface());
        EXPECT_CALL(m_renderBackend.surfaceMock, enable());

        displayController.enableContext();

        destroyDisplayController(displayController);
    }

    TEST_F(ADisplayController, activatesBufferAndClearsOnClearBuffer)
    {
        const Vector4 clearColor(0.1f, 0.2f, 0.3f, 0.4f);
        IDisplayController& displayController = createDisplayController();

        InSequence seq;
        EXPECT_CALL(m_renderBackend.deviceMock, activateRenderTarget(DeviceMock::FakeFrameBufferRenderTargetDeviceHandle));
        EXPECT_CALL(m_renderBackend.deviceMock, colorMask(true, true, true, true));
        EXPECT_CALL(m_renderBackend.deviceMock, clearColor(clearColor));
        EXPECT_CALL(m_renderBackend.deviceMock, depthWrite(EDepthWrite::Enabled));
        RenderState::ScissorRegion scissorRegion{};
        EXPECT_CALL(m_renderBackend.deviceMock, scissorTest(EScissorTest::Disabled, scissorRegion));
        EXPECT_CALL(m_renderBackend.deviceMock, clear(_));

        displayController.clearBuffer(DeviceMock::FakeFrameBufferRenderTargetDeviceHandle, clearColor);

        destroyDisplayController(displayController);
    }

    TEST_F(ADisplayController, swapsBuffersAndCallsFrameRenderedOnEndFrame)
    {
        IDisplayController& displayController = createDisplayController();

        InSequence seq;
        EXPECT_CALL(m_renderBackend, getSurface());
        EXPECT_CALL(m_renderBackend.surfaceMock, swapBuffers());
        EXPECT_CALL(m_renderBackend.surfaceMock, frameRendered());
        EXPECT_CALL(m_renderBackend, getDevice());
        EXPECT_CALL(m_renderBackend.deviceMock, validateDeviceStatusHealthy());

        displayController.swapBuffers();

        destroyDisplayController(displayController);
    }

    TEST_F(ADisplayController, canHandleWindowEvents)
    {
        IDisplayController& displayController = createDisplayController();

        InSequence seq;
        EXPECT_CALL(m_renderBackend, getSurface());
        EXPECT_CALL(m_renderBackend.surfaceMock, getWindow());
        EXPECT_CALL(m_renderBackend.surfaceMock.windowMock, handleEvents());

        displayController.handleWindowEvents();

        destroyDisplayController(displayController);
    }

    TEST_F(ADisplayController, canBeCreatedAndDestroyedWithWarping)
    {
        IDisplayController& displayController = createDisplayControllerWithWarping();
        destroyDisplayControllerWithWarping(displayController);
    }

    TEST_F(ADisplayController, hasZeroPositionAndRotationWithDefaultConfiguration)
    {
        IDisplayController& displayController = createDisplayController();

        EXPECT_EQ(Vector3(0.0f), displayController.getViewPosition());
        EXPECT_EQ(Vector3(0.0f), displayController.getViewRotation());

        destroyDisplayController(displayController);
    }

    TEST_F(ADisplayController, canSetItsPositionAndRotation)
    {
        IDisplayController& displayController = createDisplayController();

        const Vector3 position(1.0f, 2.0f, 3.0f);
        displayController.setViewPosition(position);
        EXPECT_EQ(position, displayController.getViewPosition());

        const Vector3 rotation(1.1f, 2.1f, 3.1f);
        displayController.setViewRotation(rotation);
        EXPECT_EQ(rotation, displayController.getViewRotation());

        destroyDisplayController(displayController);
    }

    TEST_F(ADisplayController, readsPixels)
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
        EXPECT_TRUE(displayController.readPixels(x, y, width, height, pixels));

        EXPECT_EQ(width * height * 4u, pixels.size());

        destroyDisplayController(displayController);
    }

    TEST_F(ADisplayController, failsToReadsPixelsIfOutOfBoundaries)
    {
        IDisplayController& displayController = createDisplayController();

        const UInt32 x = 10u;
        const UInt32 y = 11u;
        const UInt32 width = WindowMock::FakeWidth + 1u;
        const UInt32 height = 13u;

        UInt8Vector pixels;
        EXPECT_FALSE(displayController.readPixels(x, y, width, height, pixels));

        EXPECT_TRUE(pixels.empty());

        destroyDisplayController(displayController);
    }

}
