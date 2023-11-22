//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/SceneTypes.h"
#include "internal/SceneGraph/SceneAPI/EDataBufferType.h"
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "internal/SceneGraph/SceneAPI/SceneSizeInformation.h"
#include "internal/SceneGraph/SceneAPI/TextureEnums.h"
#include "internal/SceneGraph/SceneAPI/ERenderableDataSlotType.h"
#include "internal/SceneGraph/SceneAPI/ECameraProjectionType.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneGraph/SceneAPI/DataSlot.h"
#include "internal/SceneGraph/SceneAPI/DataFieldInfo.h"
#include "internal/SceneGraph/SceneAPI/MipMapSize.h"
#include "internal/SceneGraph/SceneAPI/Renderable.h"
#include "internal/SceneGraph/SceneAPI/RendererSceneState.h"
#include "internal/SceneGraph/SceneAPI/ERotationType.h"

#include "internal/PlatformAbstraction/Collections/HashMap.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "internal/Components/FlushTimeInformation.h"
#include "impl/DataTypesImpl.h"

#include <string>

namespace ramses::internal
{
    class DataLayout;
    struct SceneSizeInformation;
    struct PixelRectangle;
    struct Camera;
    struct RenderPass;
    struct RenderGroup;
    struct TextureSampler;
    struct TextureSamplerStates;
    struct TextureBuffer;
    struct GeometryDataBuffer;
    struct RenderBuffer;
    struct BlitPass;
    struct PickableObject;
    struct SceneReference;
    struct TopologyTransform;

    class IScene
    {
    public:
        IScene() = default;
        virtual ~IScene() = default;

        // scene should never be copied or moved
        IScene(const IScene&) = delete;
        IScene& operator=(const IScene&) = delete;
        IScene(IScene&&) = delete;
        IScene& operator=(IScene&&) = delete;

        [[nodiscard]] virtual const std::string&          getName                         () const = 0;
        [[nodiscard]] virtual SceneId                     getSceneId                      () const = 0;

        virtual void setEffectTimeSync(FlushTime::Clock::time_point t) = 0;
        [[nodiscard]] virtual FlushTime::Clock::time_point getEffectTimeSync() const = 0;

        virtual void                        preallocateSceneSize            (const SceneSizeInformation& sizeInfo) = 0;

        // Renderable
        virtual RenderableHandle            allocateRenderable              (NodeHandle nodeHandle, RenderableHandle handle) = 0;
        virtual void                        releaseRenderable               (RenderableHandle renderableHandle) = 0;
        [[nodiscard]] virtual bool          isRenderableAllocated           (RenderableHandle renderableHandle) const = 0;
        [[nodiscard]] virtual uint32_t      getRenderableCount              () const = 0;
        virtual void                        setRenderableDataInstance       (RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance) = 0;
        virtual void                        setRenderableStartIndex         (RenderableHandle renderableHandle, uint32_t startIndex) = 0;
        virtual void                        setRenderableIndexCount         (RenderableHandle renderableHandle, uint32_t indexCount) = 0;
        virtual void                        setRenderableRenderState        (RenderableHandle renderableHandle, RenderStateHandle stateHandle) = 0;
        virtual void                        setRenderableVisibility         (RenderableHandle renderableHandle, EVisibilityMode visibility) = 0;
        virtual void                        setRenderableInstanceCount      (RenderableHandle renderableHandle, uint32_t instanceCount) = 0;
        virtual void                        setRenderableStartVertex        (RenderableHandle renderableHandle, uint32_t startVertex) = 0;
        [[nodiscard]] virtual const Renderable& getRenderable               (RenderableHandle renderableHandle) const = 0;

