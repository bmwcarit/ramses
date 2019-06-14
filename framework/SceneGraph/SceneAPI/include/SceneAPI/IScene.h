//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ISCENE_H
#define RAMSES_ISCENE_H

#include "SceneAPI/EDataType.h"
#include "SceneAPI/EDataBufferType.h"
#include "SceneAPI/ResourceContentHash.h"
#include "SceneAPI/Handles.h"
#include "SceneAPI/SceneSizeInformation.h"
#include "SceneAPI/TextureEnums.h"
#include "SceneAPI/ERenderableDataSlotType.h"
#include "SceneAPI/ECameraProjectionType.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/DataSlot.h"
#include "SceneAPI/RenderState.h"
#include "SceneAPI/EFixedSemantics.h"
#include "SceneAPI/DataFieldInfo.h"
#include "SceneAPI/MipMapSize.h"

#include "Collections/HashMap.h"
#include "Collections/Vector.h"
#include "AnimationAPI/IAnimationSystem.h"

namespace ramses_internal
{
    class Vector2;
    class Vector3;
    class Vector4;
    class Vector2i;
    class Vector3i;
    class Vector4i;
    class Matrix22f;
    class Matrix33f;
    class Matrix44f;
    class DataLayout;
    struct SceneSizeInformation;
    struct PixelRectangle;
    struct Camera;
    struct Renderable;
    struct Viewport;
    struct Frustum;
    struct RenderPass;
    struct RenderGroup;
    struct TextureSampler;
    struct TextureSamplerStates;
    struct TextureBuffer;
    struct GeometryDataBuffer;
    struct StreamTexture;
    struct RenderBuffer;
    struct BlitPass;

    class IScene
    {
    public:
        virtual ~IScene()
        {
        }

        IScene() = default;

        // scene should never be copied or moved
        IScene(const IScene&) = delete;
        IScene& operator=(const IScene&) = delete;
        IScene(IScene&&) = delete;
        IScene& operator=(IScene&&) = delete;

        virtual const String&               getName                         () const = 0;
        virtual SceneId                     getSceneId                      () const = 0;

        virtual void                        setAckFlushState                (bool /*state*/) {}

        virtual void                        preallocateSceneSize            (const SceneSizeInformation& sizeInfo) = 0;

        // Renderable
        virtual RenderableHandle            allocateRenderable              (NodeHandle nodeHandle, RenderableHandle handle = RenderableHandle::Invalid()) = 0;
        virtual void                        releaseRenderable               (RenderableHandle renderableHandle) = 0;
        virtual Bool                        isRenderableAllocated           (RenderableHandle renderableHandle) const = 0;
        virtual UInt32                      getRenderableCount              () const = 0;
        virtual void                        setRenderableEffect             (RenderableHandle renderableHandle, const ResourceContentHash& effectHash) = 0;
        virtual void                        setRenderableDataInstance       (RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance) = 0;
        virtual void                        setRenderableStartIndex         (RenderableHandle renderableHandle, UInt32 startIndex) = 0;
        virtual void                        setRenderableIndexCount         (RenderableHandle renderableHandle, UInt32 indexCount) = 0;
        virtual void                        setRenderableRenderState        (RenderableHandle renderableHandle, RenderStateHandle stateHandle) = 0;
        virtual void                        setRenderableVisibility         (RenderableHandle renderableHandle, Bool visible) = 0;
        virtual void                        setRenderableInstanceCount      (RenderableHandle renderableHandle, UInt32 instanceCount) = 0;
        virtual const Renderable&           getRenderable                   (RenderableHandle renderableHandle) const = 0;

        // Render state
        virtual RenderStateHandle           allocateRenderState             (RenderStateHandle stateHandle = RenderStateHandle::Invalid()) = 0;
        virtual void                        releaseRenderState              (RenderStateHandle stateHandle) = 0;
        virtual Bool                        isRenderStateAllocated          (RenderStateHandle stateHandle) const = 0;
        virtual UInt32                      getRenderStateCount             () const = 0;
        virtual void                        setRenderStateBlendFactors      (RenderStateHandle stateHandle, EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha) = 0;
        virtual void                        setRenderStateBlendOperations   (RenderStateHandle stateHandle, EBlendOperation operationColor, EBlendOperation operationAlpha) = 0;
        virtual void                        setRenderStateCullMode          (RenderStateHandle stateHandle, ECullMode cullMode) = 0;
        virtual void                        setRenderStateDrawMode          (RenderStateHandle stateHandle, EDrawMode drawMode) = 0;
        virtual void                        setRenderStateDepthFunc         (RenderStateHandle stateHandle, EDepthFunc func) = 0;
        virtual void                        setRenderStateDepthWrite        (RenderStateHandle stateHandle, EDepthWrite flag) = 0;
        virtual void                        setRenderStateScissorTest       (RenderStateHandle stateHandle, EScissorTest flag, const RenderState::ScissorRegion& region) = 0;
        virtual void                        setRenderStateStencilFunc       (RenderStateHandle stateHandle, EStencilFunc func, UInt8 ref, UInt8 mask) = 0;
        virtual void                        setRenderStateStencilOps        (RenderStateHandle stateHandle, EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass) = 0;
        virtual void                        setRenderStateColorWriteMask    (RenderStateHandle stateHandle, ColorWriteMask colorMask) = 0;
        virtual const RenderState&          getRenderState                  (RenderStateHandle stateHandle) const = 0;

