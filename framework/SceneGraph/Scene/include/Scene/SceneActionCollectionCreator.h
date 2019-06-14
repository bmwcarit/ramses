//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEACTIONCOLLECTIONCREATOR_H
#define RAMSES_SCENEACTIONCOLLECTIONCREATOR_H

#include "TransformPropertyType.h"
#include "Scene/SceneActionCollection.h"
#include "AnimationAPI/IAnimationSystem.h"
#include "SceneAPI/ERenderableDataSlotType.h"
#include "SceneAPI/RenderState.h"
#include "SceneAPI/EDataType.h"
#include "SceneAPI/EFixedSemantics.h"
#include "SceneAPI/ECameraProjectionType.h"
#include "SceneAPI/TextureEnums.h"
#include "SceneAPI/EDataSlotType.h"
#include "SceneAPI/DataSlot.h"
#include "SceneAPI/SceneVersionTag.h"
#include "SceneAPI/SceneSizeInformation.h"
#include "SceneAPI/DataFieldInfo.h"
#include "SceneAPI/MipMapSize.h"
#include "Scene/SceneResourceChanges.h"
#include "Resource/TextureMetaInfo.h"
#include "Components/FlushTimeInformation.h"


namespace ramses_internal
{
    struct PixelRectangle;
    class IResource;
    enum class EDataBufferType : UInt8;
    struct FlushTimeInformation;
    struct Viewport;
    struct Frustum;
    struct Renderable;
    struct TextureSampler;
    struct RenderBuffer;

    class SceneActionCollectionCreator
    {
    public:
        SceneActionCollectionCreator(SceneActionCollection& collection_);

        void preallocateSceneSize(const SceneSizeInformation& sizeInfo);

        // Renderable allocation
        void allocateRenderable(NodeHandle nodeHandle, RenderableHandle handle);
        void releaseRenderable(RenderableHandle renderableHandle);

        // Renderable data (stuff required for rendering)
        void setRenderableEffect(RenderableHandle renderableHandle, const ResourceContentHash& effectHash);
        void setRenderableDataInstance(RenderableHandle renderableHandle, ERenderableDataSlotType slot, DataInstanceHandle newDataInstance);
        void setRenderableStartIndex(RenderableHandle renderableHandle, UInt32 startIndex);
        void setRenderableIndexCount(RenderableHandle renderableHandle, UInt32 indexCount);
        void setRenderableRenderState(RenderableHandle renderableHandle, RenderStateHandle stateHandle);
        void setRenderableVisibility(RenderableHandle renderableHandle, Bool visible);
        void setRenderableInstanceCount(RenderableHandle renderableHandle, UInt32 instanceCount);

        // Render state allocation
        void allocateRenderState(RenderStateHandle stateHandle);
        void releaseRenderState(RenderStateHandle stateHandle);

        // Render state setters
        void setRenderStateBlendFactors(RenderStateHandle stateHandle, EBlendFactor srcColor, EBlendFactor destColor, EBlendFactor srcAlpha, EBlendFactor destAlpha);
        void setRenderStateBlendOperations(RenderStateHandle stateHandle, EBlendOperation operationColor, EBlendOperation operationAlpha);
        void setRenderStateCullMode(RenderStateHandle stateHandle, ECullMode cullMode);
        void setRenderStateDrawMode(RenderStateHandle stateHandle, EDrawMode drawMode);
        void setRenderStateDepthFunc(RenderStateHandle stateHandle, EDepthFunc func);
        void setRenderStateDepthWrite(RenderStateHandle stateHandle, EDepthWrite flag);
        void setRenderStateScissorTest(RenderStateHandle stateHandle, EScissorTest flag, const RenderState::ScissorRegion& region);
        void setRenderStateStencilFunc(RenderStateHandle stateHandle, EStencilFunc func, UInt8 ref, UInt8 mask);
        void setRenderStateStencilOps(RenderStateHandle stateHandle, EStencilOp sfail, EStencilOp dpfail, EStencilOp dppass);
        void setRenderStateColorWriteMask(RenderStateHandle stateHandle, ColorWriteMask colorMask);

