//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RendererSceneResourceRegistry.h"

namespace ramses_internal
{
    RendererSceneResourceRegistry::RendererSceneResourceRegistry()
    {
    }

    RendererSceneResourceRegistry::~RendererSceneResourceRegistry()
    {
        assert(m_renderBuffers.count() == 0u);
        assert(m_renderTargets.count() == 0u);
        assert(m_streamTextures.count() == 0u);
        assert(m_blitPasses.count() == 0u);
        assert(m_dataBuffers.count() == 0u);
        assert(m_textureBuffers.count() == 0u);
    }

    void RendererSceneResourceRegistry::addRenderBuffer(RenderBufferHandle handle, DeviceResourceHandle deviceHandle, UInt32 size, bool writeOnly)
    {
        assert(!m_renderBuffers.contains(handle));
        m_renderBuffers.put(handle, { deviceHandle, size, writeOnly });
    }

    void RendererSceneResourceRegistry::removeRenderBuffer(RenderBufferHandle handle)
    {
        assert(m_renderBuffers.contains(handle));
        m_renderBuffers.remove(handle);
    }

    DeviceResourceHandle RendererSceneResourceRegistry::getRenderBufferDeviceHandle(RenderBufferHandle handle) const
    {
        assert(m_renderBuffers.contains(handle));
        return m_renderBuffers.get(handle)->deviceHandle;
    }

    UInt32 RendererSceneResourceRegistry::getRenderBufferByteSize(RenderBufferHandle handle) const
    {
        assert(m_renderBuffers.contains(handle));
        return m_renderBuffers.get(handle)->size;
    }

    void RendererSceneResourceRegistry::getAllRenderBuffers(RenderBufferHandleVector& renderBuffers) const
    {
        assert(renderBuffers.empty());
        renderBuffers.reserve(m_renderBuffers.count());
        for(const auto& renderBuffer : m_renderBuffers)
        {
            renderBuffers.push_back(renderBuffer.key);
        }
    }

    void RendererSceneResourceRegistry::addRenderTarget(RenderTargetHandle handle, DeviceResourceHandle deviceHandle)
    {
        assert(!m_renderTargets.contains(handle));
        m_renderTargets.put(handle, { deviceHandle });
    }

    void RendererSceneResourceRegistry::removeRenderTarget(RenderTargetHandle handle)
    {
        assert(m_renderTargets.contains(handle));
        m_renderTargets.remove(handle);
    }

    DeviceResourceHandle RendererSceneResourceRegistry::getRenderTargetDeviceHandle(RenderTargetHandle handle) const
    {
        return m_renderTargets.get(handle)->deviceHandle;
    }

    void RendererSceneResourceRegistry::getAllRenderTargets(RenderTargetHandleVector& renderTargets) const
    {
        assert(renderTargets.empty());
        renderTargets.reserve(m_renderTargets.count());
        for(const auto& renderTarget : m_renderTargets)
        {
            renderTargets.push_back(renderTarget.key);
        }
    }

    void RendererSceneResourceRegistry::addBlitPass(BlitPassHandle handle, DeviceResourceHandle srcRenderTargetDeviceHandle, DeviceResourceHandle dstRenderTargetDeviceHandle)
    {
        assert(!m_blitPasses.contains(handle));
        BlitPassEntry bpEntry;
        bpEntry.sourceRenderTargetDeviceHandle = srcRenderTargetDeviceHandle;
        bpEntry.destinationRenderTargetDeviceHandle = dstRenderTargetDeviceHandle;
        m_blitPasses.put(handle, bpEntry);
    }

    void RendererSceneResourceRegistry::removeBlitPass(BlitPassHandle handle)
    {
        assert(m_blitPasses.contains(handle));
        m_blitPasses.remove(handle);
    }

    void RendererSceneResourceRegistry::getBlitPassDeviceHandles(BlitPassHandle handle, DeviceResourceHandle& srcRenderTargetDeviceHandle, DeviceResourceHandle& dstRenderTargetDeviceHandle) const
    {
        const BlitPassEntry* const bpEntry = m_blitPasses.get(handle);
        assert(bpEntry != NULL);
        srcRenderTargetDeviceHandle = bpEntry->sourceRenderTargetDeviceHandle;
        dstRenderTargetDeviceHandle = bpEntry->destinationRenderTargetDeviceHandle;
    }

    void RendererSceneResourceRegistry::getAllBlitPasses(BlitPassHandleVector& blitPasses) const
    {
        assert(blitPasses.empty());
        blitPasses.reserve(m_streamTextures.count());
        for(const auto& blitPass : m_blitPasses)
        {
            blitPasses.push_back(blitPass.key);
        }
    }

    void RendererSceneResourceRegistry::addStreamTexture(StreamTextureHandle handle, StreamTextureSourceId source)
    {
        assert(!m_streamTextures.contains(handle));
        m_streamTextures.put(handle, { source });
    }

    void RendererSceneResourceRegistry::removeStreamTexture(StreamTextureHandle handle)
    {
        assert(m_streamTextures.contains(handle));
        m_streamTextures.remove(handle);
    }

    StreamTextureSourceId RendererSceneResourceRegistry::getStreamTextureSourceId(StreamTextureHandle handle) const
    {
        assert(m_streamTextures.contains(handle));
        return m_streamTextures.get(handle)->id;
    }

