//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IRESOURCEDEVICEHANDLEACCESSOR_H
#define RAMSES_IRESOURCEDEVICEHANDLEACCESSOR_H

#include "SceneAPI/ResourceContentHash.h"
#include "SceneAPI/Handles.h"
#include "SceneAPI/SceneId.h"
#include "RendererAPI/Types.h"

namespace ramses_internal
{
    class IResourceDeviceHandleAccessor
    {
    public:
        virtual ~IResourceDeviceHandleAccessor() {}

        [[nodiscard]] virtual DeviceResourceHandle getResourceDeviceHandle(const ResourceContentHash& resourceHash) const = 0;
        [[nodiscard]] virtual DeviceResourceHandle getRenderTargetDeviceHandle(RenderTargetHandle targetHandle, SceneId sceneId) const = 0;
        [[nodiscard]] virtual DeviceResourceHandle getRenderTargetBufferDeviceHandle(RenderBufferHandle bufferHandle, SceneId sceneId) const = 0;
        virtual void                 getBlitPassRenderTargetsDeviceHandle(BlitPassHandle blitPassHandle, SceneId sceneId, DeviceResourceHandle& srcRT, DeviceResourceHandle& dstRT) const = 0;
        [[nodiscard]] virtual DeviceResourceHandle getOffscreenBufferDeviceHandle(OffscreenBufferHandle bufferHandle) const = 0;
        [[nodiscard]] virtual DeviceResourceHandle getOffscreenBufferColorBufferDeviceHandle(OffscreenBufferHandle bufferHandle) const = 0;
        [[nodiscard]] virtual int                  getDmaOffscreenBufferFD(OffscreenBufferHandle bufferHandle) const = 0;
        [[nodiscard]] virtual uint32_t               getDmaOffscreenBufferStride(OffscreenBufferHandle bufferHandle) const = 0;
        [[nodiscard]] virtual OffscreenBufferHandle getOffscreenBufferHandle(DeviceResourceHandle bufferDeviceHandle) const = 0;
        [[nodiscard]] virtual DeviceResourceHandle getStreamBufferDeviceHandle(StreamBufferHandle bufferHandle) const = 0;
        [[nodiscard]] virtual DeviceResourceHandle getExternalBufferDeviceHandle(ExternalBufferHandle bufferHandle) const = 0;
        [[nodiscard]] virtual DeviceResourceHandle getEmptyExternalBufferDeviceHandle() const = 0;
        [[nodiscard]] virtual uint32_t             getExternalBufferGlId(ExternalBufferHandle externalTexHandle) const = 0;
        [[nodiscard]] virtual DeviceResourceHandle getDataBufferDeviceHandle(DataBufferHandle dataBufferHandle, SceneId sceneId) const = 0;
        [[nodiscard]] virtual DeviceResourceHandle getTextureBufferDeviceHandle(TextureBufferHandle textureBufferHandle, SceneId sceneId) const = 0;
        [[nodiscard]] virtual DeviceResourceHandle getVertexArrayDeviceHandle(RenderableHandle renderableHandle, SceneId sceneId) const = 0;
    };
}
#endif
