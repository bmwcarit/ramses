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
#include "EmbeddedCompositingManager.h"
#include <memory>


namespace ramses::internal
{
    class RendererConfig;
    class IRenderBackend;
    class RendererCachedScene;
    class VirtualUpdateScene;
    class IDevice;

    class DisplayController : public IDisplayController
    {
        friend class RendererLogger;

    public:
        explicit DisplayController(IRenderBackend& renderer, uint32_t samples = 1);

        void                    handleWindowEvents() override;
        [[nodiscard]] bool                    canRenderNewFrame() const override;
        void                    enableContext() override;
        void                    swapBuffers() override;
        SceneRenderExecutionIterator renderScene(const RendererCachedScene& scene, RenderingContext& renderContext, const FrameTimer* frameTimer) override;
        void                    clearBuffer(DeviceResourceHandle buffer, ClearFlags clearFlags, const glm::vec4& clearColor) override;

        [[nodiscard]] DeviceResourceHandle    getDisplayBuffer() const final override;
        [[nodiscard]] IRenderBackend&         getRenderBackend() const override;
        IEmbeddedCompositingManager& getEmbeddedCompositingManager() override;
        [[nodiscard]] uint32_t                  getDisplayWidth() const override;
        [[nodiscard]] uint32_t                  getDisplayHeight() const override;

        void readPixels(DeviceResourceHandle renderTargetHandle, uint32_t x, uint32_t y, uint32_t width, uint32_t height, std::vector<uint8_t>& dataOut) override;

        void validateRenderingStatusHealthy() const override;

    private:
        IRenderBackend&         m_renderBackend;
        IDevice&                m_device;
        EmbeddedCompositingManager m_embeddedCompositingManager;

        const uint32_t            m_displayWidth;
        const uint32_t            m_displayHeight;
    };
}
