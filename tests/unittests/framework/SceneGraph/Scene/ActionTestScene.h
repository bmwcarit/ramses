//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Scene/ActionCollectingScene.h"

namespace ramses::internal
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

        [[nodiscard]] const std::string&          getName                         () const override;
        [[nodiscard]] SceneId                     getSceneId                      () const override;

        void   setEffectTimeSync(FlushTime::Clock::time_point t) override;
        [[nodiscard]] FlushTime::Clock::time_point getEffectTimeSync() const override;

        // Renderable
        RenderableHandle            allocateRenderable              (NodeHandle nodeHandle, RenderableHandle handle) override;
        void                        releaseRenderable               (RenderableHandle renderableHandle) override;
        [[nodiscard]] bool          isRenderableAllocated           (RenderableHandle renderableHandle) const override;
        [[nodiscard]] uint32_t      getRenderableCount              () const override;
        void                        setRenderableDataInstance       (RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance) override;
        void                        setRenderableStartIndex         (RenderableHandle renderableHandle, uint32_t startIndex) override;
        void                        setRenderableIndexCount         (RenderableHandle renderableHandle, uint32_t indexCount) override;
        void                        setRenderableRenderState        (RenderableHandle renderableHandle, RenderStateHandle stateHandle) override;
        void                        setRenderableVisibility         (RenderableHandle renderableHandle, EVisibilityMode visible) override;
        void                        setRenderableInstanceCount      (RenderableHandle renderableHandle, uint32_t instanceCount) override;
        void                        setRenderableStartVertex        (RenderableHandle renderableHandle, uint32_t startVertex) override;
        [[nodiscard]] const Renderable&           getRenderable                   (RenderableHandle renderableHandle) const override;

        // Render state
        RenderStateHandle           allocateRenderState             (RenderStateHandle stateHandle) override;
        void                        releaseRenderState              (RenderStateHandle stateHandle) override;
        [[nodiscard]] bool          isRenderStateAllocated          (RenderStateHandle stateHandle) const override;
        [[nodiscard]] uint32_t      getRenderStateCount             () const override;
        void                        setRenderStateBlendFactors            (RenderStateHandle stateHandle, EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha) override;
        void                        setRenderStateBlendOperations         (RenderStateHandle stateHandle, EBlendOperation operationColor, EBlendOperation operationAlpha) override;
        void                        setRenderStateBlendColor              (RenderStateHandle stateHandle, const glm::vec4& color) override;
        void                        setRenderStateCullMode                (RenderStateHandle stateHandle, ECullMode cullMode) override;
        void                        setRenderStateDrawMode                (RenderStateHandle stateHandle, EDrawMode drawMode) override;
        void                        setRenderStateDepthFunc               (RenderStateHandle stateHandle, EDepthFunc func) override;
        void                        setRenderStateDepthWrite              (RenderStateHandle stateHandle, EDepthWrite flag) override;
        void                        setRenderStateScissorTest             (RenderStateHandle stateHandle, EScissorTest flag, const RenderState::ScissorRegion& region) override;
        void                        setRenderStateStencilFunc             (RenderStateHandle stateHandle, EStencilFunc func, uint8_t ref, uint8_t mask) override;
        void                        setRenderStateStencilOps              (RenderStateHandle stateHandle, EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass) override;
        void                        setRenderStateColorWriteMask          (RenderStateHandle stateHandle, ColorWriteMask colorMask) override;
        [[nodiscard]] const RenderState&          getRenderState                        (RenderStateHandle stateHandle) const override;

        // Camera
        CameraHandle                allocateCamera                  (ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle dataInstance, CameraHandle handle) override;
        void                        releaseCamera                   (CameraHandle cameraHandle) override;
        [[nodiscard]] bool                        isCameraAllocated               (CameraHandle handle) const override;
        [[nodiscard]] uint32_t                      getCameraCount                  () const override;
        [[nodiscard]] const Camera&               getCamera                       (CameraHandle cameraHandle) const override;

        // Creation/Deletion
        NodeHandle                  allocateNode                    (uint32_t childrenCount, NodeHandle handle) override;
        void                        releaseNode                     (NodeHandle nodeHandle) override;
        [[nodiscard]] bool                        isNodeAllocated                 (NodeHandle node) const override;
        [[nodiscard]] uint32_t                      getNodeCount                    () const override;

        TransformHandle             allocateTransform               (NodeHandle nodeHandle, TransformHandle handle) override;
        void                        releaseTransform                (TransformHandle transform) override;
        [[nodiscard]] bool                        isTransformAllocated            (TransformHandle transformHandle) const override;
        [[nodiscard]] uint32_t                      getTransformCount               () const override;
        [[nodiscard]] NodeHandle                  getTransformNode                (TransformHandle handle) const override;

        // Parent-child relationship
        [[nodiscard]] NodeHandle                  getParent                       (NodeHandle nodeHandle) const override;
        void                        addChildToNode                  (NodeHandle parent, NodeHandle child) override;
        void                        removeChildFromNode             (NodeHandle parent, NodeHandle child) override;
        [[nodiscard]] uint32_t                      getChildCount                   (NodeHandle parent) const override;
        [[nodiscard]] NodeHandle                  getChild                        (NodeHandle parent, uint32_t childNumber) const override;

        // Transformation
        [[nodiscard]] const glm::vec3&              getTranslation                  (TransformHandle handle) const override;
        [[nodiscard]] const glm::vec4&              getRotation                     (TransformHandle handle) const override;
        [[nodiscard]] ERotationType         getRotationType           (TransformHandle handle) const override;
        [[nodiscard]] const glm::vec3&              getScaling                      (TransformHandle handle) const override;
        void                        setTranslation                  (TransformHandle handle, const glm::vec3& translation) override;
        void                        setRotation                     (TransformHandle handle, const glm::vec4& rotation, ERotationType rotationType) override;
        void                        setScaling                      (TransformHandle handle, const glm::vec3& scaling) override;

        DataLayoutHandle            allocateDataLayout              (const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutHandle handle) override;
        void                        releaseDataLayout               (DataLayoutHandle layoutHandle) override;
        [[nodiscard]] bool                        isDataLayoutAllocated           (DataLayoutHandle layoutHandle) const override;
        [[nodiscard]] uint32_t                      getDataLayoutCount              () const override;

        [[nodiscard]] const DataLayout&           getDataLayout                   (DataLayoutHandle layoutHandle) const override;

        DataInstanceHandle          allocateDataInstance            (DataLayoutHandle finishedLayoutHandle, DataInstanceHandle instanceHandle) override;
        void                        releaseDataInstance             (DataInstanceHandle containerHandle) override;
        [[nodiscard]] bool                        isDataInstanceAllocated         (DataInstanceHandle containerHandle) const override;
        [[nodiscard]] uint32_t                      getDataInstanceCount            () const override;
        [[nodiscard]] DataLayoutHandle            getLayoutOfDataInstance         (DataInstanceHandle containerHandle) const override;

        [[nodiscard]] const float*              getDataFloatArray           (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const glm::vec2*          getDataVector2fArray        (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const glm::vec3*          getDataVector3fArray        (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const glm::vec4*          getDataVector4fArray        (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const bool*               getDataBooleanArray         (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const int32_t*            getDataIntegerArray         (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const glm::mat2*          getDataMatrix22fArray       (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const glm::mat3*          getDataMatrix33fArray       (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const glm::mat4*          getDataMatrix44fArray       (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const glm::ivec2*         getDataVector2iArray        (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const glm::ivec3*         getDataVector3iArray        (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const glm::ivec4*         getDataVector4iArray        (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const ResourceField&      getDataResource             (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] TextureSamplerHandle      getDataTextureSamplerHandle (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] DataInstanceHandle        getDataReference            (DataInstanceHandle containerHandle, DataFieldHandle field) const override;

        void setDataFloatArray           (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const float* data) override;
        void setDataVector2fArray        (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec2* data) override;
        void setDataVector3fArray        (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec3* data) override;
        void setDataVector4fArray        (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec4* data) override;
        void setDataBooleanArray         (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const bool* data) override;
        void setDataIntegerArray         (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const int32_t* data) override;
        void setDataVector2iArray        (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec2* data) override;
        void setDataVector3iArray        (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec3* data) override;
        void setDataVector4iArray        (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec4* data) override;
        void setDataMatrix22fArray       (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat2* data) override;
        void setDataMatrix33fArray       (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat3* data) override;
        void setDataMatrix44fArray       (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat4* data) override;
        void setDataResource             (DataInstanceHandle containerHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, uint32_t instancingDivisor, uint16_t offsetWithinElementInBytes, uint16_t stride) override;
        void setDataTextureSamplerHandle (DataInstanceHandle containerHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle) override;
        void setDataReference            (DataInstanceHandle containerHandle, DataFieldHandle field, DataInstanceHandle dataRef) override;

        // get/setData*Array wrappers for elementCount == 1
        [[nodiscard]] float             getDataSingleFloat     (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const glm::vec2&  getDataSingleVector2f  (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const glm::vec3&  getDataSingleVector3f  (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const glm::vec4&  getDataSingleVector4f  (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] bool              getDataSingleBoolean   (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] int32_t           getDataSingleInteger   (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const glm::mat2&  getDataSingleMatrix22f (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const glm::mat3&  getDataSingleMatrix33f (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const glm::mat4&  getDataSingleMatrix44f (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const glm::ivec3& getDataSingleVector3i  (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const glm::ivec4& getDataSingleVector4i  (DataInstanceHandle containerHandle, DataFieldHandle field) const override;
        [[nodiscard]] const glm::ivec2& getDataSingleVector2i  (DataInstanceHandle containerHandle, DataFieldHandle field) const override;

        void setDataSingleFloat     (DataInstanceHandle containerHandle, DataFieldHandle field, float data) override;
        void setDataSingleVector2f  (DataInstanceHandle containerHandle, DataFieldHandle field, const glm::vec2& data) override;
        void setDataSingleVector3f  (DataInstanceHandle containerHandle, DataFieldHandle field, const glm::vec3& data) override;
        void setDataSingleVector4f  (DataInstanceHandle containerHandle, DataFieldHandle field, const glm::vec4& data) override;
        void setDataSingleBoolean   (DataInstanceHandle containerHandle, DataFieldHandle field, bool data) override;
        void setDataSingleInteger   (DataInstanceHandle containerHandle, DataFieldHandle field, int32_t data) override;
        void setDataSingleVector2i  (DataInstanceHandle containerHandle, DataFieldHandle field, const glm::ivec2& data) override;
        void setDataSingleVector3i  (DataInstanceHandle containerHandle, DataFieldHandle field, const glm::ivec3& data) override;
        void setDataSingleVector4i  (DataInstanceHandle containerHandle, DataFieldHandle field, const glm::ivec4& data) override;
        void setDataSingleMatrix22f (DataInstanceHandle containerHandle, DataFieldHandle field, const glm::mat2& data) override;
        void setDataSingleMatrix33f (DataInstanceHandle containerHandle, DataFieldHandle field, const glm::mat3& data) override;
        void setDataSingleMatrix44f (DataInstanceHandle containerHandle, DataFieldHandle field, const glm::mat4& data) override;

        // Texture sampler description
        TextureSamplerHandle        allocateTextureSampler          (const TextureSampler& sampler, TextureSamplerHandle handle) override;
        void                        releaseTextureSampler           (TextureSamplerHandle handle) override;
        [[nodiscard]] bool                        isTextureSamplerAllocated       (TextureSamplerHandle handle) const override;
        [[nodiscard]] uint32_t                      getTextureSamplerCount          () const override;
        [[nodiscard]] const TextureSampler&       getTextureSampler               (TextureSamplerHandle handle) const override;

        // Render groups
        RenderGroupHandle           allocateRenderGroup             (uint32_t renderableCount, uint32_t nestedGroupCount, RenderGroupHandle groupHandle) override;
        void                        releaseRenderGroup              (RenderGroupHandle groupHandle) override;
        [[nodiscard]] bool                        isRenderGroupAllocated          (RenderGroupHandle groupHandle) const override;
        [[nodiscard]] uint32_t                      getRenderGroupCount             () const override;
        void                        addRenderableToRenderGroup      (RenderGroupHandle groupHandle, RenderableHandle renderableHandle, int32_t order) override;
        void                        removeRenderableFromRenderGroup (RenderGroupHandle groupHandle, RenderableHandle renderableHandle) override;
        void                        addRenderGroupToRenderGroup     (RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, int32_t order) override;
        void                        removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild) override;
        [[nodiscard]] const RenderGroup&          getRenderGroup                  (RenderGroupHandle groupHandle) const override;

        // Render passes
        RenderPassHandle            allocateRenderPass              (uint32_t renderGroupCount, RenderPassHandle passHandle) override;
        void                        releaseRenderPass               (RenderPassHandle passHandle) override;
        [[nodiscard]] bool                        isRenderPassAllocated           (RenderPassHandle passHandle) const override;
        [[nodiscard]] uint32_t                    getRenderPassCount              () const override;
        void                        setRenderPassClearColor         (RenderPassHandle passHandle, const glm::vec4& clearColor) override;
        void                        setRenderPassClearFlag          (RenderPassHandle passHandle, ClearFlags clearFlag) override;
        void                        setRenderPassCamera             (RenderPassHandle passHandle, CameraHandle cameraHandle) override;
        void                        setRenderPassRenderTarget       (RenderPassHandle passHandle, RenderTargetHandle targetHandle) override;
        void                        setRenderPassRenderOrder        (RenderPassHandle passHandle, int32_t renderOrder) override;
        void                        setRenderPassEnabled            (RenderPassHandle passHandle, bool isEnabled) override;
        void                        setRenderPassRenderOnce         (RenderPassHandle passHandle, bool enable) override;
        void                        retriggerRenderPassRenderOnce   (RenderPassHandle passHandle) override;
        void                        addRenderGroupToRenderPass      (RenderPassHandle passHandle, RenderGroupHandle groupHandle, int32_t order) override;
        void                        removeRenderGroupFromRenderPass (RenderPassHandle passHandle, RenderGroupHandle groupHandle) override;
        [[nodiscard]] const RenderPass&           getRenderPass                   (RenderPassHandle passHandle) const override;

        //Blit passes
        BlitPassHandle              allocateBlitPass                (RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle) override;
        void                        releaseBlitPass                 (BlitPassHandle passHandle) override;
        [[nodiscard]] bool                        isBlitPassAllocated             (BlitPassHandle passHandle) const override;
        [[nodiscard]] uint32_t                      getBlitPassCount                () const override;
        void                        setBlitPassRenderOrder          (BlitPassHandle passHandle, int32_t renderOrder) override;
        void                        setBlitPassEnabled              (BlitPassHandle passHandle, bool isEnabled) override;
        void                        setBlitPassRegions              (BlitPassHandle passHandle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion) override;
        [[nodiscard]] const BlitPass&             getBlitPass                     (BlitPassHandle passHandle) const override;

        //Pickable object
        PickableObjectHandle        allocatePickableObject          (DataBufferHandle geometryHandle, NodeHandle nodeHandle, PickableObjectId id, PickableObjectHandle pickableHandle) override;
        void                        releasePickableObject           (PickableObjectHandle pickableHandle) override;
        [[nodiscard]] bool                        isPickableObjectAllocated       (PickableObjectHandle pickableHandle) const final override;
        [[nodiscard]] uint32_t                      getPickableObjectCount          () const final override;
        void                        setPickableObjectId             (PickableObjectHandle pickableHandle, PickableObjectId id) override;
        void                        setPickableObjectCamera         (PickableObjectHandle pickableHandle, CameraHandle cameraHandle) override;
        void                        setPickableObjectEnabled        (PickableObjectHandle pickableHandle, bool isEnabled) override;
        [[nodiscard]] const PickableObject&       getPickableObject               (PickableObjectHandle pickableHandle) const override;

        // Render targets
        RenderTargetHandle          allocateRenderTarget            (RenderTargetHandle targetHandle) override;
        void                        releaseRenderTarget             (RenderTargetHandle targetHandle) override;
        [[nodiscard]] bool                        isRenderTargetAllocated         (RenderTargetHandle targetHandle) const override;
        [[nodiscard]] uint32_t                      getRenderTargetCount            () const  override;
        void                        addRenderTargetRenderBuffer     (RenderTargetHandle targetHandle, RenderBufferHandle bufferHandle) override;
        [[nodiscard]] uint32_t                      getRenderTargetRenderBufferCount(RenderTargetHandle targetHandle) const override;
        [[nodiscard]] RenderBufferHandle          getRenderTargetRenderBuffer     (RenderTargetHandle targetHandle, uint32_t bufferIndex) const override;

        // Render buffers
        RenderBufferHandle          allocateRenderBuffer            (const RenderBuffer& renderBuffer, RenderBufferHandle handle) override;
        void                        releaseRenderBuffer             (RenderBufferHandle handle) override;
        [[nodiscard]] bool                        isRenderBufferAllocated         (RenderBufferHandle handle) const override;
        [[nodiscard]] uint32_t                      getRenderBufferCount            () const override;
        [[nodiscard]] const RenderBuffer&         getRenderBuffer                 (RenderBufferHandle handle) const override;

        // Data buffers
        DataBufferHandle            allocateDataBuffer              (EDataBufferType dataBufferType, EDataType dataType, uint32_t maximumSizeInBytes, DataBufferHandle handle) override;
        void                        releaseDataBuffer               (DataBufferHandle handle) override;
        [[nodiscard]] uint32_t                      getDataBufferCount              () const override;
        void                        updateDataBuffer                (DataBufferHandle handle, uint32_t offsetInBytes, uint32_t dataSizeInBytes, const std::byte* data) override;
        [[nodiscard]] bool                        isDataBufferAllocated           (DataBufferHandle handle) const override;
        [[nodiscard]] const GeometryDataBuffer&   getDataBuffer                   (DataBufferHandle handle) const override;

        //Texture buffers
        TextureBufferHandle         allocateTextureBuffer           (EPixelStorageFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle) override;
        void                        releaseTextureBuffer            (TextureBufferHandle handle) override;
        [[nodiscard]] bool                        isTextureBufferAllocated        (TextureBufferHandle handle) const override;
        [[nodiscard]] uint32_t                      getTextureBufferCount           () const override;
        void                        updateTextureBuffer             (TextureBufferHandle handle, uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const std::byte* data) override;
        [[nodiscard]] const TextureBuffer&        getTextureBuffer                (TextureBufferHandle handle) const override;

        DataSlotHandle              allocateDataSlot                (const DataSlot& dataSlot, DataSlotHandle handle) override;
        void                        setDataSlotTexture              (DataSlotHandle handle, const ResourceContentHash& texture) override;
        void                        releaseDataSlot                 (DataSlotHandle handle) override;
        [[nodiscard]] bool                        isDataSlotAllocated             (DataSlotHandle handle) const override;
        [[nodiscard]] uint32_t                      getDataSlotCount                () const override;
        [[nodiscard]] const DataSlot&             getDataSlot                     (DataSlotHandle handle) const override;

        SceneReferenceHandle        allocateSceneReference          (SceneId sceneId, SceneReferenceHandle handle) override;
        void                        releaseSceneReference           (SceneReferenceHandle handle) override;
        void                        requestSceneReferenceState      (SceneReferenceHandle handle, RendererSceneState state) override;
        void                        requestSceneReferenceFlushNotifications(SceneReferenceHandle handle, bool enable) override;
        void                        setSceneReferenceRenderOrder    (SceneReferenceHandle handle, int32_t renderOrder) override;
        [[nodiscard]] bool                        isSceneReferenceAllocated       (SceneReferenceHandle handle) const final override;
        [[nodiscard]] uint32_t                      getSceneReferenceCount          () const final override;
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
