//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTERNAL_SCENE_H
#define RAMSES_INTERNAL_SCENE_H

#include "SceneAPI/IScene.h"
#include "SceneAPI/Handles.h"
#include "SceneAPI/ResourceField.h"
#include "SceneAPI/Camera.h"
#include "SceneAPI/Renderable.h"
#include "SceneAPI/RenderPass.h"
#include "SceneAPI/RenderGroup.h"
#include "SceneAPI/TextureSampler.h"
#include "SceneAPI/TextureBuffer.h"
#include "SceneAPI/GeometryDataBuffer.h"
#include "SceneAPI/RenderBuffer.h"
#include "SceneAPI/RenderTarget.h"
#include "SceneAPI/BlitPass.h"
#include "SceneAPI/PickableObject.h"
#include "SceneAPI/SceneReference.h"

#include "Scene/TopologyNode.h"
#include "Scene/TopologyTransform.h"
#include "Scene/DataLayout.h"
#include "Scene/DataInstance.h"

#include "Utils/MemoryPool.h"
#include "Utils/MemoryPoolExplicit.h"

namespace ramses_internal
{
    template <template<typename, typename> class MEMORYPOOL>
    class SceneT;

    using Scene = SceneT<MemoryPool>;
    using SceneWithExplicitMemory = SceneT<MemoryPoolExplicit>;

    template <template<typename, typename> class MEMORYPOOL>
    class SceneT : public IScene
    {
    public:
        using RenderableMemoryPool      = MEMORYPOOL<Renderable         , RenderableHandle>;
        using NodeMemoryPool            = MEMORYPOOL<TopologyNode       , NodeHandle>;
        using CameraMemoryPool          = MEMORYPOOL<Camera             , CameraHandle>;
        using RenderStateMemoryPool     = MEMORYPOOL<RenderState        , RenderStateHandle>;
        using TransformMemoryPool       = MEMORYPOOL<TopologyTransform  , TransformHandle>;
        using DataLayoutMemoryPool      = MEMORYPOOL<DataLayout         , DataLayoutHandle>;
        using DataInstanceMemoryPool    = MEMORYPOOL<DataInstance       , DataInstanceHandle>;
        using RenderGroupMemoryPool     = MEMORYPOOL<RenderGroup        , RenderGroupHandle>;
        using RenderPassMemoryPool      = MEMORYPOOL<RenderPass         , RenderPassHandle>;
        using BlitPassMemoryPool        = MEMORYPOOL<BlitPass           , BlitPassHandle>;
        using PickableObjectMemoryPool  = MEMORYPOOL<PickableObject     , PickableObjectHandle>;
        using RenderTargetMemoryPool    = MEMORYPOOL<RenderTarget       , RenderTargetHandle>;
        using RenderBufferMemoryPool    = MEMORYPOOL<RenderBuffer       , RenderBufferHandle>;
        using TextureSamplerMemoryPool  = MEMORYPOOL<TextureSampler     , TextureSamplerHandle>;
        using DataBufferMemoryPool      = MEMORYPOOL<GeometryDataBuffer , DataBufferHandle>;
        using TextureBufferMemoryPool   = MEMORYPOOL<TextureBuffer      , TextureBufferHandle>;
        using DataSlotMemoryPool        = MEMORYPOOL<DataSlot           , DataSlotHandle>;
        using SceneReferenceMemoryPool  = MEMORYPOOL<SceneReference     , SceneReferenceHandle>;

        explicit SceneT(const SceneInfo& sceneInfo = SceneInfo());
        ~SceneT() override;

        void                        preallocateSceneSize            (const SceneSizeInformation& sizeInfo) override;

        [[nodiscard]] SceneId                     getSceneId                      () const final override;
        [[nodiscard]] const String&               getName                         () const final override;

        void setEffectTimeSync(FlushTime::Clock::time_point t) override;
        [[nodiscard]] FlushTime::Clock::time_point getEffectTimeSync() const override;

