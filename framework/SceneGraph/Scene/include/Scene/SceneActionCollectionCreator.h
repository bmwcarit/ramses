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
    enum class EDataBufferType : UInt8;
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
        void setRenderableStartIndex(RenderableHandle renderableHandle, UInt32 startIndex);
        void setRenderableIndexCount(RenderableHandle renderableHandle, UInt32 indexCount);
        void setRenderableRenderState(RenderableHandle renderableHandle, RenderStateHandle stateHandle);
        void setRenderableVisibility(RenderableHandle renderableHandle, EVisibilityMode visible);
        void setRenderableInstanceCount(RenderableHandle renderableHandle, UInt32 instanceCount);
        void setRenderableStartVertex(RenderableHandle renderableHandle, UInt32 startVertex);

        // Render state allocation
        void allocateRenderState(RenderStateHandle stateHandle);
        void releaseRenderState(RenderStateHandle stateHandle);

        // Render state setters
        void setRenderStateBlendFactors(RenderStateHandle stateHandle, EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha);
        void setRenderStateBlendOperations(RenderStateHandle stateHandle, EBlendOperation operationColor, EBlendOperation operationAlpha);
        void setRenderStateBlendColor(RenderStateHandle stateHandle, const Vector4& color);
        void setRenderStateCullMode(RenderStateHandle stateHandle, ECullMode cullMode);
        void setRenderStateDrawMode(RenderStateHandle stateHandle, EDrawMode drawMode);
        void setRenderStateDepthFunc(RenderStateHandle stateHandle, EDepthFunc func);
        void setRenderStateDepthWrite(RenderStateHandle stateHandle, EDepthWrite flag);
        void setRenderStateScissorTest(RenderStateHandle stateHandle, EScissorTest flag, const RenderState::ScissorRegion& region);
        void setRenderStateStencilFunc(RenderStateHandle stateHandle, EStencilFunc func, UInt8 ref, UInt8 mask);
        void setRenderStateStencilOps(RenderStateHandle stateHandle, EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass);
        void setRenderStateColorWriteMask(RenderStateHandle stateHandle, ColorWriteMask colorMask);

        // Camera
        void allocateCamera(ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle dataInstance, CameraHandle handle);
        void releaseCamera(CameraHandle cameraHandle);

        // Creation/Deletion
        void allocateNode(UInt32 childrenCount, NodeHandle handle);
        void releaseNode(NodeHandle nodeHandle);

        void allocateTransform(NodeHandle nodeHandle, TransformHandle handle);
        void releaseTransform(TransformHandle transform);

        // Parent-child relationship
        void addChildToNode(NodeHandle parent, NodeHandle child);
        void removeChildFromNode(NodeHandle parent, NodeHandle child);

        // Transformation
        void setTranslation(TransformHandle node, const Vector3& newValue);
        void setRotation(TransformHandle node, const Vector4& newValue, ERotationType rotationType);
        void setScaling(TransformHandle node, const Vector3& newValue);

        void allocateDataLayout(const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutHandle handle);
        void releaseDataLayout(DataLayoutHandle layoutHandle);

        void allocateDataInstance(DataLayoutHandle finishedLayoutHandle, DataInstanceHandle instanceHandle);
        void releaseDataInstance(DataInstanceHandle containerHandle);

        void setDataFloatArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Float* data);
        void setDataVector2fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector2* data);
        void setDataVector3fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector3* data);
        void setDataVector4fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector4* data);
        void setDataIntegerArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Int32* data);
        void setDataVector2iArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector2i* data);
        void setDataVector3iArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector3i* data);
        void setDataVector4iArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector4i* data);
        void setDataMatrix22fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix22f* data);
        void setDataMatrix33fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix33f* data);
        void setDataMatrix44fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix44f* data);
        void setDataResource(DataInstanceHandle containerHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, UInt32 instancingDivisor, UInt16 offsetWithinElementInBytes, UInt16 stride);
        void setDataTextureSamplerHandle(DataInstanceHandle containerHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle);
        void setDataReference(DataInstanceHandle containerHandle, DataFieldHandle field, DataInstanceHandle dataRef);

        // Texture sampler description
        void allocateTextureSampler(const TextureSampler& sampler, TextureSamplerHandle handle);
        void releaseTextureSampler(TextureSamplerHandle handle);

        // Render groups
        void allocateRenderGroup(UInt32 renderableCount, UInt32 nestedGroupCount, RenderGroupHandle groupHandle);
        void releaseRenderGroup(RenderGroupHandle groupHandle);
        void addRenderableToRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle, Int32 order);
        void removeRenderableFromRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle);
        void addRenderGroupToRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, Int32 order);
        void removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild);

        // Render passes
        void allocateRenderPass(UInt32 renderGroupCount, RenderPassHandle passHandle);
        void releaseRenderPass(RenderPassHandle passHandle);
        void setRenderPassClearColor(RenderPassHandle passHandle, const Vector4& clearColor);
        void setRenderPassClearFlag(RenderPassHandle passHandle, UInt32 clearFlag);
        void setRenderPassCamera(RenderPassHandle passHandle, CameraHandle cameraHandle);
        void setRenderPassRenderTarget(RenderPassHandle passHandle, RenderTargetHandle targetHandle);
        void setRenderPassRenderOrder(RenderPassHandle passHandle, Int32 renderOrder);
        void setRenderPassEnabled(RenderPassHandle passHandle, bool isEnabled);
        void setRenderPassRenderOnce(RenderPassHandle pass, bool enabled);
        void retriggerRenderPassRenderOnce(RenderPassHandle pass);
        void addRenderGroupToRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle, Int32 order);
        void removeRenderGroupFromRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle);

        // Blit passes
        void allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle);
        void releaseBlitPass(BlitPassHandle passHandle);
        void setBlitPassRenderOrder(BlitPassHandle passHandle, Int32 renderOrder);
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
        void allocateDataBuffer(EDataBufferType dataBufferType, EDataType dataType, UInt32 maximumSizeInBytes, DataBufferHandle handle);
        void releaseDataBuffer(DataBufferHandle handle);
        void updateDataBuffer(DataBufferHandle handle, UInt32 offsetInBytes, UInt32 dataSizeInBytes, const Byte* data);

        // Texture buffers
        void allocateTextureBuffer(ETextureFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle);
        void releaseTextureBuffer(TextureBufferHandle handle);
        void updateTextureBuffer(TextureBufferHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data, UInt32 dataSize);

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
