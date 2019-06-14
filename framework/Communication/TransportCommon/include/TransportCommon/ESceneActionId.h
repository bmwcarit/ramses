//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ESCENEACTIONID_H
#define RAMSES_ESCENEACTIONID_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Collections/IOutputStream.h"
#include "Collections/IInputStream.h"
#include "Collections/Vector.h"
#include "Common/BitForgeMacro.h"

namespace ramses_internal
{
    typedef UInt32 SceneActionId;

    enum ESceneActionId
    {
        // nodes
        ESceneActionId_SetTransformComponent = 0,
        ESceneActionId_AllocateNode,
        ESceneActionId_ReleaseNode,
        ESceneActionId_AllocateTransform,
        ESceneActionId_ReleaseTransform,
        ESceneActionId_AddChildToNode,
        ESceneActionId_RemoveChildFromNode,

        // data instances
        ESceneActionId_AllocateDataInstance,
        ESceneActionId_ReleaseDataInstance,
        ESceneActionId_AllocateDataLayout,
        ESceneActionId_ReleaseDataLayout,
        ESceneActionId_SetDataIntegerArray,
        ESceneActionId_SetDataFloatArray,
        ESceneActionId_SetDataVector2fArray,
        ESceneActionId_SetDataVector3fArray,
        ESceneActionId_SetDataVector4fArray,
        ESceneActionId_SetDataVector2iArray,
        ESceneActionId_SetDataVector3iArray,
        ESceneActionId_SetDataVector4iArray,
        ESceneActionId_SetDataResource,
        ESceneActionId_SetRenderableEffect,
        ESceneActionId_SetDataTextureSamplerHandle,
        ESceneActionId_SetDataReference,
        ESceneActionId_SetDataMatrix22fArray,
        ESceneActionId_SetDataMatrix33fArray,
        ESceneActionId_SetDataMatrix44fArray,

        // Stream Texture
        ESceneActionId_AllocateStreamTexture,
        ESceneActionId_ReleaseStreamTexture,
        ESceneActionId_SetForceFallback,

        // Data Buffer
        ESceneActionId_AllocateDataBuffer,
        ESceneActionId_ReleaseDataBuffer,
        ESceneActionId_UpdateDataBuffer,

        // Texture Buffer
        ESceneActionId_AllocateTextureBuffer,
        ESceneActionId_ReleaseTextureBuffer,
        ESceneActionId_UpdateTextureBuffer,

        // renderable
        ESceneActionId_AllocateRenderable,
        ESceneActionId_ReleaseRenderable,
        ESceneActionId_SetRenderableStartIndex,
        ESceneActionId_SetRenderableIndexCount,
        ESceneActionId_SetRenderableVisibility,
        ESceneActionId_SetRenderableDataInstance,
        ESceneActionId_SetRenderableInstanceCount,

        // render states
        ESceneActionId_ReleaseState,
        ESceneActionId_AllocateRenderState,
        ESceneActionId_SetRenderableState,
        ESceneActionId_SetStateStencilOps,
        ESceneActionId_SetStateStencilFunc,
        ESceneActionId_SetStateDepthWrite,
        ESceneActionId_SetStateDepthFunc,
        ESceneActionId_SetStateScissorTest,
        ESceneActionId_SetStateCullMode,
        ESceneActionId_SetStateDrawMode,
        ESceneActionId_SetStateBlendOperations,
        ESceneActionId_SetStateBlendFactors,
        ESceneActionId_SetStateColorWriteMask,

        // camera
        ESceneActionId_AllocateCamera,
        ESceneActionId_ReleaseCamera,
        ESceneActionId_SetCameraFrustum,

        // render groups
        ESceneActionId_AllocateRenderGroup,
        ESceneActionId_ReleaseRenderGroup,
        ESceneActionId_AddRenderableToRenderGroup,
        ESceneActionId_RemoveRenderableFromRenderGroup,
        ESceneActionId_AddRenderGroupToRenderGroup,
        ESceneActionId_RemoveRenderGroupFromRenderGroup,

