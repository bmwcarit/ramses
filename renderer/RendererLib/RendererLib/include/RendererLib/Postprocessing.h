//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_POSTPROCESSING_H
#define RAMSES_POSTPROCESSING_H

#include "SceneAPI/Handles.h"
#include "RendererAPI/Types.h"

namespace ramses_internal
{
    class IDevice;
    class WarpingPass;
    class WarpingMeshData;

    class Postprocessing
    {
    public:
        explicit Postprocessing(UInt32 effectIds, UInt32 width, UInt32 height, IDevice& device);
        ~Postprocessing();

        void execute();

        UInt32 getPostEffectsMask() const;
        // The render target used to render all scenes (the framebuffer, if no postprocessing effects enabled)
        DeviceResourceHandle getScenesRenderTarget() const;
        DeviceResourceHandle getFramebuffer() const;

        void setWarpingMeshData(const WarpingMeshData& warpingMeshData);

    private:
        const UInt32            m_postEffectsMask;
        IDevice&                m_device;

        DeviceResourceHandle    m_scenesColorBuffer;
        DeviceResourceHandle    m_sceneDepthStencilBuffer;
        DeviceResourceHandle    m_scenesRenderTarget;

        const UInt32            m_displayWidth;
        const UInt32            m_displayHeight;

        const DeviceResourceHandle m_framebuffer;

        WarpingPass* m_warpingPass;
    };
}

#endif
