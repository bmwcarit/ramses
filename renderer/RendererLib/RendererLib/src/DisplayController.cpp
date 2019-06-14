//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/DisplayController.h"
#include "RendererAPI/IRenderBackend.h"
#include "RendererAPI/IDevice.h"
#include "RendererAPI/ISurface.h"
#include "RendererAPI/IWindow.h"
#include "RendererLib/RendererConfig.h"
#include "RendererLib/RendererLogContext.h"
#include "RendererLib/LoggingDevice.h"
#include "RendererLib/RendererCachedScene.h"
#include "Math3d/CameraMatrixHelper.h"
#include "Utils/LogMacros.h"
#include "RenderExecutor.h"

namespace ramses_internal
{
    DisplayController::DisplayController(IRenderBackend& renderer, UInt32 /*samples*/, UInt32 postProcessingEffectIds)
        : m_renderBackend(renderer)
        , m_device(m_renderBackend.getDevice())
        , m_embeddedCompositingManager(m_device, m_renderBackend.getEmbeddedCompositor(), m_renderBackend.getTextureUploadingAdapter())
        , m_projectionParams(ProjectionParams::Perspective(19.f, 1.f, 0.f, 1.f))
        , m_displayWidth(m_renderBackend.getSurface().getWindow().getWidth())
        , m_displayHeight(m_renderBackend.getSurface().getWindow().getHeight())
        , m_postProcessing(new Postprocessing(postProcessingEffectIds, m_displayWidth, m_displayHeight, m_device))
    {
    }

    void DisplayController::handleWindowEvents()
    {
        m_renderBackend.getSurface().getWindow().handleEvents();
    }

    Bool DisplayController::canRenderNewFrame() const
    {
        return m_renderBackend.getSurface().canRenderNewFrame();
    }

    void DisplayController::enableContext()
    {
        m_renderBackend.getSurface().enable();
    }

    void DisplayController::swapBuffers()
    {
        ISurface& surface = m_renderBackend.getSurface();
        surface.swapBuffers();
        surface.frameRendered();

        validateRenderingStatusHealthy();
    }

    SceneRenderExecutionIterator DisplayController::renderScene(const RendererCachedScene& scene, DeviceResourceHandle buffer, const Viewport& viewport, const SceneRenderExecutionIterator& renderFrom, const FrameTimer* frameTimer)
    {
        const FrameBufferInfo fbInfo(buffer, m_projectionParams, viewport);
        RenderExecutor executor(m_renderBackend.getDevice(), fbInfo, renderFrom, frameTimer);

        return executor.executeScene(scene, getViewMatrix());
    }

    void DisplayController::executePostProcessing()
    {
        m_postProcessing->execute();
    }

    void DisplayController::clearBuffer(DeviceResourceHandle buffer, const Vector4& clearColor)
    {
        m_device.activateRenderTarget(buffer);
        m_device.colorMask(true, true, true, true);
        m_device.clearColor(clearColor);
        m_device.depthWrite(EDepthWrite::Enabled);
        m_device.scissorTest(EScissorTest::Disabled, {});
        m_device.clear(EClearFlags_All);
    }

    Bool DisplayController::readPixels(UInt32 x, UInt32 y, UInt32 width, UInt32 height, std::vector<UInt8>& dataOut)
    {
        if (x + width > getDisplayWidth() ||
            y + height > getDisplayHeight())
        {
            LOG_ERROR(CONTEXT_RENDERER, "DisplayController::readPixels failed: requested area is out of display size boundaries!");
            return false;
        }

        // read pixels should always read content of what is displayed, therefore we need to read from actual framebuffer
        m_device.activateRenderTarget(m_postProcessing->getFramebuffer());

        dataOut.resize(width * height * 4u); // Assuming RGBA8 non multisampled
        m_device.readPixels(&dataOut[0], x, y, width, height);

        return true;
    }

    void DisplayController::setProjectionParams(const ProjectionParams& params)
    {
        m_projectionParams = params;
    }

    const ProjectionParams& DisplayController::getProjectionParams() const
    {
        return m_projectionParams;
    }

    void DisplayController::setViewPosition(const Vector3& position)
    {
        m_viewPosition = position;
        updateViewMatrix();
    }

    void DisplayController::setViewRotation(const Vector3& rotation)
    {
        m_viewRotation = rotation;
        updateViewMatrix();
    }

    const Vector3& DisplayController::getViewPosition() const
    {
        return m_viewPosition;
    }

    const Vector3& DisplayController::getViewRotation() const
    {
        return m_viewRotation;
    }

    const Matrix44f& DisplayController::getViewMatrix() const
    {
        return m_viewMatrix;
    }

    UInt32 DisplayController::getDisplayWidth() const
    {
        return m_displayWidth;
    }

    UInt32 DisplayController::getDisplayHeight() const
    {
        return m_displayHeight;
    }

    void DisplayController::updateViewMatrix()
    {
        m_viewMatrix = Matrix44f::RotationEulerZYX(m_viewRotation).transpose() * Matrix44f::Translation(-m_viewPosition);
    }

    DeviceResourceHandle DisplayController::getDisplayBuffer() const
    {
        assert(0 != m_postProcessing);
        return m_postProcessing->getScenesRenderTarget();
    }

    Bool DisplayController::isWarpingEnabled() const
    {
        return (m_postProcessing->getPostEffectsMask() & EPostProcessingEffect_Warping) != 0u;
    }

    void DisplayController::setWarpingMeshData(const WarpingMeshData& warpingMeshData)
    {
        m_postProcessing->setWarpingMeshData(warpingMeshData);
    }

    void DisplayController::resetView() const
    {
    }

    IRenderBackend& DisplayController::getRenderBackend() const
    {
        return m_renderBackend;
    }

    IEmbeddedCompositingManager& DisplayController::getEmbeddedCompositingManager()
    {
        return m_embeddedCompositingManager;
    }

    void DisplayController::validateRenderingStatusHealthy() const
    {
        m_renderBackend.getDevice().validateDeviceStatusHealthy();
    }
}
