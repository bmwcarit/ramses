//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "RendererLib/GpuMemorySample.h"
#include "RendererLib/RendererSceneUpdater.h"
#include "RendererLib/RendererResourceManager.h"

namespace ramses_internal
{
    GpuMemorySample::GpuMemorySample(const RendererSceneUpdater& updater)
    {
        // A realistic estimate of number of scenes
        m_memUsedPerScene.reserve(100);

        for (const auto& resourceManagerEntry : updater.m_displayResourceManagers)
        {
            const RendererResourceManager* resourceManager = static_cast<const RendererResourceManager*>(resourceManagerEntry.value);

            // Add uploaded client resources
            for (auto& resourceDescIter : resourceManager->m_clientResourceRegistry.getAllResourceDescriptors())
            {
                const auto& resourceDescriptor = resourceDescIter.value;

                if (EResourceStatus_Uploaded == resourceDescriptor.status)
                {
                    for (const auto& sceneUsage : resourceDescriptor.sceneUsage)
                    {
                        increaseSceneMemUsageForClientResource(sceneUsage, resourceDescriptor.type, resourceDescriptor.vramSize);
                    }
                }
            }

            // Add all scene resources, assume they are uploaded
            for (auto& sceneResourceUsageIter : resourceManager->m_sceneResourceRegistryMap)
            {
                SceneId sceneId = sceneResourceUsageIter.key;
                const RendererSceneResourceRegistry& sceneResourceUsage = sceneResourceUsageIter.value;

                increaseSceneMemUsageForSceneRenderBuffer(sceneId, sceneResourceUsage.getSceneResourceMemoryUsage(ESceneResourceType_RenderBuffer_WriteOnly));
                increaseSceneMemUsageForSceneDataBuffer(sceneId, sceneResourceUsage.getSceneResourceMemoryUsage(ESceneResourceType_DataBuffer));
                increaseSceneMemUsageForSceneTextureBuffer(sceneId,
                    sceneResourceUsage.getSceneResourceMemoryUsage(ESceneResourceType_TextureBuffer) +
                    sceneResourceUsage.getSceneResourceMemoryUsage(ESceneResourceType_RenderBuffer_ReadWrite) +
                    sceneResourceUsage.getSceneResourceMemoryUsage(ESceneResourceType_StreamTexture));
            }

            for (OffscreenBufferHandle i(0); i < resourceManager->m_offscreenBuffers.getTotalCount(); ++i)
            {
                if (resourceManager->m_offscreenBuffers.isAllocated(i))
                {
                    increaseMemUsageForOffscreenBuffer(resourceManager->m_offscreenBuffers.getMemory(i)->m_estimatedVRAMUsage);
                }
            }
        }
    }

    UInt64 GpuMemorySample::getTotalSceneMemoryUsage(SceneId sceneId) const
    {
        if (m_memUsedPerScene.contains(sceneId))
        {
            const SceneMemoryUsage& sceneUsage = *m_memUsedPerScene.get(sceneId);
            return sceneUsage.memUsage_Total;
        }

        return 0u;
    }

    UInt64 GpuMemorySample::getTotalMemoryUsage() const
    {
        return m_totalMemoryUsage;
    }

    UInt64 GpuMemorySample::getTextureMemoryUsage() const
    {
        return m_textureMemoryUsage;
    }

    UInt64 GpuMemorySample::getGeometryMemoryUsage() const
    {
        return m_geometryMemoryUsage;
    }

    UInt64 GpuMemorySample::getRenderbufferMemoryUsage() const
    {
        return m_renderbufferMemoryUsage;
    }

    SceneIdVector GpuMemorySample::getSampledScenes() const
    {
        SceneIdVector sampledScenes;
        sampledScenes.reserve(m_memUsedPerScene.count());
        for (const auto& scene : m_memUsedPerScene)
        {
            sampledScenes.push_back(scene.key);
        }
        return sampledScenes;
    }

}
