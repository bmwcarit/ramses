//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ACTIONCOLLECTINGSCENE_H
#define RAMSES_ACTIONCOLLECTINGSCENE_H

#include "Scene/ResourceChangeCollectingScene.h"
#include "Scene/SceneActionCollectionCreator.h"
#include "SceneReferencing/SceneReferenceAction.h"

namespace ramses_internal
{
    class ActionCollectingScene : public ResourceChangeCollectingScene
    {
    public:
        explicit ActionCollectingScene(const SceneInfo& sceneInfo = SceneInfo());

        void                        preallocateSceneSize            (const SceneSizeInformation& sizeInfo) override;

        // Renderable allocation
        RenderableHandle            allocateRenderable              (NodeHandle nodeHandle, RenderableHandle handle = RenderableHandle::Invalid()) override;
        void                        releaseRenderable               (RenderableHandle renderableHandle) override;

        // Renderable data (stuff required for rendering)
        void                        setRenderableDataInstance       (RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance) override;
        void                        setRenderableStartIndex         (RenderableHandle renderableHandle, UInt32 startIndex) override;
        void                        setRenderableIndexCount         (RenderableHandle renderableHandle, UInt32 indexCount) override;
        void                        setRenderableVisibility         (RenderableHandle renderableHandle, EVisibilityMode visibility) override;
        void                        setRenderableRenderState        (RenderableHandle renderableHandle, RenderStateHandle stateHandle) override;
        void                        setRenderableInstanceCount      (RenderableHandle renderableHandle, UInt32 instanceCount) override;
        void                        setRenderableStartVertex        (RenderableHandle renderableHandle, UInt32 startVertex) override;
        void                                setRenderableUniformsDataInstanceAndState (RenderableHandle renderableHandle, DataInstanceHandle newDataInstance, RenderStateHandle stateHandle);

        // Render state
        RenderStateHandle           allocateRenderState             (RenderStateHandle stateHandle = RenderStateHandle::Invalid()) override;
        void                        releaseRenderState              (RenderStateHandle stateHandle) override;
        void                        setRenderStateBlendFactors      (RenderStateHandle stateHandle, EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha) override;
        void                        setRenderStateBlendOperations   (RenderStateHandle stateHandle, EBlendOperation operationColor, EBlendOperation operationAlpha) override;
        void                        setRenderStateBlendColor        (RenderStateHandle stateHandle, const Vector4& color) override;
        void                        setRenderStateCullMode          (RenderStateHandle stateHandle, ECullMode cullMode) override;
        void                        setRenderStateDrawMode          (RenderStateHandle stateHandle, EDrawMode drawMode) override;
        void                        setRenderStateDepthFunc         (RenderStateHandle stateHandle, EDepthFunc func) override;
        void                        setRenderStateDepthWrite        (RenderStateHandle stateHandle, EDepthWrite flag) override;
        void                        setRenderStateScissorTest       (RenderStateHandle stateHandle, EScissorTest flag, const RenderState::ScissorRegion& region) override;
        void                        setRenderStateStencilFunc       (RenderStateHandle stateHandle, EStencilFunc func, UInt8 ref, UInt8 mask) override;
        void                        setRenderStateStencilOps        (RenderStateHandle stateHandle, EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass) override;
        void                        setRenderStateColorWriteMask    (RenderStateHandle stateHandle, ColorWriteMask colorMask) override;

        // Camera
        CameraHandle                allocateCamera                  (ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle dataInstance, CameraHandle handle = CameraHandle::Invalid()) override;
        void                        releaseCamera                   (CameraHandle cameraHandle) override;

        // Creation/Deletion
        NodeHandle                  allocateNode                    (UInt32 childrenCount = 0u, NodeHandle handle = NodeHandle::Invalid()) override;
        void                        releaseNode                     (NodeHandle nodeHandle) override;

        TransformHandle             allocateTransform               (NodeHandle nodeHandle, TransformHandle handle = TransformHandle::Invalid()) override;
        void                        releaseTransform                (TransformHandle transform) override;

        // Parent-child relationship
        void                        addChildToNode                  (NodeHandle parent, NodeHandle child) override;
        void                        removeChildFromNode             (NodeHandle parent, NodeHandle child) override;