        // Camera
        virtual CameraHandle                allocateCamera                  (ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle viewportDataInstance, CameraHandle handle = CameraHandle::Invalid()) = 0;
        virtual void                        releaseCamera                   (CameraHandle cameraHandle) = 0;
        virtual Bool                        isCameraAllocated               (CameraHandle handle) const = 0;
        virtual UInt32                      getCameraCount                  () const = 0;
        virtual void                        setCameraFrustum                (CameraHandle cameraHandle, const Frustum& frustum) = 0;
        virtual const Camera&               getCamera                       (CameraHandle cameraHandle) const = 0;

        // Nodes
        virtual NodeHandle                  allocateNode                    (UInt32 childrenCount = 0u, NodeHandle handle = NodeHandle::Invalid()) = 0;
        virtual void                        releaseNode                     (NodeHandle nodeHandle) = 0;
        virtual Bool                        isNodeAllocated                 (NodeHandle node) const = 0;
        virtual UInt32                      getNodeCount                    () const = 0;
        virtual NodeHandle                  getParent                       (NodeHandle nodeHandle) const = 0;
        virtual void                        addChildToNode                  (NodeHandle parent, NodeHandle child) = 0;
        virtual void                        removeChildFromNode             (NodeHandle parent, NodeHandle child) = 0;
        virtual UInt32                      getChildCount                   (NodeHandle parent) const = 0;
        virtual NodeHandle                  getChild                        (NodeHandle parent, UInt32 childNumber) const = 0;

        // Transformation
        virtual TransformHandle             allocateTransform               (NodeHandle nodeHandle, TransformHandle handle = TransformHandle::Invalid()) = 0;
        virtual void                        releaseTransform                (TransformHandle transform) = 0;
        virtual Bool                        isTransformAllocated            (TransformHandle transformHandle) const = 0;
        virtual UInt32                      getTransformCount               () const = 0;
        virtual NodeHandle                  getTransformNode                (TransformHandle handle) const = 0;
        virtual const Vector3&              getTranslation                  (TransformHandle handle) const = 0;
        virtual const Vector3&              getRotation                     (TransformHandle handle) const = 0;
        virtual const Vector3&              getScaling                      (TransformHandle handle) const = 0;
        virtual void                        setTranslation                  (TransformHandle handle, const Vector3& translation) = 0;
        virtual void                        setRotation                     (TransformHandle handle, const Vector3& rotation) = 0;
        virtual void                        setScaling                      (TransformHandle handle, const Vector3& scaling) = 0;

        virtual DataLayoutHandle            allocateDataLayout              (const DataFieldInfoVector& dataFields, DataLayoutHandle handle = DataLayoutHandle::Invalid()) = 0;
        virtual void                        releaseDataLayout               (DataLayoutHandle layoutHandle) = 0;
        virtual Bool                        isDataLayoutAllocated           (DataLayoutHandle layoutHandle) const = 0;
        virtual UInt32                      getDataLayoutCount              () const = 0;

        virtual const DataLayout&           getDataLayout                   (DataLayoutHandle layoutHandle) const = 0;

        virtual DataInstanceHandle          allocateDataInstance            (DataLayoutHandle finishedLayoutHandle, DataInstanceHandle instanceHandle = DataInstanceHandle::Invalid()) = 0;
        virtual void                        releaseDataInstance             (DataInstanceHandle containerHandle) = 0;
        virtual Bool                        isDataInstanceAllocated         (DataInstanceHandle containerHandle) const = 0;
        virtual UInt32                      getDataInstanceCount            () const = 0;
        virtual DataLayoutHandle            getLayoutOfDataInstance         (DataInstanceHandle containerHandle) const = 0;