        // Render state
        virtual RenderStateHandle           allocateRenderState             (RenderStateHandle stateHandle) = 0;
        virtual void                        releaseRenderState              (RenderStateHandle stateHandle) = 0;
        [[nodiscard]] virtual bool          isRenderStateAllocated          (RenderStateHandle stateHandle) const = 0;
        [[nodiscard]] virtual uint32_t      getRenderStateCount             () const = 0;
        virtual void                        setRenderStateBlendFactors      (RenderStateHandle stateHandle, EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha) = 0;
        virtual void                        setRenderStateBlendOperations   (RenderStateHandle stateHandle, EBlendOperation operationColor, EBlendOperation operationAlpha) = 0;
        virtual void                        setRenderStateBlendColor        (RenderStateHandle stateHandle, const glm::vec4& color) = 0;
        virtual void                        setRenderStateCullMode          (RenderStateHandle stateHandle, ECullMode cullMode) = 0;
        virtual void                        setRenderStateDrawMode          (RenderStateHandle stateHandle, EDrawMode drawMode) = 0;
        virtual void                        setRenderStateDepthFunc         (RenderStateHandle stateHandle, EDepthFunc func) = 0;
        virtual void                        setRenderStateDepthWrite        (RenderStateHandle stateHandle, EDepthWrite flag) = 0;
        virtual void                        setRenderStateScissorTest       (RenderStateHandle stateHandle, EScissorTest flag, const RenderState::ScissorRegion& region) = 0;
        virtual void                        setRenderStateStencilFunc       (RenderStateHandle stateHandle, EStencilFunc func, uint8_t ref, uint8_t mask) = 0;
        virtual void                        setRenderStateStencilOps        (RenderStateHandle stateHandle, EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass) = 0;
        virtual void                        setRenderStateColorWriteMask    (RenderStateHandle stateHandle, ColorWriteMask colorMask) = 0;
        [[nodiscard]] virtual const RenderState& getRenderState             (RenderStateHandle stateHandle) const = 0;

        // Camera
        virtual CameraHandle                allocateCamera                  (ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle dataInstance, CameraHandle handle) = 0;
        virtual void                        releaseCamera                   (CameraHandle cameraHandle) = 0;
        [[nodiscard]] virtual bool          isCameraAllocated               (CameraHandle handle) const = 0;
        [[nodiscard]] virtual uint32_t      getCameraCount                  () const = 0;
        [[nodiscard]] virtual const Camera& getCamera                       (CameraHandle cameraHandle) const = 0;

        // Nodes
        virtual NodeHandle                  allocateNode                    (uint32_t childrenCount, NodeHandle handle) = 0;
        virtual void                        releaseNode                     (NodeHandle nodeHandle) = 0;
        [[nodiscard]] virtual bool          isNodeAllocated                 (NodeHandle node) const = 0;
        [[nodiscard]] virtual uint32_t      getNodeCount                    () const = 0;
        [[nodiscard]] virtual NodeHandle    getParent                       (NodeHandle nodeHandle) const = 0;
        virtual void                        addChildToNode                  (NodeHandle parent, NodeHandle child) = 0;
        virtual void                        removeChildFromNode             (NodeHandle parent, NodeHandle child) = 0;
        [[nodiscard]] virtual uint32_t      getChildCount                   (NodeHandle parent) const = 0;
        [[nodiscard]] virtual NodeHandle    getChild                        (NodeHandle parent, uint32_t childNumber) const = 0;

        // Transformation
        constexpr static glm::vec3 IdentityTranslation = {0.f, 0.f, 0.f};
        constexpr static glm::vec4 IdentityRotation    = {0.f, 0.f, 0.f, 1.f}; // zero rotation for both euler and quaternion
        constexpr static glm::vec3 IdentityScaling     = {1.f, 1.f, 1.f};

        virtual TransformHandle                allocateTransform            (NodeHandle nodeHandle, TransformHandle handle) = 0;
        virtual void                           releaseTransform             (TransformHandle transform) = 0;
        [[nodiscard]] virtual bool             isTransformAllocated         (TransformHandle transformHandle) const = 0;
        [[nodiscard]] virtual uint32_t         getTransformCount            () const = 0;
        [[nodiscard]] virtual NodeHandle       getTransformNode             (TransformHandle handle) const = 0;
        [[nodiscard]] virtual const glm::vec3& getTranslation               (TransformHandle handle) const = 0;
        [[nodiscard]] virtual const glm::vec4& getRotation                  (TransformHandle handle) const = 0;
        [[nodiscard]] virtual ERotationType    getRotationType              (TransformHandle handle) const = 0;
        [[nodiscard]] virtual const glm::vec3& getScaling                   (TransformHandle handle) const = 0;
        virtual void                           setTranslation               (TransformHandle handle, const glm::vec3& translation) = 0;
        virtual void                           setRotation                  (TransformHandle handle, const glm::vec4& rotation, ERotationType rotationType) = 0;
        virtual void                           setScaling                   (TransformHandle handle, const glm::vec3& scaling) = 0;