        // Renderables
        RenderableHandle            allocateRenderable              (NodeHandle nodeHandle, RenderableHandle handle = RenderableHandle::Invalid()) override;
        void                        releaseRenderable               (RenderableHandle renderableHandle) override;
        [[nodiscard]] bool                        isRenderableAllocated           (RenderableHandle renderableHandle) const final override;
        [[nodiscard]] UInt32                      getRenderableCount              () const final override;
        void                        setRenderableDataInstance       (RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance) override;
        void                        setRenderableStartIndex         (RenderableHandle renderableHandle, UInt32 startIndex) override;
        void                        setRenderableIndexCount         (RenderableHandle renderableHandle, UInt32 indexCount) override;
        void                        setRenderableRenderState        (RenderableHandle renderableHandle, RenderStateHandle stateHandle) override;
        void                        setRenderableVisibility         (RenderableHandle renderableHandle, EVisibilityMode visibility) override;
        void                        setRenderableInstanceCount      (RenderableHandle renderableHandle, UInt32 instanceCount) override;
        void                        setRenderableStartVertex        (RenderableHandle renderableHandle, UInt32 startVertex) override;
        [[nodiscard]] const Renderable&           getRenderable                   (RenderableHandle renderableHandle) const final override;
        [[nodiscard]] const RenderableMemoryPool&         getRenderables                  () const;

        // Render state
        RenderStateHandle           allocateRenderState             (RenderStateHandle stateHandle = RenderStateHandle::Invalid()) override;
        void                        releaseRenderState              (RenderStateHandle stateHandle) override;
        [[nodiscard]] bool                        isRenderStateAllocated          (RenderStateHandle stateHandle) const final override;
        [[nodiscard]] UInt32                      getRenderStateCount             () const final override;
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
        [[nodiscard]] const RenderState&          getRenderState                  (RenderStateHandle stateHandle) const final override;
        [[nodiscard]] const RenderStateMemoryPool&              getRenderStates                 () const;

        // Camera
        CameraHandle                allocateCamera                  (ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle dataInstance, CameraHandle handle = CameraHandle::Invalid()) override;
        void                        releaseCamera                   (CameraHandle cameraHandle) override;
        [[nodiscard]] bool                        isCameraAllocated               (CameraHandle handle) const final override;
        [[nodiscard]] UInt32                      getCameraCount                  () const final override;
        [[nodiscard]] const Camera&               getCamera                       (CameraHandle cameraHandle) const final override;
        [[nodiscard]] const CameraMemoryPool&             getCameras                      () const;

        // Nodes
        NodeHandle                  allocateNode                    (UInt32 childrenCount = 0u, NodeHandle handle = NodeHandle::Invalid()) override;
        void                        releaseNode                     (NodeHandle nodeHandle) override;
        [[nodiscard]] bool                        isNodeAllocated                 (NodeHandle node) const final override;
        [[nodiscard]] UInt32                      getNodeCount                    () const final override;
        [[nodiscard]] NodeHandle                  getParent                       (NodeHandle nodeHandle) const final override;
        void                        addChildToNode                  (NodeHandle parent, NodeHandle child) override;
        void                        removeChildFromNode             (NodeHandle parent, NodeHandle child) override;
        [[nodiscard]] UInt32                      getChildCount                   (NodeHandle parent) const final override;
        [[nodiscard]] NodeHandle                  getChild                        (NodeHandle parent, UInt32 childNumber) const final override;
        [[nodiscard]] const NodeMemoryPool&                   getNodes                        () const;

        // Transformation
        TransformHandle             allocateTransform               (NodeHandle nodeHandle, TransformHandle handle = TransformHandle::Invalid()) override;
        void                        releaseTransform                (TransformHandle transform) override;
        [[nodiscard]] UInt32                      getTransformCount               () const final override;
        [[nodiscard]] bool                        isTransformAllocated            (TransformHandle transformHandle) const final override;
        [[nodiscard]] NodeHandle                  getTransformNode                (TransformHandle handle) const final override;
        [[nodiscard]] const Vector3&              getTranslation                  (TransformHandle handle) const final override;
        [[nodiscard]] const Vector4&              getRotation                     (TransformHandle handle) const final override;
        [[nodiscard]] ERotationConvention         getRotationConvention           (TransformHandle handle) const final override;
        [[nodiscard]] const Vector3&              getScaling                      (TransformHandle handle) const final override;
        void                        setTranslation                  (TransformHandle handle, const Vector3& translation) override;
        void                        setRotation                     (TransformHandle handle, const Vector4& rotation, ERotationConvention convention) override;
        void                        setScaling                      (TransformHandle handle, const Vector3& scaling) override;
        [[nodiscard]] const TransformMemoryPool&              getTransforms                   () const;