        virtual const Float*                getDataFloatArray               (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual const Vector2*              getDataVector2fArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual const Vector3*              getDataVector3fArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual const Vector4*              getDataVector4fArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual const Int32*                getDataIntegerArray             (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual const Matrix22f*            getDataMatrix22fArray           (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual const Matrix33f*            getDataMatrix33fArray           (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual const Matrix44f*            getDataMatrix44fArray           (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual const Vector2i*             getDataVector2iArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual const Vector3i*             getDataVector3iArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual const Vector4i*             getDataVector4iArray            (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual const ResourceField&        getDataResource                 (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual TextureSamplerHandle        getDataTextureSamplerHandle     (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual DataInstanceHandle          getDataReference                (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;

        virtual void                        setDataFloatArray               (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Float* data) = 0;
        virtual void                        setDataVector2fArray            (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector2* data) = 0;
        virtual void                        setDataVector3fArray            (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector3* data) = 0;
        virtual void                        setDataVector4fArray            (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector4* data) = 0;
        virtual void                        setDataIntegerArray             (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Int32* data) = 0;
        virtual void                        setDataVector2iArray            (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector2i* data) = 0;
        virtual void                        setDataVector3iArray            (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector3i* data) = 0;
        virtual void                        setDataVector4iArray            (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector4i* data) = 0;
        virtual void                        setDataMatrix22fArray           (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix22f* data) = 0;
        virtual void                        setDataMatrix33fArray           (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix33f* data) = 0;
        virtual void                        setDataMatrix44fArray           (DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix44f* data) = 0;
        virtual void                        setDataResource                 (DataInstanceHandle containerHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, UInt32 instancingDivisor) = 0;
        virtual void                        setDataTextureSamplerHandle     (DataInstanceHandle containerHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle) = 0;
        virtual void                        setDataReference                (DataInstanceHandle containerHandle, DataFieldHandle field, DataInstanceHandle dataRef) = 0;

        // get/setData*Array wrappers for elementCount == 1
        virtual Float                       getDataSingleFloat              (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual const Vector2&              getDataSingleVector2f           (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual const Vector3&              getDataSingleVector3f           (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual const Vector4&              getDataSingleVector4f           (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual Int32                       getDataSingleInteger            (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual const Matrix22f&            getDataSingleMatrix22f          (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual const Matrix33f&            getDataSingleMatrix33f          (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual const Matrix44f&            getDataSingleMatrix44f          (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual const Vector2i&             getDataSingleVector2i           (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual const Vector3i&             getDataSingleVector3i           (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        virtual const Vector4i&             getDataSingleVector4i           (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;

        virtual void                        setDataSingleFloat              (DataInstanceHandle containerHandle, DataFieldHandle field, Float data) = 0;
        virtual void                        setDataSingleVector2f           (DataInstanceHandle containerHandle, DataFieldHandle field, const Vector2& data) = 0;
        virtual void                        setDataSingleVector3f           (DataInstanceHandle containerHandle, DataFieldHandle field, const Vector3& data) = 0;
        virtual void                        setDataSingleVector4f           (DataInstanceHandle containerHandle, DataFieldHandle field, const Vector4& data) = 0;
        virtual void                        setDataSingleInteger            (DataInstanceHandle containerHandle, DataFieldHandle field, Int32 data) = 0;
        virtual void                        setDataSingleVector2i           (DataInstanceHandle containerHandle, DataFieldHandle field, const Vector2i& data) = 0;
        virtual void                        setDataSingleVector3i           (DataInstanceHandle containerHandle, DataFieldHandle field, const Vector3i& data) = 0;
        virtual void                        setDataSingleVector4i           (DataInstanceHandle containerHandle, DataFieldHandle field, const Vector4i& data) = 0;
        virtual void                        setDataSingleMatrix22f          (DataInstanceHandle containerHandle, DataFieldHandle field, const Matrix22f& data) = 0;
        virtual void                        setDataSingleMatrix33f          (DataInstanceHandle containerHandle, DataFieldHandle field, const Matrix33f& data) = 0;
        virtual void                        setDataSingleMatrix44f          (DataInstanceHandle containerHandle, DataFieldHandle field, const Matrix44f& data) = 0;

        // Texture sampler description
        virtual TextureSamplerHandle        allocateTextureSampler          (const TextureSampler& sampler, TextureSamplerHandle handle = TextureSamplerHandle::Invalid()) = 0;
        virtual void                        releaseTextureSampler           (TextureSamplerHandle handle) = 0;
        virtual Bool                        isTextureSamplerAllocated       (TextureSamplerHandle handle) const = 0;
        virtual UInt32                      getTextureSamplerCount          () const = 0;
        virtual const TextureSampler&       getTextureSampler               (TextureSamplerHandle handle) const = 0;

        // Render groups
        virtual RenderGroupHandle           allocateRenderGroup             (UInt32 renderableCount = 0u, UInt32 nestedGroupCount = 0u, RenderGroupHandle groupHandle = RenderGroupHandle::Invalid()) = 0;
        virtual void                        releaseRenderGroup              (RenderGroupHandle groupHandle) = 0;
        virtual Bool                        isRenderGroupAllocated          (RenderGroupHandle groupHandle) const = 0;
        virtual UInt32                      getRenderGroupCount             () const = 0;
        virtual void                        addRenderableToRenderGroup      (RenderGroupHandle groupHandle, RenderableHandle renderableHandle, Int32 order) = 0;
        virtual void                        removeRenderableFromRenderGroup (RenderGroupHandle groupHandle, RenderableHandle renderableHandle) = 0;
        virtual void                        addRenderGroupToRenderGroup     (RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, Int32 order) = 0;
        virtual void                        removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild) = 0;
        virtual const RenderGroup&          getRenderGroup                  (RenderGroupHandle groupHandle) const = 0;

        // Render passes
        virtual RenderPassHandle            allocateRenderPass              (UInt32 renderGroupCount = 0u, RenderPassHandle passHandle = RenderPassHandle::Invalid()) = 0;
        virtual void                        releaseRenderPass               (RenderPassHandle passHandle) = 0;
        virtual Bool                        isRenderPassAllocated           (RenderPassHandle passHandle) const = 0;
        virtual UInt32                      getRenderPassCount              () const = 0;
        virtual void                        setRenderPassClearColor         (RenderPassHandle passHandle, const Vector4& clearColor) = 0;
        virtual void                        setRenderPassClearFlag          (RenderPassHandle passHandle, UInt32 clearFlag) = 0;
        virtual void                        setRenderPassCamera             (RenderPassHandle passHandle, CameraHandle cameraHandle) = 0;
        virtual void                        setRenderPassRenderTarget       (RenderPassHandle passHandle, RenderTargetHandle targetHandle) = 0;
        virtual void                        setRenderPassRenderOrder        (RenderPassHandle passHandle, Int32 renderOrder) = 0;
        virtual void                        setRenderPassEnabled            (RenderPassHandle passHandle, Bool isEnabled) = 0;
        virtual void                        setRenderPassRenderOnce         (RenderPassHandle passHandle, Bool enable) = 0;
        virtual void                        retriggerRenderPassRenderOnce   (RenderPassHandle passHandle) = 0;
        virtual void                        addRenderGroupToRenderPass      (RenderPassHandle passHandle, RenderGroupHandle groupHandle, Int32 order) = 0;
        virtual void                        removeRenderGroupFromRenderPass (RenderPassHandle passHandle, RenderGroupHandle groupHandle) = 0;
        virtual const RenderPass&           getRenderPass                   (RenderPassHandle passHandle) const = 0;

        //Blit pass
        virtual BlitPassHandle              allocateBlitPass                (RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle = BlitPassHandle::Invalid()) = 0;
        virtual void                        releaseBlitPass                 (BlitPassHandle passHandle) = 0;
        virtual Bool                        isBlitPassAllocated             (BlitPassHandle passHandle) const = 0;
        virtual UInt32                      getBlitPassCount                () const = 0;
        virtual void                        setBlitPassRenderOrder          (BlitPassHandle passHandle, Int32 renderOrder) = 0;
        virtual void                        setBlitPassEnabled              (BlitPassHandle passHandle, Bool isEnabled) = 0;
        virtual void                        setBlitPassRegions              (BlitPassHandle passHandle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion) = 0;
        virtual const BlitPass&             getBlitPass                     (BlitPassHandle passHandle) const = 0;

        virtual SceneSizeInformation        getSceneSizeInformation() const = 0;

        // Render targets
        virtual RenderTargetHandle          allocateRenderTarget            (RenderTargetHandle targetHandle = RenderTargetHandle::Invalid()) = 0;
        virtual void                        releaseRenderTarget             (RenderTargetHandle targetHandle) = 0;
        virtual Bool                        isRenderTargetAllocated         (RenderTargetHandle targetHandle) const = 0;
        virtual UInt32                      getRenderTargetCount            () const  = 0;
        virtual void                        addRenderTargetRenderBuffer     (RenderTargetHandle targetHandle, RenderBufferHandle bufferHandle) = 0;
        virtual UInt32                      getRenderTargetRenderBufferCount(RenderTargetHandle targetHandle) const = 0;
        virtual RenderBufferHandle          getRenderTargetRenderBuffer     (RenderTargetHandle targetHandle, UInt32 bufferIndex) const = 0;

        // Render buffers
        virtual RenderBufferHandle          allocateRenderBuffer            (const RenderBuffer& renderBuffer, RenderBufferHandle handle = RenderBufferHandle::Invalid()) = 0;
        virtual void                        releaseRenderBuffer             (RenderBufferHandle handle) = 0;
        virtual Bool                        isRenderBufferAllocated         (RenderBufferHandle handle) const = 0;
        virtual UInt32                      getRenderBufferCount            () const = 0;
        virtual const RenderBuffer&         getRenderBuffer                 (RenderBufferHandle handle) const = 0;

        // Stream textures
        virtual StreamTextureHandle         allocateStreamTexture           (uint32_t streamSource, const ResourceContentHash& fallbackTextureHash, StreamTextureHandle streamTextureHandle = StreamTextureHandle::Invalid()) = 0;
        virtual void                        releaseStreamTexture            (StreamTextureHandle streamTextureHandle) = 0;
        virtual Bool                        isStreamTextureAllocated        (StreamTextureHandle streamTextureHandle) const = 0;
        virtual UInt32                      getStreamTextureCount           () const = 0;
        virtual void                        setForceFallbackImage           (StreamTextureHandle streamTextureHandle, Bool forceFallbackImage) = 0;
        virtual const StreamTexture&        getStreamTexture                (StreamTextureHandle streamTextureHandle) const = 0;

        // Data buffers
        virtual DataBufferHandle            allocateDataBuffer              (EDataBufferType dataBufferType, EDataType dataType, UInt32 maximumSizeInBytes, DataBufferHandle handle = DataBufferHandle::Invalid()) = 0;
        virtual void                        releaseDataBuffer               (DataBufferHandle handle) = 0;
        virtual Bool                        isDataBufferAllocated           (DataBufferHandle handle) const = 0;
        virtual UInt32                      getDataBufferCount              () const = 0;
        virtual void                        updateDataBuffer                (DataBufferHandle handle, UInt32 offsetInBytes, UInt32 dataSizeInBytes, const Byte* data) = 0;
        virtual const GeometryDataBuffer&   getDataBuffer                   (DataBufferHandle handle) const = 0;

        //Texture buffers
        virtual TextureBufferHandle         allocateTextureBuffer           (ETextureFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle = TextureBufferHandle::Invalid()) = 0;
        virtual void                        releaseTextureBuffer            (TextureBufferHandle handle) = 0;
        virtual Bool                        isTextureBufferAllocated        (TextureBufferHandle handle) const = 0;
        virtual UInt32                      getTextureBufferCount           () const = 0;
        virtual void                        updateTextureBuffer             (TextureBufferHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data) = 0;
        virtual const TextureBuffer&        getTextureBuffer                (TextureBufferHandle handle) const = 0;

        // Data slots
        virtual DataSlotHandle              allocateDataSlot                (const DataSlot& dataSlot, DataSlotHandle handle = DataSlotHandle::Invalid()) = 0;
        virtual void                        releaseDataSlot                 (DataSlotHandle handle) = 0;
        virtual void                        setDataSlotTexture              (DataSlotHandle handle, const ResourceContentHash& texture) = 0;
        virtual Bool                        isDataSlotAllocated             (DataSlotHandle handle) const = 0;
        virtual UInt32                      getDataSlotCount                () const = 0;
        virtual const DataSlot&             getDataSlot                     (DataSlotHandle handle) const = 0;

        //Animation system
        virtual AnimationSystemHandle       addAnimationSystem              (IAnimationSystem* animationSystem, AnimationSystemHandle externalHandle = AnimationSystemHandle::Invalid()) = 0;
        virtual void                        removeAnimationSystem           (AnimationSystemHandle animSystemHandle) = 0;
        virtual IAnimationSystem*           getAnimationSystem              (AnimationSystemHandle animSystemHandle) = 0;
        virtual const IAnimationSystem*     getAnimationSystem              (AnimationSystemHandle animSystemHandle) const = 0;
        virtual Bool                        isAnimationSystemAllocated      (AnimationSystemHandle animSystemHandle) const = 0;
        virtual UInt32                      getAnimationSystemCount         () const = 0;
    };
}

#endif