        // render pass
        ESceneActionId_AllocateRenderPass,
        ESceneActionId_ReleaseRenderPass,
        ESceneActionId_SetRenderPassClearColor,
        ESceneActionId_SetRenderPassClearFlag,
        ESceneActionId_SetRenderPassCamera,
        ESceneActionId_SetRenderPassRenderTarget,
        ESceneActionId_SetRenderPassRenderOrder,
        ESceneActionId_SetRenderPassEnabled,
        ESceneActionId_SetRenderPassRenderOnce,
        ESceneActionId_RetriggerRenderPassRenderOnce,
        ESceneActionId_AddRenderGroupToRenderPass,
        ESceneActionId_RemoveRenderGroupFromRenderPass,

        // blit pass
        ESceneActionId_AllocateBlitPass,
        ESceneActionId_ReleaseBlitPass,
        ESceneActionId_SetBlitPassRenderOrder,
        ESceneActionId_SetBlitPassEnabled,
        ESceneActionId_SetBlitPassRegions,

        // render target/buffer/sampler
        ESceneActionId_AllocateTextureSampler,
        ESceneActionId_ReleaseTextureSampler,
        ESceneActionId_AllocateRenderTarget,
        ESceneActionId_ReleaseRenderTarget,
        ESceneActionId_AddRenderTargetRenderBuffer,
        ESceneActionId_AllocateRenderBuffer,
        ESceneActionId_ReleaseRenderBuffer,

        // data links
        ESceneActionId_AllocateDataSlot,
        ESceneActionId_SetDataSlotTexture,
        ESceneActionId_ReleaseDataSlot,

        //animation
        ESceneActionId_AddAnimationSystem,
        ESceneActionId_RemoveAnimationSystem,
        ESceneActionId_AnimationSystemSetTime,
        ESceneActionId_AnimationSystemAllocateSpline,
        ESceneActionId_AnimationSystemAllocateDataBinding,
        ESceneActionId_AnimationSystemAllocateAnimationInstance,
        ESceneActionId_AnimationSystemAllocateAnimation,
        ESceneActionId_AnimationSystemAddDataBindingToAnimationInstance,
        ESceneActionId_AnimationSystemSetSplineKeyBasicBool,
        ESceneActionId_AnimationSystemSetSplineKeyBasicInt32,
        ESceneActionId_AnimationSystemSetSplineKeyBasicFloat,
        ESceneActionId_AnimationSystemSetSplineKeyBasicVector2f,
        ESceneActionId_AnimationSystemSetSplineKeyBasicVector3f,
        ESceneActionId_AnimationSystemSetSplineKeyBasicVector4f,
        ESceneActionId_AnimationSystemSetSplineKeyBasicVector2i,
        ESceneActionId_AnimationSystemSetSplineKeyBasicVector3i,
        ESceneActionId_AnimationSystemSetSplineKeyBasicVector4i,
        ESceneActionId_AnimationSystemSetSplineKeyTangentsInt32,
        ESceneActionId_AnimationSystemSetSplineKeyTangentsFloat,
        ESceneActionId_AnimationSystemSetSplineKeyTangentsVector2f,
        ESceneActionId_AnimationSystemSetSplineKeyTangentsVector3f,
        ESceneActionId_AnimationSystemSetSplineKeyTangentsVector4f,
        ESceneActionId_AnimationSystemSetSplineKeyTangentsVector2i,
        ESceneActionId_AnimationSystemSetSplineKeyTangentsVector3i,
        ESceneActionId_AnimationSystemSetSplineKeyTangentsVector4i,
        ESceneActionId_AnimationSystemRemoveSplineKey,
        ESceneActionId_AnimationSystemSetAnimationStartTime,
        ESceneActionId_AnimationSystemSetAnimationStopTime,
        ESceneActionId_AnimationSystemSetAnimationProperties,
        ESceneActionId_AnimationSystemStopAnimationAndRollback,
        ESceneActionId_AnimationSystemRemoveSpline,
        ESceneActionId_AnimationSystemRemoveDataBinding,
        ESceneActionId_AnimationSystemRemoveAnimationInstance,
        ESceneActionId_AnimationSystemRemoveAnimation,