        DataLayoutHandle            allocateDataLayout              (const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutHandle handle = DataLayoutHandle::Invalid()) override;
        void                        releaseDataLayout               (DataLayoutHandle layoutHandle) override;
        [[nodiscard]] bool                        isDataLayoutAllocated           (DataLayoutHandle layoutHandle) const final override;
        [[nodiscard]] UInt32                      getDataLayoutCount              () const final override;
        [[nodiscard]] const DataLayout&           getDataLayout                   (DataLayoutHandle layoutHandle) const final override;
        [[nodiscard]] const DataLayoutMemoryPool&             getDataLayouts                  () const;

        DataInstanceHandle          allocateDataInstance            (DataLayoutHandle finishedLayoutHandle, DataInstanceHandle instanceHandle = DataInstanceHandle::Invalid()) override;
        void                        releaseDataInstance             (DataInstanceHandle containerHandle) override;
        [[nodiscard]] bool                        isDataInstanceAllocated         (DataInstanceHandle containerHandle) const final override;
        [[nodiscard]] UInt32                      getDataInstanceCount            () const final override;
        [[nodiscard]] DataLayoutHandle            getLayoutOfDataInstance         (DataInstanceHandle containerHandle) const final override;
        [[nodiscard]] const DataInstanceMemoryPool&           getDataInstances                () const;

