//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_GPUMEMORYSAMPLE_H
#define RAMSES_GPUMEMORYSAMPLE_H

#include "SceneAPI/SceneId.h"
#include "Resource/ResourceTypes.h"
#include "Collections/HashMap.h"
#include <array>

namespace ramses_internal
{
    class RendererSceneUpdater;

    class GpuMemorySample
    {
    public:
        explicit GpuMemorySample(const RendererSceneUpdater& updater);

        UInt64 getTotalMemoryUsage() const;
        UInt64 getTextureMemoryUsage() const;
        UInt64 getGeometryMemoryUsage() const;
        UInt64 getRenderbufferMemoryUsage() const;

        SceneIdVector getSampledScenes() const;
        UInt64 getTotalSceneMemoryUsage(SceneId sceneId) const;

        // This is technically incorrect, because resources might be reused across scenes
        // However, this still works as an upper bound memory estimate, and VRAM is always an estimate
        void increaseSceneMemUsageForClientResource(SceneId sceneId, EResourceType type, UInt32 memUsageBytes)
        {
            m_memUsedPerScene[sceneId].memUsagePerClientResType[type] += memUsageBytes;
            m_memUsedPerScene[sceneId].memUsage_Total += memUsageBytes;
            m_totalMemoryUsage += memUsageBytes;

            switch (type)
            {
            case EResourceType_Texture2D:
            case EResourceType_Texture3D:
            case EResourceType_TextureCube:
                m_textureMemoryUsage += memUsageBytes;
                break;

            case EResourceType_VertexArray:
            case EResourceType_IndexArray:
                m_geometryMemoryUsage += memUsageBytes;
                break;
            default:
                break;
            }
        }

        void increaseSceneMemUsageForSceneRenderBuffer(SceneId sceneId, UInt32 memUsageBytes)
        {
            m_memUsedPerScene[sceneId].memUsage_SceneRenderBuffers += memUsageBytes;
            m_memUsedPerScene[sceneId].memUsage_Total += memUsageBytes;
            m_renderbufferMemoryUsage += memUsageBytes;
            m_totalMemoryUsage += memUsageBytes;
        }

        void increaseSceneMemUsageForSceneDataBuffer(SceneId sceneId, UInt32 memUsageBytes)
        {
            m_memUsedPerScene[sceneId].memUsage_SceneDataBuffers += memUsageBytes;
            m_memUsedPerScene[sceneId].memUsage_Total += memUsageBytes;
            m_geometryMemoryUsage += memUsageBytes;
            m_totalMemoryUsage += memUsageBytes;
        }

        void increaseSceneMemUsageForSceneTextureBuffer(SceneId sceneId, UInt32 memUsageBytes)
        {
            m_memUsedPerScene[sceneId].memUsage_SceneTextureBuffers += memUsageBytes;
            m_memUsedPerScene[sceneId].memUsage_Total += memUsageBytes;
            m_textureMemoryUsage += memUsageBytes;
            m_totalMemoryUsage += memUsageBytes;
        }

        void increaseSceneMemUsageForSceneStreamTexture(SceneId sceneId, UInt32 memUsageBytes)
        {
            m_memUsedPerScene[sceneId].memUsage_StreamTextures += memUsageBytes;
            m_memUsedPerScene[sceneId].memUsage_Total += memUsageBytes;
            m_textureMemoryUsage += memUsageBytes;
            m_totalMemoryUsage += memUsageBytes;
        }

        void increaseMemUsageForOffscreenBuffer(UInt32 memUsageBytes)
        {
            // TODO Violin make this more explicit... Currently, offscreen buffers have fixed format
            // and always take up half/half memory for color and depth data respectively
            m_textureMemoryUsage += memUsageBytes / 2;
            m_renderbufferMemoryUsage += memUsageBytes / 2;
            m_totalMemoryUsage += memUsageBytes;
        }

    private:
        struct SceneMemoryUsage
        {
            std::array<UInt64, EResourceType_NUMBER_OF_ELEMENTS> memUsagePerClientResType;

            UInt64 memUsage_SceneRenderBuffers = 0;
            UInt64 memUsage_SceneDataBuffers = 0;
            UInt64 memUsage_SceneTextureBuffers = 0;
            UInt64 memUsage_StreamTextures = 0;
            UInt64 memUsage_Total = 0;
        };

        HashMap<SceneId, SceneMemoryUsage> m_memUsedPerScene;
        UInt64 m_totalMemoryUsage = 0u;
        UInt64 m_textureMemoryUsage = 0u;
        UInt64 m_geometryMemoryUsage = 0u;
        UInt64 m_renderbufferMemoryUsage = 0u;
    };
}

#endif
