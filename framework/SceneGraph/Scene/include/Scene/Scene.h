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
#include "SceneAPI/StreamTexture.h"
#include "SceneAPI/RenderBuffer.h"
#include "SceneAPI/RenderTarget.h"
#include "SceneAPI/BlitPass.h"
#include "SceneAPI/PickableObject.h"
#include "SceneAPI/SceneReference.h"
#include "AnimationAPI/IAnimationSystem.h"

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
        using StreamTextureMemoryPool   = MEMORYPOOL<StreamTexture      , StreamTextureHandle>;
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
        using AnimationSystemMemoryPool = MEMORYPOOL<IAnimationSystem*  , AnimationSystemHandle>;

        explicit SceneT(const SceneInfo& sceneInfo = SceneInfo());
        virtual ~SceneT() override;

        virtual void                        preallocateSceneSize            (const SceneSizeInformation& sizeInfo) override;

        virtual SceneId                     getSceneId                      () const final override;
        virtual const String&               getName                         () const final override;

        // Renderables
        virtual RenderableHandle            allocateRenderable              (NodeHandle nodeHandle, RenderableHandle handle = RenderableHandle::Invalid()) override;
        virtual void                        releaseRenderable               (RenderableHandle renderableHandle) override;
        virtual bool                        isRenderableAllocated           (RenderableHandle renderableHandle) const override final;
        virtual UInt32                      getRenderableCount              () const override final;
        virtual void                        setRenderableDataInstance       (RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance) override;
        virtual void                        setRenderableStartIndex         (RenderableHandle renderableHandle, UInt32 startIndex) override;
        virtual void                        setRenderableIndexCount         (RenderableHandle renderableHandle, UInt32 indexCount) override;
        virtual void                        setRenderableRenderState        (RenderableHandle renderableHandle, RenderStateHandle stateHandle) override;
        virtual void                        setRenderableVisibility         (RenderableHandle renderableHandle, EVisibilityMode visibility) override;
        virtual void                        setRenderableInstanceCount      (RenderableHandle renderableHandle, UInt32 instanceCount) override;
        virtual void                        setRenderableStartVertex        (RenderableHandle renderableHandle, UInt32 startVertex) override;
        virtual const Renderable&           getRenderable                   (RenderableHandle renderableHandle) const override final;
        const RenderableMemoryPool&         getRenderables                  () const;

        // Render state
        virtual RenderStateHandle           allocateRenderState             (RenderStateHandle stateHandle = RenderStateHandle::Invalid()) override;
        virtual void                        releaseRenderState              (RenderStateHandle stateHandle) override;
        virtual bool                        isRenderStateAllocated          (RenderStateHandle stateHandle) const override final;
        virtual UInt32                      getRenderStateCount             () const override final;
        virtual void                        setRenderStateBlendFactors      (RenderStateHandle stateHandle, EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha) override;
        virtual void                        setRenderStateBlendOperations   (RenderStateHandle stateHandle, EBlendOperation operationColor, EBlendOperation operationAlpha) override;
        virtual void                        setRenderStateBlendColor        (RenderStateHandle stateHandle, const Vector4& color) override;
        virtual void                        setRenderStateCullMode          (RenderStateHandle stateHandle, ECullMode cullMode) override;
        virtual void                        setRenderStateDrawMode          (RenderStateHandle stateHandle, EDrawMode drawMode) override;
        virtual void                        setRenderStateDepthFunc         (RenderStateHandle stateHandle, EDepthFunc func) override;
        virtual void                        setRenderStateDepthWrite        (RenderStateHandle stateHandle, EDepthWrite flag) override;
        virtual void                        setRenderStateScissorTest       (RenderStateHandle stateHandle, EScissorTest flag, const RenderState::ScissorRegion& region) override;
        virtual void                        setRenderStateStencilFunc       (RenderStateHandle stateHandle, EStencilFunc func, UInt8 ref, UInt8 mask) override;
        virtual void                        setRenderStateStencilOps        (RenderStateHandle stateHandle, EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass) override;
        virtual void                        setRenderStateColorWriteMask    (RenderStateHandle stateHandle, ColorWriteMask colorMask) override;
        virtual const RenderState&          getRenderState                  (RenderStateHandle stateHandle) const override final;
        const RenderStateMemoryPool&              getRenderStates                 () const;

        // Camera
        virtual CameraHandle                allocateCamera                  (ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle dataInstance, CameraHandle handle = CameraHandle::Invalid()) override;
        virtual void                        releaseCamera                   (CameraHandle cameraHandle) override;
        virtual bool                        isCameraAllocated               (CameraHandle handle) const override final;
        virtual UInt32                      getCameraCount                  () const override final;
        virtual const Camera&               getCamera                       (CameraHandle cameraHandle) const override final;
        const CameraMemoryPool&             getCameras                      () const;

        // Nodes
        virtual NodeHandle                  allocateNode                    (UInt32 childrenCount = 0u, NodeHandle handle = NodeHandle::Invalid()) override;
        virtual void                        releaseNode                     (NodeHandle nodeHandle) override;
        virtual bool                        isNodeAllocated                 (NodeHandle node) const override final;
        virtual UInt32                      getNodeCount                    () const override final;
        virtual NodeHandle                  getParent                       (NodeHandle nodeHandle) const override final;
        virtual void                        addChildToNode                  (NodeHandle parent, NodeHandle child) override;
        virtual void                        removeChildFromNode             (NodeHandle parent, NodeHandle child) override;
        virtual UInt32                      getChildCount                   (NodeHandle parent) const override final;
        virtual NodeHandle                  getChild                        (NodeHandle parent, UInt32 childNumber) const override final;
        const NodeMemoryPool&                   getNodes                        () const;

        // Transformation
        virtual TransformHandle             allocateTransform               (NodeHandle nodeHandle, TransformHandle handle = TransformHandle::Invalid()) override;
        virtual void                        releaseTransform                (TransformHandle transform) override;
        virtual UInt32                      getTransformCount               () const override final;
        virtual bool                        isTransformAllocated            (TransformHandle transformHandle) const override final;
        virtual NodeHandle                  getTransformNode                (TransformHandle handle) const override final;
        virtual const Vector3&              getTranslation                  (TransformHandle handle) const override final;
        virtual const Vector3&              getRotation                     (TransformHandle handle) const override final;
        virtual ERotationConvention         getRotationConvention           (TransformHandle handle) const override final;
        virtual const Vector3&              getScaling                      (TransformHandle handle) const override final;
        virtual void                        setTranslation                  (TransformHandle handle, const Vector3& translation) override;
        virtual void                        setRotation                     (TransformHandle handle, const Vector3& rotation, ERotationConvention convention) override;
        virtual void                        setRotationForAnimation         (TransformHandle handle, const Vector3& rotation) override final;
        virtual void                        setScaling                      (TransformHandle handle, const Vector3& scaling) override;
        const TransformMemoryPool&              getTransforms                   () const;

        virtual DataLayoutHandle            allocateDataLayout              (const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutHandle handle = DataLayoutHandle::Invalid()) override;
        virtual void                        releaseDataLayout               (DataLayoutHandle layoutHandle) override;
        virtual bool                        isDataLayoutAllocated           (DataLayoutHandle layoutHandle) const override final;
        virtual UInt32                      getDataLayoutCount              () const override final;
        virtual const DataLayout&           getDataLayout                   (DataLayoutHandle layoutHandle) const override final;
        const DataLayoutMemoryPool&             getDataLayouts                  () const;

        virtual DataInstanceHandle          allocateDataInstance            (DataLayoutHandle finishedLayoutHandle, DataInstanceHandle instanceHandle = DataInstanceHandle::Invalid()) override;
        virtual void                        releaseDataInstance             (DataInstanceHandle containerHandle) override;
        virtual bool                        isDataInstanceAllocated         (DataInstanceHandle containerHandle) const override final;
        virtual UInt32                      getDataInstanceCount            () const override final;
        virtual DataLayoutHandle            getLayoutOfDataInstance         (DataInstanceHandle containerHandle) const override final;
        const DataInstanceMemoryPool&           getDataInstances                () const;

        virtual const Float*                getDataFloatArray               (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual const Vector2*              getDataVector2fArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual const Vector3*              getDataVector3fArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual const Vector4*              getDataVector4fArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual const Int32*                getDataIntegerArray             (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual const Matrix22f*            getDataMatrix22fArray           (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual const Matrix33f*            getDataMatrix33fArray           (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual const Matrix44f*            getDataMatrix44fArray           (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual const Vector2i*             getDataVector2iArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual const Vector3i*             getDataVector3iArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual const Vector4i*             getDataVector4iArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual const ResourceField&        getDataResource                 (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual TextureSamplerHandle        getDataTextureSamplerHandle     (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual DataInstanceHandle          getDataReference                (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;

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
        virtual Float                       getDataSingleFloat              (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual const Vector2&              getDataSingleVector2f           (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual const Vector3&              getDataSingleVector3f           (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual const Vector4&              getDataSingleVector4f           (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual Int32                       getDataSingleInteger            (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual const Matrix22f&            getDataSingleMatrix22f          (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual const Matrix33f&            getDataSingleMatrix33f          (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual const Matrix44f&            getDataSingleMatrix44f          (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual const Vector2i&             getDataSingleVector2i           (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual const Vector3i&             getDataSingleVector3i           (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;
        virtual const Vector4i&             getDataSingleVector4i           (DataInstanceHandle containerHandle, DataFieldHandle field) const override final;

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

        // Texture sampler
        virtual TextureSamplerHandle        allocateTextureSampler          (const TextureSampler& sampler, TextureSamplerHandle handle = TextureSamplerHandle::Invalid()) override;
        virtual void                        releaseTextureSampler           (TextureSamplerHandle handle) override;
        virtual bool                        isTextureSamplerAllocated       (TextureSamplerHandle handle) const override final;
        virtual UInt32                      getTextureSamplerCount          () const override final;
        virtual const TextureSampler&       getTextureSampler               (TextureSamplerHandle handle) const override final;
        const TextureSamplerMemoryPool&         getTextureSamplers              () const;

        //Animation system
        virtual AnimationSystemHandle       addAnimationSystem              (IAnimationSystem* animationSystem, AnimationSystemHandle externalHandle = AnimationSystemHandle::Invalid()) override;
        virtual void                        removeAnimationSystem           (AnimationSystemHandle animSystemHandle) override;
        virtual IAnimationSystem*           getAnimationSystem              (AnimationSystemHandle animSystemHandle) override final;
        virtual const IAnimationSystem*     getAnimationSystem              (AnimationSystemHandle animSystemHandle) const override final;
        virtual bool                        isAnimationSystemAllocated      (AnimationSystemHandle animSystemHandle) const override final;
        virtual UInt32                      getAnimationSystemCount         () const override final;
        const AnimationSystemMemoryPool&    getAnimationSystems             () const;

        // Render groups
        virtual RenderGroupHandle       allocateRenderGroup             (UInt32 renderableCount = 0u, UInt32 nestedGroupCount = 0u, RenderGroupHandle groupHandle = RenderGroupHandle::Invalid()) override;
        virtual void                    releaseRenderGroup              (RenderGroupHandle groupHandle) override;
        virtual bool                    isRenderGroupAllocated          (RenderGroupHandle groupHandle) const override final;
        virtual UInt32                  getRenderGroupCount             () const override final;
        virtual void                    addRenderableToRenderGroup      (RenderGroupHandle groupHandle, RenderableHandle renderableHandle, Int32 order) override;
        virtual void                    removeRenderableFromRenderGroup (RenderGroupHandle groupHandle, RenderableHandle renderableHandle) override;
        virtual void                    addRenderGroupToRenderGroup     (RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, Int32 order) override;
        virtual void                    removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild) override;
        virtual const RenderGroup&      getRenderGroup                  (RenderGroupHandle groupHandle) const override final;
        const RenderGroupMemoryPool&    getRenderGroups                 () const;

        //Render pass
        virtual RenderPassHandle        allocateRenderPass              (UInt32 renderGroupCount = 0u, RenderPassHandle passHandle = RenderPassHandle::Invalid()) override;
        virtual void                    releaseRenderPass               (RenderPassHandle passHandle) override;
        virtual bool                    isRenderPassAllocated           (RenderPassHandle pass) const override final;
        virtual UInt32                  getRenderPassCount              () const override final;
        virtual void                    setRenderPassClearColor         (RenderPassHandle passHandle, const Vector4& clearColor) override;
        virtual void                    setRenderPassClearFlag          (RenderPassHandle passHandle, UInt32 clearFlag) override;
        virtual void                    setRenderPassCamera             (RenderPassHandle passHandle, CameraHandle cameraHandle) override;
        virtual void                    setRenderPassRenderTarget       (RenderPassHandle passHandle, RenderTargetHandle targetHandle) override;
        virtual void                    setRenderPassRenderOrder        (RenderPassHandle passHandle, Int32 renderOrder) override;
        virtual void                    setRenderPassEnabled            (RenderPassHandle passHandle, bool isEnabled) override;
        virtual void                    setRenderPassRenderOnce         (RenderPassHandle passHandle, bool enable) override;
        virtual void                    retriggerRenderPassRenderOnce   (RenderPassHandle passHandle) override;
        virtual void                    addRenderGroupToRenderPass      (RenderPassHandle passHandle, RenderGroupHandle groupHandle, Int32 order) override;
        virtual void                    removeRenderGroupFromRenderPass (RenderPassHandle passHandle, RenderGroupHandle groupHandle) override;
        virtual const RenderPass&       getRenderPass                   (RenderPassHandle passHandle) const override final;
        const RenderPassMemoryPool&     getRenderPasses                 () const;

        //Blit pass
        virtual BlitPassHandle          allocateBlitPass                (RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle = BlitPassHandle::Invalid()) override;
        virtual void                    releaseBlitPass                 (BlitPassHandle passHandle) override;
        virtual bool                    isBlitPassAllocated             (BlitPassHandle passHandle) const override final;
        virtual UInt32                  getBlitPassCount                () const override final;
        virtual void                    setBlitPassRenderOrder          (BlitPassHandle passHandle, Int32 renderOrder) override;
        virtual void                    setBlitPassEnabled              (BlitPassHandle passHandle, bool isEnabled) override;
        virtual void                    setBlitPassRegions              (BlitPassHandle passHandle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion) override;
        virtual const BlitPass&         getBlitPass                     (BlitPassHandle passHandle) const override final;
        const BlitPassMemoryPool&       getBlitPasses                   () const;

        //Pickable object
        virtual PickableObjectHandle    allocatePickableObject          (DataBufferHandle geometryHandle, NodeHandle nodeHandle, PickableObjectId id, PickableObjectHandle pickableHandle = PickableObjectHandle::Invalid()) override;
        virtual void                    releasePickableObject           (PickableObjectHandle pickableHandle) override;
        virtual bool                    isPickableObjectAllocated       (PickableObjectHandle pickableHandle) const override final;
        virtual UInt32                  getPickableObjectCount          () const override final;
        virtual void                    setPickableObjectId             (PickableObjectHandle pickableHandle, PickableObjectId id) override;
        virtual void                    setPickableObjectCamera         (PickableObjectHandle pickableHandle, CameraHandle cameraHandle) override;
        virtual void                    setPickableObjectEnabled        (PickableObjectHandle pickableHandle, bool isEnabled) override;
        virtual const PickableObject&   getPickableObject               (PickableObjectHandle pickableHandle) const override final;
        const PickableObjectMemoryPool& getPickableObjects              () const;

        // Render targets
        virtual RenderTargetHandle      allocateRenderTarget            (RenderTargetHandle targetHandle = RenderTargetHandle::Invalid()) override;
        virtual void                    releaseRenderTarget             (RenderTargetHandle targetHandle) override;
        virtual bool                    isRenderTargetAllocated         (RenderTargetHandle targetHandle) const override final;
        virtual UInt32                  getRenderTargetCount            () const override final;
        virtual void                    addRenderTargetRenderBuffer     (RenderTargetHandle targetHandle, RenderBufferHandle bufferHandle) override;
        virtual UInt32                  getRenderTargetRenderBufferCount(RenderTargetHandle targetHandle) const override final;
        virtual RenderBufferHandle      getRenderTargetRenderBuffer     (RenderTargetHandle targetHandle, UInt32 bufferIndex) const override final;
        const RenderTargetMemoryPool&   getRenderTargets                () const;

        // Render buffers
        virtual RenderBufferHandle      allocateRenderBuffer            (const RenderBuffer& renderBuffer, RenderBufferHandle handle = RenderBufferHandle::Invalid()) override;
        virtual void                    releaseRenderBuffer             (RenderBufferHandle handle) override;
        virtual bool                    isRenderBufferAllocated         (RenderBufferHandle handle) const override final;
        virtual UInt32                  getRenderBufferCount            () const override final;
        virtual const RenderBuffer&     getRenderBuffer                 (RenderBufferHandle handle) const override final;
        const RenderBufferMemoryPool&   getRenderBuffers                () const;

        // Stream textures
        virtual StreamTextureHandle     allocateStreamTexture           (WaylandIviSurfaceId streamSource, const ResourceContentHash& fallbackTextureHash, StreamTextureHandle streamTextureHandle = StreamTextureHandle::Invalid()) override;
        virtual void                    releaseStreamTexture            (StreamTextureHandle streamTextureHandle) override;
        virtual bool                    isStreamTextureAllocated        (StreamTextureHandle streamTextureHandle) const override final;
        virtual UInt32                  getStreamTextureCount           () const override final;
        virtual void                    setForceFallbackImage           (StreamTextureHandle streamTextureHandle, bool forceFallbackImage) override;
        virtual const StreamTexture&    getStreamTexture                (StreamTextureHandle streamTextureHandle) const override final;
        const StreamTextureMemoryPool&  getStreamTextures               () const;

        // Data buffers
        virtual DataBufferHandle        allocateDataBuffer              (EDataBufferType dataBufferType, EDataType dataType, UInt32 maximumSizeInBytes, DataBufferHandle handle = DataBufferHandle::Invalid()) override;
        virtual void                    releaseDataBuffer               (DataBufferHandle handle) override;
        virtual UInt32                  getDataBufferCount              () const override final;
        virtual void                    updateDataBuffer                (DataBufferHandle handle, UInt32 offsetInBytes, UInt32 dataSizeInBytes, const Byte* data) override;
        virtual bool                    isDataBufferAllocated           (DataBufferHandle handle) const override final;
        virtual const GeometryDataBuffer& getDataBuffer                 (DataBufferHandle handle) const override final;
        const DataBufferMemoryPool&      getDataBuffers                  () const;

        //Texture buffers
        virtual TextureBufferHandle     allocateTextureBuffer           (ETextureFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle = TextureBufferHandle::Invalid()) override;
        virtual void                    releaseTextureBuffer            (TextureBufferHandle handle) override;
        virtual bool                    isTextureBufferAllocated        (TextureBufferHandle handle) const override final;
        virtual UInt32                  getTextureBufferCount           () const override final;
        virtual void                    updateTextureBuffer             (TextureBufferHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data) override;
        virtual const TextureBuffer&    getTextureBuffer                (TextureBufferHandle handle) const override final;
        const TextureBufferMemoryPool&      getTextureBuffers               () const;

        virtual DataSlotHandle          allocateDataSlot                (const DataSlot& dataSlot, DataSlotHandle handle = DataSlotHandle::Invalid()) override;
        virtual void                    releaseDataSlot                 (DataSlotHandle handle) override;
        virtual void                    setDataSlotTexture              (DataSlotHandle handle, const ResourceContentHash& texture) override;
        virtual bool                    isDataSlotAllocated             (DataSlotHandle handle) const override final;
        virtual UInt32                  getDataSlotCount                () const override final;
        virtual const DataSlot&         getDataSlot                     (DataSlotHandle handle) const override final;
        const DataSlotMemoryPool&       getDataSlots                    () const;

        virtual SceneReferenceHandle    allocateSceneReference          (SceneId sceneId, SceneReferenceHandle handle = {}) override;
        virtual void                    releaseSceneReference           (SceneReferenceHandle handle) override;
        virtual void                    requestSceneReferenceState      (SceneReferenceHandle handle, RendererSceneState state) override;
        virtual void                    requestSceneReferenceFlushNotifications(SceneReferenceHandle handle, bool enable) override;
        virtual void                    setSceneReferenceRenderOrder    (SceneReferenceHandle handle, int32_t renderOrder) override;
        virtual bool                    isSceneReferenceAllocated       (SceneReferenceHandle handle) const override final;
        virtual UInt32                  getSceneReferenceCount          () const override final;
        virtual const SceneReference&   getSceneReference               (SceneReferenceHandle handle) const override final;
        const SceneReferenceMemoryPool& getSceneReferences              () const;

        virtual SceneSizeInformation    getSceneSizeInformation         () const final override;

    protected:
        const TopologyNode&             getNode                         (NodeHandle handle) const;
        TextureSampler&                 getTextureSamplerInternal       (TextureSamplerHandle handle);
        RenderPass&                     getRenderPassInternal           (RenderPassHandle handle);
        RenderGroup&                    getRenderGroupInternal          (RenderGroupHandle handle);
        const TopologyTransform&        getTransform                    (TransformHandle handle) const;

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
        StreamTextureMemoryPool     m_streamTextures;
        DataBufferMemoryPool        m_dataBuffers;
        TextureBufferMemoryPool     m_textureBuffers;
        DataSlotMemoryPool          m_dataSlots;
        SceneReferenceMemoryPool    m_sceneReferences;
        AnimationSystemMemoryPool   m_animationSystems;

        const String                m_name;
        const SceneId               m_sceneId;
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
    const TYPE* SceneT<MEMORYPOOL>::getInstanceDataInternal(DataInstanceHandle dataInstanceHandle, DataFieldHandle fieldId) const
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
    inline bool SceneT<MEMORYPOOL>::isStreamTextureAllocated(StreamTextureHandle streamTextureHandle) const
    {
        return m_streamTextures.isAllocated(streamTextureHandle);
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
    inline bool SceneT<MEMORYPOOL>::isAnimationSystemAllocated(AnimationSystemHandle handle) const
    {
        return m_animationSystems.isAllocated(handle);
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
    const typename SceneT<MEMORYPOOL>::AnimationSystemMemoryPool& SceneT<MEMORYPOOL>::getAnimationSystems() const
    {
        return m_animationSystems;
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
    const typename SceneT<MEMORYPOOL>::StreamTextureMemoryPool& SceneT<MEMORYPOOL>::getStreamTextures() const
    {
        return m_streamTextures;
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
