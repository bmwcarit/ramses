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
#include "PlatformAbstraction/PlatformTypes.h"
#include "Utils/MemoryUtils.h"

namespace ramses_internal
{
    template <typename T>
    void SceneDescriber::describeScene(const T& source, SceneActionCollectionCreator& collector)
    {
        // 25% more than node+transform+renderable count
        const size_t numberOfSceneActionsEstimate =
            (source.getNodeCount() + source.getTransformCount() + source.getRenderableCount()) * 125 / 100;
        // circa 30 bytes per action
        const size_t sizeOfSceneActionsEstimate = 30u * numberOfSceneActionsEstimate;
        collector.collection.reserveAdditionalCapacity(sizeOfSceneActionsEstimate, numberOfSceneActionsEstimate);

        RecreateNodes(                   source, collector);
        RecreateTransformNodes(          source, collector);
        RecreateTransformations(         source, collector);
        RecreateRenderables(             source, collector);
        RecreateStates(                  source, collector);
        RecreateDataLayouts(             source, collector);
        RecreateDataInstances(           source, collector);
        RecreateCameras(                 source, collector);
        RecreateRenderGroups(            source, collector);
        RecreateRenderPasses(            source, collector);
        RecreateBlitPasses(              source, collector);
        RecreatePickableObjects(         source, collector);
        RecreateDataBuffers(             source, collector);
        RecreateTextureBuffers(          source, collector);
        RecreateTextureSamplers(         source, collector);
        RecreateRenderBuffersAndTargets( source, collector);
        RecreateDataSlots(               source, collector);
        RecreateSceneReferences(         source, collector);
    }

