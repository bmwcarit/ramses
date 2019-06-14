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

namespace ramses_internal
{
    class ActionCollectingScene : public ResourceChangeCollectingScene
    {
    public:
        explicit ActionCollectingScene(const SceneInfo& sceneInfo = SceneInfo());

        virtual void                        preallocateSceneSize            (const SceneSizeInformation& sizeInfo) override;

        // Renderable allocation
        virtual RenderableHandle            allocateRenderable              (NodeHandle nodeHandle, RenderableHandle handle = RenderableHandle::Invalid()) override;
        virtual void                        releaseRenderable               (RenderableHandle renderableHandle) override;

        // Renderable data (stuff required for rendering)
        virtual void                        setRenderableEffect             (RenderableHandle renderableHandle, const ResourceContentHash& effectHash) override;
        virtual void                        setRenderableDataInstance       (RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance) override;
        virtual void                        setRenderableStartIndex         (RenderableHandle renderableHandle, UInt32 startIndex) override;
        virtual void                        setRenderableIndexCount         (RenderableHandle renderableHandle, UInt32 indexCount) override;
        virtual void                        setRenderableVisibility         (RenderableHandle renderableHandle, Bool visibility) override;
        virtual void                        setRenderableRenderState        (RenderableHandle renderableHandle, RenderStateHandle stateHandle) override;
        virtual void                        setRenderableInstanceCount      (RenderableHandle renderableHandle, UInt32 instanceCount) override;
        void                                setRenderableDataInstanceAndStateAndEffect (RenderableHandle renderableHandle, DataInstanceHandle newDataInstance, RenderStateHandle stateHandle, const ResourceContentHash& effectHash);

        // Render state
        virtual RenderStateHandle           allocateRenderState             (RenderStateHandle stateHandle = RenderStateHandle::Invalid()) override;
        virtual void                        releaseRenderState              (RenderStateHandle stateHandle) override;
        virtual void                        setRenderStateBlendFactors      (RenderStateHandle stateHandle, EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha) override;
        virtual void                        setRenderStateBlendOperations   (RenderStateHandle stateHandle, EBlendOperation operationColor, EBlendOperation operationAlpha) override;
        virtual void                        setRenderStateCullMode          (RenderStateHandle stateHandle, ECullMode cullMode) override;
        virtual void                        setRenderStateDrawMode          (RenderStateHandle stateHandle, EDrawMode drawMode) override;
        virtual void                        setRenderStateDepthFunc         (RenderStateHandle stateHandle, EDepthFunc func) override;
        virtual void                        setRenderStateDepthWrite        (RenderStateHandle stateHandle, EDepthWrite flag) override;
        virtual void                        setRenderStateScissorTest       (RenderStateHandle stateHandle, EScissorTest flag, const RenderState::ScissorRegion& region) override;
        virtual void                        setRenderStateStencilFunc       (RenderStateHandle stateHandle, EStencilFunc func, UInt8 ref, UInt8 mask) override;
        virtual void                        setRenderStateStencilOps        (RenderStateHandle stateHandle, EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass) override;
        virtual void                        setRenderStateColorWriteMask    (RenderStateHandle stateHandle, ColorWriteMask colorMask) override;

        // Camera
        virtual CameraHandle                allocateCamera                  (ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle viewportDataInstance, CameraHandle handle = CameraHandle::Invalid()) override;
        virtual void                        releaseCamera                   (CameraHandle cameraHandle) override;
        virtual void                        setCameraFrustum                (CameraHandle cameraHandle, const Frustum& frustum) override;

        // Creation/Deletion
        virtual NodeHandle                  allocateNode                    (UInt32 childrenCount = 0u, NodeHandle handle = NodeHandle::Invalid()) override;
        virtual void                        releaseNode                     (NodeHandle nodeHandle) override;

        virtual TransformHandle             allocateTransform               (NodeHandle nodeHandle, TransformHandle handle = TransformHandle::Invalid()) override;
        virtual void                        releaseTransform                (TransformHandle transform) override;

