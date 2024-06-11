//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/IOutputStream.h"
#include "internal/PlatformAbstraction/Collections/IInputStream.h"
#include "internal/Core/Common/BitForgeMacro.h"

namespace ramses::internal
{
    enum class ESceneActionId : uint32_t
    {
        // nodes
        SetTranslation = 0,
        SetRotation,
        SetScaling,
        AllocateNode,
        ReleaseNode,
        AllocateTransform,
        ReleaseTransform,
        AddChildToNode,
        RemoveChildFromNode,

        // data instances
        AllocateDataInstance,
        ReleaseDataInstance,
        AllocateDataLayout,
        ReleaseDataLayout,
        SetDataBooleanArray,
        SetDataIntegerArray,
        SetDataFloatArray,
        SetDataVector2fArray,
        SetDataVector3fArray,
        SetDataVector4fArray,
        SetDataVector2iArray,
        SetDataVector3iArray,
        SetDataVector4iArray,
        SetDataResource,
        SetRenderableEffect,
        SetDataTextureSamplerHandle,
        SetDataReference,
        SetDataMatrix22fArray,
        SetDataMatrix33fArray,
        SetDataMatrix44fArray,

        // Data Buffer
        AllocateDataBuffer,
        ReleaseDataBuffer,
        UpdateDataBuffer,

        // Texture Buffer
        AllocateTextureBuffer,
        ReleaseTextureBuffer,
        UpdateTextureBuffer,

        // renderable
        AllocateRenderable,
        ReleaseRenderable,
        SetRenderableStartIndex,
        SetRenderableIndexCount,
        SetRenderableVisibility,
        SetRenderableDataInstance,
        SetRenderableInstanceCount,
        SetRenderableStartVertex,

        // render states
        ReleaseState,
        AllocateRenderState,
        SetRenderableState,
        SetStateStencilOps,
        SetStateStencilFunc,
        SetStateDepthWrite,
        SetStateDepthFunc,
        SetStateScissorTest,
        SetStateCullMode,
        SetStateDrawMode,
        SetStateBlendOperations,
        SetStateBlendFactors,
        SetStateBlendColor,
        SetStateColorWriteMask,

        // camera
        AllocateCamera,
        ReleaseCamera,

        // render groups
        AllocateRenderGroup,
        ReleaseRenderGroup,
        AddRenderableToRenderGroup,
        RemoveRenderableFromRenderGroup,
        AddRenderGroupToRenderGroup,
        RemoveRenderGroupFromRenderGroup,

        // render pass
        AllocateRenderPass,
        ReleaseRenderPass,
        SetRenderPassClearColor,
        SetRenderPassClearFlag,
        SetRenderPassCamera,
        SetRenderPassRenderTarget,
        SetRenderPassRenderOrder,
        SetRenderPassEnabled,
        SetRenderPassRenderOnce,
        RetriggerRenderPassRenderOnce,
        AddRenderGroupToRenderPass,
        RemoveRenderGroupFromRenderPass,

        // blit pass
        AllocateBlitPass,
        ReleaseBlitPass,
        SetBlitPassRenderOrder,
        SetBlitPassEnabled,
        SetBlitPassRegions,

        // pickable object
        AllocatePickableObject,
        ReleasePickableObject,
        SetPickableObjectId,
        SetPickableObjectCamera,
        SetPickableObjectEnabled,

        // render target/buffer/sampler
        AllocateTextureSampler,
        ReleaseTextureSampler,
        AllocateRenderTarget,
        ReleaseRenderTarget,
        AddRenderTargetRenderBuffer,
        AllocateRenderBuffer,
        ReleaseRenderBuffer,
        SetRenderBufferProperties,

        // data links
        AllocateDataSlot,
        SetDataSlotTexture,
        ReleaseDataSlot,

