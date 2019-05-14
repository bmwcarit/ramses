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

    MOCK_METHOD0(handleWindowEvents, void());
    MOCK_CONST_METHOD0(canRenderNewFrame, bool());
    MOCK_METHOD0(enableContext, void());
    MOCK_METHOD0(swapBuffers, void());
    MOCK_METHOD2(clearBuffer, void(DeviceResourceHandle, const Vector4&));
    MOCK_CONST_METHOD2(logSceneContent, void(RendererLogContext& context, const RendererCachedScene& scene));
    MOCK_METHOD5(renderScene, SceneRenderExecutionIterator(const RendererCachedScene&, DeviceResourceHandle, const Viewport&, const SceneRenderExecutionIterator&, const FrameTimer*));
    MOCK_METHOD0(executePostProcessing, void());
    MOCK_CONST_METHOD0(getDisplayBuffer, DeviceResourceHandle());
    MOCK_METHOD5(readPixels, ramses_internal::Bool(UInt32 x, UInt32 y, UInt32 width, UInt32 height, std::vector<UInt8>& dataOut));
    MOCK_METHOD1(setProjectionParams, void(const ProjectionParams& params));
    MOCK_CONST_METHOD0(isWarpingEnabled, bool());
    MOCK_METHOD1(setWarpingMeshData, void(const WarpingMeshData& meshData));
    MOCK_CONST_METHOD0(getProjectionParams, const ProjectionParams&());
    MOCK_METHOD1(setViewPosition, void(const Vector3& position));
    MOCK_METHOD1(setViewRotation, void(const Vector3& rotation));
    MOCK_CONST_METHOD0(getViewPosition, const Vector3&());
    MOCK_CONST_METHOD0(getViewRotation, const Vector3&());
    MOCK_CONST_METHOD0(getViewMatrix, const Matrix44f&());
    MOCK_CONST_METHOD0(getDisplayWidth, UInt32());
    MOCK_CONST_METHOD0(getDisplayHeight, UInt32());
    MOCK_CONST_METHOD0(resetView, void());
    MOCK_CONST_METHOD0(getRenderBackend, IRenderBackend&());
    MOCK_METHOD0(getEmbeddedCompositingManager, IEmbeddedCompositingManager&());
    MOCK_CONST_METHOD0(validateRenderingStatusHealthy, void());

private:
    static ramses_internal::Bool ResizePixelBuffer(UInt32 x, UInt32 y, UInt32 width, UInt32 height, std::vector<UInt8>& dataOut);
};
}
#endif