    void RendererSceneResourceRegistry::getAllStreamTextures(StreamTextureHandleVector& streamTextures) const
    {
        assert(streamTextures.empty());
        streamTextures.reserve(m_streamTextures.count());
        for(const auto& streamTexture : m_streamTextures)
        {
            streamTextures.push_back(streamTexture.key);
        }
    }

    void RendererSceneResourceRegistry::addDataBuffer(DataBufferHandle handle, DeviceResourceHandle deviceHandle, EDataBufferType dataBufferType, UInt32 size)
    {
        assert(!m_dataBuffers.contains(handle));
        m_dataBuffers.put(handle, { deviceHandle, size, dataBufferType });
    }

    void RendererSceneResourceRegistry::removeDataBuffer(DataBufferHandle handle)
    {
        assert(m_dataBuffers.contains(handle));
        m_dataBuffers.remove(handle);
    }

    DeviceResourceHandle RendererSceneResourceRegistry::getDataBufferDeviceHandle(DataBufferHandle handle) const
    {
        assert(m_dataBuffers.contains(handle));
        return m_dataBuffers.get(handle)->deviceHandle;
    }

    EDataBufferType RendererSceneResourceRegistry::getDataBufferType(DataBufferHandle handle) const
    {
        assert(m_dataBuffers.contains(handle));
        return m_dataBuffers.get(handle)->dataBufferType;
    }

    void RendererSceneResourceRegistry::getAllDataBuffers(DataBufferHandleVector& dataBuffers) const
    {
        assert(dataBuffers.empty());
        dataBuffers.reserve(m_dataBuffers.count());
        for(const auto& db : m_dataBuffers)
        {
            dataBuffers.push_back(db.key);
        }
    }

    void RendererSceneResourceRegistry::addTextureBuffer(TextureBufferHandle handle, DeviceResourceHandle deviceHandle, ETextureFormat format, UInt32 size)
    {
        assert(!m_textureBuffers.contains(handle));
        m_textureBuffers.put(handle, { deviceHandle, size, format });
    }

    void RendererSceneResourceRegistry::removeTextureBuffer(TextureBufferHandle handle)
    {
        assert(m_textureBuffers.contains(handle));
        m_textureBuffers.remove(handle);
    }

    DeviceResourceHandle RendererSceneResourceRegistry::getTextureBufferDeviceHandle(TextureBufferHandle handle) const
    {
        assert(m_textureBuffers.contains(handle));
        return m_textureBuffers.get(handle)->deviceHandle;
    }

    ETextureFormat RendererSceneResourceRegistry::getTextureBufferFormat(TextureBufferHandle handle) const
    {
        assert(m_textureBuffers.contains(handle));
        return m_textureBuffers.get(handle)->format;
    }

    UInt32 RendererSceneResourceRegistry::getTextureBufferByteSize(TextureBufferHandle handle) const
    {
        assert(m_textureBuffers.contains(handle));
        return m_textureBuffers.get(handle)->size;
    }

    void RendererSceneResourceRegistry::getAllTextureBuffers(TextureBufferHandleVector& textureBuffers) const
    {
        assert(textureBuffers.empty());
        textureBuffers.reserve(m_textureBuffers.count());
        for (const auto& tb : m_textureBuffers)
        {
            textureBuffers.push_back(tb.key);
        }
    }

    void RendererSceneResourceRegistry::addTextureSampler(TextureSamplerHandle handle, DeviceResourceHandle deviceHandle)
    {
        assert(!m_textureSamplers.contains(handle));
        m_textureSamplers.put(handle, { deviceHandle });
    }

    void RendererSceneResourceRegistry::removeTextureSampler(TextureSamplerHandle handle)
    {
        assert(m_textureSamplers.contains(handle));
        m_textureSamplers.remove(handle);
    }

    DeviceResourceHandle RendererSceneResourceRegistry::getTextureSamplerDeviceHandle(TextureSamplerHandle handle) const
    {
        assert(m_textureSamplers.contains(handle));
        return m_textureSamplers.get(handle)->deviceHandle;
    }

    void RendererSceneResourceRegistry::getAllTextureSamplers(TextureSamplerHandleVector& textureSamplers) const
    {
        assert(textureSamplers.empty());
        textureSamplers.reserve(m_textureSamplers.count());
        for(const auto& ts: m_textureSamplers)
        {
            textureSamplers.push_back(ts.key);
        }
    }

    UInt32 RendererSceneResourceRegistry::getSceneResourceMemoryUsage(ESceneResourceType resourceType) const
    {
        UInt32 result = 0;
        switch (resourceType)
        {
        case ESceneResourceType_RenderBuffer_WriteOnly:
            for (const auto& rbEntry : m_renderBuffers)
            {
                if(rbEntry.value.writeOnly)
                    result += rbEntry.value.size;
            }
            break;
        case ESceneResourceType_RenderBuffer_ReadWrite:
            for (const auto& rbEntry : m_renderBuffers)
            {
                if (!rbEntry.value.writeOnly)
                    result += rbEntry.value.size;
            }
            break;
        case ESceneResourceType_DataBuffer:
            for (const auto& dataBuffer : m_dataBuffers)
            {
                result += dataBuffer.value.size;
            }
            break;
        case ESceneResourceType_TextureBuffer:
            for (const auto& texBuffer : m_textureBuffers)
            {
                result += texBuffer.value.size;
            }
            break;
        case ESceneResourceType_StreamTexture:
            // TODO Violin add stream texture memory
            break;
        default:
            assert(false && "Invalid scene resource type");
            break;
        }

        return result;
    }
}
