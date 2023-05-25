//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYCONTROLLERMOCK_H
#define RAMSES_DISPLAYCONTROLLERMOCK_H

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "RendererAPI/IDisplayController.h"
#include "RendererAPI/IRenderBackend.h"
#include "RendererAPI/IEmbeddedCompositingManager.h"
#include "RendererAPI/RenderingContext.h"
#include "RendererLib/RendererCachedScene.h"
#include "RendererLib/RendererLogContext.h"
#include "Math3d/CameraMatrixHelper.h"

namespace ramses_internal{

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
    MOCK_METHOD(void, clearBuffer, (DeviceResourceHandle, uint32_t clearFlags, const glm::vec4&), (override));
    MOCK_METHOD(SceneRenderExecutionIterator, renderScene, (const RendererCachedScene&, RenderingContext&, const FrameTimer*), (override));
    MOCK_METHOD(DeviceResourceHandle, getDisplayBuffer, (), (const, override));
    MOCK_METHOD(void, readPixels, (DeviceResourceHandle framebufferHandle, UInt32 x, UInt32 y, UInt32 width, UInt32 height, std::vector<UInt8>& dataOut), (override));
    MOCK_METHOD(UInt32, getDisplayWidth, (), (const, override));
    MOCK_METHOD(UInt32, getDisplayHeight, (), (const, override));
    MOCK_METHOD(IRenderBackend&, getRenderBackend, (), (const, override));
    MOCK_METHOD(IEmbeddedCompositingManager&, getEmbeddedCompositingManager, (), (override));
    MOCK_METHOD(void, validateRenderingStatusHealthy, (), (const, override));
};
}
#endif
