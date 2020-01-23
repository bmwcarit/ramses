//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Scene/SceneResourceUtils.h"
#include "SceneAPI/IScene.h"
#include "SceneAPI/Renderable.h"
#include "SceneAPI/TextureSampler.h"
#include "SceneAPI/StreamTexture.h"
#include "SceneAPI/GeometryDataBuffer.h"
#include "SceneAPI/TextureBuffer.h"
#include "Scene/DataLayout.h"

namespace ramses_internal
{
    namespace SceneResourceUtils
    {
        void GetAllSceneResourcesFromScene(SceneResourceActionVector& actions, const IScene& scene, size_t& usedDataByteSize)
        {
            assert(actions.empty());

            // collect all scene resources currently in use by scene and mark as pending to be uploaded
            const UInt32 numRenderBuffers = scene.getRenderBufferCount();
            const UInt32 numRenderTargets = scene.getRenderTargetCount();
            const UInt32 numStreamTextures = scene.getStreamTextureCount();
            const UInt32 numBlitPasses = scene.getBlitPassCount();
            const UInt32 numDataBuffers = scene.getDataBufferCount();
            const UInt32 numTextureBuffers = scene.getTextureBufferCount();
            const UInt32 numSceneResources = numRenderTargets + numRenderBuffers + numStreamTextures + numBlitPasses + numDataBuffers * 2u + numTextureBuffers * 2u;

            actions.reserve(numSceneResources);
            usedDataByteSize = 0u;

            for (RenderBufferHandle rbHandle(0u); rbHandle < numRenderBuffers; ++rbHandle)
            {
                if (scene.isRenderBufferAllocated(rbHandle))
                {
                    actions.push_back({ rbHandle.asMemoryHandle(), ESceneResourceAction_CreateRenderBuffer });
                }
            }
            for (RenderTargetHandle rtHandle(0u); rtHandle < numRenderTargets; ++rtHandle)
            {
                if (scene.isRenderTargetAllocated(rtHandle))
                {
                    actions.push_back({ rtHandle.asMemoryHandle(), ESceneResourceAction_CreateRenderTarget });
                }
            }
            for (StreamTextureHandle texHandle(0u); texHandle < numStreamTextures; ++texHandle)
            {
                if (scene.isStreamTextureAllocated(texHandle))
                {
                    actions.push_back({ texHandle.asMemoryHandle(), ESceneResourceAction_CreateStreamTexture });
                }
            }
            for (BlitPassHandle bpHandle(0u); bpHandle < numBlitPasses; ++bpHandle)
            {
                if (scene.isBlitPassAllocated(bpHandle))
                {
                    actions.push_back({ bpHandle.asMemoryHandle(), ESceneResourceAction_CreateBlitPass });
                }
            }

            for (DataBufferHandle dbHandle(0u); dbHandle < numDataBuffers; ++dbHandle)
            {
                if (scene.isDataBufferAllocated(dbHandle))
                {
                    actions.push_back({ dbHandle.asMemoryHandle(), ESceneResourceAction_CreateDataBuffer });
                    actions.push_back({ dbHandle.asMemoryHandle(), ESceneResourceAction_UpdateDataBuffer });
                    usedDataByteSize += scene.getDataBuffer(dbHandle).usedSize;
                }
            }

            for (TextureBufferHandle tbHandle(0u); tbHandle < numTextureBuffers; ++tbHandle)
            {
                if (scene.isTextureBufferAllocated(tbHandle))
                {
                    actions.push_back({ tbHandle.asMemoryHandle(), ESceneResourceAction_CreateTextureBuffer });
                    actions.push_back({ tbHandle.asMemoryHandle(), ESceneResourceAction_UpdateTextureBuffer });
                    usedDataByteSize += TextureBuffer::GetMipMapDataSizeInBytes(scene.getTextureBuffer(tbHandle));
                }
            }
        }

        void GetAllClientResourcesFromScene(ResourceContentHashVector& resources, const IScene& scene)
        {
            HashSet<ResourceContentHash> resourceSet;
            resourceSet.reserve(scene.getDataInstanceCount() + scene.getRenderableCount());

            for (DataInstanceHandle instance(0u); instance < scene.getDataInstanceCount(); ++instance)
            {
                if (scene.isDataInstanceAllocated(instance))
                {
                    const DataLayoutHandle layoutHandle = scene.getLayoutOfDataInstance(instance);
                    const DataLayout& layout = scene.getDataLayout(layoutHandle);

                    for (DataFieldHandle field(0u); field < layout.getFieldCount(); ++field)
                    {
                        const EDataType fieldType = layout.getField(field).dataType;
                        if (IsBufferDataType(fieldType))
                        {
                            resourceSet.put(scene.getDataResource(instance, field).hash);
                        }
                    }
                }
            }

            for (DataLayoutHandle layout(0u); layout < scene.getDataLayoutCount(); ++layout)
            {
                if (scene.isDataLayoutAllocated(layout))
                {
                    resourceSet.put(scene.getDataLayout(layout).getEffectHash());
                }
            }

            for (TextureSamplerHandle sampler(0u); sampler < scene.getTextureSamplerCount(); ++sampler)
            {
                if (scene.isTextureSamplerAllocated(sampler))
                {
                    resourceSet.put(scene.getTextureSampler(sampler).textureResource);
                }
            }

            for (StreamTextureHandle streamTexture(0u); streamTexture < scene.getStreamTextureCount(); ++streamTexture)
            {
                if (scene.isStreamTextureAllocated(streamTexture))
                {
                    resourceSet.put(scene.getStreamTexture(streamTexture).fallbackTexture);
                }
            }

            for (DataSlotHandle slot(0u); slot < scene.getDataSlotCount(); ++slot)
            {
                if (scene.isDataSlotAllocated(slot))
                {
                    resourceSet.put(scene.getDataSlot(slot).attachedTexture);
                }
            }

            resourceSet.remove(ResourceContentHash::Invalid());
            resources.reserve(resourceSet.size());
            for (const auto hash : resourceSet)
            {
                resources.push_back(hash);
            }
        }

        void GetSceneResourceChangesFromScene(SceneResourceChanges& changes, const IScene& scene, size_t& sceneResourcesUsedDataByteSize)
        {
            assert(changes.m_addedClientResourceRefs.empty());
            assert(changes.m_removedClientResourceRefs.empty());
            assert(changes.m_sceneResourceActions.empty());

            GetAllSceneResourcesFromScene(changes.m_sceneResourceActions, scene, sceneResourcesUsedDataByteSize);
            GetAllClientResourcesFromScene(changes.m_addedClientResourceRefs, scene);
        }
    }
}