        // Transformation
        void                        setTranslation                  (TransformHandle handle, const Vector3& translation) override;
        void                        setRotation                     (TransformHandle handle, const Vector4& rotation, ERotationConvention convention) override;
        void                        setScaling                      (TransformHandle handle, const Vector3& scaling) override;


        DataLayoutHandle            allocateDataLayout              (const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutHandle handle = DataLayoutHandle::Invalid()) override;
        void                        releaseDataLayout               (DataLayoutHandle layoutHandle) override;

        DataInstanceHandle          allocateDataInstance            (DataLayoutHandle finishedLayoutHandle, DataInstanceHandle instanceHandle = DataInstanceHandle::Invalid()) override;
        void                        releaseDataInstance             (DataInstanceHandle containerHandle) override;

        void                        setDataFloatArray               (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Float* data) override;
        void                        setDataVector2fArray            (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector2* data) override;
        void                        setDataVector3fArray            (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector3* data) override;
        void                        setDataVector4fArray            (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector4* data) override;
        void                        setDataIntegerArray             (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Int32* data) override;
        void                        setDataVector2iArray            (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector2i* data) override;
        void                        setDataVector3iArray            (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector3i* data) override;
        void                        setDataVector4iArray            (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector4i* data) override;
        void                        setDataMatrix22fArray           (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix22f* data) override;
        void                        setDataMatrix33fArray           (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix33f* data) override;
        void                        setDataMatrix44fArray           (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix44f* data) override;
        void                        setDataResource                 (DataInstanceHandle containerHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, UInt32 instancingDivisor, UInt16 offsetWithinElementInBytes, UInt16 stride) override;
        void                        setDataTextureSamplerHandle     (DataInstanceHandle containerHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle) override;
        void                        setDataReference                (DataInstanceHandle containerHandle, DataFieldHandle field, DataInstanceHandle dataRef) override;

        // Texture sampler description
        TextureSamplerHandle        allocateTextureSampler          (const TextureSampler& sampler, TextureSamplerHandle handle = TextureSamplerHandle::Invalid()) override;
        void                        releaseTextureSampler           (TextureSamplerHandle handle) override;

        // Render groups
        RenderGroupHandle           allocateRenderGroup             (UInt32 renderableCount = 0u, UInt32 nestedGroupCount = 0u, RenderGroupHandle groupHandle = RenderGroupHandle::Invalid()) override;
        void                        releaseRenderGroup              (RenderGroupHandle groupHandle) override;
        void                        addRenderableToRenderGroup      (RenderGroupHandle groupHandle, RenderableHandle renderableHandle, Int32 order) override;
        void                        removeRenderableFromRenderGroup (RenderGroupHandle groupHandle, RenderableHandle renderableHandle) override;
        void                        addRenderGroupToRenderGroup     (RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, Int32 order) override;
        void                                removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild) override;

        RenderPassHandle            allocateRenderPass              (UInt32 renderGroupCount = 0u, RenderPassHandle handle = RenderPassHandle::Invalid()) override;
        void                        releaseRenderPass               (RenderPassHandle handle) override;
        void                        setRenderPassClearColor         (RenderPassHandle passHandle, const Vector4& clearColor) override;
        void                        setRenderPassClearFlag          (RenderPassHandle passHandle, UInt32 clearFlag) override;
        void                        setRenderPassCamera             (RenderPassHandle passHandle, CameraHandle cameraHandle) override;
        void                        setRenderPassRenderTarget       (RenderPassHandle passHandle, RenderTargetHandle targetHandle) override;
        void                        setRenderPassRenderOrder        (RenderPassHandle passHandle, Int32 renderOrder) override;
        void                        setRenderPassEnabled            (RenderPassHandle passHandle, bool isEnabled) override;
        void                        setRenderPassRenderOnce         (RenderPassHandle passHandle, bool enable) override;
        void                        retriggerRenderPassRenderOnce   (RenderPassHandle passHandle) override;
        void                        addRenderGroupToRenderPass      (RenderPassHandle passHandle, RenderGroupHandle groupHandle, Int32 order) override;
        void                        removeRenderGroupFromRenderPass (RenderPassHandle passHandle, RenderGroupHandle groupHandle) override;

