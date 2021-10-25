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
#include "RendererAPI/IWindow.h"
#include "RendererAPI/IContext.h"
#include "RendererLib/RendererConfig.h"
#include "RendererLib/RendererLogContext.h"
#include "RendererLib/LoggingDevice.h"
#include "RendererLib/RendererCachedScene.h"
#include "Math3d/CameraMatrixHelper.h"
#include "RenderExecutor.h"

namespace ramses_internal
{
    DisplayController::DisplayController(IRenderBackend& renderer, UInt32 /*samples*/, UInt32 postProcessingEffectIds)
        : m_renderBackend(renderer)
        , m_device(m_renderBackend.getDevice())
        , m_embeddedCompositingManager(m_device, m_renderBackend.getEmbeddedCompositor(), m_renderBackend.getTextureUploadingAdapter())
        , m_displayWidth(m_renderBackend.getWindow().getWidth())
        , m_displayHeight(m_renderBackend.getWindow().getHeight())
        , m_postProcessing(new Postprocessing(postProcessingEffectIds, m_displayWidth, m_displayHeight, m_device))
    {
    }

    void DisplayController::handleWindowEvents()
    {
        m_renderBackend.getWindow().handleEvents();
    }

    Bool DisplayController::canRenderNewFrame() const
    {
        return m_renderBackend.getWindow().canRenderNewFrame();
    }

    void DisplayController::enableContext()
    {
        m_renderBackend.getContext().enable();
    }

    void DisplayController::swapBuffers()
    {
        m_renderBackend.getContext().swapBuffers();
        m_renderBackend.getWindow().frameRendered();

        validateRenderingStatusHealthy();
    }

    SceneRenderExecutionIterator DisplayController::renderScene(const RendererCachedScene& scene, const RenderingContext& renderContext, const FrameTimer* frameTimer)
    {
        RenderExecutor executor(m_renderBackend.getDevice(), renderContext, frameTimer);

        return executor.executeScene(scene);
    }

    void DisplayController::executePostProcessing()
    {
        m_postProcessing->execute();
    }

    void DisplayController::clearBuffer(DeviceResourceHandle buffer, uint32_t clearFlags, const Vector4& clearColor)
    {
        if (clearFlags != EClearFlags_None)
        {
            m_device.activateRenderTarget(buffer);

            if (clearFlags & EClearFlags_Color)
            {
                m_device.colorMask(true, true, true, true);
                m_device.clearColor(clearColor);
            }
            if (clearFlags & EClearFlags_Depth)
                m_device.depthWrite(EDepthWrite::Enabled);

            m_device.scissorTest(EScissorTest::Disabled, {});
            m_device.clear(clearFlags);
        }
    }

    void DisplayController::readPixels(DeviceResourceHandle renderTargetHandle, UInt32 x, UInt32 y, UInt32 width, UInt32 height, std::vector<UInt8>& dataOut)
    {
        // if readPixels requested from display buffer we need to query actual framebuffer's device handle
        if(renderTargetHandle == getDisplayBuffer())
            // read from actual framebuffer where content is rendered after post-processing (not from the temporary render target on which post processing is applied)
            m_device.activateRenderTarget(m_postProcessing->getFramebuffer());
        else
            m_device.activateRenderTarget(renderTargetHandle);

        dataOut.resize(width * height * 4u); // Assuming RGBA8 non multisampled
        m_device.readPixels(&dataOut[0], x, y, width, height);
    }

    UInt32 DisplayController::getDisplayWidth() const
    {
        return m_displayWidth;
    }

    UInt32 DisplayController::getDisplayHeight() const
    {
        return m_displayHeight;
    }

    DeviceResourceHandle DisplayController::getDisplayBuffer() const
    {
        assert(nullptr != m_postProcessing);
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
