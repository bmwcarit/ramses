//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RendererSceneResourceRegistry.h"

namespace ramses::internal
{
    RendererSceneResourceRegistry::RendererSceneResourceRegistry() = default;

    RendererSceneResourceRegistry::~RendererSceneResourceRegistry()
    {
        assert(m_renderBuffers.size() == 0u);
        assert(m_renderTargets.size() == 0u);
        assert(m_blitPasses.size() == 0u);
        assert(m_dataBuffers.size() == 0u);
        assert(m_textureBuffers.size() == 0u);
        assert(m_vertexArrays.size() == 0u);
        assert(m_uniformBuffers.size() == 0u);
        assert(m_semanticUniformBuffers.size() == 0u);
    }

    void RendererSceneResourceRegistry::getBlitPassDeviceHandles(BlitPassHandle handle, DeviceResourceHandle& srcRenderTargetDeviceHandle, DeviceResourceHandle& dstRenderTargetDeviceHandle) const
    {
        const auto& bpEntry = get(handle);
        srcRenderTargetDeviceHandle = bpEntry.sourceRenderTargetDeviceHandle;
        dstRenderTargetDeviceHandle = bpEntry.destinationRenderTargetDeviceHandle;
    }

    uint32_t RendererSceneResourceRegistry::getSceneResourceMemoryUsage(ESceneResourceType resourceType) const
    {
        uint32_t result = 0;
        switch (resourceType)
        {
        case ESceneResourceType_RenderBuffer_WriteOnly:
            for (const auto& rbEntry : m_renderBuffers)
            {
                if(rbEntry.value.renderBufferProperties.accessMode == ERenderBufferAccessMode::WriteOnly)
                    result += rbEntry.value.size;
            }
            break;
        case ESceneResourceType_RenderBuffer_ReadWrite:
            for (const auto& rbEntry : m_renderBuffers)
            {
                if (rbEntry.value.renderBufferProperties.accessMode == ERenderBufferAccessMode::ReadWrite)
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
        default:
            assert(false && "Invalid scene resource type");
            break;
        }

        return result;
    }
}
