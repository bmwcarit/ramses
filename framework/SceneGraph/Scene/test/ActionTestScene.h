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

        [[nodiscard]] const String&               getName                         () const override;
        [[nodiscard]] SceneId                     getSceneId                      () const override;

        void   setEffectTimeSync(FlushTime::Clock::time_point t) override;
        [[nodiscard]] FlushTime::Clock::time_point getEffectTimeSync() const override;

        // Renderable
        RenderableHandle            allocateRenderable              (NodeHandle nodeHandle, RenderableHandle handle = RenderableHandle::Invalid()) override;
        void                        releaseRenderable               (RenderableHandle renderableHandle) override;
        [[nodiscard]] bool                        isRenderableAllocated           (RenderableHandle renderableHandle) const override;
        [[nodiscard]] UInt32                      getRenderableCount              () const override;
        void                        setRenderableDataInstance       (RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance) override;
        void                        setRenderableStartIndex         (RenderableHandle renderableHandle, UInt32 startIndex) override;
        void                        setRenderableIndexCount         (RenderableHandle renderableHandle, UInt32 indexCount) override;
        void                        setRenderableRenderState        (RenderableHandle renderableHandle, RenderStateHandle stateHandle) override;
        void                        setRenderableVisibility         (RenderableHandle renderableHandle, EVisibilityMode visible) override;
        void                        setRenderableInstanceCount      (RenderableHandle renderableHandle, UInt32 instanceCount) override;
        void                        setRenderableStartVertex        (RenderableHandle renderableHandle, UInt32 startVertex) override;
        [[nodiscard]] const Renderable&           getRenderable                   (RenderableHandle renderableHandle) const override;

        // Render state
        RenderStateHandle           allocateRenderState             (RenderStateHandle stateHandle = RenderStateHandle::Invalid()) override;
        void                        releaseRenderState              (RenderStateHandle stateHandle) override;
        [[nodiscard]] bool                        isRenderStateAllocated          (RenderStateHandle stateHandle) const override;
        [[nodiscard]] UInt32                      getRenderStateCount             () const override;
        void                        setRenderStateBlendFactors            (RenderStateHandle stateHandle, EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha) override;
        void                        setRenderStateBlendOperations         (RenderStateHandle stateHandle, EBlendOperation operationColor, EBlendOperation operationAlpha) override;
        void                        setRenderStateBlendColor              (RenderStateHandle stateHandle, const Vector4& color) override;
        void                        setRenderStateCullMode                (RenderStateHandle stateHandle, ECullMode cullMode) override;
        void                        setRenderStateDrawMode                (RenderStateHandle stateHandle, EDrawMode drawMode) override;
        void                        setRenderStateDepthFunc               (RenderStateHandle stateHandle, EDepthFunc func) override;
        void                        setRenderStateDepthWrite              (RenderStateHandle stateHandle, EDepthWrite flag) override;
        void                        setRenderStateScissorTest             (RenderStateHandle stateHandle, EScissorTest flag, const RenderState::ScissorRegion& region) override;
        void                        setRenderStateStencilFunc             (RenderStateHandle stateHandle, EStencilFunc func, UInt8 ref, UInt8 mask) override;
        void                        setRenderStateStencilOps              (RenderStateHandle stateHandle, EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass) override;
        void                        setRenderStateColorWriteMask          (RenderStateHandle stateHandle, ColorWriteMask colorMask) override;
        [[nodiscard]] const RenderState&          getRenderState                        (RenderStateHandle stateHandle) const override;

        // Camera
        CameraHandle                allocateCamera                  (ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle dataInstance, CameraHandle handle = CameraHandle::Invalid()) override;
        void                        releaseCamera                   (CameraHandle cameraHandle) override;
        [[nodiscard]] bool                        isCameraAllocated               (CameraHandle handle) const override;
        [[nodiscard]] UInt32                      getCameraCount                  () const override;
        [[nodiscard]] const Camera&               getCamera                       (CameraHandle cameraHandle) const override;

        // Creation/Deletion
        NodeHandle                  allocateNode                    (UInt32 childrenCount = 0u, NodeHandle handle = NodeHandle::Invalid()) override;
        void                        releaseNode                     (NodeHandle nodeHandle) override;
        [[nodiscard]] bool                        isNodeAllocated                 (NodeHandle node) const override;
        [[nodiscard]] UInt32                      getNodeCount                    () const override;

        TransformHandle             allocateTransform               (NodeHandle nodeHandle, TransformHandle handle = TransformHandle::Invalid()) override;
        void                        releaseTransform                (TransformHandle transform) override;
        [[nodiscard]] bool                        isTransformAllocated            (TransformHandle transformHandle) const override;
        [[nodiscard]] UInt32                      getTransformCount               () const override;
        [[nodiscard]] NodeHandle                  getTransformNode                (TransformHandle handle) const override;

        // Parent-child relationship
        [[nodiscard]] NodeHandle                  getParent                       (NodeHandle nodeHandle) const override;
        void                        addChildToNode                  (NodeHandle parent, NodeHandle child) override;
        void                        removeChildFromNode             (NodeHandle parent, NodeHandle child) override;
        [[nodiscard]] UInt32                      getChildCount                   (NodeHandle parent) const override;
        [[nodiscard]] NodeHandle                  getChild                        (NodeHandle parent, UInt32 childNumber) const override;

        // Transformation
        [[nodiscard]] const Vector3&              getTranslation                  (TransformHandle handle) const override;
        [[nodiscard]] const Vector4&              getRotation                     (TransformHandle handle) const override;
        [[nodiscard]] ERotationType         getRotationType           (TransformHandle handle) const override;
        [[nodiscard]] const Vector3&              getScaling                      (TransformHandle handle) const override;
        void                        setTranslation                  (TransformHandle handle, const Vector3& translation) override;
        void                        setRotation                     (TransformHandle handle, const Vector4& rotation, ERotationType rotationType) override;
        void                        setScaling                      (TransformHandle handle, const Vector3& scaling) override;

        DataLayoutHandle            allocateDataLayout              (const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutHandle handle = DataLayoutHandle::Invalid()) override;
        void                        releaseDataLayout               (DataLayoutHandle layoutHandle) override;
        [[nodiscard]] bool                        isDataLayoutAllocated           (DataLayoutHandle layoutHandle) const override;
        [[nodiscard]] UInt32                      getDataLayoutCount              () const override;

        [[nodiscard]] const DataLayout&           getDataLayout                   (DataLayoutHandle layoutHandle) const override;

        DataInstanceHandle          allocateDataInstance            (DataLayoutHandle finishedLayoutHandle, DataInstanceHandle instanceHandle = DataInstanceHandle::Invalid()) override;
        void                        releaseDataInstance             (DataInstanceHandle containerHandle) override;
        [[nodiscard]] bool                        isDataInstanceAllocated         (DataInstanceHandle containerHandle) const override;
        [[nodiscard]] UInt32                      getDataInstanceCount            () const override;
        [[nodiscard]] DataLayoutHandle            getLayoutOfDataInstance         (DataInstanceHandle containerHandle) const override;

        [[nodiscard]] const Float*                getDataFloatArray               (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const Vector2*              getDataVector2fArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const Vector3*              getDataVector3fArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const Vector4*              getDataVector4fArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const Int32*                getDataIntegerArray             (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const Matrix22f*            getDataMatrix22fArray           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const Matrix33f*            getDataMatrix33fArray           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const Matrix44f*            getDataMatrix44fArray           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const Vector2i*             getDataVector2iArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const Vector3i*             getDataVector3iArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const Vector4i*             getDataVector4iArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const ResourceField&        getDataResource                 (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] TextureSamplerHandle        getDataTextureSamplerHandle     (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] DataInstanceHandle          getDataReference                (DataInstanceHandle containerHandle, DataFieldHandle field) const override;

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

        // get/setData*Array wrappers for elementCount == 1
        [[nodiscard]] Float                       getDataSingleFloat              (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const Vector2&              getDataSingleVector2f           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const Vector3&              getDataSingleVector3f           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const Vector4&              getDataSingleVector4f           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] Int32                       getDataSingleInteger            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const Matrix22f&            getDataSingleMatrix22f          (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const Matrix33f&            getDataSingleMatrix33f          (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const Matrix44f&            getDataSingleMatrix44f          (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const Vector2i&             getDataSingleVector2i           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const Vector3i&             getDataSingleVector3i           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const Vector4i&             getDataSingleVector4i           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;

        void                        setDataSingleFloat              (DataInstanceHandle containerHandle, DataFieldHandle field, Float data) override;
        void                        setDataSingleVector2f           (DataInstanceHandle containerHandle, DataFieldHandle field, const Vector2& data) override;
        void                        setDataSingleVector3f           (DataInstanceHandle containerHandle, DataFieldHandle field, const Vector3& data) override;
        void                        setDataSingleVector4f           (DataInstanceHandle containerHandle, DataFieldHandle field, const Vector4& data) override;
        void                        setDataSingleInteger            (DataInstanceHandle containerHandle, DataFieldHandle field, Int32 data) override;
        void                        setDataSingleVector2i           (DataInstanceHandle containerHandle, DataFieldHandle field, const Vector2i& data) override;
        void                        setDataSingleVector3i           (DataInstanceHandle containerHandle, DataFieldHandle field, const Vector3i& data) override;
        void                        setDataSingleVector4i           (DataInstanceHandle containerHandle, DataFieldHandle field, const Vector4i& data) override;
        void                        setDataSingleMatrix22f          (DataInstanceHandle containerHandle, DataFieldHandle field, const Matrix22f& data) override;
        void                        setDataSingleMatrix33f          (DataInstanceHandle containerHandle, DataFieldHandle field, const Matrix33f& data) override;
        void                        setDataSingleMatrix44f          (DataInstanceHandle containerHandle, DataFieldHandle field, const Matrix44f& data) override;

        // Texture sampler description
        TextureSamplerHandle        allocateTextureSampler          (const TextureSampler& sampler, TextureSamplerHandle handle = TextureSamplerHandle::Invalid()) override;
        void                        releaseTextureSampler           (TextureSamplerHandle handle) override;
        [[nodiscard]] bool                        isTextureSamplerAllocated       (TextureSamplerHandle handle) const override;
        [[nodiscard]] UInt32                      getTextureSamplerCount          () const override;
        [[nodiscard]] const TextureSampler&       getTextureSampler               (TextureSamplerHandle handle) const override;

        // Render groups
        RenderGroupHandle           allocateRenderGroup             (UInt32 renderableCount = 0u, UInt32 nestedGroupCount = 0u, RenderGroupHandle groupHandle = RenderGroupHandle::Invalid()) override;
        void                        releaseRenderGroup              (RenderGroupHandle groupHandle) override;
        [[nodiscard]] bool                        isRenderGroupAllocated          (RenderGroupHandle groupHandle) const override;
        [[nodiscard]] UInt32                      getRenderGroupCount             () const override;
        void                        addRenderableToRenderGroup      (RenderGroupHandle groupHandle, RenderableHandle renderableHandle, Int32 order) override;
        void                        removeRenderableFromRenderGroup (RenderGroupHandle groupHandle, RenderableHandle renderableHandle) override;
        void                        addRenderGroupToRenderGroup     (RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, Int32 order) override;
        void                        removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild) override;
        [[nodiscard]] const RenderGroup&          getRenderGroup                  (RenderGroupHandle groupHandle) const override;

        // Render passes
        RenderPassHandle            allocateRenderPass              (UInt32 renderGroupCount = 0u, RenderPassHandle passHandle = RenderPassHandle::Invalid()) override;
        void                        releaseRenderPass               (RenderPassHandle passHandle) override;
        [[nodiscard]] bool                        isRenderPassAllocated           (RenderPassHandle passHandle) const override;
        [[nodiscard]] UInt32                      getRenderPassCount              () const override;
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
        [[nodiscard]] const RenderPass&           getRenderPass                   (RenderPassHandle passHandle) const override;

        //Blit passes
        BlitPassHandle              allocateBlitPass                (RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle = BlitPassHandle::Invalid()) override;
        void                        releaseBlitPass                 (BlitPassHandle passHandle) override;
        [[nodiscard]] bool                        isBlitPassAllocated             (BlitPassHandle passHandle) const override;
        [[nodiscard]] UInt32                      getBlitPassCount                () const override;
        void                        setBlitPassRenderOrder          (BlitPassHandle passHandle, Int32 renderOrder) override;
        void                        setBlitPassEnabled              (BlitPassHandle passHandle, bool isEnabled) override;
        void                        setBlitPassRegions              (BlitPassHandle passHandle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion) override;
        [[nodiscard]] const BlitPass&             getBlitPass                     (BlitPassHandle passHandle) const override;

        //Pickable object
        PickableObjectHandle        allocatePickableObject          (DataBufferHandle geometryHandle, NodeHandle nodeHandle, PickableObjectId id, PickableObjectHandle pickableHandle = PickableObjectHandle::Invalid()) override;
        void                        releasePickableObject           (PickableObjectHandle pickableHandle) override;
        [[nodiscard]] bool                        isPickableObjectAllocated       (PickableObjectHandle pickableHandle) const final override;
        [[nodiscard]] UInt32                      getPickableObjectCount          () const final override;
        void                        setPickableObjectId             (PickableObjectHandle pickableHandle, PickableObjectId id) override;
        void                        setPickableObjectCamera         (PickableObjectHandle pickableHandle, CameraHandle cameraHandle) override;
        void                        setPickableObjectEnabled        (PickableObjectHandle pickableHandle, bool isEnabled) override;
        [[nodiscard]] const PickableObject&       getPickableObject               (PickableObjectHandle pickableHandle) const override;

        // Render targets
        RenderTargetHandle          allocateRenderTarget            (RenderTargetHandle targetHandle = RenderTargetHandle::Invalid()) override;
        void                        releaseRenderTarget             (RenderTargetHandle targetHandle) override;
        [[nodiscard]] bool                        isRenderTargetAllocated         (RenderTargetHandle targetHandle) const override;
        [[nodiscard]] UInt32                      getRenderTargetCount            () const  override;
        void                        addRenderTargetRenderBuffer     (RenderTargetHandle targetHandle, RenderBufferHandle bufferHandle) override;
        [[nodiscard]] UInt32                      getRenderTargetRenderBufferCount(RenderTargetHandle targetHandle) const override;
        [[nodiscard]] RenderBufferHandle          getRenderTargetRenderBuffer     (RenderTargetHandle targetHandle, UInt32 bufferIndex) const override;

        // Render buffers
        RenderBufferHandle          allocateRenderBuffer            (const RenderBuffer& renderBuffer, RenderBufferHandle handle = RenderBufferHandle::Invalid()) override;
        void                        releaseRenderBuffer             (RenderBufferHandle handle) override;
        [[nodiscard]] bool                        isRenderBufferAllocated         (RenderBufferHandle handle) const override;
        [[nodiscard]] UInt32                      getRenderBufferCount            () const override;
        [[nodiscard]] const RenderBuffer&         getRenderBuffer                 (RenderBufferHandle handle) const override;

        // Data buffers
        DataBufferHandle            allocateDataBuffer              (EDataBufferType dataBufferType, EDataType dataType, UInt32 maximumSizeInBytes, DataBufferHandle handle = DataBufferHandle::Invalid()) override;
        void                        releaseDataBuffer               (DataBufferHandle handle) override;
        [[nodiscard]] UInt32                      getDataBufferCount              () const override;
        void                        updateDataBuffer                (DataBufferHandle handle, UInt32 offsetInBytes, UInt32 dataSizeInBytes, const Byte* data) override;
        [[nodiscard]] bool                        isDataBufferAllocated           (DataBufferHandle handle) const override;
        [[nodiscard]] const GeometryDataBuffer&   getDataBuffer                   (DataBufferHandle handle) const override;

        //Texture buffers
        TextureBufferHandle         allocateTextureBuffer           (ETextureFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle = TextureBufferHandle::Invalid()) override;
        void                        releaseTextureBuffer            (TextureBufferHandle handle) override;
        [[nodiscard]] bool                        isTextureBufferAllocated        (TextureBufferHandle handle) const override;
        [[nodiscard]] UInt32                      getTextureBufferCount           () const override;
        void                        updateTextureBuffer             (TextureBufferHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data) override;
        [[nodiscard]] const TextureBuffer&        getTextureBuffer                (TextureBufferHandle handle) const override;

        DataSlotHandle              allocateDataSlot                (const DataSlot& dataSlot, DataSlotHandle handle = DataSlotHandle::Invalid()) override;
        void                        setDataSlotTexture              (DataSlotHandle handle, const ResourceContentHash& texture) override;
        void                        releaseDataSlot                 (DataSlotHandle handle) override;
        [[nodiscard]] bool                        isDataSlotAllocated             (DataSlotHandle handle) const override;
        [[nodiscard]] UInt32                      getDataSlotCount                () const override;
        [[nodiscard]] const DataSlot&             getDataSlot                     (DataSlotHandle handle) const override;

        SceneReferenceHandle        allocateSceneReference          (SceneId sceneId, SceneReferenceHandle handle = {}) override;
        void                        releaseSceneReference           (SceneReferenceHandle handle) override;
        void                        requestSceneReferenceState      (SceneReferenceHandle handle, RendererSceneState state) override;
        void                        requestSceneReferenceFlushNotifications(SceneReferenceHandle handle, bool enable) override;
        void                        setSceneReferenceRenderOrder    (SceneReferenceHandle handle, int32_t renderOrder) override;
        [[nodiscard]] bool                        isSceneReferenceAllocated       (SceneReferenceHandle handle) const final override;
        [[nodiscard]] UInt32                      getSceneReferenceCount          () const final override;
        [[nodiscard]] const SceneReference&       getSceneReference               (SceneReferenceHandle handle) const final override;

        void flushPendingSceneActions();

        [[nodiscard]] SceneSizeInformation getSceneSizeInformation() const override;
        void preallocateSceneSize(const SceneSizeInformation& sizeInfo) override;

    private:
        // Internal scene which holds the actual scene content; all getters are redirected to m_scene
        const Scene m_scene;
        // Converts IScene calls to actions, collects them and applies to m_actionApplier, which applies them on m_scene
        ActionCollectingScene m_actionCollector;
    };

}

#endif
