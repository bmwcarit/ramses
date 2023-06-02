//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYCONTROLLER_H
#define RAMSES_DISPLAYCONTROLLER_H

#include "RendererAPI/IDisplayController.h"
#include "Math3d/CameraMatrixHelper.h"
#include "EmbeddedCompositingManager.h"
#include <memory>


namespace ramses_internal
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
        explicit DisplayController(IRenderBackend& renderer, UInt32 samples = 1);

        void                    handleWindowEvents() override;
        [[nodiscard]] bool                    canRenderNewFrame() const override;
        void                    enableContext() override;
        void                    swapBuffers() override;
        SceneRenderExecutionIterator renderScene(const RendererCachedScene& scene, RenderingContext& renderContext, const FrameTimer* frameTimer = nullptr) override;
        void                    clearBuffer(DeviceResourceHandle buffer, uint32_t clearFlags, const glm::vec4& clearColor) override;

        [[nodiscard]] DeviceResourceHandle    getDisplayBuffer() const final override;
        [[nodiscard]] IRenderBackend&         getRenderBackend() const override;
        IEmbeddedCompositingManager& getEmbeddedCompositingManager() override;
        [[nodiscard]] UInt32                  getDisplayWidth() const override;
        [[nodiscard]] UInt32                  getDisplayHeight() const override;

        void                    readPixels(DeviceResourceHandle renderTargetHandle, UInt32 x, UInt32 y, UInt32 width, UInt32 height, std::vector<uint8_t>& dataOut) override;

        void validateRenderingStatusHealthy() const override;

    private:
        IRenderBackend&         m_renderBackend;
        IDevice&                m_device;
        EmbeddedCompositingManager m_embeddedCompositingManager;

        const UInt32            m_displayWidth;
        const UInt32            m_displayHeight;
    };
}

#endif
