//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEACTIONCOLLECTIONCREATOR_H
#define RAMSES_SCENEACTIONCOLLECTIONCREATOR_H

#include "Scene/SceneActionCollection.h"
#include "SceneAPI/ERenderableDataSlotType.h"
#include "SceneAPI/RenderState.h"
#include "SceneAPI/EDataType.h"
#include "SceneAPI/EFixedSemantics.h"
#include "SceneAPI/ECameraProjectionType.h"
#include "SceneAPI/TextureEnums.h"
#include "SceneAPI/EDataSlotType.h"
#include "SceneAPI/DataSlot.h"
#include "SceneAPI/SceneVersionTag.h"
#include "SceneAPI/SceneSizeInformation.h"
#include "SceneAPI/DataFieldInfo.h"
#include "SceneAPI/MipMapSize.h"
#include "SceneAPI/PickableObject.h"
#include "SceneAPI/Renderable.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/RendererSceneState.h"
#include "SceneAPI/ERotationType.h"
#include "Scene/ResourceChanges.h"
#include "Resource/TextureMetaInfo.h"
#include "Components/FlushTimeInformation.h"
#include "SceneReferencing/SceneReferenceAction.h"


namespace ramses_internal
{
    struct PixelRectangle;
    class IResource;
    enum class EDataBufferType : uint8_t;
    struct FlushTimeInformation;
    struct Viewport;
    struct Frustum;
    struct TextureSampler;
    struct RenderBuffer;

    class SceneActionCollectionCreator
    {
    public:
        explicit SceneActionCollectionCreator(SceneActionCollection& collection_);

        void preallocateSceneSize(const SceneSizeInformation& sizeInfo);

        // Renderable allocation
        void allocateRenderable(NodeHandle nodeHandle, RenderableHandle handle);
        void releaseRenderable(RenderableHandle renderableHandle);

        // Renderable data (stuff required for rendering)
        void setRenderableDataInstance(RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance);
        void setRenderableStartIndex(RenderableHandle renderableHandle, uint32_t startIndex);
        void setRenderableIndexCount(RenderableHandle renderableHandle, uint32_t indexCount);
        void setRenderableRenderState(RenderableHandle renderableHandle, RenderStateHandle stateHandle);
        void setRenderableVisibility(RenderableHandle renderableHandle, EVisibilityMode visible);
        void setRenderableInstanceCount(RenderableHandle renderableHandle, uint32_t instanceCount);
        void setRenderableStartVertex(RenderableHandle renderableHandle, uint32_t startVertex);

        // Render state allocation
        void allocateRenderState(RenderStateHandle stateHandle);
        void releaseRenderState(RenderStateHandle stateHandle);

        // Render state setters
        void setRenderStateBlendFactors(RenderStateHandle stateHandle, EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha);
        void setRenderStateBlendOperations(RenderStateHandle stateHandle, EBlendOperation operationColor, EBlendOperation operationAlpha);
        void setRenderStateBlendColor(RenderStateHandle stateHandle, const glm::vec4& color);
        void setRenderStateCullMode(RenderStateHandle stateHandle, ECullMode cullMode);
        void setRenderStateDrawMode(RenderStateHandle stateHandle, EDrawMode drawMode);
        void setRenderStateDepthFunc(RenderStateHandle stateHandle, EDepthFunc func);
        void setRenderStateDepthWrite(RenderStateHandle stateHandle, EDepthWrite flag);
        void setRenderStateScissorTest(RenderStateHandle stateHandle, EScissorTest flag, const RenderState::ScissorRegion& region);
        void setRenderStateStencilFunc(RenderStateHandle stateHandle, EStencilFunc func, uint8_t ref, uint8_t mask);
        void setRenderStateStencilOps(RenderStateHandle stateHandle, EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass);
        void setRenderStateColorWriteMask(RenderStateHandle stateHandle, ColorWriteMask colorMask);

        // Camera
        void allocateCamera(ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle dataInstance, CameraHandle handle);
        void releaseCamera(CameraHandle cameraHandle);

        // Creation/Deletion
        void allocateNode(uint32_t childrenCount, NodeHandle handle);
        void releaseNode(NodeHandle nodeHandle);

        void allocateTransform(NodeHandle nodeHandle, TransformHandle handle);
        void releaseTransform(TransformHandle transform);

        // Parent-child relationship
        void addChildToNode(NodeHandle parent, NodeHandle child);
        void removeChildFromNode(NodeHandle parent, NodeHandle child);

        // Transformation
        void setTranslation(TransformHandle node, const glm::vec3& newValue);
        void setRotation(TransformHandle node, const glm::vec4& newValue, ERotationType rotationType);
        void setScaling(TransformHandle node, const glm::vec3& newValue);

        void allocateDataLayout(const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutHandle handle);
        void releaseDataLayout(DataLayoutHandle layoutHandle);

        void allocateDataInstance(DataLayoutHandle finishedLayoutHandle, DataInstanceHandle instanceHandle);
        void releaseDataInstance(DataInstanceHandle containerHandle);