        BlitPassHandle              allocateBlitPass                (RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle = BlitPassHandle::Invalid()) override;
        void                        releaseBlitPass                 (BlitPassHandle passHandle) override;
        void                        setBlitPassRenderOrder          (BlitPassHandle passHandle, Int32 renderOrder) override;
        void                        setBlitPassEnabled              (BlitPassHandle passHandle, bool isEnabled) override;
        void                        setBlitPassRegions              (BlitPassHandle passHandle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion) override;

        PickableObjectHandle        allocatePickableObject          (DataBufferHandle geometryHandle, NodeHandle nodeHandle, PickableObjectId id, PickableObjectHandle pickableHandle = PickableObjectHandle::Invalid()) override;
        void                        releasePickableObject           (PickableObjectHandle pickableHandle) override;
        void                        setPickableObjectId             (PickableObjectHandle pickableHandle, PickableObjectId id) override;
        void                        setPickableObjectCamera         (PickableObjectHandle pickableHandle, CameraHandle cameraHandle) override;
        void                        setPickableObjectEnabled        (PickableObjectHandle pickableHandle, bool isEnabled) override;

        DataSlotHandle              allocateDataSlot                (const DataSlot& dataSlot, DataSlotHandle handle = DataSlotHandle::Invalid()) override;
        void                        setDataSlotTexture              (DataSlotHandle handle, const ResourceContentHash& texture) override;
        void                        releaseDataSlot                 (DataSlotHandle handle) override;

        // Render targets
        RenderTargetHandle          allocateRenderTarget            (RenderTargetHandle targetHandle = RenderTargetHandle::Invalid()) override;
        void                        releaseRenderTarget             (RenderTargetHandle targetHandle) override;

        // Render buffers
        RenderBufferHandle          allocateRenderBuffer            (const RenderBuffer& renderBuffer, RenderBufferHandle handle = RenderBufferHandle::Invalid()) override;
        void                        releaseRenderBuffer             (RenderBufferHandle handle) override;
        void                        addRenderTargetRenderBuffer     (RenderTargetHandle targetHandle, RenderBufferHandle bufferHandle) override;

        // Data buffers
        DataBufferHandle            allocateDataBuffer              (EDataBufferType dataBufferType, EDataType dataType, UInt32 maximumSizeInBytes, DataBufferHandle handle = DataBufferHandle::Invalid()) override;
        void                        releaseDataBuffer               (DataBufferHandle handle) override;
        void                        updateDataBuffer                (DataBufferHandle handle, UInt32 offsetInBytes, UInt32 dataSizeInBytes, const Byte* data) override;

        TextureBufferHandle         allocateTextureBuffer           (ETextureFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle = TextureBufferHandle::Invalid()) override;
        void                        releaseTextureBuffer            (TextureBufferHandle handle) override;
        void                        updateTextureBuffer             (TextureBufferHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data) override;

        SceneReferenceHandle        allocateSceneReference          (SceneId sceneId, SceneReferenceHandle handle = {}) override;
        void                        releaseSceneReference           (SceneReferenceHandle handle) override;
        void                        requestSceneReferenceState      (SceneReferenceHandle handle, RendererSceneState state) override;
        void                        requestSceneReferenceFlushNotifications(SceneReferenceHandle handle, bool enable) override;
        void                        setSceneReferenceRenderOrder    (SceneReferenceHandle handle, int32_t renderOrder) override;

        [[nodiscard]] const SceneActionCollection& getSceneActionCollection() const;
        SceneActionCollection& getSceneActionCollection();

        void linkData(SceneReferenceHandle providerScene, DataSlotId providerId, SceneReferenceHandle consumerScene, DataSlotId consumerId);
        void unlinkData(SceneReferenceHandle consumerScene, DataSlotId consumerId);

        [[nodiscard]] const SceneReferenceActionVector& getSceneReferenceActions() const;
        void resetSceneReferenceActions();

    private:
        SceneActionCollection m_collection;
        SceneActionCollectionCreator m_creator;
        SceneReferenceActionVector m_sceneReferenceActions;
    };
}

#endif