        // Camera
        void allocateCamera(ECameraProjectionType type, NodeHandle nodeHandle, DataInstanceHandle viewportDataInstance, CameraHandle handle);
        void releaseCamera(CameraHandle cameraHandle);
        void setCameraViewport(CameraHandle cameraHandle, const Viewport& vp);
        void setCameraFrustum(CameraHandle cameraHandle, const Frustum& frustum);

        // Creation/Deletion
        void allocateNode(UInt32 childrenCount, NodeHandle handle);
        void releaseNode(NodeHandle nodeHandle);

        void allocateTransform(NodeHandle nodeHandle, TransformHandle handle);
        void releaseTransform(TransformHandle transform);

        // Parent-child relationship
        void addChildToNode(NodeHandle parent, NodeHandle child);
        void removeChildFromNode(NodeHandle parent, NodeHandle child);

        // Transformation
        void setTransformComponent(ETransformPropertyType propertyChanged, TransformHandle node, const Vector3& newValue);

        void allocateDataLayout(const DataFieldInfoVector& dataFields, DataLayoutHandle handle);
        void releaseDataLayout(DataLayoutHandle layoutHandle);

        void allocateDataInstance(DataLayoutHandle finishedLayoutHandle, DataInstanceHandle instanceHandle);
        void releaseDataInstance(DataInstanceHandle containerHandle);

