//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/SceneGraph/Scene/SceneActionCollectionCreator.h"

#include "internal/SceneGraph/SceneAPI/SceneSizeInformation.h"
#include "internal/SceneGraph/SceneAPI/PixelRectangle.h"
#include "internal/SceneGraph/SceneAPI/Camera.h"
#include "internal/SceneGraph/SceneAPI/Renderable.h"
#include "internal/SceneGraph/SceneAPI/Viewport.h"
#include "internal/SceneGraph/SceneAPI/TextureSampler.h"
#include "internal/SceneGraph/SceneAPI/RenderBuffer.h"
#include "internal/SceneGraph/SceneAPI/ERotationType.h"
#include "internal/Communication/TransportCommon/RamsesTransportProtocolVersion.h"
#include "internal/SceneGraph/Resource/IResource.h"
#include "internal/Components/SingleResourceSerialization.h"
#include "internal/Components/FlushTimeInformation.h"
#include "internal/Core/Utils/RawBinaryOutputStream.h"
#include "glm/gtx/range.hpp"

namespace ramses::internal
{
    SceneActionCollectionCreator::SceneActionCollectionCreator(SceneActionCollection& collection_, EFeatureLevel featureLevel)
        : collection{ collection_ }
        , m_featureLevel{ featureLevel }
    {
    }

    void SceneActionCollectionCreator::preallocateSceneSize(const SceneSizeInformation& sizeInfo)
    {
        collection.beginWriteSceneAction(ESceneActionId::PreallocateSceneSize);
        putSceneSizeInformation(sizeInfo);
    }

