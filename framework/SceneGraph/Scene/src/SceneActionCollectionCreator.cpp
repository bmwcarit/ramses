//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Scene/SceneActionCollectionCreator.h"

#include "SceneAPI/SceneSizeInformation.h"
#include "SceneAPI/PixelRectangle.h"
#include "SceneAPI/Camera.h"
#include "SceneAPI/Renderable.h"
#include "SceneAPI/Viewport.h"
#include "SceneAPI/TextureSampler.h"
#include "SceneAPI/RenderBuffer.h"
#include "Math3d/Vector2.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector4.h"
#include "Math3d/Vector2i.h"
#include "Math3d/Vector3i.h"
#include "Math3d/Vector4i.h"
#include "Math3d/Matrix22f.h"
#include "Math3d/Matrix33f.h"
#include "Math3d/Matrix44f.h"
#include "TransportCommon/RamsesTransportProtocolVersion.h"
#include "Resource/IResource.h"
#include "Components/SingleResourceSerialization.h"
#include "Components/FlushTimeInformation.h"
#include "Utils/RawBinaryOutputStream.h"

namespace ramses_internal
{
    SceneActionCollectionCreator::SceneActionCollectionCreator(SceneActionCollection& collection_)
        : collection(collection_)
    {
    }

    void SceneActionCollectionCreator::preallocateSceneSize(const SceneSizeInformation& sizeInfo)
    {
        collection.beginWriteSceneAction(ESceneActionId_PreallocateSceneSize);
        putSceneSizeInformation(sizeInfo);
    }