        void setDataFloatArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Float* data);
        void setDataVector2fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector2* data);
        void setDataVector3fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector3* data);
        void setDataVector4fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector4* data);
        void setDataIntegerArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Int32* data);
        void setDataVector2iArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector2i* data);
        void setDataVector3iArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector3i* data);
        void setDataVector4iArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Vector4i* data);
        void setDataMatrix22fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix22f* data);
        void setDataMatrix33fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix33f* data);
        void setDataMatrix44fArray(DataInstanceHandle containerHandle, DataFieldHandle field, UInt32 elementCount, const Matrix44f* data);
        void setDataResource(DataInstanceHandle containerHandle, DataFieldHandle field, const ResourceContentHash& hash, DataBufferHandle dataBuffer, UInt32 instancingDivisor);
        void setDataTextureSamplerHandle(DataInstanceHandle containerHandle, DataFieldHandle field, TextureSamplerHandle samplerHandle);
        void setDataReference(DataInstanceHandle containerHandle, DataFieldHandle field, DataInstanceHandle dataRef);

        // Texture sampler description
        void allocateTextureSampler(const TextureSampler& sampler, TextureSamplerHandle handle);
        void releaseTextureSampler(TextureSamplerHandle handle);

        // Render groups
        void allocateRenderGroup(UInt32 renderableCount, UInt32 nestedGroupCount, RenderGroupHandle groupHandle);
        void releaseRenderGroup(RenderGroupHandle groupHandle);
        void addRenderableToRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle, Int32 order);
        void removeRenderableFromRenderGroup(RenderGroupHandle groupHandle, RenderableHandle renderableHandle);
        void addRenderGroupToRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild, Int32 order);
        void removeRenderGroupFromRenderGroup(RenderGroupHandle groupHandleParent, RenderGroupHandle groupHandleChild);

        // Render passes
        void allocateRenderPass(UInt32 renderGroupCount, RenderPassHandle passHandle);
        void releaseRenderPass(RenderPassHandle passHandle);
        void setRenderPassClearColor(RenderPassHandle passHandle, const Vector4& clearColor);
        void setRenderPassClearFlag(RenderPassHandle passHandle, UInt32 clearFlag);
        void setRenderPassCamera(RenderPassHandle passHandle, CameraHandle cameraHandle);
        void setRenderPassRenderTarget(RenderPassHandle passHandle, RenderTargetHandle targetHandle);
        void setRenderPassRenderOrder(RenderPassHandle passHandle, Int32 renderOrder);
        void setRenderPassEnabled(RenderPassHandle passHandle, Bool isEnabled);
        void setRenderPassRenderOnce(RenderPassHandle pass, Bool enabled);
        void retriggerRenderPassRenderOnce(RenderPassHandle pass);
        void addRenderGroupToRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle, Int32 order);
        void removeRenderGroupFromRenderPass(RenderPassHandle passHandle, RenderGroupHandle groupHandle);

        // Blit passes
        void allocateBlitPass(RenderBufferHandle sourceRenderBufferHandle, RenderBufferHandle destinationRenderBufferHandle, BlitPassHandle passHandle);
        void releaseBlitPass(BlitPassHandle passHandle);
        void setBlitPassRenderOrder(BlitPassHandle passHandle, Int32 renderOrder);
        void setBlitPassEnabled(BlitPassHandle passHandle, Bool isEnabled);
        void setBlitPassRegions(BlitPassHandle passHandle, const PixelRectangle& sourceRegion, const PixelRectangle& destinationRegion);

        // Render targets
        void allocateRenderTarget(RenderTargetHandle targetHandle);
        void releaseRenderTarget(RenderTargetHandle targetHandle);
        void addRenderTargetRenderBuffer(RenderTargetHandle targetHandle, RenderBufferHandle bufferHandle);

        // Render buffers
        void allocateRenderBuffer(const RenderBuffer& renderBuffer, RenderBufferHandle handle);
        void releaseRenderBuffer(RenderBufferHandle handle);

        // Stream textures
        void allocateStreamTexture(uint32_t streamSource, const ResourceContentHash& fallbackTextureHash, StreamTextureHandle streamTextureHandle);
        void releaseStreamTexture(StreamTextureHandle streamTextureHandle);
        void setStreamTextureForceFallback(StreamTextureHandle streamTextureHandle, Bool forceFallbackImage);

        // Data buffers
        void allocateDataBuffer(EDataBufferType dataBufferType, EDataType dataType, UInt32 maximumSizeInBytes, DataBufferHandle handle);
        void releaseDataBuffer(DataBufferHandle handle);
        void updateDataBuffer(DataBufferHandle handle, UInt32 offsetInBytes, UInt32 dataSizeInBytes, const Byte* data);

        // Texture buffers
        void allocateTextureBuffer(ETextureFormat textureFormat, const MipMapDimensions& mipMapDimensions, TextureBufferHandle handle);
        void releaseTextureBuffer(TextureBufferHandle handle);
        void updateTextureBuffer(TextureBufferHandle handle, UInt32 mipLevel, UInt32 x, UInt32 y, UInt32 width, UInt32 height, const Byte* data, UInt32 dataSize);

        void allocateDataSlot(const DataSlot& dataSlot, DataSlotHandle handle);
        void setDataSlotTexture(DataSlotHandle handle, const ResourceContentHash& texture);
        void releaseDataSlot(DataSlotHandle handle);

        void addAnimationSystem(AnimationSystemHandle animSystemhandle, UInt32 flags, const AnimationSystemSizeInformation& sizeInfo);
        void removeAnimationSystem(AnimationSystemHandle animSystemHandle);

        void animationSystemSetTime(AnimationSystemHandle animSystemHandle, const AnimationTime& globalTime);
        void animationSystemAllocateSpline(AnimationSystemHandle animSystemHandle, ESplineKeyType keyType, EDataTypeID dataTypeID, SplineHandle handle);
        void animationSystemAllocateDataBinding(AnimationSystemHandle animSystemHandle, TDataBindID dataBindID, MemoryHandle handle1, MemoryHandle handle2, DataBindHandle dataBindHandle);
        void animationSystemAllocateAnimationInstance(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, EInterpolationType interpolationType, EVectorComponent vectorComponent, AnimationInstanceHandle handle);
        void animationSystemAllocateAnimation(AnimationSystemHandle animSystemHandle, AnimationInstanceHandle animInstHandle, AnimationHandle handle);
        void animationSystemAddDataBindingToAnimationInstance(AnimationSystemHandle animSystemHandle, AnimationInstanceHandle handle, DataBindHandle dataBindHandle);
        void animationSystemSetSplineKeyBasicBool(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, Bool value);
        void animationSystemSetSplineKeyBasicInt32(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, Int32 value);
        void animationSystemSetSplineKeyBasicFloat(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, Float value);
        void animationSystemSetSplineKeyBasicVector2f(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2& value);
        void animationSystemSetSplineKeyBasicVector3f(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3& value);
        void animationSystemSetSplineKeyBasicVector4f(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4& value);
        void animationSystemSetSplineKeyBasicVector2i(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2i& value);
        void animationSystemSetSplineKeyBasicVector3i(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3i& value);
        void animationSystemSetSplineKeyBasicVector4i(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4i& value);
        void animationSystemSetSplineKeyTangentsInt32(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, Int32 value, const Vector2& tanIn, const Vector2& tanOut);
        void animationSystemSetSplineKeyTangentsFloat(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, Float value, const Vector2& tanIn, const Vector2& tanOut);
        void animationSystemSetSplineKeyTangentsVector2f(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2& value, const Vector2& tanIn, const Vector2& tanOut);
        void animationSystemSetSplineKeyTangentsVector3f(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3& value, const Vector2& tanIn, const Vector2& tanOut);
        void animationSystemSetSplineKeyTangentsVector4f(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4& value, const Vector2& tanIn, const Vector2& tanOut);
        void animationSystemSetSplineKeyTangentsVector2i(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2i& value, const Vector2& tanIn, const Vector2& tanOut);
        void animationSystemSetSplineKeyTangentsVector3i(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3i& value, const Vector2& tanIn, const Vector2& tanOut);
        void animationSystemSetSplineKeyTangentsVector4i(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4i& value, const Vector2& tanIn, const Vector2& tanOut);
        void animationSystemRemoveSplineKey(AnimationSystemHandle animSystemHandle, SplineHandle splineHandle, SplineKeyIndex keyIndex);
        void animationSystemSetAnimationStartTime(AnimationSystemHandle animSystemHandle, AnimationHandle handle, const AnimationTime& timeStamp);
        void animationSystemSetAnimationStopTime(AnimationSystemHandle animSystemHandle, AnimationHandle handle, const AnimationTime& timeStamp);
        void animationSystemSetAnimationProperties(AnimationSystemHandle animSystemHandle, AnimationHandle handle, Float playbackSpeed, UInt32 flags, AnimationTime::Duration loopDuration, const AnimationTime& timeStamp);
        void animationSystemStopAnimationAndRollback(AnimationSystemHandle animSystemHandle, AnimationHandle handle);
        void animationSystemRemoveSpline(AnimationSystemHandle animSystemHandle, SplineHandle handle);
        void animationSystemRemoveDataBinding(AnimationSystemHandle animSystemHandle, DataBindHandle handle);
        void animationSystemRemoveAnimationInstance(AnimationSystemHandle animSystemHandle, AnimationInstanceHandle handle);
        void animationSystemRemoveAnimation(AnimationSystemHandle animSystemHandle, AnimationHandle handle);

        void pushResource(const IResource& resource);

        void setAckFlushState(bool state);
        void flush(
            UInt64 flushIndex,
            Bool synchronous,
            Bool addSizeInfo,
            const SceneSizeInformation& sizeInfo = SceneSizeInformation(),
            const SceneResourceChanges& resourceChanges = SceneResourceChanges(),
            const FlushTimeInformation& flushTimeInfo = {},
            SceneVersionTag versionTag = InvalidSceneVersionTag,
            std::initializer_list<UInt64> additionalTimestamps = {});

        // compound actions
        void compoundRenderableEffectData(RenderableHandle renderableHandle
                                            , DataInstanceHandle uniformInstanceHandle
                                            , RenderStateHandle stateHandle
                                            , const ResourceContentHash& effectHash);

        void compoundRenderable(RenderableHandle renderableHandle, const Renderable& renderable);

        void compoundState(RenderStateHandle handle, const RenderState& rs);

        SceneActionCollection& collection;

    private:
        void putSceneSizeInformation(const SceneSizeInformation& sizeInfo);
    };
}

#endif
