//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ACTIONTESTSCENE_H
#define RAMSES_ACTIONTESTSCENE_H

#include "framework_common_gmock_header.h"
#include "Scene/ActionCollectingScene.h"
#include "Scene/SceneActionApplierHelper.h"

namespace ramses_internal
{
    // This test facade tests multiple things:
    // It tests that scene actions are created symmetrically
    //      -> Multiplexing scene creates scene actions (forward direction)
    //      -> SceneActionApplier reads scene actions and applies them to a scene (reverse direction)
    // It also indirectly tests MultiplexingScene
    // It also indirectly tests SceneActionHelper
    // It also indirectly tests StandardScene
    class ActionTestScene : public IScene
    {
    public:
        explicit ActionTestScene(const SceneInfo& sceneInfo = SceneInfo());

        virtual const String&               getName                         () const override;
        virtual SceneId                     getSceneId                      () const override;

        // Renderable
        virtual RenderableHandle            allocateRenderable              (NodeHandle nodeHandle, RenderableHandle handle = RenderableHandle::Invalid()) override;
        virtual void                        releaseRenderable               (RenderableHandle renderableHandle) override;
        virtual bool                        isRenderableAllocated           (RenderableHandle renderableHandle) const override;
        virtual UInt32                      getRenderableCount              () const override;
        virtual void                        setRenderableDataInstance       (RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance) override;
        virtual void                        setRenderableStartIndex         (RenderableHandle renderableHandle, UInt32 startIndex) override;
        virtual void                        setRenderableIndexCount         (RenderableHandle renderableHandle, UInt32 indexCount) override;
        virtual void                        setRenderableRenderState        (RenderableHandle renderableHandle, RenderStateHandle stateHandle) override;
        virtual void                        setRenderableVisibility         (RenderableHandle renderableHandle, EVisibilityMode visible) override;
        virtual void                        setRenderableInstanceCount      (RenderableHandle renderableHandle, UInt32 instanceCount) override;
        virtual void                        setRenderableStartVertex        (RenderableHandle renderableHandle, UInt32 startVertex) override;
        virtual const Renderable&           getRenderable                   (RenderableHandle renderableHandle) const override;

        // Render state
        virtual RenderStateHandle           allocateRenderState             (RenderStateHandle stateHandle = RenderStateHandle::Invalid()) override;
        virtual void                        releaseRenderState              (RenderStateHandle stateHandle) override;
        virtual bool                        isRenderStateAllocated          (RenderStateHandle stateHandle) const override;
        virtual UInt32                      getRenderStateCount             () const override;
        virtual void                        setRenderStateBlendFactors            (RenderStateHandle stateHandle, EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha) override;
        virtual void                        setRenderStateBlendOperations         (RenderStateHandle stateHandle, EBlendOperation operationColor, EBlendOperation operationAlpha) override;
        virtual void                        setRenderStateBlendColor              (RenderStateHandle stateHandle, const Vector4& color) override;
        virtual void                        setRenderStateCullMode                (RenderStateHandle stateHandle, ECullMode cullMode) override;
        virtual void                        setRenderStateDrawMode                (RenderStateHandle stateHandle, EDrawMode drawMode) override;
        virtual void                        setRenderStateDepthFunc               (RenderStateHandle stateHandle, EDepthFunc func) override;
        virtual void                        setRenderStateDepthWrite              (RenderStateHandle stateHandle, EDepthWrite flag) override;
        virtual void                        setRenderStateScissorTest             (RenderStateHandle stateHandle, EScissorTest flag, const RenderState::ScissorRegion& region) override;
        virtual void                        setRenderStateStencilFunc             (RenderStateHandle stateHandle, EStencilFunc func, UInt8 ref, UInt8 mask) override;
        virtual void                        setRenderStateStencilOps              (RenderStateHandle stateHandle, EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass) override;
        virtual void                        setRenderStateColorWriteMask          (RenderStateHandle stateHandle, ColorWriteMask colorMask) override;
        virtual const RenderState&          getRenderState                        (RenderStateHandle stateHandle) const override;

        // Camera
        virtual CameraHandle                allocateCamera                  (ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle dataInstance, CameraHandle handle = CameraHandle::Invalid()) override;
        virtual void                        releaseCamera                   (CameraHandle cameraHandle) override;
        virtual bool                        isCameraAllocated               (CameraHandle handle) const override;
        virtual UInt32                      getCameraCount                  () const override;
        virtual const Camera&               getCamera                       (CameraHandle cameraHandle) const override;

