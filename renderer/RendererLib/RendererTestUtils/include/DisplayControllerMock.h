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
#include "Math3d/CameraMatrixHelper.h"
#include "RendererLib/RendererCachedScene.h"
#include "RendererLib/WarpingMeshData.h"
#include "RendererLib/RendererLogContext.h"
#include "RendererAPI/IEmbeddedCompositingManager.h"

namespace ramses_internal{

class DisplayControllerMock : public IDisplayController
{
public:
    DisplayControllerMock();
    virtual ~DisplayControllerMock();

    static const DeviceResourceHandle FakeFrameBufferHandle;
    static const ProjectionParams FakeProjectionParams;

    MOCK_METHOD(void, handleWindowEvents, (), (override));
    MOCK_METHOD(bool, canRenderNewFrame, (), (const, override));
    MOCK_METHOD(void, enableContext, (), (override));
    MOCK_METHOD(void, swapBuffers, (), (override));
    MOCK_METHOD(void, clearBuffer, (DeviceResourceHandle, const Vector4&), (override));
    MOCK_METHOD(SceneRenderExecutionIterator, renderScene, (const RendererCachedScene&, DeviceResourceHandle, const Viewport&, const SceneRenderExecutionIterator&, const FrameTimer*), (override));
    MOCK_METHOD(void, executePostProcessing, (), (override));
    MOCK_METHOD(DeviceResourceHandle, getDisplayBuffer, (), (const, override));
    MOCK_METHOD(ramses_internal::Bool, readPixels, (UInt32 x, UInt32 y, UInt32 width, UInt32 height, std::vector<UInt8>& dataOut), (override));
    MOCK_METHOD(void, setProjectionParams, (const ProjectionParams& params), (override));
    MOCK_METHOD(bool, isWarpingEnabled, (), (const, override));
    MOCK_METHOD(void, setWarpingMeshData, (const WarpingMeshData& meshData), (override));
    MOCK_METHOD(const ProjectionParams&, getProjectionParams, (), (const, override));
    MOCK_METHOD(void, setViewPosition, (const Vector3& position), (override));
    MOCK_METHOD(void, setViewRotation, (const Vector3& rotation), (override));
    MOCK_METHOD(const Vector3&, getViewPosition, (), (const, override));
    MOCK_METHOD(const Vector3&, getViewRotation, (), (const, override));
    MOCK_METHOD(const Matrix44f&, getViewMatrix, (), (const, override));
    MOCK_METHOD(UInt32, getDisplayWidth, (), (const, override));
    MOCK_METHOD(UInt32, getDisplayHeight, (), (const, override));
    MOCK_METHOD(void, resetView, (), (const, override));
    MOCK_METHOD(IRenderBackend&, getRenderBackend, (), (const, override));
    MOCK_METHOD(IEmbeddedCompositingManager&, getEmbeddedCompositingManager, (), (override));
    MOCK_METHOD(void, validateRenderingStatusHealthy, (), (const, override));

private:
    static ramses_internal::Bool ResizePixelBuffer(UInt32 x, UInt32 y, UInt32 width, UInt32 height, std::vector<UInt8>& dataOut);
};
}
#endif