    void SceneDescriber::RecreateNodes(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const uint32_t totalNodeCount = source.getNodeCount();
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
                const uint32_t childCount = source.getChildCount(n);
                for (uint32_t child = 0; child < childCount; ++child)
                {
                    collector.addChildToNode(n, source.getChild(n, child));
                }
            }
        }
    }

    void SceneDescriber::RecreateCameras(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const uint32_t totalCameraCount = source.getCameraCount();
        for (CameraHandle c(0u); c < totalCameraCount; ++c)
        {
            if (source.isCameraAllocated(c))
            {
                const Camera& camera = source.getCamera(c);
                collector.allocateCamera(camera.projectionType, camera.node, camera.dataInstance, c);
            }
        }
    }

    void SceneDescriber::RecreateTransformNodes(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const uint32_t totalTransformCount = source.getTransformCount();
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
        const uint32_t totalTransformCount = source.getTransformCount();
        for (TransformHandle t(0u); t < totalTransformCount; ++t)
        {
            if (source.isTransformAllocated(t))
            {
                const auto& translation = source.getTranslation(t);
                if (translation != IScene::IdentityTranslation)
                {
                    collector.setTranslation(t, translation);
                }
                const auto& rotation = source.getRotation(t);
                if (rotation != IScene::IdentityRotation)
                {
                    const auto rotationType = source.getRotationType(t);
                    collector.setRotation(t, rotation, rotationType);
                }
                const auto& scaling = source.getScaling(t);
                if (scaling != IScene::IdentityScaling)
                {
                    collector.setScaling(t, scaling);
                }
            }
        }
    }

    void SceneDescriber::RecreateRenderables(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const uint32_t totalRenderableCount = source.getRenderableCount();
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
        const uint32_t totalStateCount = source.getRenderStateCount();
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
        const uint32_t totalDataLayoutCount = source.getDataLayoutCount();
        for (DataLayoutHandle l(0u); l < totalDataLayoutCount; ++l)
        {
            if (source.isDataLayoutAllocated(l))
            {
                const DataLayout& layout = source.getDataLayout(l);
                collector.allocateDataLayout(layout.getDataFields(), layout.getEffectHash(), l);
            }
        }
    }

    // overload for a client scene which compacts data layouts and those must be expanded while serializing
    void SceneDescriber::RecreateDataLayouts(const ClientScene& source, SceneActionCollectionCreator& collector)
    {
        const uint32_t totalDataLayoutCount = source.getDataLayoutCount();
        for (DataLayoutHandle l(0u); l < totalDataLayoutCount; ++l)
        {
            if (source.isDataLayoutAllocated(l))
            {
                const DataLayout& layout = source.getDataLayout(l);
                const DataFieldInfoVector& layoutFields = layout.getDataFields();
                const ResourceContentHash& effectHash = layout.getEffectHash();

                const uint32_t numRefs = source.getNumDataLayoutReferences(l);
                for (uint32_t r = 0u; r < numRefs; ++r)
                {
                    collector.allocateDataLayout(layoutFields, effectHash, l);
                }
            }
        }
    }

    void SceneDescriber::RecreateDataInstances(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const uint32_t totalDataInstanceCount = source.getDataInstanceCount();
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
                    const uint32_t elementCount = field.elementCount;

                    switch (field.dataType)
                    {
                    case EDataType::Float:
                    {
                        const float* value = source.getDataFloatArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataFloatArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType::Vector2F:
                    {
                        const auto* value = source.getDataVector2fArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataVector2fArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType::Vector3F:
                    {
                        const auto* value = source.getDataVector3fArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataVector3fArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType::Vector4F:
                    {
                        const auto* value = source.getDataVector4fArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataVector4fArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType::Matrix22F:
                    {
                        const glm::mat2* value = source.getDataMatrix22fArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataMatrix22fArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType::Matrix33F:
                    {
                        const auto* value = source.getDataMatrix33fArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataMatrix33fArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType::Matrix44F:
                    {
                        const auto* value = source.getDataMatrix44fArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataMatrix44fArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType::Int32:
                    {
                        const int32_t* value = source.getDataIntegerArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataIntegerArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType::Vector2I:
                    {
                        const auto* value = source.getDataVector2iArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataVector2iArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType::Vector3I:
                    {
                        const auto* value = source.getDataVector3iArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataVector3iArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType::Vector4I:
                    {
                        const auto* value = source.getDataVector4iArray(i, f);
                        if (!MemoryUtils::AreAllBytesZero(value, elementCount))
                        {
                            collector.setDataVector4iArray(i, f, elementCount, value);
                        }
                        break;
                    }
                    case EDataType::TextureSampler2D:
                    case EDataType::TextureSampler2DMS:
                    case EDataType::TextureSamplerExternal:
                    case EDataType::TextureSampler3D:
                    case EDataType::TextureSamplerCube:
                    {
                        TextureSamplerHandle samplerHandle = source.getDataTextureSamplerHandle(i, f);
                        collector.setDataTextureSamplerHandle(i, f, samplerHandle);
                        break;
                    }
                    case EDataType::DataReference:
                    {
                        DataInstanceHandle dataRef = source.getDataReference(i, f);
                        collector.setDataReference(i, f, dataRef);
                        break;
                    }
                    case EDataType::Indices:
                    case EDataType::UInt16Buffer:
                    case EDataType::FloatBuffer:
                    case EDataType::Vector2Buffer:
                    case EDataType::Vector3Buffer:
                    case EDataType::Vector4Buffer:
                    {
                        const ResourceField& dataResource = source.getDataResource(i, f);
                        if (dataResource.hash.isValid() || dataResource.dataBuffer.isValid())
                        {
                            collector.setDataResource(i, f, dataResource.hash, dataResource.dataBuffer, source.getDataResource(i, f).instancingDivisor, dataResource.offsetWithinElementInBytes, dataResource.stride);
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

    void SceneDescriber::RecreateRenderGroups(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const uint32_t renderGroupTotalCount = source.getRenderGroupCount();
        for (RenderGroupHandle renderGroup(0u); renderGroup < renderGroupTotalCount; ++renderGroup)
        {
            if (source.isRenderGroupAllocated(renderGroup))
            {
                const RenderGroup& rg = source.getRenderGroup(renderGroup);
                collector.allocateRenderGroup(static_cast<uint32_t>(rg.renderables.size()), static_cast<uint32_t>(rg.renderGroups.size()), renderGroup);

                for (const auto& renderableEntry : rg.renderables)
                    collector.addRenderableToRenderGroup(renderGroup, renderableEntry.renderable, renderableEntry.order);
                for (const auto& rgEntry : rg.renderGroups)
                    collector.addRenderGroupToRenderGroup(renderGroup, rgEntry.renderGroup, rgEntry.order);
            }
        }
    }

    void SceneDescriber::RecreateRenderPasses(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const uint32_t renderPassTotalCount = source.getRenderPassCount();
        for (RenderPassHandle renderPass(0u); renderPass < renderPassTotalCount; ++renderPass)
        {
            if (source.isRenderPassAllocated(renderPass))
            {
                const RenderPass& rp = source.getRenderPass(renderPass);
                collector.allocateRenderPass(static_cast<uint32_t>(rp.renderGroups.size()), renderPass);
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
        const uint32_t blitPassTotalCount = source.getBlitPassCount();
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

    void SceneDescriber::RecreatePickableObjects(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const uint32_t pickableObjectTotalCount = source.getPickableObjectCount();
        for (PickableObjectHandle handle(0u); handle < pickableObjectTotalCount; ++handle)
        {
            if (source.isPickableObjectAllocated(handle))
            {
                const PickableObject& pickableObject = source.getPickableObject(handle);
                collector.allocatePickableObject(pickableObject.geometryHandle, pickableObject.nodeHandle, pickableObject.id, handle);
                collector.setPickableObjectCamera(handle, pickableObject.cameraHandle);
                collector.setPickableObjectEnabled(handle, pickableObject.isEnabled);
            }
        }
    }

    void SceneDescriber::RecreateDataBuffers(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const uint32_t dataBufferTotalCount = source.getDataBufferCount();
        for (DataBufferHandle handle(0u); handle < dataBufferTotalCount; ++handle)
        {
            if (source.isDataBufferAllocated(handle))
            {
                const GeometryDataBuffer& dataBuffer = source.getDataBuffer(handle);
                collector.allocateDataBuffer(dataBuffer.bufferType, dataBuffer.dataType, static_cast<uint32_t>(dataBuffer.data.size()), handle);
                collector.updateDataBuffer(handle, 0u, dataBuffer.usedSize, dataBuffer.data.data());
            }
        }
    }

    void SceneDescriber::RecreateTextureBuffers(const IScene& source, SceneActionCollectionCreator& collector)
    {
        std::vector<Byte> tempForCopyingUsedTextureDataBuffer;

        const uint32_t textureBufferTotalCount = source.getTextureBufferCount();
        for (TextureBufferHandle textureBufferHandle(0u); textureBufferHandle < textureBufferTotalCount; ++textureBufferHandle)
        {
            if (source.isTextureBufferAllocated(textureBufferHandle))
            {
                const TextureBuffer& textureBuffer = source.getTextureBuffer(textureBufferHandle);

                const auto texelSize = GetTexelSizeFromFormat(textureBuffer.textureFormat);

                const MipMapDimensions mipMapDimensions([&textureBuffer]()
                {
                    MipMapDimensions result;
                    result.reserve(textureBuffer.mipMaps.size());
                    std::transform(textureBuffer.mipMaps.cbegin(), textureBuffer.mipMaps.cend(), std::back_inserter(result), [](const MipMap& mip) {return MipMapSize{ mip.width, mip.height }; });

                    return result;
                }());

                collector.allocateTextureBuffer(textureBuffer.textureFormat, mipMapDimensions, textureBufferHandle);

                for (uint32_t mipMapLevel = 0u; mipMapLevel < static_cast<uint32_t>(mipMapDimensions.size()); ++mipMapLevel)
                {
                    const auto& mip = textureBuffer.mipMaps[mipMapLevel];
                    const auto& usedRegion = mip.usedRegion;
                    //If no data is set for mip (area of used region is zero), then no need to send scene action for setting empty data
                    if (0 == usedRegion.getArea())
                        continue;

                    const auto& mipMapSize = mipMapDimensions[mipMapLevel];
                    const uint32_t usedDataSize = usedRegion.width * usedRegion.height * texelSize;
                    const Byte* mipMapData = mip.data.data();

                    const Byte* updatedData = mipMapData;

                    //Iff the width of used region is different from (smaller than) the width of the mip map level
                    //then the data of the used region must be copied from the mip map data into an intermediate (temp)
                    //memory because the data has non-zero stride ,i.e., the data for each row in the
                    //used region is not continuous in memory.
                    //P.S: Mip map (or used region) height is not a factor in this situation.
                    if (usedRegion.width < static_cast<int32_t>(mipMapSize.width))
                    {
                        //increase size iff needed
                        tempForCopyingUsedTextureDataBuffer.resize(usedDataSize);

                        updatedData = tempForCopyingUsedTextureDataBuffer.data();

                        //copy the used texture buffer data since rows have non-zero stride
                        //i.e, not all data for every used row must be copied

                        const uint32_t dataRowSize = usedRegion.width * texelSize;
                        const uint32_t mipLevelRowSize = mipMapSize.width * texelSize;

                        const Byte* sourcePtr = mipMapData + usedRegion.x * texelSize + usedRegion.y * mipLevelRowSize;
                        Byte* destinationPtr = tempForCopyingUsedTextureDataBuffer.data();
                        for (int32_t i = 0; i < usedRegion.height; ++i)
                        {
                            PlatformMemory::Copy(destinationPtr, sourcePtr, dataRowSize);
                            sourcePtr += mipLevelRowSize;
                            destinationPtr += dataRowSize;
                        }
                    }

                    collector.updateTextureBuffer(textureBufferHandle, mipMapLevel, usedRegion.x, usedRegion.y, usedRegion.width, usedRegion.height, updatedData, usedDataSize);
                }
            }
        }
    }

    void SceneDescriber::RecreateTextureSamplers(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const uint32_t samplerCount = source.getTextureSamplerCount();
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
        const uint32_t renderBufferCount = source.getRenderBufferCount();
        for (RenderBufferHandle renderBufferHandle(0u); renderBufferHandle < renderBufferCount; ++renderBufferHandle)
        {
            if (source.isRenderBufferAllocated(renderBufferHandle))
            {
                collector.allocateRenderBuffer(source.getRenderBuffer(renderBufferHandle), renderBufferHandle);
            }
        }

        const uint32_t renderTargetCount = source.getRenderTargetCount();
        for (RenderTargetHandle renderTargetHandle(0u); renderTargetHandle < renderTargetCount; ++renderTargetHandle)
        {
            if (source.isRenderTargetAllocated(renderTargetHandle))
            {
                collector.allocateRenderTarget(renderTargetHandle);

                const uint32_t bufferCount = source.getRenderTargetRenderBufferCount(renderTargetHandle);
                for (uint32_t bufferIdx = 0u; bufferIdx < bufferCount; ++bufferIdx)
                {
                    const RenderBufferHandle buffer = source.getRenderTargetRenderBuffer(renderTargetHandle, bufferIdx);
                    collector.addRenderTargetRenderBuffer(renderTargetHandle, buffer);
                }
            }
        }
    }

    void SceneDescriber::RecreateDataSlots(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const uint32_t DataSlotCount = source.getDataSlotCount();
        for (DataSlotHandle dltHandle(0u); dltHandle < DataSlotCount; ++dltHandle)
        {
            if (source.isDataSlotAllocated(dltHandle))
            {
                collector.allocateDataSlot(source.getDataSlot(dltHandle), dltHandle);
            }
        }
    }

    void SceneDescriber::RecreateSceneReferences(const IScene& source, SceneActionCollectionCreator& collector)
    {
        const uint32_t count = source.getSceneReferenceCount();
        for (SceneReferenceHandle handle{ 0u }; handle < count; ++handle)
        {
            if (source.isSceneReferenceAllocated(handle))
            {
                const auto& sr = source.getSceneReference(handle);
                collector.allocateSceneReference(sr.sceneId, handle);
                collector.requestSceneReferenceState(handle, sr.requestedState);
                collector.requestSceneReferenceFlushNotifications(handle, sr.flushNotifications);
                collector.setSceneReferenceRenderOrder(handle, sr.renderOrder);
            }
        }
    }

    template void SceneDescriber::describeScene<IScene>(const IScene& source, SceneActionCollectionCreator& collector);
    template void SceneDescriber::describeScene<ClientScene>(const ClientScene& source, SceneActionCollectionCreator& collector);
}