    void SceneActionCollectionCreator::setTranslation(TransformHandle node, const glm::vec3& newValue)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetTranslation);
        collection.write(node);
        collection.write(newValue);
    }

    void SceneActionCollectionCreator::setRotation(TransformHandle node, const glm::vec4& newValue, ERotationType rotationType)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetRotation);
        collection.write(node);
        collection.write(newValue);
        collection.write(rotationType);
    }

    void SceneActionCollectionCreator::setScaling(TransformHandle node, const glm::vec3& newValue)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetScaling);
        collection.write(node);
        collection.write(newValue);
    }

    void SceneActionCollectionCreator::allocateRenderable(NodeHandle nodeHandle, RenderableHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::AllocateRenderable);
        collection.write(nodeHandle);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::releaseRenderable(RenderableHandle renderableHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::ReleaseRenderable);
        collection.write(renderableHandle);
    }

    void SceneActionCollectionCreator::setDataResource(DataInstanceHandle handle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, uint32_t instancingDivisor, uint16_t offsetWithinElementInBytes, uint16_t stride)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetDataResource);
        collection.write(handle);
        collection.write(field);
        collection.write(hash);
        collection.write(dataBuffer);
        collection.write(instancingDivisor);
        collection.write(offsetWithinElementInBytes);
        collection.write(stride);
    }

    void SceneActionCollectionCreator::setDataMatrix22fArray(DataInstanceHandle handle, DataFieldHandle field, uint32_t elementCount, const glm::mat2* data)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetDataMatrix22fArray);
        collection.write(handle);
        collection.write(field);
        collection.write(elementCount);
        for (uint32_t i = 0; i < elementCount; ++i)
            collection.write(data[i]);
    }

    void SceneActionCollectionCreator::setDataMatrix33fArray(DataInstanceHandle handle, DataFieldHandle field, uint32_t elementCount, const glm::mat3* data)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetDataMatrix33fArray);
        collection.write(handle);
        collection.write(field);
        collection.write(elementCount);
        for (uint32_t i = 0; i < elementCount; ++i)
            collection.write(data[i]);
    }

    void SceneActionCollectionCreator::setDataMatrix44fArray(DataInstanceHandle handle, DataFieldHandle field, uint32_t elementCount, const glm::mat4* data)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetDataMatrix44fArray);
        collection.write(handle);
        collection.write(field);
        collection.write(elementCount);
        for (uint32_t i = 0; i < elementCount; ++i)
            collection.write(data[i]);
    }

    void SceneActionCollectionCreator::setDataVector4fArray(DataInstanceHandle handle, DataFieldHandle field, uint32_t elementCount, const glm::vec4* data)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetDataVector4fArray);
        collection.write(handle);
        collection.write(field);
        collection.write(elementCount);
        for (uint32_t i = 0; i < elementCount; ++i)
            collection.write(data[i]);
    }

    void SceneActionCollectionCreator::setDataVector4iArray(DataInstanceHandle handle, DataFieldHandle field, uint32_t elementCount, const glm::ivec4* data)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetDataVector4iArray);
        collection.write(handle);
        collection.write(field);
        collection.write(elementCount);
        for (uint32_t i = 0; i < elementCount; ++i)
            collection.write(data[i]);
    }

    void SceneActionCollectionCreator::setDataVector3fArray(DataInstanceHandle handle, DataFieldHandle field, uint32_t elementCount, const glm::vec3* data)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetDataVector3fArray);
        collection.write(handle);
        collection.write(field);
        collection.write(elementCount);
        for (uint32_t i = 0; i < elementCount; ++i)
            collection.write(data[i]);
    }

    void SceneActionCollectionCreator::setDataVector3iArray(DataInstanceHandle handle, DataFieldHandle field, uint32_t elementCount, const glm::ivec3* data)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetDataVector3iArray);
        collection.write(handle);
        collection.write(field);
        collection.write(elementCount);
        for (uint32_t i = 0; i < elementCount; ++i)
            collection.write(data[i]);
    }

    void SceneActionCollectionCreator::setDataVector2fArray(DataInstanceHandle handle, DataFieldHandle field, uint32_t elementCount, const glm::vec2* data)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetDataVector2fArray);
        collection.write(handle);
        collection.write(field);
        collection.write(elementCount);
        for (uint32_t i = 0; i < elementCount; ++i)
            collection.write(data[i]);
    }

    void SceneActionCollectionCreator::setDataVector2iArray(DataInstanceHandle handle, DataFieldHandle field, uint32_t elementCount, const glm::ivec2* data)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetDataVector2iArray);
        collection.write(handle);
        collection.write(field);
        collection.write(elementCount);
        for (uint32_t i = 0; i < elementCount; ++i)
            collection.write(data[i]);
    }

    void SceneActionCollectionCreator::setDataFloatArray(DataInstanceHandle handle, DataFieldHandle field, uint32_t elementCount, const float* data)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetDataFloatArray);
        collection.write(handle);
        collection.write(field);
        collection.write(data, elementCount);
    }

    void SceneActionCollectionCreator::setDataBooleanArray(DataInstanceHandle handle, DataFieldHandle field, uint32_t elementCount, const bool* data)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetDataBooleanArray);
        collection.write(handle);
        collection.write(field);
        collection.write(data, elementCount);
    }

    void SceneActionCollectionCreator::setDataIntegerArray(DataInstanceHandle handle, DataFieldHandle field, uint32_t elementCount, const int32_t* data)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetDataIntegerArray);
        collection.write(handle);
        collection.write(field);
        collection.write(data, elementCount);
    }

    void SceneActionCollectionCreator::releaseDataInstance(DataInstanceHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::ReleaseDataInstance);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::allocateDataInstance(DataLayoutHandle layoutHandle, DataInstanceHandle dataInstanceHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::AllocateDataInstance);
        collection.write(layoutHandle);
        collection.write(dataInstanceHandle);
    }

    void SceneActionCollectionCreator::releaseTransform(TransformHandle transform)
    {
        collection.beginWriteSceneAction(ESceneActionId::ReleaseTransform);
        collection.write(transform);
    }

    void SceneActionCollectionCreator::allocateTransform(NodeHandle nodeHandle, TransformHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::AllocateTransform);
        collection.write(nodeHandle);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::allocateNode(uint32_t childrenCount, NodeHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::AllocateNode);
        collection.write(childrenCount);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::releaseNode(NodeHandle nodeHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::ReleaseNode);
        collection.write(nodeHandle);
    }

    void SceneActionCollectionCreator::addChildToNode(NodeHandle parent, NodeHandle child)
    {
        collection.beginWriteSceneAction(ESceneActionId::AddChildToNode);
        collection.write(parent);
        collection.write(child);
    }

    void SceneActionCollectionCreator::removeChildFromNode(NodeHandle parent, NodeHandle child)
    {
        collection.beginWriteSceneAction(ESceneActionId::RemoveChildFromNode);
        collection.write(parent);
        collection.write(child);
    }

    void SceneActionCollectionCreator::setRenderableIndexCount(RenderableHandle renderableHandle, uint32_t indexCount)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetRenderableIndexCount);
        collection.write(renderableHandle);
        collection.write(indexCount);
    }

    void SceneActionCollectionCreator::setRenderableStartIndex(RenderableHandle renderableHandle, uint32_t startIndex)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetRenderableStartIndex);
        collection.write(renderableHandle);
        collection.write(startIndex);
    }

    void SceneActionCollectionCreator::setRenderableVisibility(RenderableHandle renderableHandle, EVisibilityMode visible)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetRenderableVisibility);
        collection.write(renderableHandle);
        collection.write(visible);
    }

    void SceneActionCollectionCreator::setRenderableInstanceCount(RenderableHandle renderableHandle, uint32_t instanceCount)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetRenderableInstanceCount);
        collection.write(renderableHandle);
        collection.write(instanceCount);
    }

    void SceneActionCollectionCreator::setRenderableStartVertex(RenderableHandle renderableHandle, uint32_t startVertex)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetRenderableStartVertex);
        collection.write(renderableHandle);
        collection.write(startVertex);
    }

    void SceneActionCollectionCreator::setRenderableDataInstance(RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetRenderableDataInstance);
        collection.write(renderableHandle);
        collection.write(static_cast<uint32_t>(slot));
        collection.write(newDataInstance);
    }

    void SceneActionCollectionCreator::releaseDataLayout(DataLayoutHandle layoutHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::ReleaseDataLayout);
        collection.write(layoutHandle);
    }

    void SceneActionCollectionCreator::allocateDataLayout(const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::AllocateDataLayout);
        collection.write(handle);
        collection.write(static_cast<uint32_t>(dataFields.size()));
        for (const auto& dataField : dataFields)
        {
            collection.write(static_cast<uint32_t>(dataField.dataType));
            collection.write(dataField.elementCount);
            collection.write(static_cast<uint32_t>(dataField.semantics));
        }
        collection.write(effectHash);
    }

    void SceneActionCollectionCreator::setRenderStateStencilOps(RenderStateHandle stateHandle, EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetStateStencilOps);
        collection.write(stateHandle);
        collection.write(sfail);
        collection.write(dpfail);
        collection.write(dppass);
    }

    void SceneActionCollectionCreator::setRenderStateStencilFunc(RenderStateHandle stateHandle, EStencilFunc func, uint8_t ref, uint8_t mask)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetStateStencilFunc);
        collection.write(stateHandle);
        collection.write(func);
        collection.write(ref);
        collection.write(mask);
    }

    void SceneActionCollectionCreator::setRenderStateDepthWrite(RenderStateHandle stateHandle, EDepthWrite flag)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetStateDepthWrite);
        collection.write(stateHandle);
        collection.write(flag);
    }

    void SceneActionCollectionCreator::setRenderStateDepthFunc(RenderStateHandle stateHandle, EDepthFunc func)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetStateDepthFunc);
        collection.write(stateHandle);
        collection.write(func);
    }

    void SceneActionCollectionCreator::setRenderStateScissorTest(RenderStateHandle stateHandle, EScissorTest flag, const RenderState::ScissorRegion& region)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetStateScissorTest);
        collection.write(stateHandle);
        collection.write(flag);
        collection.write(region.x);
        collection.write(region.y);
        collection.write(region.width);
        collection.write(region.height);
    }

    void SceneActionCollectionCreator::setRenderStateCullMode(RenderStateHandle stateHandle, ECullMode cullMode)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetStateCullMode);
        collection.write(stateHandle);
        collection.write(cullMode);
    }

    void SceneActionCollectionCreator::setRenderStateDrawMode(RenderStateHandle stateHandle, EDrawMode drawMode)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetStateDrawMode);
        collection.write(stateHandle);
        collection.write(drawMode);
    }

    void SceneActionCollectionCreator::setRenderStateBlendOperations(RenderStateHandle stateHandle, EBlendOperation operationColor, EBlendOperation operationAlpha)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetStateBlendOperations);
        collection.write(stateHandle);
        collection.write(operationColor);
        collection.write(operationAlpha);
    }

    void SceneActionCollectionCreator::setRenderStateBlendFactors(RenderStateHandle stateHandle, EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetStateBlendFactors);
        collection.write(stateHandle);
        collection.write(srcColor);
        collection.write(destColor);
        collection.write(srcAlpha);
        collection.write(destAlpha);
    }

    void SceneActionCollectionCreator::setRenderStateBlendColor(RenderStateHandle stateHandle, const glm::vec4& color)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetStateBlendColor);
        collection.write(stateHandle);
        collection.write(color.r);
        collection.write(color.g);
        collection.write(color.b);
        collection.write(color.a);
    }

    void SceneActionCollectionCreator::setRenderStateColorWriteMask(RenderStateHandle stateHandle, ColorWriteMask colorMask)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetStateColorWriteMask);
        collection.write(stateHandle);
        collection.write(colorMask);
    }

    void SceneActionCollectionCreator::releaseRenderState(RenderStateHandle stateHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::ReleaseState);
        collection.write(stateHandle);
    }

    void SceneActionCollectionCreator::allocateRenderState(RenderStateHandle stateHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::AllocateRenderState);
        collection.write(stateHandle);
    }

    void SceneActionCollectionCreator::setRenderableRenderState(RenderableHandle renderableHandle, RenderStateHandle stateHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetRenderableState);
        collection.write(renderableHandle);
        collection.write(stateHandle);
    }

    void SceneActionCollectionCreator::allocateCamera(ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle dataInstance, CameraHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::AllocateCamera);
        collection.write(static_cast<uint32_t>(type));
        collection.write(nodeHandle);
        collection.write(dataInstance);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::releaseCamera(CameraHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::ReleaseCamera);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::allocateRenderGroup(uint32_t renderableCount, uint32_t nestedGroupCount, RenderGroupHandle groupHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::AllocateRenderGroup);
        collection.write(renderableCount);
        collection.write(nestedGroupCount);
        collection.write(groupHandle);
    }

    void SceneActionCollectionCreator::releaseRenderGroup(RenderGroupHandle groupHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::ReleaseRenderGroup);
        collection.write(groupHandle);
    }

    void SceneActionCollectionCreator::addRenderableToRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle, int32_t order)
    {
        collection.beginWriteSceneAction(ESceneActionId::AddRenderableToRenderGroup);
        collection.write(groupHandle);
        collection.write(renderableHandle);
        collection.write(order);
    }

    void SceneActionCollectionCreator::removeRenderableFromRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::RemoveRenderableFromRenderGroup);
        collection.write(groupHandle);
        collection.write(renderableHandle);
    }

    void SceneActionCollectionCreator::addRenderGroupToRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, int32_t order)
    {
        collection.beginWriteSceneAction(ESceneActionId::AddRenderGroupToRenderGroup);
        collection.write(groupHandleParent);
        collection.write(groupHandleChild);
        collection.write(order);
    }

    void SceneActionCollectionCreator::removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild)
    {
        collection.beginWriteSceneAction(ESceneActionId::RemoveRenderGroupFromRenderGroup);
        collection.write(groupHandleParent);
        collection.write(groupHandleChild);
    }

    void SceneActionCollectionCreator::allocateRenderPass(uint32_t renderGroupCount, RenderPassHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::AllocateRenderPass);
        collection.write(renderGroupCount);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::releaseRenderPass(RenderPassHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::ReleaseRenderPass);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::setRenderPassCamera(RenderPassHandle pass, CameraHandle camera)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetRenderPassCamera);
        collection.write(pass);
        collection.write(camera);
    }

    void SceneActionCollectionCreator::setRenderPassRenderTarget(RenderPassHandle pass, RenderTargetHandle targetHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetRenderPassRenderTarget);
        collection.write(pass);
        collection.write(targetHandle);
    }

    void SceneActionCollectionCreator::setRenderPassRenderOrder(RenderPassHandle pass, int32_t renderOrder)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetRenderPassRenderOrder);
        collection.write(pass);
        collection.write(renderOrder);
    }

    void SceneActionCollectionCreator::setRenderPassEnabled(RenderPassHandle pass, bool isEnabled)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetRenderPassEnabled);
        collection.write(pass);
        collection.write(isEnabled);
    }

    void SceneActionCollectionCreator::setRenderPassRenderOnce(RenderPassHandle pass, bool enabled)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetRenderPassRenderOnce);
        collection.write(pass);
        collection.write(enabled);
    }

    void SceneActionCollectionCreator::retriggerRenderPassRenderOnce(RenderPassHandle pass)
    {
        collection.beginWriteSceneAction(ESceneActionId::RetriggerRenderPassRenderOnce);
        collection.write(pass);
    }

    void SceneActionCollectionCreator::addRenderGroupToRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle, int32_t order)
    {
        collection.beginWriteSceneAction(ESceneActionId::AddRenderGroupToRenderPass);
        collection.write(passHandle);
        collection.write(groupHandle);
        collection.write(order);
    }

    void SceneActionCollectionCreator::removeRenderGroupFromRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::RemoveRenderGroupFromRenderPass);
        collection.write(passHandle);
        collection.write(groupHandle);
    }


    void SceneActionCollectionCreator::allocatePickableObject(DataBufferHandle geometryHandle, NodeHandle nodeHandle, PickableObjectId id, PickableObjectHandle pickableHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::AllocatePickableObject);
        collection.write(geometryHandle);
        collection.write(nodeHandle);
        collection.write(id);
        collection.write(pickableHandle);
    }

    void SceneActionCollectionCreator::releasePickableObject(PickableObjectHandle pickableHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::ReleasePickableObject);
        collection.write(pickableHandle);
    }

    void SceneActionCollectionCreator::setPickableObjectId(PickableObjectHandle pickableHandle, PickableObjectId id)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetPickableObjectId);
        collection.write(pickableHandle);
        collection.write(id);
    }

    void SceneActionCollectionCreator::setPickableObjectCamera(PickableObjectHandle pickableHandle, CameraHandle cameraHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetPickableObjectCamera);
        collection.write(pickableHandle);
        collection.write(cameraHandle);
    }

    void SceneActionCollectionCreator::setPickableObjectEnabled(PickableObjectHandle pickableHandle, bool isEnabled)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetPickableObjectEnabled);
        collection.write(pickableHandle);
        collection.write(isEnabled);
    }

    void SceneActionCollectionCreator::allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::AllocateBlitPass);
        collection.write(sourceRenderBufferHandle);
        collection.write(destinationRenderBufferHandle);
        collection.write(passHandle);
    }

    void SceneActionCollectionCreator::releaseBlitPass(BlitPassHandle passHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::ReleaseBlitPass);
        collection.write(passHandle);
    }

    void SceneActionCollectionCreator::setBlitPassRenderOrder(BlitPassHandle passHandle, int32_t renderOrder)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetBlitPassRenderOrder);
        collection.write(passHandle);
        collection.write(renderOrder);
    }

    void SceneActionCollectionCreator::setBlitPassEnabled(BlitPassHandle passHandle, bool isEnabled)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetBlitPassEnabled);
        collection.write(passHandle);
        collection.write(isEnabled);
    }

    void SceneActionCollectionCreator::setBlitPassRegions(BlitPassHandle passHandle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetBlitPassRegions);
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
        collection.beginWriteSceneAction(ESceneActionId::SetDataTextureSamplerHandle);
        collection.write(handle);
        collection.write(field);
        collection.write(samplerHandle);
    }

    void SceneActionCollectionCreator::setDataReference(DataInstanceHandle handle, DataFieldHandle field, DataInstanceHandle dataRef)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetDataReference);
        collection.write(handle);
        collection.write(field);
        collection.write(dataRef);
    }

    void SceneActionCollectionCreator::setDataUniformBuffer(DataInstanceHandle handle, DataFieldHandle field, UniformBufferHandle uniformBufferHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetDataUniformBuffer);
        collection.write(handle);
        collection.write(field);
        collection.write(uniformBufferHandle);
    }

    void SceneActionCollectionCreator::allocateTextureSampler(const TextureSampler& sampler, TextureSamplerHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::AllocateTextureSampler);
        collection.write(handle);
        collection.write(sampler.states.m_addressModeU);
        collection.write(sampler.states.m_addressModeV);
        collection.write(sampler.states.m_addressModeR);
        collection.write(sampler.states.m_minSamplingMode);
        collection.write(sampler.states.m_magSamplingMode);
        collection.write(sampler.states.m_anisotropyLevel);
        collection.write(sampler.contentType);
        if (sampler.contentType == TextureSampler::ContentType::ClientTexture)
        {
            collection.write(sampler.textureResource);
        }
        else
        {
            collection.write(sampler.contentHandle);
        }
    }

    void SceneActionCollectionCreator::releaseTextureSampler(TextureSamplerHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::ReleaseTextureSampler);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::allocateRenderTarget(RenderTargetHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::AllocateRenderTarget);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::releaseRenderTarget(RenderTargetHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::ReleaseRenderTarget);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::addRenderTargetRenderBuffer(RenderTargetHandle targetHandle, RenderBufferHandle bufferHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::AddRenderTargetRenderBuffer);
        collection.write(targetHandle);
        collection.write(bufferHandle);
    }

    void SceneActionCollectionCreator::allocateRenderBuffer(const RenderBuffer& renderBuffer, RenderBufferHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::AllocateRenderBuffer);
        collection.write(renderBuffer.width);
        collection.write(renderBuffer.height);
        collection.write(handle);
        collection.write(static_cast<uint32_t>(renderBuffer.format));
        collection.write(static_cast<uint32_t>(renderBuffer.accessMode));
        collection.write(renderBuffer.sampleCount);
    }

    void SceneActionCollectionCreator::releaseRenderBuffer(RenderBufferHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::ReleaseRenderBuffer);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::setRenderBufferProperties(RenderBufferHandle handle, uint32_t width, uint32_t height, uint32_t sampleCount)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetRenderBufferProperties);
        collection.write(handle);
        collection.write(width);
        collection.write(height);
        collection.write(sampleCount);
    }

    void SceneActionCollectionCreator::allocateDataBuffer(EDataBufferType dataBufferType, EDataType dataType, uint32_t maximumSizeInBytes, DataBufferHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::AllocateDataBuffer);
        collection.write(static_cast<uint32_t>(dataBufferType));
        collection.write(static_cast<uint32_t>(dataType));
        collection.write(maximumSizeInBytes);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::releaseDataBuffer(DataBufferHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::ReleaseDataBuffer);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::updateDataBuffer(DataBufferHandle handle, uint32_t offsetInBytes, uint32_t dataSizeInBytes, const std::byte* data)
    {
        collection.beginWriteSceneAction(ESceneActionId::UpdateDataBuffer);
        collection.write(handle);
        collection.write(offsetInBytes);
        collection.write(data, dataSizeInBytes);
    }

    void SceneActionCollectionCreator::allocateUniformBuffer(uint32_t size, UniformBufferHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::AllocateUniformBuffer);
        collection.write(handle);
        collection.write(size);
    }

    void SceneActionCollectionCreator::releaseUniformBuffer(UniformBufferHandle uniformBufferHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::ReleaseUniformBuffer);
        collection.write(uniformBufferHandle);
    }

    void SceneActionCollectionCreator::updateUniformBuffer(UniformBufferHandle handle, uint32_t offset, uint32_t size, const std::byte* data)
    {
        collection.beginWriteSceneAction(ESceneActionId::UpdateUniformBuffer);
        collection.write(handle);
        collection.write(offset);
        collection.write(data, size);
    }

    void SceneActionCollectionCreator::allocateTextureBuffer(EPixelStorageFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::AllocateTextureBuffer);
        collection.write(static_cast<uint32_t>(textureFormat));
        collection.write(static_cast<uint32_t>(mipMapDimensions.size()));
        for (const auto& size : mipMapDimensions)
        {
            collection.write(size.width);
            collection.write(size.height);
        }
        collection.write(handle);
    }

    void SceneActionCollectionCreator::releaseTextureBuffer(TextureBufferHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::ReleaseTextureBuffer);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::updateTextureBuffer(TextureBufferHandle handle, uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const std::byte* data, uint32_t dataSize)
    {
        collection.beginWriteSceneAction(ESceneActionId::UpdateTextureBuffer);
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
        collection.beginWriteSceneAction(ESceneActionId::AllocateDataSlot);
        collection.write(static_cast<uint32_t>(dataSlot.type));
        collection.write(dataSlot.id);
        collection.write(dataSlot.attachedNode);
        collection.write(dataSlot.attachedDataReference);
        collection.write(dataSlot.attachedTexture);
        collection.write(dataSlot.attachedTextureSampler);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::setDataSlotTexture(DataSlotHandle handle, const ResourceContentHash& texture)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetDataSlotTexture);
        collection.write(handle);
        collection.write(texture);
    }

    void SceneActionCollectionCreator::releaseDataSlot(DataSlotHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::ReleaseDataSlot);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::allocateSceneReference(SceneId sceneId, SceneReferenceHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::AllocateSceneReference);
        collection.write(handle);
        collection.write(sceneId);
    }

    void SceneActionCollectionCreator::releaseSceneReference(SceneReferenceHandle handle)
    {
        collection.beginWriteSceneAction(ESceneActionId::ReleaseSceneReference);
        collection.write(handle);
    }

    void SceneActionCollectionCreator::requestSceneReferenceState(SceneReferenceHandle handle, RendererSceneState state)
    {
        collection.beginWriteSceneAction(ESceneActionId::RequestSceneReferenceState);
        collection.write(handle);
        collection.write(state);
    }

    void SceneActionCollectionCreator::requestSceneReferenceFlushNotifications(SceneReferenceHandle handle, bool enable)
    {
        collection.beginWriteSceneAction(ESceneActionId::RequestSceneReferenceFlushNotifications);
        collection.write(handle);
        collection.write(enable);
    }

    void SceneActionCollectionCreator::setSceneReferenceRenderOrder(SceneReferenceHandle handle, int32_t renderOrder)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetSceneReferenceRenderOrder);
        collection.write(handle);
        collection.write(renderOrder);
    }

    void SceneActionCollectionCreator::setRenderPassClearColor(RenderPassHandle handle, const glm::vec4& clearColor)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetRenderPassClearColor);
        collection.write(handle);
        collection.write(clearColor);
    }

    void SceneActionCollectionCreator::setRenderPassClearFlag(RenderPassHandle handle, ClearFlags clearFlag)
    {
        collection.beginWriteSceneAction(ESceneActionId::SetRenderPassClearFlag);
        collection.write(handle);
        collection.write(clearFlag.value());
    }

    void SceneActionCollectionCreator::compoundRenderableData(
        RenderableHandle renderableHandle,
        DataInstanceHandle uniformInstanceHandle,
        RenderStateHandle stateHandle)
    {
        collection.beginWriteSceneAction(ESceneActionId::CompoundRenderableEffectData);
        collection.write(renderableHandle);
        collection.write(uniformInstanceHandle);
        collection.write(stateHandle);
    }

    void SceneActionCollectionCreator::compoundRenderable(RenderableHandle renderableHandle, const Renderable& renderable)
    {
        static_assert(ERenderableDataSlotType_MAX_SLOTS == 2u
            , "Expected ERenderableDataSlotType containing 2 elements, adjust ESceneActionId::CompoundRenderable SceneAction handling");

        collection.beginWriteSceneAction(ESceneActionId::CompoundRenderable);
        collection.write(renderableHandle);
        collection.write(renderable.node);
        collection.write(renderable.startIndex);
        collection.write(renderable.indexCount);
        collection.write(renderable.renderState);
        collection.write(renderable.visibilityMode);
        collection.write(renderable.instanceCount);
        collection.write(renderable.startVertex);
        collection.write(renderable.dataInstances[ERenderableDataSlotType_Geometry]);
        collection.write(renderable.dataInstances[ERenderableDataSlotType_Uniforms]);
    }

    void SceneActionCollectionCreator::compoundState(RenderStateHandle handle, const RenderState& rs)
    {
        collection.beginWriteSceneAction(ESceneActionId::CompoundState);
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
        collection.write(rs.blendColor.r);
        collection.write(rs.blendColor.g);
        collection.write(rs.blendColor.b);
        collection.write(rs.blendColor.a);
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

    void SceneActionCollectionCreator::putSceneSizeInformation(const SceneSizeInformation& sizeInfo)
    {
        collection.write(sizeInfo.nodeCount);
        collection.write(sizeInfo.cameraCount);
        collection.write(sizeInfo.transformCount);
        collection.write(sizeInfo.renderableCount);
        collection.write(sizeInfo.renderStateCount);
        collection.write(sizeInfo.datalayoutCount);
        collection.write(sizeInfo.datainstanceCount);
        if (m_featureLevel >= EFeatureLevel_02)
            collection.write(sizeInfo.uniformBufferCount);
        collection.write(sizeInfo.renderGroupCount);
        collection.write(sizeInfo.renderPassCount);
        collection.write(sizeInfo.blitPassCount);
        collection.write(sizeInfo.renderTargetCount);
        collection.write(sizeInfo.renderBufferCount);
        collection.write(sizeInfo.textureSamplerCount);
        collection.write(sizeInfo.dataSlotCount);
        collection.write(sizeInfo.dataBufferCount);
        collection.write(sizeInfo.textureBufferCount);
        collection.write(sizeInfo.pickableObjectCount);
        collection.write(sizeInfo.sceneReferenceCount);
    }
}