    void SceneActionCollectionCreator::setTransformComponent(ETransformPropertyType propertyChanged, TransformHandle node, const Vector3& newValue)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetTransformComponent);
        collection.write(static_cast<UInt32>(propertyChanged));
        collection.write(node);
        collection.write(newValue.data);
    }

    void SceneActionCollectionCreator::allocateRenderable(NodeHandle nodeHandle, RenderableHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AllocateRenderable);
        collection.write(nodeHandle);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::releaseRenderable(RenderableHandle renderableHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId_ReleaseRenderable);
        collection.write(renderableHandle);
    }

    void SceneActionCollectionCreator::setRenderableEffect(RenderableHandle renderableHandle, const ResourceContentHash& effectHash)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetRenderableEffect);
        collection.write(renderableHandle);
        collection.write(effectHash);
    }

    void SceneActionCollectionCreator::setDataResource(DataInstanceHandle handle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, UInt32 instancingDivisor)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetDataResource);
        collection.write(handle);
        collection.write(field);
        collection.write(hash);
        collection.write(dataBuffer);
        collection.write(instancingDivisor);
    }

    void SceneActionCollectionCreator::setDataMatrix22fArray(DataInstanceHandle handle, DataFieldHandle field, UInt32 elementCount, const Matrix22f* data)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetDataMatrix22fArray);
        collection.write(handle);
        collection.write(field);
        collection.write(elementCount);
        for (UInt32 i = 0; i < elementCount; ++i)
            collection.write(data[i].data);
    }

    void SceneActionCollectionCreator::setDataMatrix33fArray(DataInstanceHandle handle, DataFieldHandle field, UInt32 elementCount, const Matrix33f* data)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetDataMatrix33fArray);
        collection.write(handle);
        collection.write(field);
        collection.write(elementCount);
        for (UInt32 i = 0; i < elementCount; ++i)
            collection.write(data[i].data);
    }

    void SceneActionCollectionCreator::setDataMatrix44fArray(DataInstanceHandle handle, DataFieldHandle field, UInt32 elementCount, const Matrix44f* data)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetDataMatrix44fArray);
        collection.write(handle);
        collection.write(field);
        collection.write(elementCount);
        for (UInt32 i = 0; i < elementCount; ++i)
            collection.write(data[i].data);
    }

    void SceneActionCollectionCreator::setDataVector4fArray(DataInstanceHandle handle, DataFieldHandle field, UInt32 elementCount, const Vector4* data)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetDataVector4fArray);
        collection.write(handle);
        collection.write(field);
        collection.write(elementCount);
        for (UInt32 i = 0; i < elementCount; ++i)
            collection.write(data[i].data);
    }

    void SceneActionCollectionCreator::setDataVector4iArray(DataInstanceHandle handle, DataFieldHandle field, UInt32 elementCount, const Vector4i* data)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetDataVector4iArray);
        collection.write(handle);
        collection.write(field);
        collection.write(elementCount);
        for (UInt32 i = 0; i < elementCount; ++i)
            collection.write(data[i].data);
    }

    void SceneActionCollectionCreator::setDataVector3fArray(DataInstanceHandle handle, DataFieldHandle field, UInt32 elementCount, const Vector3* data)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetDataVector3fArray);
        collection.write(handle);
        collection.write(field);
        collection.write(elementCount);
        for (UInt32 i = 0; i < elementCount; ++i)
            collection.write(data[i].data);
    }

    void SceneActionCollectionCreator::setDataVector3iArray(DataInstanceHandle handle, DataFieldHandle field, UInt32 elementCount, const Vector3i* data)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetDataVector3iArray);
        collection.write(handle);
        collection.write(field);
        collection.write(elementCount);
        for (UInt32 i = 0; i < elementCount; ++i)
            collection.write(data[i].data);
    }

    void SceneActionCollectionCreator::setDataVector2fArray(DataInstanceHandle handle, DataFieldHandle field, UInt32 elementCount, const Vector2* data)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetDataVector2fArray);
        collection.write(handle);
        collection.write(field);
        collection.write(elementCount);
        for (UInt32 i = 0; i < elementCount; ++i)
            collection.write(data[i].data);
    }

    void SceneActionCollectionCreator::setDataVector2iArray(DataInstanceHandle handle, DataFieldHandle field, UInt32 elementCount, const Vector2i* data)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetDataVector2iArray);
        collection.write(handle);
        collection.write(field);
        collection.write(elementCount);
        for (UInt32 i = 0; i < elementCount; ++i)
            collection.write(data[i].data);
    }

    void SceneActionCollectionCreator::setDataFloatArray(DataInstanceHandle handle, DataFieldHandle field, UInt32 elementCount, const Float* data)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetDataFloatArray);
        collection.write(handle);
        collection.write(field);
        collection.write(data, elementCount);
    }

    void SceneActionCollectionCreator::setDataIntegerArray(DataInstanceHandle handle, DataFieldHandle field, UInt32 elementCount, const Int32* data)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetDataIntegerArray);
        collection.write(handle);
        collection.write(field);
        collection.write(data, elementCount);
    }

    void SceneActionCollectionCreator::releaseDataInstance(DataInstanceHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_ReleaseDataInstance);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::allocateDataInstance(DataLayoutHandle layoutHandle, DataInstanceHandle dataInstanceHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AllocateDataInstance);
        collection.write(layoutHandle);
        collection.write(dataInstanceHandle);
    }

    void SceneActionCollectionCreator::releaseTransform(TransformHandle transform)
    {
        collection.beginWriteSceneAction(ESceneActionId_ReleaseTransform);
        collection.write(transform);
    }

    void SceneActionCollectionCreator::allocateTransform(NodeHandle nodeHandle, TransformHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AllocateTransform);
        collection.write(nodeHandle);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::allocateNode(UInt32 childrenCount, NodeHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AllocateNode);
        collection.write(childrenCount);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::releaseNode(NodeHandle nodeToBeReleased)
    {
        collection.beginWriteSceneAction(ESceneActionId_ReleaseNode);
        collection.write(nodeToBeReleased);
    }

    void SceneActionCollectionCreator::addChildToNode(NodeHandle parent, NodeHandle child)
    {
        collection.beginWriteSceneAction(ESceneActionId_AddChildToNode);
        collection.write(parent);
        collection.write(child);
    }

    void SceneActionCollectionCreator::removeChildFromNode(NodeHandle parent, NodeHandle child)
    {
        collection.beginWriteSceneAction(ESceneActionId_RemoveChildFromNode);
        collection.write(parent);
        collection.write(child);
    }

    void SceneActionCollectionCreator::setRenderableIndexCount(RenderableHandle renderableHandle, UInt32 indexCount)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetRenderableIndexCount);
        collection.write(renderableHandle);
        collection.write(indexCount);
    }

    void SceneActionCollectionCreator::setRenderableStartIndex(RenderableHandle renderableHandle, UInt32 startIndex)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetRenderableStartIndex);
        collection.write(renderableHandle);
        collection.write(startIndex);
    }

    void SceneActionCollectionCreator::setRenderableVisibility(RenderableHandle renderableHandle, Bool visible)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetRenderableVisibility);
        collection.write(renderableHandle);
        collection.write(visible);
    }

    void SceneActionCollectionCreator::setRenderableInstanceCount(RenderableHandle renderableHandle, UInt32 instanceCount)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetRenderableInstanceCount);
        collection.write(renderableHandle);
        collection.write(instanceCount);
    }

    void SceneActionCollectionCreator::setRenderableDataInstance(RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetRenderableDataInstance);
        collection.write(renderableHandle);
        collection.write(static_cast<UInt32>(slot));
        collection.write(newDataInstance);
    }

    void SceneActionCollectionCreator::releaseDataLayout(DataLayoutHandle layoutHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId_ReleaseDataLayout);
        collection.write(layoutHandle);
    }

    void SceneActionCollectionCreator::allocateDataLayout(const DataFieldInfoVector& dataFields, DataLayoutHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AllocateDataLayout);
        collection.write(handle);
        collection.write(static_cast<UInt32>(dataFields.size()));
        for (const auto& dataField : dataFields)
        {
            collection.write(static_cast<UInt32>(dataField.dataType));
            collection.write(dataField.elementCount);
            collection.write(static_cast<UInt32>(dataField.semantics));
        }
    }

    void SceneActionCollectionCreator::setRenderStateStencilOps(RenderStateHandle stateHandle, EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetStateStencilOps);
        collection.write(stateHandle);
        collection.write(sfail);
        collection.write(dpfail);
        collection.write(dppass);
    }

    void SceneActionCollectionCreator::setRenderStateStencilFunc(RenderStateHandle stateHandle, EStencilFunc func, UInt8 ref, UInt8 mask)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetStateStencilFunc);
        collection.write(stateHandle);
        collection.write(func);
        collection.write(ref);
        collection.write(mask);
    }

    void SceneActionCollectionCreator::setRenderStateDepthWrite(RenderStateHandle stateHandle, EDepthWrite flag)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetStateDepthWrite);
        collection.write(stateHandle);
        collection.write(flag);
    }

    void SceneActionCollectionCreator::setRenderStateDepthFunc(RenderStateHandle stateHandle, EDepthFunc func)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetStateDepthFunc);
        collection.write(stateHandle);
        collection.write(func);
    }

    void SceneActionCollectionCreator::setRenderStateScissorTest(RenderStateHandle stateHandle, EScissorTest flag, const RenderState::ScissorRegion& region)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetStateScissorTest);
        collection.write(stateHandle);
        collection.write(flag);
        collection.write(region.x);
        collection.write(region.y);
        collection.write(region.width);
        collection.write(region.height);
    }

    void SceneActionCollectionCreator::setRenderStateCullMode(RenderStateHandle stateHandle, ECullMode cullMode)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetStateCullMode);
        collection.write(stateHandle);
        collection.write(cullMode);
    }

    void SceneActionCollectionCreator::setRenderStateDrawMode(RenderStateHandle stateHandle, EDrawMode drawMode)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetStateDrawMode);
        collection.write(stateHandle);
        collection.write(drawMode);
    }

    void SceneActionCollectionCreator::setRenderStateBlendOperations(RenderStateHandle stateHandle, EBlendOperation operationColor, EBlendOperation operationAlpha)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetStateBlendOperations);
        collection.write(stateHandle);
        collection.write(operationColor);
        collection.write(operationAlpha);
    }

    void SceneActionCollectionCreator::setRenderStateBlendFactors(RenderStateHandle stateHandle, EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetStateBlendFactors);
        collection.write(stateHandle);
        collection.write(srcColor);
        collection.write(destColor);
        collection.write(srcAlpha);
        collection.write(destAlpha);
    }

    void SceneActionCollectionCreator::setRenderStateColorWriteMask(RenderStateHandle stateHandle, ColorWriteMask colorMask)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetStateColorWriteMask);
        collection.write(stateHandle);
        collection.write(colorMask);
    }

    void SceneActionCollectionCreator::releaseRenderState(RenderStateHandle stateHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId_ReleaseState);
        collection.write(stateHandle);
    }

    void SceneActionCollectionCreator::allocateRenderState(RenderStateHandle stateHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AllocateRenderState);
        collection.write(stateHandle);
    }

    void SceneActionCollectionCreator::setRenderableRenderState(RenderableHandle renderableHandle, RenderStateHandle stateHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetRenderableState);
        collection.write(renderableHandle);
        collection.write(stateHandle);
    }

    void SceneActionCollectionCreator::setCameraFrustum(CameraHandle cameraHandle, const Frustum& frustum)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetCameraFrustum);
        collection.write(cameraHandle);
        collection.write(frustum.leftPlane);
        collection.write(frustum.rightPlane);
        collection.write(frustum.bottomPlane);
        collection.write(frustum.topPlane);
        collection.write(frustum.nearPlane);
        collection.write(frustum.farPlane);
    }

    void SceneActionCollectionCreator::allocateCamera(ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle viewportDataInstance, CameraHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AllocateCamera);
        collection.write(static_cast<UInt32>(type));
        collection.write(nodeHandle);
        collection.write(viewportDataInstance);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::releaseCamera(CameraHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_ReleaseCamera);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::allocateRenderGroup(UInt32 renderableCount, UInt32 nestedGroupCount, RenderGroupHandle groupHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AllocateRenderGroup);
        collection.write(renderableCount);
        collection.write(nestedGroupCount);
        collection.write(groupHandle);
    }

    void SceneActionCollectionCreator::releaseRenderGroup(RenderGroupHandle groupHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId_ReleaseRenderGroup);
        collection.write(groupHandle);
    }

    void SceneActionCollectionCreator::addRenderableToRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle, Int32 order)
    {
        collection.beginWriteSceneAction(ESceneActionId_AddRenderableToRenderGroup);
        collection.write(groupHandle);
        collection.write(renderableHandle);
        collection.write(order);
    }

    void SceneActionCollectionCreator::removeRenderableFromRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId_RemoveRenderableFromRenderGroup);
        collection.write(groupHandle);
        collection.write(renderableHandle);
    }

    void SceneActionCollectionCreator::addRenderGroupToRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, Int32 order)
    {
        collection.beginWriteSceneAction(ESceneActionId_AddRenderGroupToRenderGroup);
        collection.write(groupHandleParent);
        collection.write(groupHandleChild);
        collection.write(order);
    }

    void SceneActionCollectionCreator::removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild)
    {
        collection.beginWriteSceneAction(ESceneActionId_RemoveRenderGroupFromRenderGroup);
        collection.write(groupHandleParent);
        collection.write(groupHandleChild);
    }

    void SceneActionCollectionCreator::allocateRenderPass(UInt32 renderGroupCount, RenderPassHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AllocateRenderPass);
        collection.write(renderGroupCount);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::releaseRenderPass(RenderPassHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_ReleaseRenderPass);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::setRenderPassCamera(RenderPassHandle pass, CameraHandle camera)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetRenderPassCamera);
        collection.write(pass);
        collection.write(camera);
    }

    void SceneActionCollectionCreator::setRenderPassRenderTarget(RenderPassHandle pass, RenderTargetHandle targetHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetRenderPassRenderTarget);
        collection.write(pass);
        collection.write(targetHandle);
    }

    void SceneActionCollectionCreator::setRenderPassRenderOrder(RenderPassHandle pass, Int32 renderOrder)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetRenderPassRenderOrder);
        collection.write(pass);
        collection.write(renderOrder);
    }

    void SceneActionCollectionCreator::setRenderPassEnabled(RenderPassHandle pass, Bool isEnabled)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetRenderPassEnabled);
        collection.write(pass);
        collection.write(isEnabled);
    }

    void SceneActionCollectionCreator::setRenderPassRenderOnce(RenderPassHandle pass, Bool enabled)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetRenderPassRenderOnce);
        collection.write(pass);
        collection.write(enabled);
    }

    void SceneActionCollectionCreator::retriggerRenderPassRenderOnce(RenderPassHandle pass)
    {
        collection.beginWriteSceneAction(ESceneActionId_RetriggerRenderPassRenderOnce);
        collection.write(pass);
    }

    void SceneActionCollectionCreator::addRenderGroupToRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle, Int32 order)
    {
        collection.beginWriteSceneAction(ESceneActionId_AddRenderGroupToRenderPass);
        collection.write(passHandle);
        collection.write(groupHandle);
        collection.write(order);
    }

    void SceneActionCollectionCreator::removeRenderGroupFromRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId_RemoveRenderGroupFromRenderPass);
        collection.write(passHandle);
        collection.write(groupHandle);
    }

    void SceneActionCollectionCreator::allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AllocateBlitPass);
        collection.write(sourceRenderBufferHandle);
        collection.write(destinationRenderBufferHandle);
        collection.write(passHandle);
    }

    void SceneActionCollectionCreator::releaseBlitPass(BlitPassHandle passHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId_ReleaseBlitPass);
        collection.write(passHandle);
    }

    void SceneActionCollectionCreator::setBlitPassRenderOrder(BlitPassHandle passHandle, Int32 renderOrder)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetBlitPassRenderOrder);
        collection.write(passHandle);
        collection.write(renderOrder);
    }

    void SceneActionCollectionCreator::setBlitPassEnabled(BlitPassHandle passHandle, Bool isEnabled)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetBlitPassEnabled);
        collection.write(passHandle);
        collection.write(isEnabled);
    }

    void SceneActionCollectionCreator::setBlitPassRegions(BlitPassHandle passHandle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetBlitPassRegions);
        collection.write(passHandle);
        collection.write(sourceRegion.x);
        collection.write(sourceRegion.y);
        collection.write(sourceRegion.width);
        collection.write(sourceRegion.height);
        collection.write(destinationRegion.x);
        collection.write(destinationRegion.y);
        collection.write(destinationRegion.width);
        collection.write(destinationRegion.height);
    }

    void SceneActionCollectionCreator::setDataTextureSamplerHandle(DataInstanceHandle handle, DataFieldHandle field, TextureSamplerHandle samplerHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetDataTextureSamplerHandle);
        collection.write(handle);
        collection.write(field);
        collection.write(samplerHandle);
    }

    void SceneActionCollectionCreator::setDataReference(DataInstanceHandle handle, DataFieldHandle field, DataInstanceHandle dataRef)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetDataReference);
        collection.write(handle);
        collection.write(field);
        collection.write(dataRef);
    }

    void SceneActionCollectionCreator::allocateTextureSampler(const TextureSampler& sampler, TextureSamplerHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AllocateTextureSampler);
        collection.write(handle);
        collection.write(sampler.states.m_addressModeU);
        collection.write(sampler.states.m_addressModeV);
        collection.write(sampler.states.m_addressModeR);
        collection.write(sampler.states.m_minSamplingMode);
        collection.write(sampler.states.m_magSamplingMode);
        collection.write(sampler.states.m_anisotropyLevel);
        collection.write(sampler.contentType);
        if (sampler.contentType == TextureSampler::ContentType::ClientTexture)
            collection.write(sampler.textureResource);
        else
            collection.write(sampler.contentHandle);
    }

    void SceneActionCollectionCreator::releaseTextureSampler(TextureSamplerHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_ReleaseTextureSampler);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::allocateRenderTarget(RenderTargetHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AllocateRenderTarget);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::releaseRenderTarget(RenderTargetHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_ReleaseRenderTarget);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::addRenderTargetRenderBuffer(RenderTargetHandle targetHandle, RenderBufferHandle bufferHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AddRenderTargetRenderBuffer);
        collection.write(targetHandle);
        collection.write(bufferHandle);
    }

    void SceneActionCollectionCreator::allocateRenderBuffer(const RenderBuffer& renderBuffer, RenderBufferHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AllocateRenderBuffer);
        collection.write(renderBuffer.width);
        collection.write(renderBuffer.height);
        collection.write(handle);
        collection.write(static_cast<UInt32>(renderBuffer.type));
        collection.write(static_cast<UInt32>(renderBuffer.format));
        collection.write(static_cast<UInt32>(renderBuffer.accessMode));
        collection.write(renderBuffer.sampleCount);
    }

    void SceneActionCollectionCreator::releaseRenderBuffer(RenderBufferHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_ReleaseRenderBuffer);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::allocateStreamTexture(uint32_t streamSource, const ResourceContentHash& fallbackTextureHash, StreamTextureHandle streamTextureHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AllocateStreamTexture);
        collection.write(streamTextureHandle);
        collection.write(streamSource);
        collection.write(fallbackTextureHash);
    }

    void SceneActionCollectionCreator::releaseStreamTexture(StreamTextureHandle streamTextureHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId_ReleaseStreamTexture);
        collection.write(streamTextureHandle);
    }

    void SceneActionCollectionCreator::setStreamTextureForceFallback(StreamTextureHandle streamTextureHandle, Bool forceFallbackImage)
    {
        assert(streamTextureHandle.isValid());
        collection.beginWriteSceneAction(ESceneActionId_SetForceFallback);
        collection.write(streamTextureHandle);
        collection.write(forceFallbackImage);
    }

    void SceneActionCollectionCreator::allocateDataBuffer(EDataBufferType dataBufferType, EDataType dataType, UInt32 maximumSizeInBytes, DataBufferHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AllocateDataBuffer);
        collection.write(static_cast<UInt32>(dataBufferType));
        collection.write(static_cast<UInt32>(dataType));
        collection.write(maximumSizeInBytes);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::releaseDataBuffer(DataBufferHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_ReleaseDataBuffer);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::updateDataBuffer(DataBufferHandle handle, UInt32 offsetInBytes, UInt32 dataSizeInBytes, const Byte* data)
    {
        collection.beginWriteSceneAction(ESceneActionId_UpdateDataBuffer);
        collection.write(handle);
        collection.write(offsetInBytes);
        collection.write(data, dataSizeInBytes);
    }

    void SceneActionCollectionCreator::allocateTextureBuffer(ETextureFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AllocateTextureBuffer);
        collection.write(static_cast<UInt32>(textureFormat));
        collection.write(static_cast<UInt32>(mipMapDimensions.size()));
        for (const auto& size : mipMapDimensions)
        {
            collection.write(size.width);
            collection.write(size.height);
        }
        collection.write(handle);
    }

    void SceneActionCollectionCreator::releaseTextureBuffer(TextureBufferHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_ReleaseTextureBuffer);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::updateTextureBuffer(TextureBufferHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data, UInt32 dataSize)
    {
        collection.beginWriteSceneAction(ESceneActionId_UpdateTextureBuffer);
        collection.write(handle);
        collection.write(mipLevel);
        collection.write(x);
        collection.write(y);
        collection.write(width);
        collection.write(height);
        collection.write(data, dataSize);
    }

    void SceneActionCollectionCreator::allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AllocateDataSlot);
        collection.write(static_cast<UInt32>(dataSlot.type));
        collection.write(dataSlot.id);
        collection.write(dataSlot.attachedNode);
        collection.write(dataSlot.attachedDataReference);
        collection.write(dataSlot.attachedTexture);
        collection.write(dataSlot.attachedTextureSampler);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::setDataSlotTexture(DataSlotHandle handle, const ResourceContentHash& texture)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetDataSlotTexture);
        collection.write(handle);
        collection.write(texture);
    }

    void SceneActionCollectionCreator::releaseDataSlot(DataSlotHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_ReleaseDataSlot);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::setRenderPassClearColor(RenderPassHandle handle, const Vector4& clearColor)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetRenderPassClearColor);
        collection.write(handle);
        collection.write(clearColor.data);
    }

    void SceneActionCollectionCreator::setRenderPassClearFlag(RenderPassHandle handle, UInt32 clearFlag)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetRenderPassClearFlag);
        collection.write(handle);
        collection.write(clearFlag);
    }

    void SceneActionCollectionCreator::addAnimationSystem(AnimationSystemHandle animSystemHandle, UInt32 flags, const AnimationSystemSizeInformation& sizeInfo)
    {
        collection.beginWriteSceneAction(ESceneActionId_AddAnimationSystem);
        collection.write(animSystemHandle);
        collection.write(flags);
        collection.write(sizeInfo.splineCount);
        collection.write(sizeInfo.dataBindCount);
        collection.write(sizeInfo.animationInstanceCount);
        collection.write(sizeInfo.animationCount);
    }

    void SceneActionCollectionCreator::removeAnimationSystem(AnimationSystemHandle animSystemHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId_RemoveAnimationSystem);
        collection.write(animSystemHandle);
    }

    void SceneActionCollectionCreator::animationSystemSetTime(AnimationSystemHandle animSystemHandle, const AnimationTime& globalTime)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetTime);
        collection.write(animSystemHandle);
        collection.write(globalTime.getTimeStamp());
    }

    void SceneActionCollectionCreator::animationSystemAllocateSpline(AnimationSystemHandle animSystemHandle, ESplineKeyType keyType, EDataTypeID dataTypeID, SplineHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemAllocateSpline);
        collection.write(animSystemHandle);
        collection.write(static_cast<UInt32>(keyType));
        collection.write(static_cast<UInt32>(dataTypeID));
        collection.write(handle);
    }

    void SceneActionCollectionCreator::animationSystemAllocateDataBinding(AnimationSystemHandle animSystemHandle, TDataBindID dataBindID, MemoryHandle handle1, MemoryHandle handle2, DataBindHandle dataBindHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemAllocateDataBinding);
        collection.write(animSystemHandle);
        collection.write(dataBindID);
        collection.write(handle1);
        collection.write(handle2);
        collection.write(dataBindHandle);
    }

    void SceneActionCollectionCreator::animationSystemAllocateAnimationInstance(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, EInterpolationType interpolationType, EVectorComponent vectorComponent, AnimationInstanceHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemAllocateAnimationInstance);
        collection.write(animSystemHandle);
        collection.write(splineHandle);
        collection.write(static_cast<UInt32>(interpolationType));
        collection.write(static_cast<UInt32>(vectorComponent));
        collection.write(handle);
    }

    void SceneActionCollectionCreator::animationSystemAllocateAnimation(AnimationSystemHandle animSystemHandle, AnimationInstanceHandle animInstHandle, AnimationHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemAllocateAnimation);
        collection.write(animSystemHandle);
        collection.write(animInstHandle);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::animationSystemAddDataBindingToAnimationInstance(AnimationSystemHandle animSystemHandle, AnimationInstanceHandle handle, DataBindHandle dataBindHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemAddDataBindingToAnimationInstance);
        collection.write(animSystemHandle);
        collection.write(handle);
        collection.write(dataBindHandle);
    }

    void SceneActionCollectionCreator::animationSystemSetSplineKeyBasicBool(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, Bool value)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetSplineKeyBasicBool);
        collection.write(animSystemHandle);
        collection.write(splineHandle);
        collection.write(timeStamp);
        collection.write(value);
    }

    void SceneActionCollectionCreator::animationSystemSetSplineKeyBasicInt32(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, Int32 value)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetSplineKeyBasicInt32);
        collection.write(animSystemHandle);
        collection.write(splineHandle);
        collection.write(timeStamp);
        collection.write(value);
    }

    void SceneActionCollectionCreator::animationSystemSetSplineKeyBasicFloat(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, Float value)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetSplineKeyBasicFloat);
        collection.write(animSystemHandle);
        collection.write(splineHandle);
        collection.write(timeStamp);
        collection.write(value);
    }

    void SceneActionCollectionCreator::animationSystemSetSplineKeyBasicVector2f(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2& value)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetSplineKeyBasicVector2f);
        collection.write(animSystemHandle);
        collection.write(splineHandle);
        collection.write(timeStamp);
        collection.write(value.data);
    }

    void SceneActionCollectionCreator::animationSystemSetSplineKeyBasicVector3f(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3& value)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetSplineKeyBasicVector3f);
        collection.write(animSystemHandle);
        collection.write(splineHandle);
        collection.write(timeStamp);
        collection.write(value.data);
    }

    void SceneActionCollectionCreator::animationSystemSetSplineKeyBasicVector4f(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4& value)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetSplineKeyBasicVector4f);
        collection.write(animSystemHandle);
        collection.write(splineHandle);
        collection.write(timeStamp);
        collection.write(value.data);
    }

    void SceneActionCollectionCreator::animationSystemSetSplineKeyBasicVector2i(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2i& value)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetSplineKeyBasicVector2i);
        collection.write(animSystemHandle);
        collection.write(splineHandle);
        collection.write(timeStamp);
        collection.write(value.data);
    }

    void SceneActionCollectionCreator::animationSystemSetSplineKeyBasicVector3i(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3i& value)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetSplineKeyBasicVector3i);
        collection.write(animSystemHandle);
        collection.write(splineHandle);
        collection.write(timeStamp);
        collection.write(value.data);
    }

    void SceneActionCollectionCreator::animationSystemSetSplineKeyBasicVector4i(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4i& value)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetSplineKeyBasicVector4i);
        collection.write(animSystemHandle);
        collection.write(splineHandle);
        collection.write(timeStamp);
        collection.write(value.data);
    }

    void SceneActionCollectionCreator::animationSystemSetSplineKeyTangentsInt32(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, Int32 value, const Vector2& tanIn, const Vector2& tanOut)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetSplineKeyTangentsInt32);
        collection.write(animSystemHandle);
        collection.write(splineHandle);
        collection.write(timeStamp);
        collection.write(value);
        collection.write(tanIn.data);
        collection.write(tanOut.data);
    }

    void SceneActionCollectionCreator::animationSystemSetSplineKeyTangentsFloat(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, Float value, const Vector2& tanIn, const Vector2& tanOut)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetSplineKeyTangentsFloat);
        collection.write(animSystemHandle);
        collection.write(splineHandle);
        collection.write(timeStamp);
        collection.write(value);
        collection.write(tanIn.data);
        collection.write(tanOut.data);
    }

    void SceneActionCollectionCreator::animationSystemSetSplineKeyTangentsVector2f(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2& value, const Vector2& tanIn, const Vector2& tanOut)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetSplineKeyTangentsVector2f);
        collection.write(animSystemHandle);
        collection.write(splineHandle);
        collection.write(timeStamp);
        collection.write(value.data);
        collection.write(tanIn.data);
        collection.write(tanOut.data);
    }

    void SceneActionCollectionCreator::animationSystemSetSplineKeyTangentsVector3f(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3& value, const Vector2& tanIn, const Vector2& tanOut)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetSplineKeyTangentsVector3f);
        collection.write(animSystemHandle);
        collection.write(splineHandle);
        collection.write(timeStamp);
        collection.write(value.data);
        collection.write(tanIn.data);
        collection.write(tanOut.data);
    }

    void SceneActionCollectionCreator::animationSystemSetSplineKeyTangentsVector4f(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4& value, const Vector2& tanIn, const Vector2& tanOut)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetSplineKeyTangentsVector4f);
        collection.write(animSystemHandle);
        collection.write(splineHandle);
        collection.write(timeStamp);
        collection.write(value.data);
        collection.write(tanIn.data);
        collection.write(tanOut.data);
    }

    void SceneActionCollectionCreator::animationSystemSetSplineKeyTangentsVector2i(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2i& value, const Vector2& tanIn, const Vector2& tanOut)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetSplineKeyTangentsVector2i);
        collection.write(animSystemHandle);
        collection.write(splineHandle);
        collection.write(timeStamp);
        collection.write(value.data);
        collection.write(tanIn.data);
        collection.write(tanOut.data);
    }

    void SceneActionCollectionCreator::animationSystemSetSplineKeyTangentsVector3i(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3i& value, const Vector2& tanIn, const Vector2& tanOut)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetSplineKeyTangentsVector3i);
        collection.write(animSystemHandle);
        collection.write(splineHandle);
        collection.write(timeStamp);
        collection.write(value.data);
        collection.write(tanIn.data);
        collection.write(tanOut.data);
    }

    void SceneActionCollectionCreator::animationSystemSetSplineKeyTangentsVector4i(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4i& value, const Vector2& tanIn, const Vector2& tanOut)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetSplineKeyTangentsVector4i);
        collection.write(animSystemHandle);
        collection.write(splineHandle);
        collection.write(timeStamp);
        collection.write(value.data);
        collection.write(tanIn.data);
        collection.write(tanOut.data);
    }

    void SceneActionCollectionCreator::animationSystemRemoveSplineKey(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineKeyIndex keyIndex)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemRemoveSplineKey);
        collection.write(animSystemHandle);
        collection.write(splineHandle);
        collection.write(keyIndex);
    }

    void SceneActionCollectionCreator::animationSystemSetAnimationStartTime(AnimationSystemHandle animSystemHandle, AnimationHandle handle, const AnimationTime& timeStamp)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetAnimationStartTime);
        collection.write(animSystemHandle);
        collection.write(handle);
        collection.write(timeStamp.getTimeStamp());
    }

    void SceneActionCollectionCreator::animationSystemSetAnimationStopTime(AnimationSystemHandle animSystemHandle, AnimationHandle handle, const AnimationTime& timeStamp)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetAnimationStopTime);
        collection.write(animSystemHandle);
        collection.write(handle);
        collection.write(timeStamp.getTimeStamp());
    }

    void SceneActionCollectionCreator::animationSystemSetAnimationProperties(AnimationSystemHandle animSystemHandle, AnimationHandle handle, Float playbackSpeed, UInt32 flags, AnimationTime::Duration loopDuration, const AnimationTime& timeStamp)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemSetAnimationProperties);
        collection.write(animSystemHandle);
        collection.write(handle);
        collection.write(playbackSpeed);
        collection.write(flags);
        collection.write(loopDuration);
        collection.write(timeStamp.getTimeStamp());
    }

    void SceneActionCollectionCreator::animationSystemStopAnimationAndRollback(AnimationSystemHandle animSystemHandle, AnimationHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemStopAnimationAndRollback);
        collection.write(animSystemHandle);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::animationSystemRemoveSpline(AnimationSystemHandle animSystemHandle, SplineHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemRemoveSpline);
        collection.write(animSystemHandle);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::animationSystemRemoveDataBinding(AnimationSystemHandle animSystemHandle, DataBindHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemRemoveDataBinding);
        collection.write(animSystemHandle);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::animationSystemRemoveAnimationInstance(AnimationSystemHandle animSystemHandle, AnimationInstanceHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemRemoveAnimationInstance);
        collection.write(animSystemHandle);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::animationSystemRemoveAnimation(AnimationSystemHandle animSystemHandle, AnimationHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId_AnimationSystemRemoveAnimation);
        collection.write(animSystemHandle);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::compoundRenderableEffectData(
        RenderableHandle renderableHandle,
        DataInstanceHandle uniformInstanceHandle,
        RenderStateHandle stateHandle,
        const ResourceContentHash& effectHash)
    {
        collection.beginWriteSceneAction(ESceneActionId_CompoundRenderableEffectData);
        collection.write(renderableHandle);
        collection.write(uniformInstanceHandle);
        collection.write(stateHandle);
        collection.write(effectHash);
    }

    void SceneActionCollectionCreator::compoundRenderable(RenderableHandle renderableHandle, const Renderable& renderable)
    {
        static_assert(ERenderableDataSlotType_MAX_SLOTS == 2u
            , "Expected ERenderableDataSlotType containing 2 elements, adjust ESceneActionId_CompoundRenderable SceneAction handling");

        collection.beginWriteSceneAction(ESceneActionId_CompoundRenderable);
        collection.write(renderableHandle);
        collection.write(renderable.node);
        collection.write(renderable.startIndex);
        collection.write(renderable.indexCount);
        collection.write(renderable.effectResource);
        collection.write(renderable.renderState);
        collection.write(renderable.isVisible);
        collection.write(renderable.instanceCount);
        collection.write(renderable.dataInstances[ERenderableDataSlotType_Geometry]);
        collection.write(renderable.dataInstances[ERenderableDataSlotType_Uniforms]);
    }

    void SceneActionCollectionCreator::compoundState(RenderStateHandle handle, const RenderState& rs)
    {
        collection.beginWriteSceneAction(ESceneActionId_CompoundState);
        collection.write(handle);
        collection.write(rs.scissorRegion.x);
        collection.write(rs.scissorRegion.y);
        collection.write(rs.scissorRegion.width);
        collection.write(rs.scissorRegion.height);
        collection.write(rs.blendFactorSrcColor);
        collection.write(rs.blendFactorDstColor);
        collection.write(rs.blendFactorSrcAlpha);
        collection.write(rs.blendFactorDstAlpha);
        collection.write(rs.blendOperationColor);
        collection.write(rs.blendOperationAlpha);
        collection.write(rs.cullMode);
        collection.write(rs.drawMode);
        collection.write(rs.depthWrite);
        collection.write(rs.depthFunc);
        collection.write(rs.scissorTest);
        collection.write(rs.stencilFunc);
        collection.write(rs.stencilRefValue);
        collection.write(rs.stencilMask);
        collection.write(rs.stencilOpFail);
        collection.write(rs.stencilOpDepthFail);
        collection.write(rs.stencilOpDepthPass);
        collection.write(rs.colorWriteMask);
    }

    void SceneActionCollectionCreator::pushResource(const IResource& resource)
    {
        collection.beginWriteSceneAction(ESceneActionId_PushResource);
        collection.write(resource.getHash());

        const UInt32 resourceSize = SingleResourceSerialization::SizeOfSerializedResource(resource);
        collection.write(resourceSize);

        // Serialize resource to collection by simulating a byte output stream pointing to collection buffer
        std::vector<Byte>& actionData = collection.getRawDataForDirectWriting();
        UInt writeStartIdx = actionData.size();
        actionData.resize(writeStartIdx + resourceSize);
        RawBinaryOutputStream stream(actionData.data() + writeStartIdx, resourceSize);
        SingleResourceSerialization::SerializeResource(stream, resource);
    }

    void SceneActionCollectionCreator::setAckFlushState(bool state)
    {
        collection.beginWriteSceneAction(ESceneActionId_SetAckFlushState);
        collection.write(state);
    }

    void SceneActionCollectionCreator::flush(
        UInt64 flushIndex,
        Bool synchronous,
        Bool addSizeInfo,
        const SceneSizeInformation& sizeInfo,
        const SceneResourceChanges& resourceChanges,
        const FlushTimeInformation& flushTimeInfo,
        SceneVersionTag versionTag,
        std::initializer_list<UInt64> additionalTimestamps)
    {
        const bool hasTimestamps = additionalTimestamps.size() > 0;
        const uint8_t flushFlags =
            (synchronous ? ESceneActionFlushBits_Synchronous : 0u) |
            (addSizeInfo ? ESceneActionFlushBits_HasSizeInfo : 0u) |
            (hasTimestamps ? ESceneActionFlushBits_HasTimestamps : 0u);

        const UInt estimatedDataSize = sizeof(flushIndex) + sizeof(flushFlags) +
            (addSizeInfo ? sizeof(SceneSizeInformation) : 0) +
            resourceChanges.getPutSizeEstimate() +
            (hasTimestamps ? sizeof(UInt64) * (additionalTimestamps.size() + 1) : 0) +
            2 * sizeof(UInt64);
        collection.reserveAdditionalCapacity(estimatedDataSize, 1u);

        collection.beginWriteSceneAction(ESceneActionId_Flush);
        collection.write(flushIndex);
        collection.write(flushFlags);
        if (addSizeInfo)
        {
            putSceneSizeInformation(sizeInfo);
        }
        resourceChanges.putToSceneAction(collection);

        collection.write(static_cast<uint64_t>(std::chrono::time_point_cast<std::chrono::milliseconds>(flushTimeInfo.expirationTimestamp).time_since_epoch().count()));
        collection.write(static_cast<uint64_t>(std::chrono::time_point_cast<std::chrono::milliseconds>(flushTimeInfo.internalTimestamp).time_since_epoch().count()));

        collection.write(versionTag);

        if (hasTimestamps)
        {
            collection.write(static_cast<UInt32>(additionalTimestamps.size()));
            for (UInt64 ts : additionalTimestamps)
            {
                collection.write(ts);
            }
        }
    }

    void SceneActionCollectionCreator::putSceneSizeInformation(const SceneSizeInformation& sizeInfo)
    {
        collection.write(sizeInfo.nodeCount);
        collection.write(sizeInfo.cameraCount);
        collection.write(sizeInfo.transformCount);
        collection.write(sizeInfo.renderableCount);
        collection.write(sizeInfo.renderStateCount);
        collection.write(sizeInfo.datalayoutCount);
        collection.write(sizeInfo.datainstanceCount);
        collection.write(sizeInfo.renderGroupCount);
        collection.write(sizeInfo.renderPassCount);
        collection.write(sizeInfo.blitPassCount);
        collection.write(sizeInfo.renderTargetCount);
        collection.write(sizeInfo.renderBufferCount);
        collection.write(sizeInfo.textureSamplerCount);
        collection.write(sizeInfo.streamTextureCount);
        collection.write(sizeInfo.dataSlotCount);
        collection.write(sizeInfo.dataBufferCount);
        collection.write(sizeInfo.animationSystemCount);
        collection.write(sizeInfo.textureBufferCount);
    }
}
