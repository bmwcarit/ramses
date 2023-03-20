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

        [[nodiscard]] virtual const String&               getName                         () const override;
        [[nodiscard]] virtual SceneId                     getSceneId                      () const override;

        virtual void   setEffectTimeSync(FlushTime::Clock::time_point t) override;
        [[nodiscard]] virtual FlushTime::Clock::time_point getEffectTimeSync() const override;

        // Renderable
        virtual RenderableHandle            allocateRenderable              (NodeHandle nodeHandle, RenderableHandle handle = RenderableHandle::Invalid()) override;
        virtual void                        releaseRenderable               (RenderableHandle renderableHandle) override;
        [[nodiscard]] virtual bool                        isRenderableAllocated           (RenderableHandle renderableHandle) const override;
        [[nodiscard]] virtual UInt32                      getRenderableCount              () const override;
        virtual void                        setRenderableDataInstance       (RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance) override;
        virtual void                        setRenderableStartIndex         (RenderableHandle renderableHandle, UInt32 startIndex) override;
        virtual void                        setRenderableIndexCount         (RenderableHandle renderableHandle, UInt32 indexCount) override;
        virtual void                        setRenderableRenderState        (RenderableHandle renderableHandle, RenderStateHandle stateHandle) override;
        virtual void                        setRenderableVisibility         (RenderableHandle renderableHandle, EVisibilityMode visible) override;
        virtual void                        setRenderableInstanceCount      (RenderableHandle renderableHandle, UInt32 instanceCount) override;
        virtual void                        setRenderableStartVertex        (RenderableHandle renderableHandle, UInt32 startVertex) override;
        [[nodiscard]] virtual const Renderable&           getRenderable                   (RenderableHandle renderableHandle) const override;

        // Render state
        virtual RenderStateHandle           allocateRenderState             (RenderStateHandle stateHandle = RenderStateHandle::Invalid()) override;
        virtual void                        releaseRenderState              (RenderStateHandle stateHandle) override;
        [[nodiscard]] virtual bool                        isRenderStateAllocated          (RenderStateHandle stateHandle) const override;
        [[nodiscard]] virtual UInt32                      getRenderStateCount             () const override;
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
        [[nodiscard]] virtual const RenderState&          getRenderState                        (RenderStateHandle stateHandle) const override;

        // Camera
        virtual CameraHandle                allocateCamera                  (ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle dataInstance, CameraHandle handle = CameraHandle::Invalid()) override;
        virtual void                        releaseCamera                   (CameraHandle cameraHandle) override;
        [[nodiscard]] virtual bool                        isCameraAllocated               (CameraHandle handle) const override;
        [[nodiscard]] virtual UInt32                      getCameraCount                  () const override;
        [[nodiscard]] virtual const Camera&               getCamera                       (CameraHandle cameraHandle) const override;

        // Creation/Deletion
        virtual NodeHandle                  allocateNode                    (UInt32 childrenCount = 0u, NodeHandle handle = NodeHandle::Invalid()) override;
        virtual void                        releaseNode                     (NodeHandle nodeHandle) override;
        [[nodiscard]] virtual bool                        isNodeAllocated                 (NodeHandle node) const override;
        [[nodiscard]] virtual UInt32                      getNodeCount                    () const override;

        virtual TransformHandle             allocateTransform               (NodeHandle nodeHandle, TransformHandle handle = TransformHandle::Invalid()) override;
        virtual void                        releaseTransform                (TransformHandle transform) override;
        [[nodiscard]] virtual bool                        isTransformAllocated            (TransformHandle transformHandle) const override;
        [[nodiscard]] virtual UInt32                      getTransformCount               () const override;
        [[nodiscard]] virtual NodeHandle                  getTransformNode                (TransformHandle handle) const override;

        // Parent-child relationship
        [[nodiscard]] virtual NodeHandle                  getParent                       (NodeHandle nodeHandle) const override;
        virtual void                        addChildToNode                  (NodeHandle parent, NodeHandle child) override;
        virtual void                        removeChildFromNode             (NodeHandle parent, NodeHandle child) override;
        [[nodiscard]] virtual UInt32                      getChildCount                   (NodeHandle parent) const override;
        [[nodiscard]] virtual NodeHandle                  getChild                        (NodeHandle parent, UInt32 childNumber) const override;

        // Transformation
        [[nodiscard]] virtual const Vector3&              getTranslation                  (TransformHandle handle) const override;
        [[nodiscard]] virtual const Vector3&              getRotation                     (TransformHandle handle) const override;
        [[nodiscard]] virtual ERotationConvention         getRotationConvention           (TransformHandle handle) const override;
        [[nodiscard]] virtual const Vector3&              getScaling                      (TransformHandle handle) const override;
        virtual void                        setTranslation                  (TransformHandle handle, const Vector3& translation) override;
        virtual void                        setRotation                     (TransformHandle handle, const Vector3& rotation, ERotationConvention convention) override;
        virtual void                        setRotationForAnimation         (TransformHandle handle, const Vector3& rotation) override;
        virtual void                        setScaling                      (TransformHandle handle, const Vector3& scaling) override;

        virtual DataLayoutHandle            allocateDataLayout              (const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutHandle handle = DataLayoutHandle::Invalid()) override;
        virtual void                        releaseDataLayout               (DataLayoutHandle layoutHandle) override;
        [[nodiscard]] virtual bool                        isDataLayoutAllocated           (DataLayoutHandle layoutHandle) const override;
        [[nodiscard]] virtual UInt32                      getDataLayoutCount              () const override;

        [[nodiscard]] virtual const DataLayout&           getDataLayout                   (DataLayoutHandle layoutHandle) const override;

        virtual DataInstanceHandle          allocateDataInstance            (DataLayoutHandle finishedLayoutHandle, DataInstanceHandle instanceHandle = DataInstanceHandle::Invalid()) override;
        virtual void                        releaseDataInstance             (DataInstanceHandle containerHandle) override;
        [[nodiscard]] virtual bool                        isDataInstanceAllocated         (DataInstanceHandle containerHandle) const override;
        [[nodiscard]] virtual UInt32                      getDataInstanceCount            () const override;
        [[nodiscard]] virtual DataLayoutHandle            getLayoutOfDataInstance         (DataInstanceHandle containerHandle) const override;

        [[nodiscard]] virtual const Float*                getDataFloatArray               (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual const Vector2*              getDataVector2fArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual const Vector3*              getDataVector3fArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual const Vector4*              getDataVector4fArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual const Int32*                getDataIntegerArray             (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual const Matrix22f*            getDataMatrix22fArray           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual const Matrix33f*            getDataMatrix33fArray           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual const Matrix44f*            getDataMatrix44fArray           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual const Vector2i*             getDataVector2iArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual const Vector3i*             getDataVector3iArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual const Vector4i*             getDataVector4iArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual const ResourceField&        getDataResource                 (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual TextureSamplerHandle        getDataTextureSamplerHandle     (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual DataInstanceHandle          getDataReference                (DataInstanceHandle containerHandle, DataFieldHandle field) const override;

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
        [[nodiscard]] virtual Float                       getDataSingleFloat              (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual const Vector2&              getDataSingleVector2f           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual const Vector3&              getDataSingleVector3f           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual const Vector4&              getDataSingleVector4f           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual Int32                       getDataSingleInteger            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual const Matrix22f&            getDataSingleMatrix22f          (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual const Matrix33f&            getDataSingleMatrix33f          (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual const Matrix44f&            getDataSingleMatrix44f          (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual const Vector2i&             getDataSingleVector2i           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual const Vector3i&             getDataSingleVector3i           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] virtual const Vector4i&             getDataSingleVector4i           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;

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
        [[nodiscard]] virtual bool                        isTextureSamplerAllocated       (TextureSamplerHandle handle) const override;
        [[nodiscard]] virtual UInt32                      getTextureSamplerCount          () const override;
        [[nodiscard]] virtual const TextureSampler&       getTextureSampler               (TextureSamplerHandle handle) const override;

        // Render groups
        virtual RenderGroupHandle           allocateRenderGroup             (UInt32 renderableCount = 0u, UInt32 nestedGroupCount = 0u, RenderGroupHandle groupHandle = RenderGroupHandle::Invalid()) override;
        virtual void                        releaseRenderGroup              (RenderGroupHandle groupHandle) override;
        [[nodiscard]] virtual bool                        isRenderGroupAllocated          (RenderGroupHandle groupHandle) const override;
        [[nodiscard]] virtual UInt32                      getRenderGroupCount             () const override;
        virtual void                        addRenderableToRenderGroup      (RenderGroupHandle groupHandle, RenderableHandle renderableHandle, Int32 order) override;
        virtual void                        removeRenderableFromRenderGroup (RenderGroupHandle groupHandle, RenderableHandle renderableHandle) override;
        virtual void                        addRenderGroupToRenderGroup     (RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, Int32 order) override;
        virtual void                        removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild) override;
        [[nodiscard]] virtual const RenderGroup&          getRenderGroup                  (RenderGroupHandle groupHandle) const override;

        // Render passes
        virtual RenderPassHandle            allocateRenderPass              (UInt32 renderGroupCount = 0u, RenderPassHandle passHandle = RenderPassHandle::Invalid()) override;
        virtual void                        releaseRenderPass               (RenderPassHandle passHandle) override;
        [[nodiscard]] virtual bool                        isRenderPassAllocated           (RenderPassHandle passHandle) const override;
        [[nodiscard]] virtual UInt32                      getRenderPassCount              () const override;
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
        [[nodiscard]] virtual const RenderPass&           getRenderPass                   (RenderPassHandle passHandle) const override;

        //Blit passes
        virtual BlitPassHandle              allocateBlitPass                (RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle = BlitPassHandle::Invalid()) override;
        virtual void                        releaseBlitPass                 (BlitPassHandle passHandle) override;
        [[nodiscard]] virtual bool                        isBlitPassAllocated             (BlitPassHandle passHandle) const override;
        [[nodiscard]] virtual UInt32                      getBlitPassCount                () const override;
        virtual void                        setBlitPassRenderOrder          (BlitPassHandle passHandle, Int32 renderOrder) override;
        virtual void                        setBlitPassEnabled              (BlitPassHandle passHandle, bool isEnabled) override;
        virtual void                        setBlitPassRegions              (BlitPassHandle passHandle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion) override;
        [[nodiscard]] virtual const BlitPass&             getBlitPass                     (BlitPassHandle passHandle) const override;

        //Pickable object
        virtual PickableObjectHandle        allocatePickableObject          (DataBufferHandle geometryHandle, NodeHandle nodeHandle, PickableObjectId id, PickableObjectHandle pickableHandle = PickableObjectHandle::Invalid()) override;
        virtual void                        releasePickableObject           (PickableObjectHandle pickableHandle) override;
        [[nodiscard]] virtual bool                        isPickableObjectAllocated       (PickableObjectHandle pickableHandle) const override final;
        [[nodiscard]] virtual UInt32                      getPickableObjectCount          () const override final;
        virtual void                        setPickableObjectId             (PickableObjectHandle pickableHandle, PickableObjectId id) override;
        virtual void                        setPickableObjectCamera         (PickableObjectHandle pickableHandle, CameraHandle cameraHandle) override;
        virtual void                        setPickableObjectEnabled        (PickableObjectHandle pickableHandle, bool isEnabled) override;
        [[nodiscard]] virtual const PickableObject&       getPickableObject               (PickableObjectHandle pickableHandle) const override;

        // Render targets
        virtual RenderTargetHandle          allocateRenderTarget            (RenderTargetHandle targetHandle = RenderTargetHandle::Invalid()) override;
        virtual void                        releaseRenderTarget             (RenderTargetHandle targetHandle) override;
        [[nodiscard]] virtual bool                        isRenderTargetAllocated         (RenderTargetHandle targetHandle) const override;
        [[nodiscard]] virtual UInt32                      getRenderTargetCount            () const  override;
        virtual void                        addRenderTargetRenderBuffer     (RenderTargetHandle targetHandle, RenderBufferHandle bufferHandle) override;
        [[nodiscard]] virtual UInt32                      getRenderTargetRenderBufferCount(RenderTargetHandle targetHandle) const override;
        [[nodiscard]] virtual RenderBufferHandle          getRenderTargetRenderBuffer     (RenderTargetHandle targetHandle, UInt32 bufferIndex) const override;

        // Render buffers
        virtual RenderBufferHandle          allocateRenderBuffer            (const RenderBuffer& renderBuffer, RenderBufferHandle handle = RenderBufferHandle::Invalid()) override;
        virtual void                        releaseRenderBuffer             (RenderBufferHandle handle) override;
        [[nodiscard]] virtual bool                        isRenderBufferAllocated         (RenderBufferHandle handle) const override;
        [[nodiscard]] virtual UInt32                      getRenderBufferCount            () const override;
        [[nodiscard]] virtual const RenderBuffer&         getRenderBuffer                 (RenderBufferHandle handle) const override;

        // Data buffers
        virtual DataBufferHandle            allocateDataBuffer              (EDataBufferType dataBufferType, EDataType dataType, UInt32 maximumSizeInBytes, DataBufferHandle handle = DataBufferHandle::Invalid()) override;
        virtual void                        releaseDataBuffer               (DataBufferHandle handle) override;
        [[nodiscard]] virtual UInt32                      getDataBufferCount              () const override;
        virtual void                        updateDataBuffer                (DataBufferHandle handle, UInt32 offsetInBytes, UInt32 dataSizeInBytes, const Byte* data) override;
        [[nodiscard]] virtual bool                        isDataBufferAllocated           (DataBufferHandle handle) const override;
        [[nodiscard]] virtual const GeometryDataBuffer&   getDataBuffer                   (DataBufferHandle handle) const override;

        //Texture buffers
        virtual TextureBufferHandle         allocateTextureBuffer           (ETextureFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle = TextureBufferHandle::Invalid()) override;
        virtual void                        releaseTextureBuffer            (TextureBufferHandle handle) override;
        [[nodiscard]] virtual bool                        isTextureBufferAllocated        (TextureBufferHandle handle) const override;
        [[nodiscard]] virtual UInt32                      getTextureBufferCount           () const override;
        virtual void                        updateTextureBuffer             (TextureBufferHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data) override;
        [[nodiscard]] virtual const TextureBuffer&        getTextureBuffer                (TextureBufferHandle handle) const override;

        virtual DataSlotHandle              allocateDataSlot                (const DataSlot& dataSlot, DataSlotHandle handle = DataSlotHandle::Invalid()) override;
        virtual void                        setDataSlotTexture              (DataSlotHandle handle, const ResourceContentHash& texture) override;
        virtual void                        releaseDataSlot                 (DataSlotHandle handle) override;
        [[nodiscard]] virtual bool                        isDataSlotAllocated             (DataSlotHandle handle) const override;
        [[nodiscard]] virtual UInt32                      getDataSlotCount                () const override;
        [[nodiscard]] virtual const DataSlot&             getDataSlot                     (DataSlotHandle handle) const override;

        virtual SceneReferenceHandle        allocateSceneReference          (SceneId sceneId, SceneReferenceHandle handle = {}) override;
        virtual void                        releaseSceneReference           (SceneReferenceHandle handle) override;
        virtual void                        requestSceneReferenceState      (SceneReferenceHandle handle, RendererSceneState state) override;
        virtual void                        requestSceneReferenceFlushNotifications(SceneReferenceHandle handle, bool enable) override;
        virtual void                        setSceneReferenceRenderOrder    (SceneReferenceHandle handle, int32_t renderOrder) override;
        [[nodiscard]] virtual bool                        isSceneReferenceAllocated       (SceneReferenceHandle handle) const override final;
        [[nodiscard]] virtual UInt32                      getSceneReferenceCount          () const override final;
        [[nodiscard]] virtual const SceneReference&       getSceneReference               (SceneReferenceHandle handle) const override final;

        void flushPendingSceneActions();

        [[nodiscard]] virtual SceneSizeInformation getSceneSizeInformation() const override;
        virtual void preallocateSceneSize(const SceneSizeInformation& sizeInfo) override;

    private:
        // Internal scene which holds the actual scene content; all getters are redirected to m_scene
        const Scene m_scene;
        // Converts IScene calls to actions, collects them and applies to m_actionApplier, which applies them on m_scene
        ActionCollectingScene m_actionCollector;
    };

}

#endif
