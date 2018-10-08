//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Scene/SceneDescriber.h"
#include "Scene/SceneActionCollectionCreator.h"
#include "Scene/ClientScene.h"
#include "SceneAPI/Handles.h"
#include "SceneAPI/RenderGroup.h"
#include "SceneAPI/PixelRectangle.h"
#include "SceneAPI/TextureSamplerStates.h"
#include "SceneAPI/StreamTexture.h"
#include "Animation/AnimationSystemDescriber.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "Utils/MemoryUtils.h"

namespace ramses_internal
{
    template <typename T>
    void SceneDescriber::describeScene(const T& source, SceneActionCollectionCreator& collector)
    {
        // 25% more than node+transform+renderable count
        const UInt numberOfSceneActionsEstimate =
            (source.getNodeCount() + source.getTransformCount() + source.getRenderableCount()) * 125 / 100;
        // circa 30 bytes per action
        const UInt sizeOfSceneActionsEstimate = 30u * numberOfSceneActionsEstimate;
        collector.collection.reserveAdditionalCapacity(sizeOfSceneActionsEstimate, numberOfSceneActionsEstimate);

        RecreateNodes(                   source, collector);
        RecreateTransformNodes(          source, collector);
        RecreateTransformations(         source, collector);
        RecreateRenderables(             source, collector);
        RecreateStates(                  source, collector);
        RecreateDataLayouts(             source, collector);
        RecreateDataInstances(           source, collector);
        RecreateCameras(                 source, collector);
        RecreateAnimationSystems(        source, collector);
        RecreateRenderGroups(            source, collector);
        RecreateRenderPasses(            source, collector);
        RecreateBlitPasses(              source, collector);
        RecreateDataBuffers(             source, collector);
        RecreateTextureBuffers(          source, collector);
        RecreateTextureSamplers(         source, collector);
        RecreateRenderBuffersAndTargets( source, collector);
        RecreateStreamTextures(          source, collector);
        RecreateDataSlots(               source, collector);
        RecreateSceneVersionTag(         source, collector);
    }

    void SceneDescriber::RecreateNodes(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const UInt32 totalNodeCount = source.getNodeCount();
        for (NodeHandle n(0u); n < totalNodeCount; ++n)
        {
            if (source.isNodeAllocated(n))
            {
                collector.allocateNode(source.getChildCount(n), n);
            }
        }
        for (NodeHandle n(0u); n < totalNodeCount; ++n)
        {
            if (source.isNodeAllocated(n))
            {
                // Copy children
                const UInt32 childCount = source.getChildCount(n);
                for (UInt32 child = 0; child < childCount; ++child)
                {
                    collector.addChildToNode(n, source.getChild(n, child));
                }
            }
        }
    }

    void SceneDescriber::RecreateCameras(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const UInt32 totalCameraCount = source.getCameraCount();
        for (CameraHandle c(0u); c < totalCameraCount; ++c)
        {
            if (source.isCameraAllocated(c))
            {
                const Camera& camera = source.getCamera(c);
                collector.allocateCamera(camera.projectionType, camera.node, c);

                collector.setCameraViewport(c, camera.viewport);
                if (camera.projectionType != ECameraProjectionType_Renderer)
                {
                    collector.setCameraFrustum(c, camera.frustum);
                }
            }
        }
    }

    void SceneDescriber::RecreateTransformNodes(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const UInt32 totalTransformCount = source.getTransformCount();
        for (TransformHandle t(0u); t < totalTransformCount; ++t)
        {
            if (source.isTransformAllocated(t))
            {
                collector.allocateTransform(source.getTransformNode(t), t);
            }
        }
    }

    void SceneDescriber::RecreateTransformations(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const UInt32 totalTransformCount = source.getTransformCount();
        for (TransformHandle t(0u); t < totalTransformCount; ++t)
        {
            if (source.isTransformAllocated(t))
            {
                const Vector3& translation = source.getTranslation(t);
                if (translation != Vector3::Empty)
                {
                    collector.setTransformComponent(ETransformPropertyType_Translation, t, translation);
                }
                const Vector3& rotation = source.getRotation(t);
                if (rotation != Vector3::Empty)
                {
                    collector.setTransformComponent(ETransformPropertyType_Rotation, t, rotation);
                }
                const Vector3& scaling = source.getScaling(t);
                if (scaling != Vector3::Identity)
                {
                    collector.setTransformComponent(ETransformPropertyType_Scaling, t, scaling);
                }
            }
        }
    }

    void SceneDescriber::RecreateRenderables(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const UInt32 totalRenderableCount = source.getRenderableCount();
        for (RenderableHandle r(0u); r < totalRenderableCount; ++r)
        {
            if (source.isRenderableAllocated(r))
            {
                collector.compoundRenderable(r, source.getRenderable(r));
            }
        }
    }

