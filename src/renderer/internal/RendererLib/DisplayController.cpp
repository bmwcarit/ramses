//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/DisplayController.h"
#include "internal/RendererLib/PlatformInterface/IRenderBackend.h"
#include "internal/RendererLib/PlatformInterface/IDevice.h"
#include "internal/RendererLib/PlatformInterface/IWindow.h"
#include "internal/RendererLib/PlatformInterface/IContext.h"
#include "internal/RendererLib/RendererConfig.h"
#include "internal/RendererLib/RendererLogContext.h"
#include "internal/RendererLib/LoggingDevice.h"
#include "internal/RendererLib/RendererCachedScene.h"
#include "internal/Core/Math3d/CameraMatrixHelper.h"
#include "internal/RendererLib/RenderExecutor.h"

namespace ramses::internal
{
    DisplayController::DisplayController(IRenderBackend& renderer, uint32_t /*samples*/)
        : m_renderBackend(renderer)
        , m_device(m_renderBackend.getDevice())
        , m_embeddedCompositingManager(m_device, m_renderBackend.getEmbeddedCompositor(), m_renderBackend.getTextureUploadingAdapter())
        , m_displayWidth(m_renderBackend.getWindow().getWidth())
        , m_displayHeight(m_renderBackend.getWindow().getHeight())
    {
    }

    void DisplayController::handleWindowEvents()
    {
        m_renderBackend.getWindow().handleEvents();
    }

    bool DisplayController::canRenderNewFrame() const
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

    SceneRenderExecutionIterator DisplayController::renderScene(const RendererCachedScene& scene, RenderingContext& renderContext, const FrameTimer* frameTimer)
    {
        RenderExecutor executor(m_renderBackend.getDevice(), renderContext, frameTimer);

        return executor.executeScene(scene);
    }

    void DisplayController::clearBuffer(DeviceResourceHandle buffer, ClearFlags clearFlags, const glm::vec4& clearColor)
    {
        if (clearFlags != EClearFlag::None)
        {
            m_device.activateRenderTarget(buffer);

            if (clearFlags.isSet(EClearFlag::Color))
            {
                m_device.colorMask(true, true, true, true);
                m_device.clearColor(clearColor);
            }
            if (clearFlags.isSet(EClearFlag::Depth))
                m_device.depthWrite(EDepthWrite::Enabled);

            m_device.scissorTest(EScissorTest::Disabled, {});
            m_device.clear(clearFlags);
        }
    }

    void DisplayController::readPixels(DeviceResourceHandle renderTargetHandle, uint32_t x, uint32_t y, uint32_t width, uint32_t height, std::vector<uint8_t>& dataOut)
    {
        m_device.activateRenderTarget(renderTargetHandle);
        dataOut.resize(width * height * 4u); // Assuming RGBA8 non multisampled
        m_device.readPixels(&dataOut[0], x, y, width, height);
    }

    uint32_t DisplayController::getDisplayWidth() const
    {
        return m_displayWidth;
    }

    uint32_t DisplayController::getDisplayHeight() const
    {
        return m_displayHeight;
    }

    DeviceResourceHandle DisplayController::getDisplayBuffer() const
    {
        return m_device.getFramebufferRenderTarget();
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