        // Creation/Deletion
        virtual NodeHandle                  allocateNode                    (UInt32 childrenCount = 0u, NodeHandle handle = NodeHandle::Invalid()) override;
        virtual void                        releaseNode                     (NodeHandle nodeHandle) override;
        virtual bool                        isNodeAllocated                 (NodeHandle node) const override;
        virtual UInt32                      getNodeCount                    () const override;

        virtual TransformHandle             allocateTransform               (NodeHandle nodeHandle, TransformHandle handle = TransformHandle::Invalid()) override;
        virtual void                        releaseTransform                (TransformHandle transform) override;
        virtual bool                        isTransformAllocated            (TransformHandle transformHandle) const override;
        virtual UInt32                      getTransformCount               () const override;
        virtual NodeHandle                  getTransformNode                (TransformHandle handle) const override;

        // Parent-child relationship
        virtual NodeHandle                  getParent                       (NodeHandle nodeHandle) const override;
        virtual void                        addChildToNode                  (NodeHandle parent, NodeHandle child) override;
        virtual void                        removeChildFromNode             (NodeHandle parent, NodeHandle child) override;
        virtual UInt32                      getChildCount                   (NodeHandle parent) const override;
        virtual NodeHandle                  getChild                        (NodeHandle parent, UInt32 childNumber) const override;

        // Transformation
        virtual const Vector3&              getTranslation                  (TransformHandle handle) const override;
        virtual const Vector3&              getRotation                     (TransformHandle handle) const override;
        virtual ERotationConvention         getRotationConvention           (TransformHandle handle) const override;
        virtual const Vector3&              getScaling                      (TransformHandle handle) const override;
        virtual void                        setTranslation                  (TransformHandle handle, const Vector3& translation) override;
        virtual void                        setRotation                     (TransformHandle handle, const Vector3& rotation, ERotationConvention convention) override;
        virtual void                        setRotationForAnimation         (TransformHandle handle, const Vector3& rotation) override;
        virtual void                        setScaling                      (TransformHandle handle, const Vector3& scaling) override;

        virtual DataLayoutHandle            allocateDataLayout              (const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutHandle handle = DataLayoutHandle::Invalid()) override;
        virtual void                        releaseDataLayout               (DataLayoutHandle layoutHandle) override;
        virtual bool                        isDataLayoutAllocated           (DataLayoutHandle layoutHandle) const override;
        virtual UInt32                      getDataLayoutCount              () const override;

        virtual const DataLayout&           getDataLayout                   (DataLayoutHandle layoutHandle) const override;

        virtual DataInstanceHandle          allocateDataInstance            (DataLayoutHandle finishedLayoutHandle, DataInstanceHandle instanceHandle = DataInstanceHandle::Invalid()) override;
        virtual void                        releaseDataInstance             (DataInstanceHandle containerHandle) override;
        virtual bool                        isDataInstanceAllocated         (DataInstanceHandle containerHandle) const override;
        virtual UInt32                      getDataInstanceCount            () const override;
        virtual DataLayoutHandle            getLayoutOfDataInstance         (DataInstanceHandle containerHandle) const override;