        // scene references
        AllocateSceneReference,
        ReleaseSceneReference,
        RequestSceneReferenceState,
        SetSceneReferenceRenderOrder,
        RequestSceneReferenceFlushNotifications,
        //animation
        AddAnimationSystem,
        RemoveAnimationSystem,
        AnimationSystemSetTime,
        AnimationSystemAllocateSpline,
        AnimationSystemAllocateDataBinding,
        AnimationSystemAllocateAnimationInstance,
        AnimationSystemAllocateAnimation,
        AnimationSystemAddDataBindingToAnimationInstance,
        AnimationSystemSetSplineKeyBasicBool,
        AnimationSystemSetSplineKeyBasicInt32,
        AnimationSystemSetSplineKeyBasicFloat,
        AnimationSystemSetSplineKeyBasicVector2f,
        AnimationSystemSetSplineKeyBasicVector3f,
        AnimationSystemSetSplineKeyBasicVector4f,
        AnimationSystemSetSplineKeyBasicVector2i,
        AnimationSystemSetSplineKeyBasicVector3i,
        AnimationSystemSetSplineKeyBasicVector4i,
        AnimationSystemSetSplineKeyTangentsInt32,
        AnimationSystemSetSplineKeyTangentsFloat,
        AnimationSystemSetSplineKeyTangentsVector2f,
        AnimationSystemSetSplineKeyTangentsVector3f,
        AnimationSystemSetSplineKeyTangentsVector4f,
        AnimationSystemSetSplineKeyTangentsVector2i,
        AnimationSystemSetSplineKeyTangentsVector3i,
        AnimationSystemSetSplineKeyTangentsVector4i,
        AnimationSystemRemoveSplineKey,
        AnimationSystemSetAnimationStartTime,
        AnimationSystemSetAnimationStopTime,
        AnimationSystemSetAnimationProperties,
        AnimationSystemStopAnimationAndRollback,
        AnimationSystemRemoveSpline,
        AnimationSystemRemoveDataBinding,
        AnimationSystemRemoveAnimationInstance,
        AnimationSystemRemoveAnimation,

        PreallocateSceneSize,

        TestAction,

        CompoundRenderable,
        CompoundRenderableEffectData,
        CompoundState,

        Incomplete,

        // Uniform Buffer
        AllocateUniformBuffer,
        ReleaseUniformBuffer,
        UpdateUniformBuffer,
        SetDataUniformBuffer,

        NUMBER_OF_TYPES
    };

    static constexpr const uint32_t NumOfSceneActionTypes = static_cast<uint32_t>(ESceneActionId::NUMBER_OF_TYPES);

#ifndef CreateNameForEnumID
#define CreateNameForEnumID(ENUMVALUE) \
case ENUMVALUE: return #ENUMVALUE
#endif