    void SceneDescriber::RecreateStates(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const UInt32 totalStateCount = source.getRenderStateCount();
        for (RenderStateHandle s(0u); s < totalStateCount; ++s)
        {
            if (source.isRenderStateAllocated(s))
            {
                collector.compoundState(s, source.getRenderState(s));
            }
        }
    }

    void SceneDescriber::RecreateDataLayouts(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const UInt32 totalDataLayoutCount = source.getDataLayoutCount();
        for (DataLayoutHandle l(0u); l < totalDataLayoutCount; ++l)
        {
            if (source.isDataLayoutAllocated(l))
            {
                const DataLayout& layout = source.getDataLayout(l);
                collector.allocateDataLayout(layout.getDataFields(), l);
            }
        }
    }

    // overload for a client scene which compacts data layouts and those must be expanded while serializing
    void SceneDescriber::RecreateDataLayouts(const ClientScene& source, SceneActionCollectionCreator& collector)
    {
        const UInt32 totalDataLayoutCount = source.getDataLayoutCount();
        for (DataLayoutHandle l(0u); l < totalDataLayoutCount; ++l)
        {
            if (source.isDataLayoutAllocated(l))
            {
                const DataFieldInfoVector& layoutFields = source.getDataLayout(l).getDataFields();

                const UInt32 numRefs = source.getNumDataLayoutReferences(l);
                for (UInt32 r = 0u; r < numRefs; ++r)
                {
                    collector.allocateDataLayout(layoutFields, l);
                }
            }
        }
    }