        void setDataFloatArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const float* data);
        void setDataVector2fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec2* data);
        void setDataVector3fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec3* data);
        void setDataVector4fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec4* data);
        void setDataIntegerArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const int32_t* data);
        void setDataVector2iArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec2* data);
        void setDataVector3iArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec3* data);
        void setDataVector4iArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec4* data);
        void setDataMatrix22fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat2* data);
        void setDataMatrix33fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat3* data);
        void setDataMatrix44fArray(DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat4* data);
        void setDataResource(DataInstanceHandle containerHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, uint32_t instancingDivisor, uint16_t offsetWithinElementInBytes, uint16_t stride);
        void setDataTextureSamplerHandle(DataInstanceHandle containerHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle);
        void setDataReference(DataInstanceHandle containerHandle, DataFieldHandle field, DataInstanceHandle dataRef);

        // Texture sampler description
        void allocateTextureSampler(const TextureSampler& sampler, TextureSamplerHandle handle);
        void releaseTextureSampler(TextureSamplerHandle handle);

        // Render groups
        void allocateRenderGroup(uint32_t renderableCount, uint32_t nestedGroupCount, RenderGroupHandle groupHandle);
        void releaseRenderGroup(RenderGroupHandle groupHandle);
        void addRenderableToRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle, int32_t order);
        void removeRenderableFromRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle);
        void addRenderGroupToRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, int32_t order);
        void removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild);

        // Render passes
        void allocateRenderPass(uint32_t renderGroupCount, RenderPassHandle passHandle);
        void releaseRenderPass(RenderPassHandle passHandle);
        void setRenderPassClearColor(RenderPassHandle passHandle, const glm::vec4& clearColor);
        void setRenderPassClearFlag(RenderPassHandle passHandle, uint32_t clearFlag);
        void setRenderPassCamera(RenderPassHandle passHandle, CameraHandle cameraHandle);
        void setRenderPassRenderTarget(RenderPassHandle passHandle, RenderTargetHandle targetHandle);
        void setRenderPassRenderOrder(RenderPassHandle passHandle, int32_t renderOrder);
        void setRenderPassEnabled(RenderPassHandle passHandle, bool isEnabled);
        void setRenderPassRenderOnce(RenderPassHandle pass, bool enabled);
        void retriggerRenderPassRenderOnce(RenderPassHandle pass);
        void addRenderGroupToRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle, int32_t order);
        void removeRenderGroupFromRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle);

        // Blit passes
        void allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle);
        void releaseBlitPass(BlitPassHandle passHandle);
        void setBlitPassRenderOrder(BlitPassHandle passHandle, int32_t renderOrder);
        void setBlitPassEnabled(BlitPassHandle passHandle, bool isEnabled);
        void setBlitPassRegions(BlitPassHandle passHandle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion);

        // Pickable object
        void allocatePickableObject(DataBufferHandle geometryHandle, NodeHandle nodeHandle, PickableObjectId id, PickableObjectHandle pickableHandle);
        void releasePickableObject(PickableObjectHandle pickableHandle);
        void setPickableObjectId(PickableObjectHandle pickableHandle, PickableObjectId id);
        void setPickableObjectCamera(PickableObjectHandle pickableHandle, CameraHandle cameraHandle);
        void setPickableObjectEnabled(PickableObjectHandle pickableHandle, bool isEnabled);

        // Render targets
        void allocateRenderTarget(RenderTargetHandle targetHandle);
        void releaseRenderTarget(RenderTargetHandle targetHandle);
        void addRenderTargetRenderBuffer(RenderTargetHandle targetHandle, RenderBufferHandle bufferHandle);

        // Render buffers
        void allocateRenderBuffer(const RenderBuffer& renderBuffer, RenderBufferHandle handle);
        void releaseRenderBuffer(RenderBufferHandle handle);

        // Data buffers
        void allocateDataBuffer(EDataBufferType dataBufferType, EDataType dataType, uint32_t maximumSizeInBytes, DataBufferHandle handle);
        void releaseDataBuffer(DataBufferHandle handle);
        void updateDataBuffer(DataBufferHandle handle, uint32_t offsetInBytes, uint32_t dataSizeInBytes, const Byte* data);

        // Texture buffers
        void allocateTextureBuffer(ETextureFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle);
        void releaseTextureBuffer(TextureBufferHandle handle);
        void updateTextureBuffer(TextureBufferHandle handle, uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const Byte* data, uint32_t dataSize);

        void allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle);
        void setDataSlotTexture(DataSlotHandle handle, const ResourceContentHash& texture);
        void releaseDataSlot(DataSlotHandle handle);

        void allocateSceneReference(SceneId sceneId, SceneReferenceHandle handle);
        void releaseSceneReference(SceneReferenceHandle handle);
        void requestSceneReferenceState(SceneReferenceHandle handle, RendererSceneState state);
        void requestSceneReferenceFlushNotifications(SceneReferenceHandle handle, bool enable);
        void setSceneReferenceRenderOrder(SceneReferenceHandle handle, int32_t renderOrder);

        // compound actions
        void compoundRenderableData(RenderableHandle renderableHandle
                                            , DataInstanceHandle uniformInstanceHandle
                                            , RenderStateHandle stateHandle);

        void compoundRenderable(RenderableHandle renderableHandle, const Renderable& renderable);

        void compoundState(RenderStateHandle handle, const RenderState& rs);

        SceneActionCollection& collection;

    private:
        void putSceneSizeInformation(const SceneSizeInformation& sizeInfo);
    };
}

#endif
