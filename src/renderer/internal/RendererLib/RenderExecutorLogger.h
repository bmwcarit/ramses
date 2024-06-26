//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/RenderExecutor.h"

namespace ramses::internal
{
    class RendererLogContext;
    struct RenderingContext;

    class RenderExecutorLogger : public RenderExecutor
    {
    public:
        RenderExecutorLogger(IDevice& device, RenderingContext& renderContext, RendererLogContext& context);

        // Equivalent to RenderExecutor::executeScene(), but logs instead of rendering
        void logScene(const RendererCachedScene& scene) const;

    private:
        void logDirtyResources(const RendererCachedScene& scene, RenderableHandle renderable) const;
        void logRenderPass(const RendererCachedScene& scene, const RenderPassHandle pass) const;
        void logBlitPass(const RendererCachedScene& scene, const BlitPassHandle pass) const;
        static DeviceResourceHandle GetDeviceHandleForRenderBuffer(RenderBufferHandle rbHandle, const RendererCachedScene& scene);

        RendererLogContext& m_logContext;
    };

}