        virtual DataLayoutHandle                allocateDataLayout          (const DataFieldInfoVector& dataFields, const ResourceContentHash& effectHash, DataLayoutHandle handle) = 0;
        virtual void                            releaseDataLayout           (DataLayoutHandle layoutHandle) = 0;
        [[nodiscard]] virtual bool              isDataLayoutAllocated       (DataLayoutHandle layoutHandle) const = 0;
        [[nodiscard]] virtual uint32_t          getDataLayoutCount          () const = 0;
        [[nodiscard]] virtual const DataLayout& getDataLayout               (DataLayoutHandle layoutHandle) const = 0;

        virtual DataInstanceHandle             allocateDataInstance         (DataLayoutHandle finishedLayoutHandle, DataInstanceHandle instanceHandle) = 0;
        virtual void                           releaseDataInstance          (DataInstanceHandle containerHandle) = 0;
        [[nodiscard]] virtual bool             isDataInstanceAllocated      (DataInstanceHandle containerHandle) const = 0;
        [[nodiscard]] virtual uint32_t         getDataInstanceCount         () const = 0;
        [[nodiscard]] virtual DataLayoutHandle getLayoutOfDataInstance      (DataInstanceHandle containerHandle) const = 0;

        [[nodiscard]] virtual const float*         getDataFloatArray           (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const glm::vec2*     getDataVector2fArray        (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const glm::vec3*     getDataVector3fArray        (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const glm::vec4*     getDataVector4fArray        (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const bool*          getDataBooleanArray         (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const int32_t*       getDataIntegerArray         (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const glm::mat2*     getDataMatrix22fArray       (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const glm::mat3*     getDataMatrix33fArray       (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const glm::mat4*     getDataMatrix44fArray       (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const glm::ivec2*    getDataVector2iArray        (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const glm::ivec3*    getDataVector3iArray        (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const glm::ivec4*    getDataVector4iArray        (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const ResourceField& getDataResource             (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual TextureSamplerHandle getDataTextureSamplerHandle (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual DataInstanceHandle   getDataReference            (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;

        virtual void setDataFloatArray           (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const float* data) = 0;
        virtual void setDataVector2fArray        (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec2* data) = 0;
        virtual void setDataVector3fArray        (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec3* data) = 0;
        virtual void setDataVector4fArray        (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::vec4* data) = 0;
        virtual void setDataBooleanArray         (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const bool* data) = 0;
        virtual void setDataIntegerArray         (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const int32_t* data) = 0;
        virtual void setDataVector2iArray        (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec2* data) = 0;
        virtual void setDataVector3iArray        (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec3* data) = 0;
        virtual void setDataVector4iArray        (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::ivec4* data) = 0;
        virtual void setDataMatrix22fArray       (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat2* data) = 0;
        virtual void setDataMatrix33fArray       (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat3* data) = 0;
        virtual void setDataMatrix44fArray       (DataInstanceHandle containerHandle, DataFieldHandle field, uint32_t elementCount, const glm::mat4* data) = 0;
        virtual void setDataResource             (DataInstanceHandle containerHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, uint32_t instancingDivisor, uint16_t offsetWithinElementInBytes, uint16_t stride) = 0;
        virtual void setDataTextureSamplerHandle (DataInstanceHandle containerHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle) = 0;
        virtual void setDataReference            (DataInstanceHandle containerHandle, DataFieldHandle field, DataInstanceHandle dataRef) = 0;

        // get/setData*Array wrappers for elementCount == 1
        [[nodiscard]] virtual float              getDataSingleFloat          (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const glm::vec2&   getDataSingleVector2f       (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const glm::vec3&   getDataSingleVector3f       (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const glm::vec4&   getDataSingleVector4f       (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual bool               getDataSingleBoolean        (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual int32_t            getDataSingleInteger        (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const glm::mat2&   getDataSingleMatrix22f      (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const glm::mat3&   getDataSingleMatrix33f      (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const glm::mat4&   getDataSingleMatrix44f      (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const glm::ivec2&  getDataSingleVector2i       (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const glm::ivec3&  getDataSingleVector3i       (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;
        [[nodiscard]] virtual const glm::ivec4&  getDataSingleVector4i       (DataInstanceHandle containerHandle, DataFieldHandle field) const = 0;

        virtual void setDataSingleFloat          (DataInstanceHandle containerHandle, DataFieldHandle field, float data) = 0;
        virtual void setDataSingleVector2f       (DataInstanceHandle containerHandle, DataFieldHandle field, const glm::vec2& data) = 0;
        virtual void setDataSingleVector3f       (DataInstanceHandle containerHandle, DataFieldHandle field, const glm::vec3& data) = 0;
        virtual void setDataSingleVector4f       (DataInstanceHandle containerHandle, DataFieldHandle field, const glm::vec4& data) = 0;
        virtual void setDataSingleBoolean        (DataInstanceHandle containerHandle, DataFieldHandle field, bool data) = 0;
        virtual void setDataSingleInteger        (DataInstanceHandle containerHandle, DataFieldHandle field, int32_t data) = 0;
        virtual void setDataSingleVector2i       (DataInstanceHandle containerHandle, DataFieldHandle field, const glm::ivec2& data) = 0;
        virtual void setDataSingleVector3i       (DataInstanceHandle containerHandle, DataFieldHandle field, const glm::ivec3& data) = 0;
        virtual void setDataSingleVector4i       (DataInstanceHandle containerHandle, DataFieldHandle field, const glm::ivec4& data) = 0;
        virtual void setDataSingleMatrix22f      (DataInstanceHandle containerHandle, DataFieldHandle field, const glm::mat2& data) = 0;
        virtual void setDataSingleMatrix33f      (DataInstanceHandle containerHandle, DataFieldHandle field, const glm::mat3& data) = 0;
        virtual void setDataSingleMatrix44f      (DataInstanceHandle containerHandle, DataFieldHandle field, const glm::mat4& data) = 0;

        // Texture sampler description
        virtual TextureSamplerHandle                allocateTextureSampler    (const TextureSampler& sampler, TextureSamplerHandle handle) = 0;
        virtual void                                releaseTextureSampler     (TextureSamplerHandle handle) = 0;
        [[nodiscard]] virtual bool                  isTextureSamplerAllocated (TextureSamplerHandle handle) const = 0;
        [[nodiscard]] virtual uint32_t              getTextureSamplerCount    () const = 0;
        [[nodiscard]] virtual const TextureSampler& getTextureSampler         (TextureSamplerHandle handle) const = 0;

        // Render groups
        virtual RenderGroupHandle           allocateRenderGroup             (uint32_t renderableCount, uint32_t nestedGroupCount, RenderGroupHandle groupHandle) = 0;
        virtual void                        releaseRenderGroup              (RenderGroupHandle groupHandle) = 0;
        [[nodiscard]] virtual bool          isRenderGroupAllocated          (RenderGroupHandle groupHandle) const = 0;
        [[nodiscard]] virtual uint32_t      getRenderGroupCount             () const = 0;
        virtual void                        addRenderableToRenderGroup      (RenderGroupHandle groupHandle, RenderableHandle renderableHandle, int32_t order) = 0;
        virtual void                        removeRenderableFromRenderGroup (RenderGroupHandle groupHandle, RenderableHandle renderableHandle) = 0;
        virtual void                        addRenderGroupToRenderGroup     (RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, int32_t order) = 0;
        virtual void                        removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild) = 0;
        [[nodiscard]] virtual const RenderGroup& getRenderGroup             (RenderGroupHandle groupHandle) const = 0;

        // Render passes
        virtual RenderPassHandle            allocateRenderPass              (uint32_t renderGroupCount, RenderPassHandle passHandle) = 0;
        virtual void                        releaseRenderPass               (RenderPassHandle passHandle) = 0;
        [[nodiscard]] virtual bool          isRenderPassAllocated           (RenderPassHandle passHandle) const = 0;
        [[nodiscard]] virtual uint32_t      getRenderPassCount              () const = 0;
        virtual void                        setRenderPassClearColor         (RenderPassHandle passHandle, const glm::vec4& clearColor) = 0;
        virtual void                        setRenderPassClearFlag          (RenderPassHandle passHandle, ClearFlags clearFlag) = 0;
        virtual void                        setRenderPassCamera             (RenderPassHandle passHandle, CameraHandle cameraHandle) = 0;
        virtual void                        setRenderPassRenderTarget       (RenderPassHandle passHandle, RenderTargetHandle targetHandle) = 0;
        virtual void                        setRenderPassRenderOrder        (RenderPassHandle passHandle, int32_t renderOrder) = 0;
        virtual void                        setRenderPassEnabled            (RenderPassHandle passHandle, bool isEnabled) = 0;
        virtual void                        setRenderPassRenderOnce         (RenderPassHandle passHandle, bool enable) = 0;
        virtual void                        retriggerRenderPassRenderOnce   (RenderPassHandle passHandle) = 0;
        virtual void                        addRenderGroupToRenderPass      (RenderPassHandle passHandle, RenderGroupHandle groupHandle, int32_t order) = 0;
        virtual void                        removeRenderGroupFromRenderPass (RenderPassHandle passHandle, RenderGroupHandle groupHandle) = 0;
        [[nodiscard]] virtual const RenderPass&           getRenderPass     (RenderPassHandle passHandle) const = 0;

        //Blit pass
        virtual BlitPassHandle              allocateBlitPass                (RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle) = 0;
        virtual void                        releaseBlitPass                 (BlitPassHandle passHandle) = 0;
        [[nodiscard]] virtual bool          isBlitPassAllocated             (BlitPassHandle passHandle) const = 0;
        [[nodiscard]] virtual uint32_t      getBlitPassCount                () const = 0;
        virtual void                        setBlitPassRenderOrder          (BlitPassHandle passHandle, int32_t renderOrder) = 0;
        virtual void                        setBlitPassEnabled              (BlitPassHandle passHandle, bool isEnabled) = 0;
        virtual void                        setBlitPassRegions              (BlitPassHandle passHandle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion) = 0;
        [[nodiscard]] virtual const BlitPass& getBlitPass                     (BlitPassHandle passHandle) const = 0;

        [[nodiscard]] virtual SceneSizeInformation getSceneSizeInformation() const = 0;

        //Pickable object
        virtual PickableObjectHandle        allocatePickableObject          (DataBufferHandle geometryHandle, NodeHandle nodeHandle, PickableObjectId id, PickableObjectHandle pickableHandle) = 0;
        virtual void                        releasePickableObject           (PickableObjectHandle pickableHandle) = 0;
        [[nodiscard]] virtual bool          isPickableObjectAllocated       (PickableObjectHandle pickableHandle) const = 0;
        [[nodiscard]] virtual uint32_t      getPickableObjectCount          () const = 0;
        virtual void                        setPickableObjectId             (PickableObjectHandle pickableHandle, PickableObjectId id) = 0;
        virtual void                        setPickableObjectCamera         (PickableObjectHandle pickableHandle, CameraHandle cameraHandle) = 0;
        virtual void                        setPickableObjectEnabled(PickableObjectHandle pickableHandel, bool isEnabled) = 0;
        [[nodiscard]] virtual const PickableObject& getPickableObject               (PickableObjectHandle pickableHandle) const  = 0;

        // Render targets
        virtual RenderTargetHandle          allocateRenderTarget            (RenderTargetHandle targetHandle) = 0;
        virtual void                        releaseRenderTarget             (RenderTargetHandle targetHandle) = 0;
        [[nodiscard]] virtual bool          isRenderTargetAllocated         (RenderTargetHandle targetHandle) const = 0;
        [[nodiscard]] virtual uint32_t      getRenderTargetCount            () const  = 0;
        virtual void                        addRenderTargetRenderBuffer     (RenderTargetHandle targetHandle, RenderBufferHandle bufferHandle) = 0;
        [[nodiscard]] virtual uint32_t           getRenderTargetRenderBufferCount(RenderTargetHandle targetHandle) const = 0;
        [[nodiscard]] virtual RenderBufferHandle getRenderTargetRenderBuffer     (RenderTargetHandle targetHandle, uint32_t bufferIndex) const = 0;

        // Render buffers
        virtual RenderBufferHandle          allocateRenderBuffer            (const RenderBuffer& renderBuffer, RenderBufferHandle handle) = 0;
        virtual void                        releaseRenderBuffer             (RenderBufferHandle handle) = 0;
        virtual void                        setRenderBufferProperties       (RenderBufferHandle handle, uint32_t width, uint32_t height, uint32_t sampleCount) = 0;
        [[nodiscard]] virtual bool                isRenderBufferAllocated   (RenderBufferHandle handle) const = 0;
        [[nodiscard]] virtual uint32_t            getRenderBufferCount      () const = 0;
        [[nodiscard]] virtual const RenderBuffer& getRenderBuffer           (RenderBufferHandle handle) const = 0;

        // Data buffers
        virtual DataBufferHandle            allocateDataBuffer              (EDataBufferType dataBufferType, EDataType dataType, uint32_t maximumSizeInBytes, DataBufferHandle handle) = 0;
        virtual void                        releaseDataBuffer               (DataBufferHandle handle) = 0;
        [[nodiscard]] virtual bool          isDataBufferAllocated           (DataBufferHandle handle) const = 0;
        [[nodiscard]] virtual uint32_t      getDataBufferCount              () const = 0;
        virtual void                        updateDataBuffer                (DataBufferHandle handle, uint32_t offsetInBytes, uint32_t dataSizeInBytes, const std::byte* data) = 0;
        [[nodiscard]] virtual const GeometryDataBuffer& getDataBuffer       (DataBufferHandle handle) const = 0;

        //Texture buffers
        virtual TextureBufferHandle         allocateTextureBuffer           (EPixelStorageFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle) = 0;
        virtual void                        releaseTextureBuffer            (TextureBufferHandle handle) = 0;
        [[nodiscard]] virtual bool          isTextureBufferAllocated        (TextureBufferHandle handle) const = 0;
        [[nodiscard]] virtual uint32_t      getTextureBufferCount           () const = 0;
        virtual void                        updateTextureBuffer             (TextureBufferHandle handle, uint32_t mipLevel, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const std::byte* data) = 0;
        [[nodiscard]] virtual const TextureBuffer& getTextureBuffer         (TextureBufferHandle handle) const = 0;

        // Data slots
        virtual DataSlotHandle                allocateDataSlot                (const DataSlot& dataSlot, DataSlotHandle handle) = 0;
        virtual void                          releaseDataSlot                 (DataSlotHandle handle) = 0;
        virtual void                          setDataSlotTexture              (DataSlotHandle handle, const ResourceContentHash& texture) = 0;
        [[nodiscard]] virtual bool            isDataSlotAllocated             (DataSlotHandle handle) const = 0;
        [[nodiscard]] virtual uint32_t        getDataSlotCount                () const = 0;
        [[nodiscard]] virtual const DataSlot& getDataSlot                     (DataSlotHandle handle) const = 0;

        // Scene references
        virtual SceneReferenceHandle        allocateSceneReference          (SceneId sceneId, SceneReferenceHandle handle) = 0;
        virtual void                        releaseSceneReference           (SceneReferenceHandle handle) = 0;
        virtual void                        requestSceneReferenceState      (SceneReferenceHandle handle, RendererSceneState state) = 0;
        virtual void                        requestSceneReferenceFlushNotifications(SceneReferenceHandle handle, bool enable) = 0;
        virtual void                        setSceneReferenceRenderOrder    (SceneReferenceHandle handle, int32_t renderOrder) = 0;
        [[nodiscard]] virtual bool                  isSceneReferenceAllocated(SceneReferenceHandle handle) const = 0;
        [[nodiscard]] virtual uint32_t              getSceneReferenceCount   () const = 0;
        [[nodiscard]] virtual const SceneReference& getSceneReference        (SceneReferenceHandle handle) const = 0;
    };
}
