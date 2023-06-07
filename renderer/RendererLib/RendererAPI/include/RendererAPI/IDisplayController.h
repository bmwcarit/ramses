//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IDISPLAYCONTROLLER_H
#define RAMSES_IDISPLAYCONTROLLER_H

#include "RendererAPI/Types.h"
#include "RendererAPI/SceneRenderExecutionIterator.h"
#include "DataTypesImpl.h"

namespace ramses_internal
{
    class IRenderBackend;
    class IEmbeddedCompositingManager;
    struct RenderingContext;
    class RendererCachedScene;
    class ProjectionParams;
    class FrameTimer;

    class IDisplayController
    {
    public:
        virtual ~IDisplayController() {};

        virtual void                    handleWindowEvents() = 0;
        [[nodiscard]] virtual bool                    canRenderNewFrame() const = 0;
        virtual void                    enableContext() = 0;
        virtual void                    swapBuffers() = 0;
        virtual SceneRenderExecutionIterator renderScene(const RendererCachedScene& scene, RenderingContext& renderContext, const FrameTimer* frameTimer = nullptr) = 0;
        virtual void                    clearBuffer(DeviceResourceHandle buffer, uint32_t clearFlags, const glm::vec4& clearColor) = 0;

        [[nodiscard]] virtual DeviceResourceHandle    getDisplayBuffer() const = 0;
        [[nodiscard]] virtual IRenderBackend&         getRenderBackend() const = 0;
        virtual IEmbeddedCompositingManager& getEmbeddedCompositingManager() = 0;
        [[nodiscard]] virtual uint32_t                  getDisplayWidth() const = 0;
        [[nodiscard]] virtual uint32_t                  getDisplayHeight() const = 0;

        virtual void                    readPixels(DeviceResourceHandle renderTargetHandle, uint32_t x, uint32_t y, uint32_t width, uint32_t height, std::vector<uint8_t>& dataOut) = 0;

        virtual void                    validateRenderingStatusHealthy() const = 0;
    };
}

#endif
