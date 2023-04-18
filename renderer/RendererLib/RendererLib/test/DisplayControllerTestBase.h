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
            EXPECT_CALL(m_renderBackend, getWindow());
            EXPECT_CALL(m_renderBackend.windowMock, getWidth());
            EXPECT_CALL(m_renderBackend, getWindow());
            EXPECT_CALL(m_renderBackend.windowMock, getHeight());
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

        void destroyDisplayController(IDisplayController& controller)
        {
            delete &controller;
        }

        ::testing::StrictMock<RenderBackendStrictMock> m_renderBackend;
    };
}

#endif
