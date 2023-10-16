//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/SceneTypes.h"
#include "internal/SceneGraph/SceneAPI/TextureEnums.h"
#include "internal/SceneGraph/SceneAPI/EDataBufferType.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"

namespace ramses::internal
{
    enum ESceneResourceType
    {
        ESceneResourceType_RenderBuffer_WriteOnly = 0,
        ESceneResourceType_RenderBuffer_ReadWrite,
        ESceneResourceType_DataBuffer,
        ESceneResourceType_TextureBuffer,
        ESceneResourceType_NUMBER_OF_ELEMENTS
    };

    class RendererSceneResourceRegistry
    {
    public:
        RendererSceneResourceRegistry();
        ~RendererSceneResourceRegistry();

        void                            addRenderBuffer             (RenderBufferHandle handle, DeviceResourceHandle deviceHandle, uint32_t size, bool writeOnly);
        void                            removeRenderBuffer          (RenderBufferHandle handle);
        [[nodiscard]] DeviceResourceHandle            getRenderBufferDeviceHandle (RenderBufferHandle handle) const;
        [[nodiscard]] uint32_t                          getRenderBufferByteSize     (RenderBufferHandle handle) const;
        void                            getAllRenderBuffers         (RenderBufferHandleVector& renderBuffers) const;

        void                            addRenderTarget             (RenderTargetHandle handle, DeviceResourceHandle deviceHandle);
        void                            removeRenderTarget          (RenderTargetHandle handle);
        [[nodiscard]] DeviceResourceHandle            getRenderTargetDeviceHandle (RenderTargetHandle handle) const;
        void                            getAllRenderTargets         (RenderTargetHandleVector& renderTargets) const;

        void                            addBlitPass                 (BlitPassHandle handle, DeviceResourceHandle srcRenderTargetDeviceHandle, DeviceResourceHandle dstRenderTargetDeviceHandle);
        void                            removeBlitPass              (BlitPassHandle handle);
        void                            getBlitPassDeviceHandles    (BlitPassHandle handle, DeviceResourceHandle& srcRenderTargetDeviceHandle, DeviceResourceHandle& dstRenderTargetDeviceHandle) const;
        void                            getAllBlitPasses            (BlitPassHandleVector& blitPasses) const;

        void                            addDataBuffer               (DataBufferHandle handle, DeviceResourceHandle deviceHandle, EDataBufferType dataBufferType, uint32_t size);
        void                            removeDataBuffer            (DataBufferHandle handle);
        [[nodiscard]] DeviceResourceHandle            getDataBufferDeviceHandle   (DataBufferHandle handle) const;
        [[nodiscard]] EDataBufferType                 getDataBufferType           (DataBufferHandle handle) const;
        void                            getAllDataBuffers           (DataBufferHandleVector& dataBuffers) const;

        void                               addTextureBuffer            (TextureBufferHandle handle, DeviceResourceHandle deviceHandle, EPixelStorageFormat format, uint32_t size);
        void                               removeTextureBuffer         (TextureBufferHandle handle);
        [[nodiscard]] DeviceResourceHandle getTextureBufferDeviceHandle(TextureBufferHandle handle) const;
        [[nodiscard]] EPixelStorageFormat  getTextureBufferFormat      (TextureBufferHandle handle) const;
        [[nodiscard]] uint32_t             getTextureBufferByteSize    (TextureBufferHandle handle) const;
        void                               getAllTextureBuffers        (TextureBufferHandleVector& textureBuffers) const;

        void                            addVertexArray(RenderableHandle renderableHandle, DeviceResourceHandle deviceHandle);
        void                            removeVertexArray(RenderableHandle renderableHandle);
        [[nodiscard]] DeviceResourceHandle            getVertexArrayDeviceHandle(RenderableHandle renderableHandle) const;
        void                            getAllVertexArrayRenderables(RenderableVector& vertexArrayRenderables) const;

        [[nodiscard]] uint32_t                          getSceneResourceMemoryUsage(ESceneResourceType resourceType) const;

    private:
        struct TextureBufferEntry
        {
            DeviceResourceHandle deviceHandle;
            uint32_t size = 0u;
            EPixelStorageFormat format = EPixelStorageFormat::Invalid;
        };

        struct RenderBufferEntry
        {
            DeviceResourceHandle deviceHandle;
            uint32_t size = 0u;
            bool writeOnly = false;
        };

        struct BlitPassEntry
        {
            DeviceResourceHandle sourceRenderTargetDeviceHandle;
            DeviceResourceHandle destinationRenderTargetDeviceHandle;
        };

        struct DataBufferEntry
        {
            DeviceResourceHandle deviceHandle;
            uint32_t size = 0u;
            EDataBufferType dataBufferType = EDataBufferType::Invalid;
        };

        using RenderBufferMap        = HashMap<RenderBufferHandle,   RenderBufferEntry>;
        using RenderTargetMap        = HashMap<RenderTargetHandle,   DeviceResourceHandle>;
        using BlitPassMap            = HashMap<BlitPassHandle,       BlitPassEntry>;
        using DataBufferMap          = HashMap<DataBufferHandle,     DataBufferEntry>;
        using TextureBufferMap       = HashMap<TextureBufferHandle,  TextureBufferEntry>;
        using TextureSamplerMap      = HashMap<TextureSamplerHandle, DeviceResourceHandle>;
        using VertexArrayMap         = HashMap<RenderableHandle,   DeviceResourceHandle>;

        RenderBufferMap        m_renderBuffers;
        RenderTargetMap        m_renderTargets;
        BlitPassMap            m_blitPasses;
        DataBufferMap          m_dataBuffers;
        TextureBufferMap       m_textureBuffers;
        VertexArrayMap         m_vertexArrays;
    };
}
