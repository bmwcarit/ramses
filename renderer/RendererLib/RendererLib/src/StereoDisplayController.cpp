//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/StereoDisplayController.h"
#include "RendererAPI/IRenderBackend.h"
#include "RendererAPI/IDevice.h"
#include "RendererAPI/ISurface.h"
#include "SceneAPI/PixelRectangle.h"
#include "Math3d/CameraMatrixHelper.h"
#include "Math3d/Vector2i.h"
#include "RenderExecutor.h"
#include "SceneAPI/RenderBuffer.h"

namespace ramses_internal
{
    StereoDisplayController::StereoDisplayController(IRenderBackend& rendererBackend)
        : DisplayController(rendererBackend)
    {
        for (UInt32 eye = 0; eye < 2; eye++)
        {
            const UInt32 desiredWidth = getDisplayWidth() / 2;
            const UInt32 desiredHeight = getDisplayHeight();

            // give the old projection matrix to construct the eye depended matrix in combination with display information
            // connecting a custom hmd display will adjust this later on
            const Float fov = 106.188766f;
            const Float ar = static_cast<Float>(desiredWidth) / desiredHeight;

            // the handle is currently set during begin eye render step directly before rendering to the buffer
            DeviceHandleVector buffers;
            buffers.push_back(rendererBackend.getDevice().uploadRenderBuffer({ desiredWidth, desiredHeight, ERenderBufferType_ColorBuffer, ETextureFormat_RGBA8, ERenderBufferAccessMode_ReadWrite, 0u }));
            buffers.push_back(rendererBackend.getDevice().uploadRenderBuffer({ desiredWidth, desiredHeight, ERenderBufferType_DepthBuffer, ETextureFormat_Depth24, ERenderBufferAccessMode_ReadWrite, 0u }));

            auto& fbInfo = m_viewInfo[eye].m_fbInfo;
            fbInfo.deviceHandle = rendererBackend.getDevice().uploadRenderTarget(buffers);
            fbInfo.projectionParams = ProjectionParams::Perspective(fov, ar, getProjectionParams().nearPlane, getProjectionParams().farPlane);
            fbInfo.viewport = Viewport(0, 0, desiredWidth, desiredHeight);
        }
    }

    void StereoDisplayController::enableContext()
    {
        DisplayController::enableContext();
        IDevice& device = getRenderBackend().getDevice();

        // combine view orientation with user head orientation
        const Matrix44f renderView      = (Matrix44f::RotationEulerZYX(getViewRotation()));
        const Matrix44f renderHeadView  = renderView.transpose();

        for (UInt32 eyeIndex = 0; eyeIndex < 2; eyeIndex++)
        {
            const Vector3 worldSpaceEyePos = renderView.rotate(m_viewInfo[eyeIndex].m_eyePosition);
            m_viewInfo[eyeIndex].m_viewMatrix = renderHeadView * Matrix44f::Translation(-getViewPosition() - worldSpaceEyePos);

            // Clear render target one time before both eyes are rendered
            device.activateRenderTarget(m_viewInfo[eyeIndex].m_fbInfo.deviceHandle);
            device.colorMask(true, true, true, true);
            device.clearColor({ 0.f, 0.f, 0.f, 1.f });
            device.depthWrite(EDepthWrite::Enabled);
            device.scissorTest(EScissorTest::Disabled, {});
            device.clear(EClearFlags_All);
        }
    }

    // Stereo display controller creates its own buffers and viewports per eye
    SceneRenderExecutionIterator StereoDisplayController::renderScene(const RendererCachedScene& scene, DeviceResourceHandle, const Viewport&, const SceneRenderExecutionIterator& renderFrom, const FrameTimer*)
    {
        // Stereo Rendering, one pass for left and one for right eye
        for (UInt32 eyeIndex = 0; eyeIndex < 2; eyeIndex++)
        {
            const RenderExecutor executor(getRenderBackend().getDevice(), m_viewInfo[eyeIndex].m_fbInfo, renderFrom);
            executor.executeScene(scene, m_viewInfo[eyeIndex].m_viewMatrix);
        }

        return SceneRenderExecutionIterator();
    }

    void StereoDisplayController::executePostProcessing()
    {
        // If no display is attached copy stereo content to scene back buffer
        for (UInt32 eye = 0; eye < 2; eye++)
        {
            FrameBufferInfo& fbInfo = m_viewInfo[eye].m_fbInfo;

            // copy the two render targets to the backbuffer aka m_sceneRenderTarget...
            PixelRectangle dest;
            dest.width  = getDisplayWidth() / 2;
            dest.height = getDisplayHeight();
            dest.x      = eye == 0 ? 0 : getDisplayWidth() / 2;
            dest.y      = 0;

            PixelRectangle src;
            src.width  = fbInfo.viewport.width;
            src.height = fbInfo.viewport.height;
            src.x      = 0;
            src.y      = 0;
            getRenderBackend().getDevice().blitRenderTargets(fbInfo.deviceHandle, DisplayController::getDisplayBuffer(), src, dest, true);
        }

        DisplayController::executePostProcessing();
    }

    void StereoDisplayController::setProjectionParams(const ProjectionParams& params)
    {
        // we have to store a projection matrix adjusted to the special aspect ratio of the render target and eye depending FOV
        DisplayController::setProjectionParams(params);

        // Aspect ratio is halved, because of stereo rendering
        const ProjectionParams stereoParams = ProjectionParams::Perspective(ProjectionParams::GetPerspectiveFovY(params),
            ProjectionParams::GetAspectRatio(params) * 0.5f, params.nearPlane, params.farPlane);
        m_viewInfo[0].m_fbInfo.projectionParams = m_viewInfo[1].m_fbInfo.projectionParams = stereoParams;
    }
}