        [[nodiscard]] const Float*                getDataFloatArray               (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] const Vector2*              getDataVector2fArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] const Vector3*              getDataVector3fArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] const Vector4*              getDataVector4fArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] const Int32*                getDataIntegerArray             (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] const Matrix22f*            getDataMatrix22fArray           (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] const Matrix33f*            getDataMatrix33fArray           (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] const Matrix44f*            getDataMatrix44fArray           (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] const Vector2i*             getDataVector2iArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] const Vector3i*             getDataVector3iArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] const Vector4i*             getDataVector4iArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] const ResourceField&        getDataResource                 (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] TextureSamplerHandle        getDataTextureSamplerHandle     (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] DataInstanceHandle          getDataReference                (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;

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
        [[nodiscard]] Float                       getDataSingleFloat              (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] const Vector2&              getDataSingleVector2f           (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] const Vector3&              getDataSingleVector3f           (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] const Vector4&              getDataSingleVector4f           (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] Int32                       getDataSingleInteger            (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] const Matrix22f&            getDataSingleMatrix22f          (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] const Matrix33f&            getDataSingleMatrix33f          (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] const Matrix44f&            getDataSingleMatrix44f          (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] const Vector2i&             getDataSingleVector2i           (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] const Vector3i&             getDataSingleVector3i           (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;
        [[nodiscard]] const Vector4i&             getDataSingleVector4i           (DataInstanceHandle containerHandle, DataFieldHandle field) const final override;

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

        // Texture sampler
        TextureSamplerHandle        allocateTextureSampler          (const TextureSampler& sampler, TextureSamplerHandle handle = TextureSamplerHandle::Invalid()) override;
        void                        releaseTextureSampler           (TextureSamplerHandle handle) override;
        [[nodiscard]] bool                        isTextureSamplerAllocated       (TextureSamplerHandle handle) const final override;
        [[nodiscard]] UInt32                      getTextureSamplerCount          () const final override;
        [[nodiscard]] const TextureSampler&       getTextureSampler               (TextureSamplerHandle handle) const final override;
        [[nodiscard]] const TextureSamplerMemoryPool&         getTextureSamplers              () const;

        // Render groups
        RenderGroupHandle       allocateRenderGroup             (UInt32 renderableCount = 0u, UInt32 nestedGroupCount = 0u, RenderGroupHandle groupHandle = RenderGroupHandle::Invalid()) override;
        void                    releaseRenderGroup              (RenderGroupHandle groupHandle) override;
        [[nodiscard]] bool                    isRenderGroupAllocated          (RenderGroupHandle groupHandle) const final override;
        [[nodiscard]] UInt32                  getRenderGroupCount             () const final override;
        void                    addRenderableToRenderGroup      (RenderGroupHandle groupHandle, RenderableHandle renderableHandle, Int32 order) override;
        void                    removeRenderableFromRenderGroup (RenderGroupHandle groupHandle, RenderableHandle renderableHandle) override;
        void                    addRenderGroupToRenderGroup     (RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, Int32 order) override;
        void                    removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild) override;
        [[nodiscard]] const RenderGroup&      getRenderGroup                  (RenderGroupHandle groupHandle) const final override;
        [[nodiscard]] const RenderGroupMemoryPool&    getRenderGroups                 () const;

        //Render pass
        RenderPassHandle        allocateRenderPass              (UInt32 renderGroupCount = 0u, RenderPassHandle passHandle = RenderPassHandle::Invalid()) override;
        void                    releaseRenderPass               (RenderPassHandle passHandle) override;
        [[nodiscard]] bool                    isRenderPassAllocated           (RenderPassHandle pass) const final override;
        [[nodiscard]] UInt32                  getRenderPassCount              () const final override;
        void                    setRenderPassClearColor         (RenderPassHandle passHandle, const Vector4& clearColor) override;
        void                    setRenderPassClearFlag          (RenderPassHandle passHandle, UInt32 clearFlag) override;
        void                    setRenderPassCamera             (RenderPassHandle passHandle, CameraHandle cameraHandle) override;
        void                    setRenderPassRenderTarget       (RenderPassHandle passHandle, RenderTargetHandle targetHandle) override;
        void                    setRenderPassRenderOrder        (RenderPassHandle passHandle, Int32 renderOrder) override;
        void                    setRenderPassEnabled            (RenderPassHandle passHandle, bool isEnabled) override;
        void                    setRenderPassRenderOnce         (RenderPassHandle passHandle, bool enable) override;
        void                    retriggerRenderPassRenderOnce   (RenderPassHandle passHandle) override;
        void                    addRenderGroupToRenderPass      (RenderPassHandle passHandle, RenderGroupHandle groupHandle, Int32 order) override;
        void                    removeRenderGroupFromRenderPass (RenderPassHandle passHandle, RenderGroupHandle groupHandle) override;
        [[nodiscard]] const RenderPass&       getRenderPass                   (RenderPassHandle passHandle) const final override;
        [[nodiscard]] const RenderPassMemoryPool&     getRenderPasses                 () const;

        //Blit pass
        BlitPassHandle          allocateBlitPass                (RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle = BlitPassHandle::Invalid()) override;
        void                    releaseBlitPass                 (BlitPassHandle passHandle) override;
        [[nodiscard]] bool                    isBlitPassAllocated             (BlitPassHandle passHandle) const final override;
        [[nodiscard]] UInt32                  getBlitPassCount                () const final override;
        void                    setBlitPassRenderOrder          (BlitPassHandle passHandle, Int32 renderOrder) override;
        void                    setBlitPassEnabled              (BlitPassHandle passHandle, bool isEnabled) override;
        void                    setBlitPassRegions              (BlitPassHandle passHandle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion) override;
        [[nodiscard]] const BlitPass&         getBlitPass                     (BlitPassHandle passHandle) const final override;
        [[nodiscard]] const BlitPassMemoryPool&       getBlitPasses                   () const;

        //Pickable object
        PickableObjectHandle    allocatePickableObject          (DataBufferHandle geometryHandle, NodeHandle nodeHandle, PickableObjectId id, PickableObjectHandle pickableHandle = PickableObjectHandle::Invalid()) override;
        void                    releasePickableObject           (PickableObjectHandle pickableHandle) override;
        [[nodiscard]] bool                    isPickableObjectAllocated       (PickableObjectHandle pickableHandle) const final override;
        [[nodiscard]] UInt32                  getPickableObjectCount          () const final override;
        void                    setPickableObjectId             (PickableObjectHandle pickableHandle, PickableObjectId id) override;
        void                    setPickableObjectCamera         (PickableObjectHandle pickableHandle, CameraHandle cameraHandle) override;
        void                    setPickableObjectEnabled        (PickableObjectHandle pickableHandle, bool isEnabled) override;
        [[nodiscard]] const PickableObject&   getPickableObject               (PickableObjectHandle pickableHandle) const final override;
        [[nodiscard]] const PickableObjectMemoryPool& getPickableObjects              () const;

        // Render targets
        RenderTargetHandle      allocateRenderTarget            (RenderTargetHandle targetHandle = RenderTargetHandle::Invalid()) override;
        void                    releaseRenderTarget             (RenderTargetHandle targetHandle) override;
        [[nodiscard]] bool                    isRenderTargetAllocated         (RenderTargetHandle targetHandle) const final override;
        [[nodiscard]] UInt32                  getRenderTargetCount            () const final override;
        void                    addRenderTargetRenderBuffer     (RenderTargetHandle targetHandle, RenderBufferHandle bufferHandle) override;
        [[nodiscard]] UInt32                  getRenderTargetRenderBufferCount(RenderTargetHandle targetHandle) const final override;
        [[nodiscard]] RenderBufferHandle      getRenderTargetRenderBuffer     (RenderTargetHandle targetHandle, UInt32 bufferIndex) const final override;
        [[nodiscard]] const RenderTargetMemoryPool&   getRenderTargets                () const;

        // Render buffers
        RenderBufferHandle      allocateRenderBuffer            (const RenderBuffer& renderBuffer, RenderBufferHandle handle = RenderBufferHandle::Invalid()) override;
        void                    releaseRenderBuffer             (RenderBufferHandle handle) override;
        [[nodiscard]] bool                    isRenderBufferAllocated         (RenderBufferHandle handle) const final override;
        [[nodiscard]] UInt32                  getRenderBufferCount            () const final override;
        [[nodiscard]] const RenderBuffer&     getRenderBuffer                 (RenderBufferHandle handle) const final override;
        [[nodiscard]] const RenderBufferMemoryPool&   getRenderBuffers                () const;

        // Data buffers
        DataBufferHandle        allocateDataBuffer              (EDataBufferType dataBufferType, EDataType dataType, UInt32 maximumSizeInBytes, DataBufferHandle handle = DataBufferHandle::Invalid()) override;
        void                    releaseDataBuffer               (DataBufferHandle handle) override;
        [[nodiscard]] UInt32                  getDataBufferCount              () const final override;
        void                    updateDataBuffer                (DataBufferHandle handle, UInt32 offsetInBytes, UInt32 dataSizeInBytes, const Byte* data) override;
        [[nodiscard]] bool                    isDataBufferAllocated           (DataBufferHandle handle) const final override;
        [[nodiscard]] const GeometryDataBuffer& getDataBuffer                 (DataBufferHandle handle) const final override;
        [[nodiscard]] const DataBufferMemoryPool&      getDataBuffers                  () const;

        //Texture buffers
        TextureBufferHandle     allocateTextureBuffer           (ETextureFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle = TextureBufferHandle::Invalid()) override;
        void                    releaseTextureBuffer            (TextureBufferHandle handle) override;
        [[nodiscard]] bool                    isTextureBufferAllocated        (TextureBufferHandle handle) const final override;
        [[nodiscard]] UInt32                  getTextureBufferCount           () const final override;
        void                    updateTextureBuffer             (TextureBufferHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data) override;
        [[nodiscard]] const TextureBuffer&    getTextureBuffer                (TextureBufferHandle handle) const final override;
        [[nodiscard]] const TextureBufferMemoryPool&      getTextureBuffers               () const;

        DataSlotHandle          allocateDataSlot                (const DataSlot& dataSlot, DataSlotHandle handle = DataSlotHandle::Invalid()) override;
        void                    releaseDataSlot                 (DataSlotHandle handle) override;
        void                    setDataSlotTexture              (DataSlotHandle handle, const ResourceContentHash& texture) override;
        [[nodiscard]] bool                    isDataSlotAllocated             (DataSlotHandle handle) const final override;
        [[nodiscard]] UInt32                  getDataSlotCount                () const final override;
        [[nodiscard]] const DataSlot&         getDataSlot                     (DataSlotHandle handle) const final override;
        [[nodiscard]] const DataSlotMemoryPool&       getDataSlots                    () const;

        SceneReferenceHandle    allocateSceneReference          (SceneId sceneId, SceneReferenceHandle handle = {}) override;
        void                    releaseSceneReference           (SceneReferenceHandle handle) override;
        void                    requestSceneReferenceState      (SceneReferenceHandle handle, RendererSceneState state) override;
        void                    requestSceneReferenceFlushNotifications(SceneReferenceHandle handle, bool enable) override;
        void                    setSceneReferenceRenderOrder    (SceneReferenceHandle handle, int32_t renderOrder) override;
        [[nodiscard]] bool                    isSceneReferenceAllocated       (SceneReferenceHandle handle) const final override;
        [[nodiscard]] UInt32                  getSceneReferenceCount          () const final override;
        [[nodiscard]] const SceneReference&   getSceneReference               (SceneReferenceHandle handle) const final override;
        [[nodiscard]] const SceneReferenceMemoryPool& getSceneReferences              () const;

        [[nodiscard]] SceneSizeInformation    getSceneSizeInformation         () const final  override;

    protected:
        [[nodiscard]] const TopologyNode&             getNode                         (NodeHandle handle) const;
        TextureSampler&                 getTextureSamplerInternal       (TextureSamplerHandle handle);
        RenderPass&                     getRenderPassInternal           (RenderPassHandle handle);
        RenderGroup&                    getRenderGroupInternal          (RenderGroupHandle handle);
        [[nodiscard]] const TopologyTransform&        getTransform                    (TransformHandle handle) const;

    private:
        template <typename TYPE>
        const TYPE* getInstanceDataInternal(DataInstanceHandle dataInstanceHandle, DataFieldHandle fieldId) const;
        template <typename TYPE>
        void setInstanceDataInternal(DataInstanceHandle dataInstanceHandle, DataFieldHandle fieldId, UInt32 elementCount, const TYPE* newValue);

        NodeMemoryPool              m_nodes;
        CameraMemoryPool            m_cameras;
        RenderableMemoryPool        m_renderables;
        RenderStateMemoryPool       m_states;
        TransformMemoryPool         m_transforms;
        DataLayoutMemoryPool        m_dataLayoutMemory;
        DataInstanceMemoryPool      m_dataInstanceMemory;
        RenderGroupMemoryPool       m_renderGroups;
        RenderPassMemoryPool        m_renderPasses;
        BlitPassMemoryPool          m_blitPasses;
        PickableObjectMemoryPool    m_pickableObjects;
        RenderTargetMemoryPool      m_renderTargets;
        RenderBufferMemoryPool      m_renderBuffers;
        TextureSamplerMemoryPool    m_textureSamplers;
        DataBufferMemoryPool        m_dataBuffers;
        TextureBufferMemoryPool     m_textureBuffers;
        DataSlotMemoryPool          m_dataSlots;
        SceneReferenceMemoryPool    m_sceneReferences;

        const String                m_name;
        const SceneId               m_sceneId;

        FlushTime::Clock::time_point m_effectTimeSync;
    };

    template <template<typename, typename> class MEMORYPOOL>
    RenderPass& SceneT<MEMORYPOOL>::getRenderPassInternal(RenderPassHandle handle)
    {
        return *m_renderPasses.getMemory(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    RenderGroup& SceneT<MEMORYPOOL>::getRenderGroupInternal(RenderGroupHandle handle)
    {
        return *m_renderGroups.getMemory(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    template <typename TYPE>
    [[nodiscard]] const TYPE* SceneT<MEMORYPOOL>::getInstanceDataInternal(DataInstanceHandle dataInstanceHandle, DataFieldHandle fieldId) const
    {
        const DataInstance* dataInstance = m_dataInstanceMemory.getMemory(dataInstanceHandle);
        const DataLayout* dataLayout = m_dataLayoutMemory.getMemory(dataInstance->getLayoutHandle());
        assert(TypeMatchesEDataType<TYPE>(dataLayout->getField(fieldId).dataType));
        return dataInstance->getTypedDataPointer<TYPE>(dataLayout->getFieldOffset(fieldId));
    }

    template <template<typename, typename> class MEMORYPOOL>
    template <typename TYPE>
    void SceneT<MEMORYPOOL>::setInstanceDataInternal(DataInstanceHandle dataInstanceHandle, DataFieldHandle fieldId, UInt32 elementCount, const TYPE* newValue)
    {
        DataInstance* dataInstance = m_dataInstanceMemory.getMemory(dataInstanceHandle);
        const DataLayout* dataLayout = m_dataLayoutMemory.getMemory(dataInstance->getLayoutHandle());
        assert(TypeMatchesEDataType<TYPE>(dataLayout->getField(fieldId).dataType));
        assert(elementCount == dataLayout->getField(fieldId).elementCount);
        dataInstance->setTypedData<TYPE>(dataLayout->getFieldOffset(fieldId), elementCount, newValue);
    }

    // SceneT::is*Allocated calls are inlined because heavily used. Allow devirtualization and inlining together with final
    template <template<typename, typename> class MEMORYPOOL>
    inline bool SceneT<MEMORYPOOL>::isCameraAllocated(CameraHandle handle) const
    {
        return m_cameras.isAllocated(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline bool SceneT<MEMORYPOOL>::isRenderPassAllocated(RenderPassHandle pass) const
    {
        return m_renderPasses.isAllocated(pass);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline bool SceneT<MEMORYPOOL>::isRenderTargetAllocated(RenderTargetHandle targetHandle) const
    {
        return m_renderTargets.isAllocated(targetHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline bool SceneT<MEMORYPOOL>::isRenderBufferAllocated(RenderBufferHandle handle) const
    {
        return m_renderBuffers.isAllocated(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline bool SceneT<MEMORYPOOL>::isDataBufferAllocated(DataBufferHandle handle) const
    {
        return m_dataBuffers.isAllocated(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline bool SceneT<MEMORYPOOL>::isTextureBufferAllocated(TextureBufferHandle handle) const
    {
        return m_textureBuffers.isAllocated(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline bool SceneT<MEMORYPOOL>::isDataSlotAllocated(DataSlotHandle handle) const
    {
        return m_dataSlots.isAllocated(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline bool SceneT<MEMORYPOOL>::isBlitPassAllocated(BlitPassHandle passHandle) const
    {
        return m_blitPasses.isAllocated(passHandle);
    }

    template <template <typename, typename> class MEMORYPOOL>
    inline bool SceneT<MEMORYPOOL>::isPickableObjectAllocated(PickableObjectHandle pickableHandle) const
    {
        return m_pickableObjects.isAllocated(pickableHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline bool SceneT<MEMORYPOOL>::isRenderStateAllocated(RenderStateHandle stateHandle) const
    {
        return m_states.isAllocated(stateHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline bool SceneT<MEMORYPOOL>::isTextureSamplerAllocated(TextureSamplerHandle handle) const
    {
        return m_textureSamplers.isAllocated(handle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline bool SceneT<MEMORYPOOL>::isRenderGroupAllocated(RenderGroupHandle groupHandle) const
    {
        return m_renderGroups.isAllocated(groupHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline bool SceneT<MEMORYPOOL>::isDataInstanceAllocated(DataInstanceHandle containerHandle) const
    {
        return m_dataInstanceMemory.isAllocated(containerHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline bool SceneT<MEMORYPOOL>::isDataLayoutAllocated(DataLayoutHandle layoutHandle) const
    {
        return m_dataLayoutMemory.isAllocated(layoutHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline bool SceneT<MEMORYPOOL>::isRenderableAllocated(RenderableHandle renderableHandle) const
    {
        return m_renderables.isAllocated(renderableHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline bool SceneT<MEMORYPOOL>::isTransformAllocated(TransformHandle transformHandle) const
    {
        return m_transforms.isAllocated(transformHandle);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline bool SceneT<MEMORYPOOL>::isNodeAllocated(NodeHandle node) const
    {
        return m_nodes.isAllocated(node);
    }

    template <template<typename, typename> class MEMORYPOOL>
    bool SceneT<MEMORYPOOL>::isSceneReferenceAllocated(SceneReferenceHandle handle) const
    {
        return m_sceneReferences.isAllocated(handle);
    }

    // inline often called getters
    template <template<typename, typename> class MEMORYPOOL>
    inline SceneId SceneT<MEMORYPOOL>::getSceneId() const
    {
        return m_sceneId;
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline FlushTime::Clock::time_point SceneT<MEMORYPOOL>::getEffectTimeSync() const
    {
        return m_effectTimeSync;
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline const DataLayout& SceneT<MEMORYPOOL>::getDataLayout(DataLayoutHandle layoutHandle) const
    {
        return *m_dataLayoutMemory.getMemory(layoutHandle);
    }


    template <template<typename, typename> class MEMORYPOOL>
    inline
    const typename SceneT<MEMORYPOOL>::RenderableMemoryPool& SceneT<MEMORYPOOL>::getRenderables() const
    {
        return m_renderables;
    }


    template <template<typename, typename> class MEMORYPOOL>
    inline
    const typename SceneT<MEMORYPOOL>::RenderStateMemoryPool& SceneT<MEMORYPOOL>::getRenderStates() const
    {
        return m_states;
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline
    const typename SceneT<MEMORYPOOL>::CameraMemoryPool& SceneT<MEMORYPOOL>::getCameras() const
    {
        return m_cameras;
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline
    const typename SceneT<MEMORYPOOL>::NodeMemoryPool& SceneT<MEMORYPOOL>::getNodes() const
    {
        return m_nodes;
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline
    const typename SceneT<MEMORYPOOL>::TransformMemoryPool& SceneT<MEMORYPOOL>::getTransforms() const
    {
        return m_transforms;
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline
    const typename SceneT<MEMORYPOOL>::DataLayoutMemoryPool& SceneT<MEMORYPOOL>::getDataLayouts() const
    {
        return m_dataLayoutMemory;
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline
    const typename SceneT<MEMORYPOOL>::DataInstanceMemoryPool& SceneT<MEMORYPOOL>::getDataInstances() const
    {
        return m_dataInstanceMemory;
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline
    const typename SceneT<MEMORYPOOL>::TextureSamplerMemoryPool& SceneT<MEMORYPOOL>::getTextureSamplers() const
    {
        return m_textureSamplers;
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline
    const typename SceneT<MEMORYPOOL>::RenderGroupMemoryPool& SceneT<MEMORYPOOL>::getRenderGroups() const
    {
        return m_renderGroups;
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline
    const typename SceneT<MEMORYPOOL>::RenderPassMemoryPool& SceneT<MEMORYPOOL>::getRenderPasses() const
    {
        return m_renderPasses;
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline
    const typename SceneT<MEMORYPOOL>::BlitPassMemoryPool& SceneT<MEMORYPOOL>::getBlitPasses() const
    {
        return m_blitPasses;
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline
    const typename SceneT<MEMORYPOOL>::PickableObjectMemoryPool& SceneT<MEMORYPOOL>::getPickableObjects() const
    {
        return m_pickableObjects;
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline
    const typename SceneT<MEMORYPOOL>::RenderTargetMemoryPool& SceneT<MEMORYPOOL>::getRenderTargets() const
    {
        return m_renderTargets;
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline
    const typename SceneT<MEMORYPOOL>::RenderBufferMemoryPool& SceneT<MEMORYPOOL>::getRenderBuffers() const
    {
        return m_renderBuffers;
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline
    const typename SceneT<MEMORYPOOL>::DataBufferMemoryPool& SceneT<MEMORYPOOL>::getDataBuffers() const
    {
        return m_dataBuffers;
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline
    const typename SceneT<MEMORYPOOL>::TextureBufferMemoryPool& SceneT<MEMORYPOOL>::getTextureBuffers() const
    {
        return m_textureBuffers;
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline
    const typename SceneT<MEMORYPOOL>::DataSlotMemoryPool& SceneT<MEMORYPOOL>::getDataSlots() const
    {
        return m_dataSlots;
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline
    const typename SceneT<MEMORYPOOL>::SceneReferenceMemoryPool& SceneT<MEMORYPOOL>::getSceneReferences() const
    {
        return m_sceneReferences;
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline const Float* SceneT<MEMORYPOOL>::getDataFloatArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Float>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline const Matrix22f* SceneT<MEMORYPOOL>::getDataMatrix22fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Matrix22f>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline const Matrix33f* SceneT<MEMORYPOOL>::getDataMatrix33fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Matrix33f>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline const Matrix44f* SceneT<MEMORYPOOL>::getDataMatrix44fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Matrix44f>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline const Vector2* SceneT<MEMORYPOOL>::getDataVector2fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Vector2>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline const Vector3* SceneT<MEMORYPOOL>::getDataVector3fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Vector3>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline const Vector4* SceneT<MEMORYPOOL>::getDataVector4fArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Vector4>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline const Int32* SceneT<MEMORYPOOL>::getDataIntegerArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Int32>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline const Vector2i* SceneT<MEMORYPOOL>::getDataVector2iArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Vector2i>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline const Vector3i* SceneT<MEMORYPOOL>::getDataVector3iArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Vector3i>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline const Vector4i* SceneT<MEMORYPOOL>::getDataVector4iArray(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return getInstanceDataInternal<Vector4i>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline TextureSamplerHandle SceneT<MEMORYPOOL>::getDataTextureSamplerHandle(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return *getInstanceDataInternal<TextureSamplerHandle>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline DataInstanceHandle SceneT<MEMORYPOOL>::getDataReference(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return *getInstanceDataInternal<DataInstanceHandle>(containerHandle, fieldId);
    }

    template <template<typename, typename> class MEMORYPOOL>
    inline const ResourceField& SceneT<MEMORYPOOL>::getDataResource(DataInstanceHandle containerHandle, DataFieldHandle fieldId) const
    {
        return *getInstanceDataInternal<ResourceField>(containerHandle, fieldId);
    }
}

#endif