        ESceneActionId_PreallocateSceneSize,

        ESceneActionId_PushResource,

        ESceneActionId_SetAckFlushState,
        ESceneActionId_Flush,

        ESceneActionId_TestAction,

        ESceneActionId_CompoundRenderable,
        ESceneActionId_CompoundRenderableEffectData,
        ESceneActionId_CompoundState,

        ESceneActionId_Incomplete,

        ESceneActionId_NUMBER_OF_TYPES
    };

    typedef std::vector<ESceneActionId> SceneActionIdVector;
    typedef std::vector<UInt64> TimeStampVector;

    enum ESceneActionFlushBits : uint32_t
    {
        ESceneActionFlushBits_Synchronous               = BIT(0),
        ESceneActionFlushBits_HasSizeInfo               = BIT(1),
        ESceneActionFlushBits_HasTimestamps             = BIT(2)
    };

#ifndef CreateNameForEnumID
#define CreateNameForEnumID(ENUMVALUE) \
case ENUMVALUE: return #ENUMVALUE
#endif

    inline
        const Char* GetNameForSceneActionId(ESceneActionId type)
    {
        switch (type)
        {
            // nodes
            CreateNameForEnumID(ESceneActionId_SetTransformComponent);
            CreateNameForEnumID(ESceneActionId_AllocateNode);
            CreateNameForEnumID(ESceneActionId_ReleaseNode);
            CreateNameForEnumID(ESceneActionId_AllocateTransform);
            CreateNameForEnumID(ESceneActionId_ReleaseTransform);
            CreateNameForEnumID(ESceneActionId_AddChildToNode);
            CreateNameForEnumID(ESceneActionId_RemoveChildFromNode);

            // data instances
            CreateNameForEnumID(ESceneActionId_AllocateDataInstance);
            CreateNameForEnumID(ESceneActionId_ReleaseDataInstance);
            CreateNameForEnumID(ESceneActionId_AllocateDataLayout);
            CreateNameForEnumID(ESceneActionId_ReleaseDataLayout);
            CreateNameForEnumID(ESceneActionId_SetDataIntegerArray);
            CreateNameForEnumID(ESceneActionId_SetDataFloatArray);
            CreateNameForEnumID(ESceneActionId_SetDataVector2fArray);
            CreateNameForEnumID(ESceneActionId_SetDataVector3fArray);
            CreateNameForEnumID(ESceneActionId_SetDataVector4fArray);
            CreateNameForEnumID(ESceneActionId_SetDataVector2iArray);
            CreateNameForEnumID(ESceneActionId_SetDataVector3iArray);
            CreateNameForEnumID(ESceneActionId_SetDataVector4iArray);
            CreateNameForEnumID(ESceneActionId_SetDataResource);
            CreateNameForEnumID(ESceneActionId_SetRenderableEffect);
            CreateNameForEnumID(ESceneActionId_SetDataTextureSamplerHandle);
            CreateNameForEnumID(ESceneActionId_SetDataReference);
            CreateNameForEnumID(ESceneActionId_SetDataMatrix22fArray);
            CreateNameForEnumID(ESceneActionId_SetDataMatrix33fArray);
            CreateNameForEnumID(ESceneActionId_SetDataMatrix44fArray);

            // Stream texture
            CreateNameForEnumID(ESceneActionId_AllocateStreamTexture);
            CreateNameForEnumID(ESceneActionId_ReleaseStreamTexture);
            CreateNameForEnumID(ESceneActionId_SetForceFallback);

            // Data Buffer
            CreateNameForEnumID(ESceneActionId_AllocateDataBuffer);
            CreateNameForEnumID(ESceneActionId_ReleaseDataBuffer);
            CreateNameForEnumID(ESceneActionId_UpdateDataBuffer);

            // Texture Buffer
            CreateNameForEnumID(ESceneActionId_AllocateTextureBuffer);
            CreateNameForEnumID(ESceneActionId_ReleaseTextureBuffer);
            CreateNameForEnumID(ESceneActionId_UpdateTextureBuffer);

            // renderable
            CreateNameForEnumID(ESceneActionId_AllocateRenderable);
            CreateNameForEnumID(ESceneActionId_ReleaseRenderable);
            CreateNameForEnumID(ESceneActionId_SetRenderableStartIndex);
            CreateNameForEnumID(ESceneActionId_SetRenderableIndexCount);
            CreateNameForEnumID(ESceneActionId_SetRenderableVisibility);
            CreateNameForEnumID(ESceneActionId_SetRenderableDataInstance);
            CreateNameForEnumID(ESceneActionId_SetRenderableInstanceCount);

            // render states
            CreateNameForEnumID(ESceneActionId_ReleaseState);
            CreateNameForEnumID(ESceneActionId_AllocateRenderState);
            CreateNameForEnumID(ESceneActionId_SetRenderableState);
            CreateNameForEnumID(ESceneActionId_SetStateStencilOps);
            CreateNameForEnumID(ESceneActionId_SetStateStencilFunc);
            CreateNameForEnumID(ESceneActionId_SetStateDepthWrite);
            CreateNameForEnumID(ESceneActionId_SetStateDepthFunc);
            CreateNameForEnumID(ESceneActionId_SetStateScissorTest);
            CreateNameForEnumID(ESceneActionId_SetStateCullMode);
            CreateNameForEnumID(ESceneActionId_SetStateDrawMode);
            CreateNameForEnumID(ESceneActionId_SetStateBlendOperations);
            CreateNameForEnumID(ESceneActionId_SetStateBlendFactors);
            CreateNameForEnumID(ESceneActionId_SetStateColorWriteMask);

            // camera
            CreateNameForEnumID(ESceneActionId_AllocateCamera);
            CreateNameForEnumID(ESceneActionId_ReleaseCamera);
            CreateNameForEnumID(ESceneActionId_SetCameraFrustum);

            // render group
            CreateNameForEnumID(ESceneActionId_AllocateRenderGroup);
            CreateNameForEnumID(ESceneActionId_ReleaseRenderGroup);
            CreateNameForEnumID(ESceneActionId_AddRenderableToRenderGroup);
            CreateNameForEnumID(ESceneActionId_RemoveRenderableFromRenderGroup);
            CreateNameForEnumID(ESceneActionId_AddRenderGroupToRenderGroup);
            CreateNameForEnumID(ESceneActionId_RemoveRenderGroupFromRenderGroup);

            // render pass
            CreateNameForEnumID(ESceneActionId_AllocateRenderPass);
            CreateNameForEnumID(ESceneActionId_ReleaseRenderPass);
            CreateNameForEnumID(ESceneActionId_SetRenderPassClearColor);
            CreateNameForEnumID(ESceneActionId_SetRenderPassClearFlag);
            CreateNameForEnumID(ESceneActionId_SetRenderPassCamera);
            CreateNameForEnumID(ESceneActionId_SetRenderPassRenderTarget);
            CreateNameForEnumID(ESceneActionId_SetRenderPassRenderOrder);
            CreateNameForEnumID(ESceneActionId_SetRenderPassEnabled);
            CreateNameForEnumID(ESceneActionId_SetRenderPassRenderOnce);
            CreateNameForEnumID(ESceneActionId_RetriggerRenderPassRenderOnce);
            CreateNameForEnumID(ESceneActionId_AddRenderGroupToRenderPass);
            CreateNameForEnumID(ESceneActionId_RemoveRenderGroupFromRenderPass);

            // blit pass
            CreateNameForEnumID(ESceneActionId_AllocateBlitPass);
            CreateNameForEnumID(ESceneActionId_ReleaseBlitPass);
            CreateNameForEnumID(ESceneActionId_SetBlitPassRenderOrder);
            CreateNameForEnumID(ESceneActionId_SetBlitPassEnabled);
            CreateNameForEnumID(ESceneActionId_SetBlitPassRegions);

            // render target/buffer/sampler
            CreateNameForEnumID(ESceneActionId_AllocateTextureSampler);
            CreateNameForEnumID(ESceneActionId_ReleaseTextureSampler);
            CreateNameForEnumID(ESceneActionId_AllocateRenderTarget);
            CreateNameForEnumID(ESceneActionId_ReleaseRenderTarget);
            CreateNameForEnumID(ESceneActionId_AddRenderTargetRenderBuffer);
            CreateNameForEnumID(ESceneActionId_AllocateRenderBuffer);
            CreateNameForEnumID(ESceneActionId_ReleaseRenderBuffer);

            // data links
            CreateNameForEnumID(ESceneActionId_AllocateDataSlot);
            CreateNameForEnumID(ESceneActionId_SetDataSlotTexture);
            CreateNameForEnumID(ESceneActionId_ReleaseDataSlot);

            //animation
            CreateNameForEnumID(ESceneActionId_AddAnimationSystem);
            CreateNameForEnumID(ESceneActionId_RemoveAnimationSystem);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetTime);
            CreateNameForEnumID(ESceneActionId_AnimationSystemAllocateSpline);
            CreateNameForEnumID(ESceneActionId_AnimationSystemAllocateDataBinding);
            CreateNameForEnumID(ESceneActionId_AnimationSystemAllocateAnimationInstance);
            CreateNameForEnumID(ESceneActionId_AnimationSystemAllocateAnimation);
            CreateNameForEnumID(ESceneActionId_AnimationSystemAddDataBindingToAnimationInstance);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetSplineKeyBasicBool);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetSplineKeyBasicInt32);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetSplineKeyBasicFloat);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetSplineKeyBasicVector2f);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetSplineKeyBasicVector3f);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetSplineKeyBasicVector4f);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetSplineKeyBasicVector2i);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetSplineKeyBasicVector3i);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetSplineKeyBasicVector4i);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetSplineKeyTangentsInt32);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetSplineKeyTangentsFloat);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetSplineKeyTangentsVector2f);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetSplineKeyTangentsVector3f);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetSplineKeyTangentsVector4f);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetSplineKeyTangentsVector2i);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetSplineKeyTangentsVector3i);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetSplineKeyTangentsVector4i);
            CreateNameForEnumID(ESceneActionId_AnimationSystemRemoveSplineKey);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetAnimationStartTime);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetAnimationStopTime);
            CreateNameForEnumID(ESceneActionId_AnimationSystemSetAnimationProperties);
            CreateNameForEnumID(ESceneActionId_AnimationSystemStopAnimationAndRollback);
            CreateNameForEnumID(ESceneActionId_AnimationSystemRemoveSpline);
            CreateNameForEnumID(ESceneActionId_AnimationSystemRemoveDataBinding);
            CreateNameForEnumID(ESceneActionId_AnimationSystemRemoveAnimationInstance);
            CreateNameForEnumID(ESceneActionId_AnimationSystemRemoveAnimation);

            CreateNameForEnumID(ESceneActionId_PreallocateSceneSize);

            CreateNameForEnumID(ESceneActionId_PushResource);

            CreateNameForEnumID(ESceneActionId_SetAckFlushState);
            CreateNameForEnumID(ESceneActionId_Flush);

            CreateNameForEnumID(ESceneActionId_TestAction);

            CreateNameForEnumID(ESceneActionId_CompoundRenderable);
            CreateNameForEnumID(ESceneActionId_CompoundRenderableEffectData);
            CreateNameForEnumID(ESceneActionId_CompoundState);

            CreateNameForEnumID(ESceneActionId_Incomplete);

        case ESceneActionId_NUMBER_OF_TYPES:
            break;
        }
        return "Unknown Message Type";
    }

    inline
        IOutputStream& operator<<(IOutputStream& outputStream, ESceneActionId messageId)
    {
        outputStream << static_cast<UInt32>(messageId);
        return outputStream;
    }

    inline
    IInputStream& operator>>(IInputStream& inputStream, ESceneActionId& messageId)
    {
        inputStream >> reinterpret_cast<UInt32&>(messageId);
        return inputStream;
    }
}

#endif