        virtual const Float*                getDataFloatArray               (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual const Vector2*              getDataVector2fArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual const Vector3*              getDataVector3fArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual const Vector4*              getDataVector4fArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual const Int32*                getDataIntegerArray             (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual const Matrix22f*            getDataMatrix22fArray           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual const Matrix33f*            getDataMatrix33fArray           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual const Matrix44f*            getDataMatrix44fArray           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual const Vector2i*             getDataVector2iArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual const Vector3i*             getDataVector3iArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual const Vector4i*             getDataVector4iArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual const ResourceField&        getDataResource                 (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual TextureSamplerHandle        getDataTextureSamplerHandle     (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual DataInstanceHandle          getDataReference                (DataInstanceHandle containerHandle, DataFieldHandle field) const override;

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
        virtual void                        setDataResource                 (DataInstanceHandle containerHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, UInt32 instancingDivisor, UInt16 offsetWithinElementInBytes, UInt16 stride) override;
        virtual void                        setDataTextureSamplerHandle     (DataInstanceHandle containerHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle) override;
        virtual void                        setDataReference                (DataInstanceHandle containerHandle, DataFieldHandle field, DataInstanceHandle dataRef) override;

        // get/setData*Array wrappers for elementCount == 1
        virtual Float                       getDataSingleFloat              (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual const Vector2&              getDataSingleVector2f           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual const Vector3&              getDataSingleVector3f           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual const Vector4&              getDataSingleVector4f           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual Int32                       getDataSingleInteger            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual const Matrix22f&            getDataSingleMatrix22f          (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual const Matrix33f&            getDataSingleMatrix33f          (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual const Matrix44f&            getDataSingleMatrix44f          (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual const Vector2i&             getDataSingleVector2i           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual const Vector3i&             getDataSingleVector3i           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        virtual const Vector4i&             getDataSingleVector4i           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;

        virtual void                        setDataSingleFloat              (DataInstanceHandle containerHandle, DataFieldHandle field, Float data) override;
        virtual void                        setDataSingleVector2f           (DataInstanceHandle containerHandle, DataFieldHandle field, const Vector2& data) override;
        virtual void                        setDataSingleVector3f           (DataInstanceHandle containerHandle, DataFieldHandle field, const Vector3& data) override;
        virtual void                        setDataSingleVector4f           (DataInstanceHandle containerHandle, DataFieldHandle field, const Vector4& data) override;
        virtual void                        setDataSingleInteger            (DataInstanceHandle containerHandle, DataFieldHandle field, Int32 data) override;
        virtual void                        setDataSingleVector2i           (DataInstanceHandle containerHandle, DataFieldHandle field, const Vector2i& data) override;
        virtual void                        setDataSingleVector3i           (DataInstanceHandle containerHandle, DataFieldHandle field, const Vector3i& data) override;
        virtual void                        setDataSingleVector4i           (DataInstanceHandle containerHandle, DataFieldHandle field, const Vector4i& data) override;
        virtual void                        setDataSingleMatrix22f          (DataInstanceHandle containerHandle, DataFieldHandle field, const Matrix22f& data) override;
        virtual void                        setDataSingleMatrix33f          (DataInstanceHandle containerHandle, DataFieldHandle field, const Matrix33f& data) override;
        virtual void                        setDataSingleMatrix44f          (DataInstanceHandle containerHandle, DataFieldHandle field, const Matrix44f& data) override;

        // Texture sampler description
        virtual TextureSamplerHandle        allocateTextureSampler          (const TextureSampler& sampler, TextureSamplerHandle handle = TextureSamplerHandle::Invalid()) override;
        virtual void                        releaseTextureSampler           (TextureSamplerHandle handle) override;
        virtual bool                        isTextureSamplerAllocated       (TextureSamplerHandle handle) const override;
        virtual UInt32                      getTextureSamplerCount          () const override;
        virtual const TextureSampler&       getTextureSampler               (TextureSamplerHandle handle) const override;

        // Render groups
        virtual RenderGroupHandle           allocateRenderGroup             (UInt32 renderableCount = 0u, UInt32 nestedGroupCount = 0u, RenderGroupHandle groupHandle = RenderGroupHandle::Invalid()) override;
        virtual void                        releaseRenderGroup              (RenderGroupHandle groupHandle) override;
        virtual bool                        isRenderGroupAllocated          (RenderGroupHandle groupHandle) const override;
        virtual UInt32                      getRenderGroupCount             () const override;
        virtual void                        addRenderableToRenderGroup      (RenderGroupHandle groupHandle, RenderableHandle renderableHandle, Int32 order) override;
        virtual void                        removeRenderableFromRenderGroup (RenderGroupHandle groupHandle, RenderableHandle renderableHandle) override;
        virtual void                        addRenderGroupToRenderGroup     (RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, Int32 order) override;
        virtual void                        removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild) override;
        virtual const RenderGroup&          getRenderGroup                  (RenderGroupHandle groupHandle) const override;

        // Render passes
        virtual RenderPassHandle            allocateRenderPass              (UInt32 renderGroupCount = 0u, RenderPassHandle passHandle = RenderPassHandle::Invalid()) override;
        virtual void                        releaseRenderPass               (RenderPassHandle passHandle) override;
        virtual bool                        isRenderPassAllocated           (RenderPassHandle passHandle) const override;
        virtual UInt32                      getRenderPassCount              () const override;
        virtual void                        setRenderPassClearColor         (RenderPassHandle passHandle, const Vector4& clearColor) override;
        virtual void                        setRenderPassClearFlag          (RenderPassHandle passHandle, UInt32 clearFlag) override;
        virtual void                        setRenderPassCamera             (RenderPassHandle passHandle, CameraHandle cameraHandle) override;
        virtual void                        setRenderPassRenderTarget       (RenderPassHandle passHandle, RenderTargetHandle targetHandle) override;
        virtual void                        setRenderPassRenderOrder        (RenderPassHandle passHandle, Int32 renderOrder) override;
        virtual void                        setRenderPassEnabled            (RenderPassHandle passHandle, bool isEnabled) override;
        virtual void                        setRenderPassRenderOnce         (RenderPassHandle passHandle, bool enable) override;
        virtual void                        retriggerRenderPassRenderOnce   (RenderPassHandle passHandle) override;
        virtual void                        addRenderGroupToRenderPass      (RenderPassHandle passHandle, RenderGroupHandle groupHandle, Int32 order) override;
        virtual void                        removeRenderGroupFromRenderPass (RenderPassHandle passHandle, RenderGroupHandle groupHandle) override;
        virtual const RenderPass&           getRenderPass                   (RenderPassHandle passHandle) const override;

        //Blit passes
        virtual BlitPassHandle              allocateBlitPass                (RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle = BlitPassHandle::Invalid()) override;
        virtual void                        releaseBlitPass                 (BlitPassHandle passHandle) override;
        virtual bool                        isBlitPassAllocated             (BlitPassHandle passHandle) const override;
        virtual UInt32                      getBlitPassCount                () const override;
        virtual void                        setBlitPassRenderOrder          (BlitPassHandle passHandle, Int32 renderOrder) override;
        virtual void                        setBlitPassEnabled              (BlitPassHandle passHandle, bool isEnabled) override;
        virtual void                        setBlitPassRegions              (BlitPassHandle passHandle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion) override;
        virtual const BlitPass&             getBlitPass                     (BlitPassHandle passHandle) const override;

        //Pickable object
        virtual PickableObjectHandle        allocatePickableObject          (DataBufferHandle geometryHandle, NodeHandle nodeHandle, PickableObjectId id, PickableObjectHandle pickableHandle = PickableObjectHandle::Invalid()) override;
        virtual void                        releasePickableObject           (PickableObjectHandle pickableHandle) override;
        virtual bool                        isPickableObjectAllocated       (PickableObjectHandle pickableHandle) const override final;
        virtual UInt32                      getPickableObjectCount          () const override final;
        virtual void                        setPickableObjectId             (PickableObjectHandle pickableHandle, PickableObjectId id) override;
        virtual void                        setPickableObjectCamera         (PickableObjectHandle pickableHandle, CameraHandle cameraHandle) override;
        virtual void                        setPickableObjectEnabled        (PickableObjectHandle pickableHandle, bool isEnabled) override;
        virtual const PickableObject&       getPickableObject               (PickableObjectHandle pickableHandle) const override;

        // Render targets
        virtual RenderTargetHandle          allocateRenderTarget            (RenderTargetHandle targetHandle = RenderTargetHandle::Invalid()) override;
        virtual void                        releaseRenderTarget             (RenderTargetHandle targetHandle) override;
        virtual bool                        isRenderTargetAllocated         (RenderTargetHandle targetHandle) const override;
        virtual UInt32                      getRenderTargetCount            () const  override;
        virtual void                        addRenderTargetRenderBuffer     (RenderTargetHandle targetHandle, RenderBufferHandle bufferHandle) override;
        virtual UInt32                      getRenderTargetRenderBufferCount(RenderTargetHandle targetHandle) const override;
        virtual RenderBufferHandle          getRenderTargetRenderBuffer     (RenderTargetHandle targetHandle, UInt32 bufferIndex) const override;

        // Render buffers
        virtual RenderBufferHandle          allocateRenderBuffer            (const RenderBuffer& renderBuffer, RenderBufferHandle handle = RenderBufferHandle::Invalid()) override;
        virtual void                        releaseRenderBuffer             (RenderBufferHandle handle) override;
        virtual bool                        isRenderBufferAllocated         (RenderBufferHandle handle) const override;
        virtual UInt32                      getRenderBufferCount            () const override;
        virtual const RenderBuffer&         getRenderBuffer                 (RenderBufferHandle handle) const override;

        // stream texture
        virtual StreamTextureHandle         allocateStreamTexture           (WaylandIviSurfaceId streamSource, const ResourceContentHash& fallbackTextureHash, StreamTextureHandle streamTextureHandle = StreamTextureHandle::Invalid()) override;
        virtual void                        releaseStreamTexture            (StreamTextureHandle streamTextureHandle) override;
        virtual bool                        isStreamTextureAllocated        (StreamTextureHandle streamTextureHandle) const override;
        virtual UInt32                      getStreamTextureCount           () const override;
        virtual void                        setForceFallbackImage           (StreamTextureHandle streamTextureHandle, bool forceFallbackImage) override;
        virtual const StreamTexture&        getStreamTexture                (StreamTextureHandle streamTextureHandle) const override;

        // Data buffers
        virtual DataBufferHandle            allocateDataBuffer              (EDataBufferType dataBufferType, EDataType dataType, UInt32 maximumSizeInBytes, DataBufferHandle handle = DataBufferHandle::Invalid()) override;
        virtual void                        releaseDataBuffer               (DataBufferHandle handle) override;
        virtual UInt32                      getDataBufferCount              () const override;
        virtual void                        updateDataBuffer                (DataBufferHandle handle, UInt32 offsetInBytes, UInt32 dataSizeInBytes, const Byte* data) override;
        virtual bool                        isDataBufferAllocated           (DataBufferHandle handle) const override;
        virtual const GeometryDataBuffer&   getDataBuffer                   (DataBufferHandle handle) const override;

        //Texture buffers
        virtual TextureBufferHandle         allocateTextureBuffer           (ETextureFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle = TextureBufferHandle::Invalid()) override;
        virtual void                        releaseTextureBuffer            (TextureBufferHandle handle) override;
        virtual bool                        isTextureBufferAllocated        (TextureBufferHandle handle) const override;
        virtual UInt32                      getTextureBufferCount           () const override;
        virtual void                        updateTextureBuffer             (TextureBufferHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data) override;
        virtual const TextureBuffer&        getTextureBuffer                (TextureBufferHandle handle) const override;

        virtual DataSlotHandle              allocateDataSlot                (const DataSlot& dataSlot, DataSlotHandle handle = DataSlotHandle::Invalid()) override;
        virtual void                        setDataSlotTexture              (DataSlotHandle handle, const ResourceContentHash& texture) override;
        virtual void                        releaseDataSlot                 (DataSlotHandle handle) override;
        virtual bool                        isDataSlotAllocated             (DataSlotHandle handle) const override;
        virtual UInt32                      getDataSlotCount                () const override;
        virtual const DataSlot&             getDataSlot                     (DataSlotHandle handle) const override;

        virtual SceneReferenceHandle        allocateSceneReference          (SceneId sceneId, SceneReferenceHandle handle = {}) override;
        virtual void                        releaseSceneReference           (SceneReferenceHandle handle) override;
        virtual void                        requestSceneReferenceState      (SceneReferenceHandle handle, RendererSceneState state) override;
        virtual void                        requestSceneReferenceFlushNotifications(SceneReferenceHandle handle, bool enable) override;
        virtual void                        setSceneReferenceRenderOrder    (SceneReferenceHandle handle, int32_t renderOrder) override;
        virtual bool                        isSceneReferenceAllocated       (SceneReferenceHandle handle) const override final;
        virtual UInt32                      getSceneReferenceCount          () const override final;
        virtual const SceneReference&       getSceneReference               (SceneReferenceHandle handle) const override final;
        //Animation system
        virtual AnimationSystemHandle       addAnimationSystem              (IAnimationSystem* animationSystem, AnimationSystemHandle animSystemHandle = AnimationSystemHandle::Invalid()) override;
        virtual void                        removeAnimationSystem           (AnimationSystemHandle animSystemHandle) override;
        virtual IAnimationSystem*           getAnimationSystem              (AnimationSystemHandle animSystemHandle) override;
        virtual const IAnimationSystem*     getAnimationSystem              (AnimationSystemHandle animSystemHandle) const override;
        virtual bool                        isAnimationSystemAllocated      (AnimationSystemHandle animSystemHandle) const override;
        virtual UInt32                      getAnimationSystemCount         () const override;

        void flushPendingSceneActions();

        virtual SceneSizeInformation getSceneSizeInformation() const override;
        virtual void preallocateSceneSize(const SceneSizeInformation& sizeInfo) override;

    private:
        // Internal scene which holds the actual scene content; all getters are redirected to m_scene
        const Scene m_scene;
        // Redirects scene actions to m_scene
        SceneActionApplierHelper m_actionApplier;
        // Converts IScene calls to actions, collects them and applies to m_actionApplier, which applies them on m_scene
        ActionCollectingScene m_actionCollector;
    };

}

#endif
