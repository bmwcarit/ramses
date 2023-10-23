//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IDisplayController.h"
#include "internal/Core/Math3d/CameraMatrixHelper.h"
#include "gmock/gmock.h"

namespace ramses::internal{

    class DisplayControllerMock : public IDisplayController
    {
    public:
        DisplayControllerMock();
        ~DisplayControllerMock() override;

        static const DeviceResourceHandle FakeFrameBufferHandle;
        static const ProjectionParams FakeProjectionParams;

        MOCK_METHOD(void, handleWindowEvents, (), (override));
        MOCK_METHOD(bool, canRenderNewFrame, (), (const, override));
        MOCK_METHOD(void, enableContext, (), (override));
        MOCK_METHOD(void, swapBuffers, (), (override));
        MOCK_METHOD(void, clearBuffer, (DeviceResourceHandle, ClearFlags clearFlags, const glm::vec4&), (override));
        MOCK_METHOD(SceneRenderExecutionIterator, renderScene, (const RendererCachedScene&, RenderingContext&, const FrameTimer*), (override));
        MOCK_METHOD(DeviceResourceHandle, getDisplayBuffer, (), (const, override));
        MOCK_METHOD(void, readPixels, (DeviceResourceHandle framebufferHandle, uint32_t x, uint32_t y, uint32_t width, uint32_t height, std::vector<uint8_t>& dataOut), (override));
        MOCK_METHOD(uint32_t, getDisplayWidth, (), (const, override));
        MOCK_METHOD(uint32_t, getDisplayHeight, (), (const, override));
        MOCK_METHOD(IRenderBackend&, getRenderBackend, (), (const, override));
        MOCK_METHOD(IEmbeddedCompositingManager&, getEmbeddedCompositingManager, (), (override));
        MOCK_METHOD(void, validateRenderingStatusHealthy, (), (const, override));
    };
}