    inline
        const char* GetNameForSceneActionId(ESceneActionId type)
    {
        switch (type)
        {
            // nodes
            CreateNameForEnumID(ESceneActionId::SetTranslation);
            CreateNameForEnumID(ESceneActionId::SetRotation);
            CreateNameForEnumID(ESceneActionId::SetScaling);
            CreateNameForEnumID(ESceneActionId::AllocateNode);
            CreateNameForEnumID(ESceneActionId::ReleaseNode);
            CreateNameForEnumID(ESceneActionId::AllocateTransform);
            CreateNameForEnumID(ESceneActionId::ReleaseTransform);
            CreateNameForEnumID(ESceneActionId::AddChildToNode);
            CreateNameForEnumID(ESceneActionId::RemoveChildFromNode);

            // data instances
            CreateNameForEnumID(ESceneActionId::AllocateDataInstance);
            CreateNameForEnumID(ESceneActionId::ReleaseDataInstance);
            CreateNameForEnumID(ESceneActionId::AllocateDataLayout);
            CreateNameForEnumID(ESceneActionId::ReleaseDataLayout);
            CreateNameForEnumID(ESceneActionId::SetDataBooleanArray);
            CreateNameForEnumID(ESceneActionId::SetDataIntegerArray);
            CreateNameForEnumID(ESceneActionId::SetDataFloatArray);
            CreateNameForEnumID(ESceneActionId::SetDataVector2fArray);
            CreateNameForEnumID(ESceneActionId::SetDataVector3fArray);
            CreateNameForEnumID(ESceneActionId::SetDataVector4fArray);
            CreateNameForEnumID(ESceneActionId::SetDataVector2iArray);
            CreateNameForEnumID(ESceneActionId::SetDataVector3iArray);
            CreateNameForEnumID(ESceneActionId::SetDataVector4iArray);
            CreateNameForEnumID(ESceneActionId::SetDataResource);
            CreateNameForEnumID(ESceneActionId::SetRenderableEffect);
            CreateNameForEnumID(ESceneActionId::SetDataTextureSamplerHandle);
            CreateNameForEnumID(ESceneActionId::SetDataReference);
            CreateNameForEnumID(ESceneActionId::SetDataUniformBuffer);
            CreateNameForEnumID(ESceneActionId::SetDataMatrix22fArray);
            CreateNameForEnumID(ESceneActionId::SetDataMatrix33fArray);
            CreateNameForEnumID(ESceneActionId::SetDataMatrix44fArray);

            // Data Buffer
            CreateNameForEnumID(ESceneActionId::AllocateDataBuffer);
            CreateNameForEnumID(ESceneActionId::ReleaseDataBuffer);
            CreateNameForEnumID(ESceneActionId::UpdateDataBuffer);

            // Uniform buffer
            CreateNameForEnumID(ESceneActionId::AllocateUniformBuffer);
            CreateNameForEnumID(ESceneActionId::ReleaseUniformBuffer);
            CreateNameForEnumID(ESceneActionId::UpdateUniformBuffer);

            // Texture Buffer
            CreateNameForEnumID(ESceneActionId::AllocateTextureBuffer);
            CreateNameForEnumID(ESceneActionId::ReleaseTextureBuffer);
            CreateNameForEnumID(ESceneActionId::UpdateTextureBuffer);

            // renderable
            CreateNameForEnumID(ESceneActionId::AllocateRenderable);
            CreateNameForEnumID(ESceneActionId::ReleaseRenderable);
            CreateNameForEnumID(ESceneActionId::SetRenderableStartIndex);
            CreateNameForEnumID(ESceneActionId::SetRenderableIndexCount);
            CreateNameForEnumID(ESceneActionId::SetRenderableVisibility);
            CreateNameForEnumID(ESceneActionId::SetRenderableDataInstance);
            CreateNameForEnumID(ESceneActionId::SetRenderableInstanceCount);
            CreateNameForEnumID(ESceneActionId::SetRenderableStartVertex);

            // render states
            CreateNameForEnumID(ESceneActionId::ReleaseState);
            CreateNameForEnumID(ESceneActionId::AllocateRenderState);
            CreateNameForEnumID(ESceneActionId::SetRenderableState);
            CreateNameForEnumID(ESceneActionId::SetStateStencilOps);
            CreateNameForEnumID(ESceneActionId::SetStateStencilFunc);
            CreateNameForEnumID(ESceneActionId::SetStateDepthWrite);
            CreateNameForEnumID(ESceneActionId::SetStateDepthFunc);
            CreateNameForEnumID(ESceneActionId::SetStateScissorTest);
            CreateNameForEnumID(ESceneActionId::SetStateCullMode);
            CreateNameForEnumID(ESceneActionId::SetStateDrawMode);
            CreateNameForEnumID(ESceneActionId::SetStateBlendOperations);
            CreateNameForEnumID(ESceneActionId::SetStateBlendFactors);
            CreateNameForEnumID(ESceneActionId::SetStateBlendColor);
            CreateNameForEnumID(ESceneActionId::SetStateColorWriteMask);

            // camera
            CreateNameForEnumID(ESceneActionId::AllocateCamera);
            CreateNameForEnumID(ESceneActionId::ReleaseCamera);

            // render group
            CreateNameForEnumID(ESceneActionId::AllocateRenderGroup);
            CreateNameForEnumID(ESceneActionId::ReleaseRenderGroup);
            CreateNameForEnumID(ESceneActionId::AddRenderableToRenderGroup);
            CreateNameForEnumID(ESceneActionId::RemoveRenderableFromRenderGroup);
            CreateNameForEnumID(ESceneActionId::AddRenderGroupToRenderGroup);
            CreateNameForEnumID(ESceneActionId::RemoveRenderGroupFromRenderGroup);

            // render pass
            CreateNameForEnumID(ESceneActionId::AllocateRenderPass);
            CreateNameForEnumID(ESceneActionId::ReleaseRenderPass);
            CreateNameForEnumID(ESceneActionId::SetRenderPassClearColor);
            CreateNameForEnumID(ESceneActionId::SetRenderPassClearFlag);
            CreateNameForEnumID(ESceneActionId::SetRenderPassCamera);
            CreateNameForEnumID(ESceneActionId::SetRenderPassRenderTarget);
            CreateNameForEnumID(ESceneActionId::SetRenderPassRenderOrder);
            CreateNameForEnumID(ESceneActionId::SetRenderPassEnabled);
            CreateNameForEnumID(ESceneActionId::SetRenderPassRenderOnce);
            CreateNameForEnumID(ESceneActionId::RetriggerRenderPassRenderOnce);
            CreateNameForEnumID(ESceneActionId::AddRenderGroupToRenderPass);
            CreateNameForEnumID(ESceneActionId::RemoveRenderGroupFromRenderPass);

            // blit pass
            CreateNameForEnumID(ESceneActionId::AllocateBlitPass);
            CreateNameForEnumID(ESceneActionId::ReleaseBlitPass);
            CreateNameForEnumID(ESceneActionId::SetBlitPassRenderOrder);
            CreateNameForEnumID(ESceneActionId::SetBlitPassEnabled);
            CreateNameForEnumID(ESceneActionId::SetBlitPassRegions);

            // pickable object
            CreateNameForEnumID(ESceneActionId::AllocatePickableObject);
            CreateNameForEnumID(ESceneActionId::ReleasePickableObject);
            CreateNameForEnumID(ESceneActionId::SetPickableObjectId);
            CreateNameForEnumID(ESceneActionId::SetPickableObjectCamera);
            CreateNameForEnumID(ESceneActionId::SetPickableObjectEnabled);

            // render target/buffer/sampler
            CreateNameForEnumID(ESceneActionId::AllocateTextureSampler);
            CreateNameForEnumID(ESceneActionId::ReleaseTextureSampler);
            CreateNameForEnumID(ESceneActionId::AllocateRenderTarget);
            CreateNameForEnumID(ESceneActionId::ReleaseRenderTarget);
            CreateNameForEnumID(ESceneActionId::AddRenderTargetRenderBuffer);
            CreateNameForEnumID(ESceneActionId::AllocateRenderBuffer);
            CreateNameForEnumID(ESceneActionId::ReleaseRenderBuffer);
            CreateNameForEnumID(ESceneActionId::SetRenderBufferProperties);

            // data links
            CreateNameForEnumID(ESceneActionId::AllocateDataSlot);
            CreateNameForEnumID(ESceneActionId::SetDataSlotTexture);
            CreateNameForEnumID(ESceneActionId::ReleaseDataSlot);

            CreateNameForEnumID(ESceneActionId::AllocateSceneReference);
            CreateNameForEnumID(ESceneActionId::ReleaseSceneReference);
            CreateNameForEnumID(ESceneActionId::RequestSceneReferenceState);
            CreateNameForEnumID(ESceneActionId::SetSceneReferenceRenderOrder);
            CreateNameForEnumID(ESceneActionId::RequestSceneReferenceFlushNotifications);
            //animation
            CreateNameForEnumID(ESceneActionId::AddAnimationSystem);
            CreateNameForEnumID(ESceneActionId::RemoveAnimationSystem);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetTime);
            CreateNameForEnumID(ESceneActionId::AnimationSystemAllocateSpline);
            CreateNameForEnumID(ESceneActionId::AnimationSystemAllocateDataBinding);
            CreateNameForEnumID(ESceneActionId::AnimationSystemAllocateAnimationInstance);
            CreateNameForEnumID(ESceneActionId::AnimationSystemAllocateAnimation);
            CreateNameForEnumID(ESceneActionId::AnimationSystemAddDataBindingToAnimationInstance);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetSplineKeyBasicBool);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetSplineKeyBasicInt32);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetSplineKeyBasicFloat);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetSplineKeyBasicVector2f);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetSplineKeyBasicVector3f);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetSplineKeyBasicVector4f);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetSplineKeyBasicVector2i);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetSplineKeyBasicVector3i);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetSplineKeyBasicVector4i);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetSplineKeyTangentsInt32);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetSplineKeyTangentsFloat);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetSplineKeyTangentsVector2f);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetSplineKeyTangentsVector3f);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetSplineKeyTangentsVector4f);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetSplineKeyTangentsVector2i);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetSplineKeyTangentsVector3i);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetSplineKeyTangentsVector4i);
            CreateNameForEnumID(ESceneActionId::AnimationSystemRemoveSplineKey);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetAnimationStartTime);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetAnimationStopTime);
            CreateNameForEnumID(ESceneActionId::AnimationSystemSetAnimationProperties);
            CreateNameForEnumID(ESceneActionId::AnimationSystemStopAnimationAndRollback);
            CreateNameForEnumID(ESceneActionId::AnimationSystemRemoveSpline);
            CreateNameForEnumID(ESceneActionId::AnimationSystemRemoveDataBinding);
            CreateNameForEnumID(ESceneActionId::AnimationSystemRemoveAnimationInstance);
            CreateNameForEnumID(ESceneActionId::AnimationSystemRemoveAnimation);

            CreateNameForEnumID(ESceneActionId::PreallocateSceneSize);

            CreateNameForEnumID(ESceneActionId::TestAction);

            CreateNameForEnumID(ESceneActionId::CompoundRenderable);
            CreateNameForEnumID(ESceneActionId::CompoundRenderableEffectData);
            CreateNameForEnumID(ESceneActionId::CompoundState);

            CreateNameForEnumID(ESceneActionId::Incomplete);

        case ESceneActionId::NUMBER_OF_TYPES:
            break;
        }
        return "Unknown Message Type";
    }
}