    void SceneDescriber::RecreateDataInstances(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const UInt32 totalDataInstanceCount = source.getDataInstanceCount();
        for (DataInstanceHandle i(0u); i < totalDataInstanceCount; ++i)
        {
            if (source.isDataInstanceAllocated(i))
            {
                const DataLayoutHandle layoutHandle = source.getLayoutOfDataInstance(i);
                collector.allocateDataInstance(layoutHandle, i);

                const DataLayout& layout = source.getDataLayout(layoutHandle);
                for (DataFieldHandle f(0u); f < layout.getFieldCount(); ++f)
                {
                    const DataFieldInfo& field = layout.getField(f);
                    const UInt32 elementCount = field.elementCount;

                    switch (field.dataType)
                    {
                    case EDataType_Float:
                    {
                        const Float* value = source.getDataFloatArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataFloatArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType_Vector2F:
                    {
                        const Vector2* value = source.getDataVector2fArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataVector2fArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType_Vector3F:
                    {
                        const Vector3* value = source.getDataVector3fArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataVector3fArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType_Vector4F:
                    {
                        const Vector4* value = source.getDataVector4fArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataVector4fArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType_Matrix22F:
                    {
                        const Matrix22f* value = source.getDataMatrix22fArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataMatrix22fArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType_Matrix33F:
                    {
                        const Matrix33f* value = source.getDataMatrix33fArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataMatrix33fArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType_Matrix44F:
                    {
                        const Matrix44f* value = source.getDataMatrix44fArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataMatrix44fArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType_Int32:
                    {
                        const Int32* value = source.getDataIntegerArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataIntegerArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType_Vector2I:
                    {
                        const Vector2i* value = source.getDataVector2iArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataVector2iArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType_Vector3I:
                    {
                        const Vector3i* value = source.getDataVector3iArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataVector3iArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType_Vector4I:
                    {
                        const Vector4i* value = source.getDataVector4iArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataVector4iArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType_TextureSampler:
                    {
                        TextureSamplerHandle samplerHandle = source.getDataTextureSamplerHandle(i, f);
                        collector.setDataTextureSamplerHandle(i, f, samplerHandle);
                        break;
                    }
                    case EDataType_DataReference:
                    {
                        DataInstanceHandle dataRef = source.getDataReference(i, f);
                        collector.setDataReference(i, f, dataRef);
                        break;
                    }
                    case EDataType_Indices:
                    case EDataType_UInt16Buffer:
                    case EDataType_FloatBuffer:
                    case EDataType_Vector2Buffer:
                    case EDataType_Vector3Buffer:
                    case EDataType_Vector4Buffer:
                    {
                        const ResourceField& dataResource = source.getDataResource(i, f);
                        if (dataResource.hash.isValid() || dataResource.dataBuffer.isValid())
                        {
                            collector.setDataResource(i, f, dataResource.hash, dataResource.dataBuffer, source.getDataResource(i, f).instancingDivisor);
                        }
                        break;
                    }
                    default:
                        assert(false);
                        break;
                    }
                }
            }
        }
    }

    void SceneDescriber::RecreateAnimationSystems(const IScene& source, SceneActionCollectionCreator& collector)
    {
        // send all animation systems
        for (auto animId = AnimationSystemHandle(0); animId < source.getAnimationSystemCount(); ++animId)
        {
            if (source.isAnimationSystemAllocated(animId))
            {
                // animation system
                const IAnimationSystem* animSystem = source.getAnimationSystem(animId);
                assert(animSystem != NULL);
                collector.addAnimationSystem(animId, animSystem->getFlags(), animSystem->getTotalSizeInformation());
                AnimationSystemDescriber::DescribeAnimationSystem(*animSystem, collector, animId);
            }
        }
    }

    void SceneDescriber::RecreateRenderGroups(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const UInt32 renderGroupTotalCount = source.getRenderGroupCount();
        for (RenderGroupHandle renderGroup(0u); renderGroup < renderGroupTotalCount; ++renderGroup)
        {
            if (source.isRenderGroupAllocated(renderGroup))
            {
                const RenderGroup& rg = source.getRenderGroup(renderGroup);
                collector.allocateRenderGroup(static_cast<UInt32>(rg.renderables.size()), static_cast<UInt32>(rg.renderGroups.size()), renderGroup);

                for (const auto& renderableEntry : rg.renderables)
                    collector.addRenderableToRenderGroup(renderGroup, renderableEntry.renderable, renderableEntry.order);
                for (const auto& rgEntry : rg.renderGroups)
                    collector.addRenderGroupToRenderGroup(renderGroup, rgEntry.renderGroup, rgEntry.order);
            }
        }
    }

    void SceneDescriber::RecreateRenderPasses(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const UInt32 renderPassTotalCount = source.getRenderPassCount();
        for (RenderPassHandle renderPass(0u); renderPass < renderPassTotalCount; ++renderPass)
        {
            if (source.isRenderPassAllocated(renderPass))
            {
                const RenderPass& rp = source.getRenderPass(renderPass);
                collector.allocateRenderPass(static_cast<UInt32>(rp.renderGroups.size()), renderPass);
                collector.setRenderPassRenderOrder(renderPass, rp.renderOrder);
                collector.setRenderPassClearColor(renderPass, rp.clearColor);
                collector.setRenderPassClearFlag(renderPass, rp.clearFlags);
                if (rp.camera.isValid())
                    collector.setRenderPassCamera(renderPass, rp.camera);
                collector.setRenderPassRenderTarget(renderPass, rp.renderTarget);
                collector.setRenderPassEnabled(renderPass, rp.isEnabled);
                if (rp.isRenderOnce)
                    collector.setRenderPassRenderOnce(renderPass, true);
                for (const auto& rgEntry : rp.renderGroups)
                    collector.addRenderGroupToRenderPass(renderPass, rgEntry.renderGroup, rgEntry.order);
            }
        }
    }

    void SceneDescriber::RecreateBlitPasses(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const UInt32 blitPassTotalCount = source.getBlitPassCount();
        for (BlitPassHandle blitPassHandle(0u); blitPassHandle < blitPassTotalCount; ++blitPassHandle)
        {
            if (source.isBlitPassAllocated(blitPassHandle))
            {
                const BlitPass& blitPass = source.getBlitPass(blitPassHandle);
                collector.allocateBlitPass(blitPass.sourceRenderBuffer, blitPass.destinationRenderBuffer, blitPassHandle);
                collector.setBlitPassRegions(blitPassHandle, blitPass.sourceRegion, blitPass.destinationRegion);
                collector.setBlitPassRenderOrder(blitPassHandle, blitPass.renderOrder);
                collector.setBlitPassEnabled(blitPassHandle, blitPass.isEnabled);
            }
        }
    }

    void SceneDescriber::RecreateDataBuffers(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const UInt32 dataBufferTotalCount = source.getDataBufferCount();
        for (DataBufferHandle handle(0u); handle < dataBufferTotalCount; ++handle)
        {
            if (source.isDataBufferAllocated(handle))
            {
                const GeometryDataBuffer& dataBuffer = source.getDataBuffer(handle);
                collector.allocateDataBuffer(dataBuffer.bufferType, dataBuffer.dataType, static_cast<UInt32>(dataBuffer.data.size()), handle);
                collector.updateDataBuffer(handle, 0u, dataBuffer.usedSize, dataBuffer.data.data());
            }
        }
    }

    void SceneDescriber::RecreateTextureBuffers(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const UInt32 textureBufferTotalCount = source.getTextureBufferCount();
        for (TextureBufferHandle textureBufferHandle(0u); textureBufferHandle < textureBufferTotalCount; ++textureBufferHandle)
        {
            if (source.isTextureBufferAllocated(textureBufferHandle))
            {
                const TextureBuffer& textureBuffer = source.getTextureBuffer(textureBufferHandle);
                const MipMapDimensions& mipMapDimensions = textureBuffer.mipMapDimensions;
                collector.allocateTextureBuffer(textureBuffer.textureFormat, mipMapDimensions, textureBufferHandle);

                for (UInt32 mipMapLevel = 0u; mipMapLevel < mipMapDimensions.size(); ++mipMapLevel)
                {
                    const MipMapSize mipMapSize = mipMapDimensions[mipMapLevel];
                    const Byte* mipMapData = textureBuffer.mipMapData[mipMapLevel].data();
                    const UInt32 mipLevelDataSize = mipMapSize.width * mipMapSize.height * GetTexelSizeFromFormat(textureBuffer.textureFormat);
                    collector.updateTextureBuffer(textureBufferHandle, mipMapLevel, 0u, 0u, mipMapSize.width, mipMapSize.height, mipMapData, mipLevelDataSize);
                }
            }
        }
    }

    void SceneDescriber::RecreateTextureSamplers(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const UInt32 samplerCount = source.getTextureSamplerCount();
        for (TextureSamplerHandle samplerHandle(0u); samplerHandle < samplerCount; ++samplerHandle)
        {
            if (source.isTextureSamplerAllocated(samplerHandle))
            {
                collector.allocateTextureSampler(source.getTextureSampler(samplerHandle), samplerHandle);
            }
        }
    }

    void SceneDescriber::RecreateRenderBuffersAndTargets(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const UInt32 renderBufferCount = source.getRenderBufferCount();
        for (RenderBufferHandle renderBufferHandle(0u); renderBufferHandle < renderBufferCount; ++renderBufferHandle)
        {
            if (source.isRenderBufferAllocated(renderBufferHandle))
            {
                collector.allocateRenderBuffer(source.getRenderBuffer(renderBufferHandle), renderBufferHandle);
            }
        }

        const UInt32 renderTargetCount = source.getRenderTargetCount();
        for (RenderTargetHandle renderTargetHandle(0u); renderTargetHandle < renderTargetCount; ++renderTargetHandle)
        {
            if (source.isRenderTargetAllocated(renderTargetHandle))
            {
                collector.allocateRenderTarget(renderTargetHandle);

                const UInt32 bufferCount = source.getRenderTargetRenderBufferCount(renderTargetHandle);
                for (UInt32 bufferIdx = 0u; bufferIdx < bufferCount; ++bufferIdx)
                {
                    const RenderBufferHandle buffer = source.getRenderTargetRenderBuffer(renderTargetHandle, bufferIdx);
                    collector.addRenderTargetRenderBuffer(renderTargetHandle, buffer);
                }
            }
        }
    }

    void SceneDescriber::RecreateStreamTextures(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const UInt32 streamTextureCount = source.getStreamTextureCount();
        for (StreamTextureHandle streamTextureHandle(0u); streamTextureHandle < streamTextureCount; ++streamTextureHandle)
        {
            if (source.isStreamTextureAllocated(streamTextureHandle))
            {
                const StreamTexture& streamTexture = source.getStreamTexture(streamTextureHandle);
                collector.allocateStreamTexture(streamTexture.source, streamTexture.fallbackTexture, streamTextureHandle);
                collector.setStreamTextureForceFallback(streamTextureHandle, streamTexture.forceFallbackTexture);
            }
        }
    }

    void SceneDescriber::RecreateDataSlots(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const UInt32 DataSlotCount = source.getDataSlotCount();
        for (DataSlotHandle dltHandle(0u); dltHandle < DataSlotCount; ++dltHandle)
        {
            if (source.isDataSlotAllocated(dltHandle))
            {
                collector.allocateDataSlot(source.getDataSlot(dltHandle), dltHandle);
            }
        }
    }

    void SceneDescriber::RecreateSceneVersionTag(const IScene& source, SceneActionCollectionCreator& collector)
    {
        SceneVersionTag versionTag = source.getSceneVersionTag();
        if (versionTag != InvalidSceneVersionTag)
        {
            collector.setSceneVersionTag(versionTag);
        }
    }

    template void SceneDescriber::describeScene<IScene>(const IScene& source, SceneActionCollectionCreator& collector);
    template void SceneDescriber::describeScene<ClientScene>(const ClientScene& source, SceneActionCollectionCreator& collector);
}
