//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYCONTROLLERTESTBASE_H
#define RAMSES_DISPLAYCONTROLLERTESTBASE_H

#include "renderer_common_gmock_header.h"
#include "RendererLib/DisplayController.h"
#include "RenderBackendMock.h"

namespace ramses_internal
{
    class DisplayControllerTestBase
    {
    public:
        virtual ~DisplayControllerTestBase()
        {
        }

    protected:
        void expectCreationSequence()
        {
            ::testing::InSequence seq;
            EXPECT_CALL(m_renderBackend, getDevice());
            EXPECT_CALL(m_renderBackend, getSurface());
            EXPECT_CALL(m_renderBackend.surfaceMock, getWindow());
            EXPECT_CALL(m_renderBackend.surfaceMock.windowMock, getWidth());
            EXPECT_CALL(m_renderBackend, getSurface());
            EXPECT_CALL(m_renderBackend.surfaceMock, getWindow());
            EXPECT_CALL(m_renderBackend.surfaceMock.windowMock, getHeight());
            EXPECT_CALL(m_renderBackend.deviceMock, getFramebufferRenderTarget());
        }

        IDisplayController& createDisplayController()
        {
            expectCreationSequence();
            IDisplayController& controller = *new DisplayController(m_renderBackend);
            const DeviceResourceHandle expectedFramebufferHandle = DeviceMock::FakeFrameBufferRenderTargetDeviceHandle;
            EXPECT_EQ(expectedFramebufferHandle, controller.getDisplayBuffer());

            return controller;
        }

        IDisplayController& createDisplayControllerWithWarping()
        {
            using namespace testing;
            InSequence seq;
            expectCreationSequence();

            EXPECT_CALL(m_renderBackend.deviceMock, uploadRenderBuffer(_)).Times(2u);
            EXPECT_CALL(m_renderBackend.deviceMock, uploadRenderTarget(_));

            EXPECT_CALL(m_renderBackend.deviceMock, uploadShader(_));
            EXPECT_CALL(m_renderBackend.deviceMock, allocateVertexBuffer(_, _));
            EXPECT_CALL(m_renderBackend.deviceMock, uploadVertexBufferData(_, _, _));
            EXPECT_CALL(m_renderBackend.deviceMock, allocateVertexBuffer(_, _));
            EXPECT_CALL(m_renderBackend.deviceMock, uploadVertexBufferData(_, _, _));
            EXPECT_CALL(m_renderBackend.deviceMock, allocateIndexBuffer(_, _));
            EXPECT_CALL(m_renderBackend.deviceMock, uploadIndexBufferData(_, _, _));

            IDisplayController& controller = *new DisplayController(m_renderBackend, 1u, EPostProcessingEffect_Warping);
            const DeviceResourceHandle expectedRenderTargetHandle = DeviceMock::FakeRenderTargetDeviceHandle;
            EXPECT_EQ(expectedRenderTargetHandle, controller.getDisplayBuffer());

            return controller;
        }

        void destroyDisplayController(IDisplayController& controller)
        {
            delete &controller;
        }

        void destroyDisplayControllerWithWarping(IDisplayController& controller)
        {
            using namespace testing;
            InSequence seq;
            EXPECT_CALL(m_renderBackend.deviceMock, deleteVertexBuffer(_));
            EXPECT_CALL(m_renderBackend.deviceMock, deleteIndexBuffer(_));
            EXPECT_CALL(m_renderBackend.deviceMock, deleteVertexBuffer(_));
            EXPECT_CALL(m_renderBackend.deviceMock, deleteShader(_));

            EXPECT_CALL(m_renderBackend.deviceMock, deleteRenderTarget(_));
            EXPECT_CALL(m_renderBackend.deviceMock, deleteRenderBuffer(_));
            EXPECT_CALL(m_renderBackend.deviceMock, deleteRenderBuffer(_));

            delete &controller;
        }

        ::testing::StrictMock<RenderBackendStrictMock> m_renderBackend;
    };
}

#endif
