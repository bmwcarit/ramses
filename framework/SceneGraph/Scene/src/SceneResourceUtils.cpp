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

namespace
{
    inline void pushBackIfValid(ramses_internal::ResourceContentHashVector& vec, ramses_internal::ResourceContentHash const& hash)
    {
        if (hash.isValid())
            vec.push_back(hash);
    }
}

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
            assert(resources.empty());
            for (RenderableHandle rend(0); rend < scene.getRenderableCount(); ++rend)
            {
                if (!scene.isRenderableAllocated(rend))
                    continue;

                const auto& renderable = scene.getRenderable(rend);
                if (renderable.visibilityMode == EVisibilityMode::Off)
                    continue;

                for (auto type : {ERenderableDataSlotType_Geometry, ERenderableDataSlotType_Uniforms})
                {
                    const auto instanceHandle = renderable.dataInstances[type];
                    if (!instanceHandle.isValid() || !scene.isDataInstanceAllocated(instanceHandle))
                        continue;

                    const auto& layout = scene.getDataLayout(scene.getLayoutOfDataInstance(instanceHandle));
                    pushBackIfValid(resources, layout.getEffectHash());

                    for (DataFieldHandle fieldHandle(0u); fieldHandle < layout.getFieldCount(); ++fieldHandle)
                    {
                        const EDataType fieldType = layout.getField(fieldHandle).dataType;
                        if (IsBufferDataType(fieldType))
                        {
                            pushBackIfValid(resources, scene.getDataResource(instanceHandle, fieldHandle).hash);
                        }
                        else if (fieldType == EDataType_TextureSampler)
                        {
                            const auto samplerHandle = scene.getDataTextureSamplerHandle(instanceHandle, fieldHandle);
                            if (scene.isTextureSamplerAllocated(samplerHandle))
                            {
                                pushBackIfValid(resources, scene.getTextureSampler(samplerHandle).textureResource);
                            }
                        }
                    }
                }
            }

            for (StreamTextureHandle streamTexture(0u); streamTexture < scene.getStreamTextureCount(); ++streamTexture)
            {
                if (scene.isStreamTextureAllocated(streamTexture))
                {
                    pushBackIfValid(resources, scene.getStreamTexture(streamTexture).fallbackTexture);
                }
            }

            for (DataSlotHandle slot(0u); slot < scene.getDataSlotCount(); ++slot)
            {
                if (scene.isDataSlotAllocated(slot))
                {
                    pushBackIfValid(resources, scene.getDataSlot(slot).attachedTexture);
                }
            }

            std::sort(resources.begin(), resources.end());
            resources.erase(std::unique(resources.begin(), resources.end()), resources.end());
        }

        void DiffClientResources(ResourceContentHashVector const& old, ResourceContentHashVector const& curr, SceneResourceChanges& changes)
        {
            std::set_difference(curr.begin(), curr.end(), old.begin(), old.end(), std::back_inserter(changes.m_addedClientResourceRefs));
            std::set_difference(old.begin(), old.end(), curr.begin(), curr.end(), std::back_inserter(changes.m_removedClientResourceRefs));
        }
    }
}