        // Parent-child relationship
        virtual void                        addChildToNode                  (NodeHandle parent, NodeHandle child) override;
        virtual void                        removeChildFromNode             (NodeHandle parent, NodeHandle child) override;

        // Transformation
        virtual void                        setTranslation                  (TransformHandle handle, const Vector3& translation) override;
        virtual void                        setRotation                     (TransformHandle handle, const Vector3& rotation) override;
        virtual void                        setScaling                      (TransformHandle handle, const Vector3& scaling) override;


        virtual DataLayoutHandle            allocateDataLayout              (const DataFieldInfoVector& dataFields, DataLayoutHandle handle = DataLayoutHandle::Invalid()) override;
        virtual void                        releaseDataLayout               (DataLayoutHandle layoutHandle) override;

        virtual DataInstanceHandle          allocateDataInstance            (DataLayoutHandle finishedLayoutHandle, DataInstanceHandle instanceHandle = DataInstanceHandle::Invalid()) override;
        virtual void                        releaseDataInstance             (DataInstanceHandle containerHandle) override;

        virtual void                        setDataFloatArray               (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Float* data) override;
        virtual void                        setDataVector2fArray            (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector2* data) override;
        virtual void                        setDataVector3fArray            (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector3* data) override;
        virtual void                        setDataVector4fArray            (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector4* data) override;
        virtual void                        setDataIntegerArray             (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Int32* data) override;
        virtual void                        setDataVector2iArray            (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector2i* data) override;
        virtual void                        setDataVector3iArray            (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector3i* data) override;
        virtual void                        setDataVector4iArray            (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector4i* data) override;
        virtual void                        setDataMatrix22fArray           (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix22f* data) override;
        virtual void                        setDataMatrix33fArray           (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix33f* data) override;
        virtual void                        setDataMatrix44fArray           (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix44f* data) override;
        virtual void                        setDataResource                 (DataInstanceHandle containerHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, UInt32 instancingDivisor) override;
        virtual void                        setDataTextureSamplerHandle     (DataInstanceHandle containerHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle) override;
        virtual void                        setDataReference                (DataInstanceHandle containerHandle, DataFieldHandle field, DataInstanceHandle dataRef) override;

        // Texture sampler description
        virtual TextureSamplerHandle        allocateTextureSampler          (const TextureSampler& sampler, TextureSamplerHandle handle = TextureSamplerHandle::Invalid()) override;
        virtual void                        releaseTextureSampler           (TextureSamplerHandle handle) override;

        virtual AnimationSystemHandle       addAnimationSystem              (IAnimationSystem* animationSystem, AnimationSystemHandle externalHandle = AnimationSystemHandle::Invalid()) override;
        virtual void                        removeAnimationSystem           (AnimationSystemHandle animSystemHandle) override;

        // Render groups
        virtual RenderGroupHandle           allocateRenderGroup             (UInt32 renderableCount = 0u, UInt32 nestedGroupCount = 0u, RenderGroupHandle groupHandle = RenderGroupHandle::Invalid()) override;
        virtual void                        releaseRenderGroup              (RenderGroupHandle groupHandle) override;
        virtual void                        addRenderableToRenderGroup      (RenderGroupHandle groupHandle, RenderableHandle renderableHandle, Int32 order) override;
        virtual void                        removeRenderableFromRenderGroup (RenderGroupHandle groupHandle, RenderableHandle renderableHandle) override;
        virtual void                        addRenderGroupToRenderGroup     (RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, Int32 order) override;
        void                                removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild) override;

        virtual RenderPassHandle            allocateRenderPass              (UInt32 renderGroupCount = 0u, RenderPassHandle handle = RenderPassHandle::Invalid()) override;
        virtual void                        releaseRenderPass               (RenderPassHandle handle) override;
        virtual void                        setRenderPassClearColor         (RenderPassHandle passHandle, const Vector4& clearColor) override;
        virtual void                        setRenderPassClearFlag          (RenderPassHandle passHandle, UInt32 clearFlag) override;
        virtual void                        setRenderPassCamera             (RenderPassHandle passHandle, CameraHandle cameraHandle) override;
        virtual void                        setRenderPassRenderTarget       (RenderPassHandle passHandle, RenderTargetHandle targetHandle) override;
        virtual void                        setRenderPassRenderOrder        (RenderPassHandle passHandle, Int32 renderOrder) override;
        virtual void                        setRenderPassEnabled            (RenderPassHandle passHandle, Bool isEnabled) override;
        virtual void                        setRenderPassRenderOnce         (RenderPassHandle passHandle, Bool enable) override;
        virtual void                        retriggerRenderPassRenderOnce   (RenderPassHandle passHandle) override;
        virtual void                        addRenderGroupToRenderPass      (RenderPassHandle passHandle, RenderGroupHandle groupHandle, Int32 order) override;
        virtual void                        removeRenderGroupFromRenderPass (RenderPassHandle passHandle, RenderGroupHandle groupHandle) override;

        virtual BlitPassHandle              allocateBlitPass                (RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle = BlitPassHandle::Invalid()) override;
        virtual void                        releaseBlitPass                 (BlitPassHandle passHandle) override;
        virtual void                        setBlitPassRenderOrder          (BlitPassHandle passHandle, Int32 renderOrder) override;
        virtual void                        setBlitPassEnabled              (BlitPassHandle passHandle, Bool isEnabled) override;
        virtual void                        setBlitPassRegions              (BlitPassHandle passHandle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion) override;

        virtual DataSlotHandle              allocateDataSlot                (const DataSlot& dataSlot, DataSlotHandle handle = DataSlotHandle::Invalid()) override;
        virtual void                        setDataSlotTexture              (DataSlotHandle handle, const ResourceContentHash& texture) override;
        virtual void                        releaseDataSlot                 (DataSlotHandle handle) override;

        // Render targets
        virtual RenderTargetHandle          allocateRenderTarget            (RenderTargetHandle targetHandle = RenderTargetHandle::Invalid()) override;
        virtual void                        releaseRenderTarget             (RenderTargetHandle targetHandle) override;

        // Render buffers
        virtual RenderBufferHandle          allocateRenderBuffer            (const RenderBuffer& renderBuffer, RenderBufferHandle handle = RenderBufferHandle::Invalid()) override;
        virtual void                        releaseRenderBuffer             (RenderBufferHandle handle) override;
        virtual void                        addRenderTargetRenderBuffer     (RenderTargetHandle targetHandle, RenderBufferHandle bufferHandle) override;

        // Stream textures
        virtual StreamTextureHandle         allocateStreamTexture           (uint32_t streamSource, const ResourceContentHash& fallbackTextureHash, StreamTextureHandle streamTextureHandle = StreamTextureHandle::Invalid()) override;
        virtual void                        releaseStreamTexture            (StreamTextureHandle streamTextureHandle) override;
        virtual void                        setForceFallbackImage           (StreamTextureHandle streamTextureHandle, Bool forceFallbackImage) override;

        // Data buffers
        virtual DataBufferHandle            allocateDataBuffer              (EDataBufferType dataBufferType, EDataType dataType, UInt32 maximumSizeInBytes, DataBufferHandle handle = DataBufferHandle::Invalid()) override;
        virtual void                        releaseDataBuffer               (DataBufferHandle handle) override;
        virtual void                        updateDataBuffer                (DataBufferHandle handle, UInt32 offsetInBytes, UInt32 dataSizeInBytes, const Byte* data) override;

        virtual TextureBufferHandle         allocateTextureBuffer           (ETextureFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle = TextureBufferHandle::Invalid()) override;
        virtual void                        releaseTextureBuffer            (TextureBufferHandle handle) override;
        virtual void                        updateTextureBuffer             (TextureBufferHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data) override;

        const SceneActionCollection& getSceneActionCollection() const;
        SceneActionCollection& getSceneActionCollection();

    private:
        SceneActionCollection m_collection;
        SceneActionCollectionCreator m_creator;
    };
}

#endif
