//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Animation/SplineBase.h"
#include "Animation/AnimationDataBind.h"
#include "Animation/AnimationInstance.h"
#include "Animation/Animation.h"
#include "Animation/AnimationSystemDescriber.h"
#include "Scene/SceneActionCollectionCreator.h"

namespace ramses_internal
{
    void AnimationSystemDescriber::DescribeAnimationSystem(const IAnimationSystem& animationSystemSource, SceneActionCollectionCreator& collector, AnimationSystemHandle animSystemHandle)
    {
        CopySplines(animationSystemSource, collector, animSystemHandle);
        CopyDataBinds(animationSystemSource, collector, animSystemHandle);
        CopyAnimationInstances(animationSystemSource, collector, animSystemHandle);
        CopyAnimations(animationSystemSource, collector, animSystemHandle);

        collector.animationSystemSetTime(animSystemHandle, animationSystemSource.getTime());
    }

    void AnimationSystemDescriber::CopySplines(const IAnimationSystem& animationSystemSource, SceneActionCollectionCreator& collector, AnimationSystemHandle animSystemHandle)
    {
        const UInt32 numSplines = animationSystemSource.getTotalSplineCount();
        for (SplineHandle handle(0u); handle < numSplines; ++handle)
        {
            if (animationSystemSource.containsSpline(handle))
            {
                const SplineBase* const spline = animationSystemSource.getSpline(handle);
                if (spline != nullptr)
                {
                    collector.animationSystemAllocateSpline(animSystemHandle, spline->getKeyType(), spline->getDataType(), handle);
                    CopySpline(collector, animSystemHandle, spline, handle);
                }
            }
        }
    }

    void AnimationSystemDescriber::CopySpline(SceneActionCollectionCreator& collector, AnimationSystemHandle animSystemHandle, const SplineBase* const splineSource, SplineHandle splineTarget)
    {
        switch (splineSource->getKeyType())
        {
        case ESplineKeyType_Basic:
            CopySplineBasic(collector, animSystemHandle, splineSource, splineTarget);
            break;
        case ESplineKeyType_Tangents:
            CopySplineTangents(collector, animSystemHandle, splineSource, splineTarget);
            break;
        default:
            assert(false);
        }
    }

    void AnimationSystemDescriber::CopySplineBasic(SceneActionCollectionCreator& collector, AnimationSystemHandle animSystemHandle, const SplineBase* const splineSource, SplineHandle splineTarget)
    {
        const UInt32 numKeys = splineSource->getNumKeys();
        for (UInt32 i = 0u; i < numKeys; ++i)
        {
            const SplineTimeStamp timeStamp = splineSource->getTimeStamp(i);
            switch (splineSource->getDataType())
            {
            case EDataTypeID_Boolean:
                collector.animationSystemSetSplineKeyBasicBool(animSystemHandle, splineTarget, timeStamp, GetSplineKeyValue<bool>(splineSource, i));
                break;
            case EDataTypeID_Int32:
                collector.animationSystemSetSplineKeyBasicInt32(animSystemHandle, splineTarget, timeStamp, GetSplineKeyValue<Int32>(splineSource, i));
                break;
            case EDataTypeID_Float:
                collector.animationSystemSetSplineKeyBasicFloat(animSystemHandle, splineTarget, timeStamp, GetSplineKeyValue<Float>(splineSource, i));
                break;
            case EDataTypeID_Vector2f:
                collector.animationSystemSetSplineKeyBasicVector2f(animSystemHandle, splineTarget, timeStamp, GetSplineKeyValue<Vector2>(splineSource, i));
                break;
            case EDataTypeID_Vector3f:
                collector.animationSystemSetSplineKeyBasicVector3f(animSystemHandle, splineTarget, timeStamp, GetSplineKeyValue<Vector3>(splineSource, i));
                break;
            case EDataTypeID_Vector4f:
                collector.animationSystemSetSplineKeyBasicVector4f(animSystemHandle, splineTarget, timeStamp, GetSplineKeyValue<Vector4>(splineSource, i));
                break;
            case EDataTypeID_Vector2i:
                collector.animationSystemSetSplineKeyBasicVector2i(animSystemHandle, splineTarget, timeStamp, GetSplineKeyValue<Vector2i>(splineSource, i));
                break;
            case EDataTypeID_Vector3i:
                collector.animationSystemSetSplineKeyBasicVector3i(animSystemHandle, splineTarget, timeStamp, GetSplineKeyValue<Vector3i>(splineSource, i));
                break;
            case EDataTypeID_Vector4i:
                collector.animationSystemSetSplineKeyBasicVector4i(animSystemHandle, splineTarget, timeStamp, GetSplineKeyValue<Vector4i>(splineSource, i));
                break;
            default:
                assert(false);
                break;
            }
        }
    }

    void AnimationSystemDescriber::CopySplineTangents(SceneActionCollectionCreator& collector, AnimationSystemHandle animSystemHandle, const SplineBase* const splineSource, SplineHandle splineTarget)
    {
        Vector2 tanIn;
        Vector2 tanOut;

        const UInt32 numKeys = splineSource->getNumKeys();
        for (UInt32 i = 0u; i < numKeys; ++i)
        {
            const SplineTimeStamp timeStamp = splineSource->getTimeStamp(i);
            switch (splineSource->getDataType())
            {
            case EDataTypeID_Int32:
            {
                const Int32 value = GetSplineKeyTangentsValue<Int32>(splineSource, i, tanIn, tanOut);
                collector.animationSystemSetSplineKeyTangentsInt32(animSystemHandle, splineTarget, timeStamp, value, tanIn, tanOut);
                break;
            }
            case EDataTypeID_Float:
            {
                const Float value = GetSplineKeyTangentsValue<Float>(splineSource, i, tanIn, tanOut);
                collector.animationSystemSetSplineKeyTangentsFloat(animSystemHandle, splineTarget, timeStamp, value, tanIn, tanOut);
                break;
            }
            case EDataTypeID_Vector2f:
            {
                const Vector2 value = GetSplineKeyTangentsValue<Vector2>(splineSource, i, tanIn, tanOut);
                collector.animationSystemSetSplineKeyTangentsVector2f(animSystemHandle, splineTarget, timeStamp, value, tanIn, tanOut);
                break;
            }
            case EDataTypeID_Vector3f:
            {
                const Vector3 value = GetSplineKeyTangentsValue<Vector3>(splineSource, i, tanIn, tanOut);
                collector.animationSystemSetSplineKeyTangentsVector3f(animSystemHandle, splineTarget, timeStamp, value, tanIn, tanOut);
                break;
            }
            case EDataTypeID_Vector4f:
            {
                const Vector4 value = GetSplineKeyTangentsValue<Vector4>(splineSource, i, tanIn, tanOut);
                collector.animationSystemSetSplineKeyTangentsVector4f(animSystemHandle, splineTarget, timeStamp, value, tanIn, tanOut);
                break;
            }
            case EDataTypeID_Vector2i:
            {
                const Vector2i value = GetSplineKeyTangentsValue<Vector2i>(splineSource, i, tanIn, tanOut);
                collector.animationSystemSetSplineKeyTangentsVector2i(animSystemHandle, splineTarget, timeStamp, value, tanIn, tanOut);
                break;
            }
            case EDataTypeID_Vector3i:
            {
                const Vector3i value = GetSplineKeyTangentsValue<Vector3i>(splineSource, i, tanIn, tanOut);
                collector.animationSystemSetSplineKeyTangentsVector3i(animSystemHandle, splineTarget, timeStamp, value, tanIn, tanOut);
                break;
            }
            case EDataTypeID_Vector4i:
            {
                const Vector4i value = GetSplineKeyTangentsValue<Vector4i>(splineSource, i, tanIn, tanOut);
                collector.animationSystemSetSplineKeyTangentsVector4i(animSystemHandle, splineTarget, timeStamp, value, tanIn, tanOut);
                break;
            }
            case EDataTypeID_Boolean:
            default:
                assert(false);
                break;
            }
        }
    }

    void AnimationSystemDescriber::CopyDataBinds(const IAnimationSystem& animationSystemSource, SceneActionCollectionCreator& collector, AnimationSystemHandle animSystemHandle)
    {
        const UInt32 numDataBinds = animationSystemSource.getTotalDataBindCount();
        for (DataBindHandle handle(0u); handle < numDataBinds; ++handle)
        {
            if (animationSystemSource.containsDataBinding(handle))
            {
                const AnimationDataBindBase* const dataBind = animationSystemSource.getDataBinding(handle);
                if (dataBind != nullptr)
                {
                    collector.animationSystemAllocateDataBinding(animSystemHandle, dataBind->getBindID(), dataBind->getHandle(), dataBind->getHandle2(), handle);
                }
            }
        }
    }

    void AnimationSystemDescriber::CopyAnimationInstances(const IAnimationSystem& animationSystemSource, SceneActionCollectionCreator& collector, AnimationSystemHandle animSystemHandle)
    {
        const UInt32 numAnimInstances = animationSystemSource.getTotalAnimationInstanceCount();
        for (AnimationInstanceHandle handle(0u); handle < numAnimInstances; ++handle)
        {
            if (animationSystemSource.containsAnimationInstance(handle))
            {
                const AnimationInstance& animInstance = animationSystemSource.getAnimationInstance(handle);
                collector.animationSystemAllocateAnimationInstance(animSystemHandle, animInstance.getSplineHandle(),
                    animInstance.getInterpolationType(), animInstance.getVectorComponentFlag(), handle);

                const DataBindHandleVector& dataBinds = animInstance.getDataBindings();
                for (const auto bind : dataBinds)
                {
                    collector.animationSystemAddDataBindingToAnimationInstance(animSystemHandle, handle, bind);
                }
            }
        }
    }

    void AnimationSystemDescriber::CopyAnimations(const IAnimationSystem& animationSystemSource, SceneActionCollectionCreator& collector, AnimationSystemHandle animSystemHandle)
    {
        const UInt32 numAnims = animationSystemSource.getTotalAnimationCount();
        for (AnimationHandle handle(0u); handle < numAnims; ++handle)
        {
            if (animationSystemSource.containsAnimation(handle))
            {
                const Animation& anim = animationSystemSource.getAnimation(handle);
                collector.animationSystemAllocateAnimation(animSystemHandle, anim.m_animationInstanceHandle, handle);

                collector.animationSystemSetAnimationProperties(animSystemHandle, handle, anim.m_playbackSpeed, anim.m_flags, anim.m_loopDuration, animationSystemSource.getTime());
                collector.animationSystemSetAnimationStartTime(animSystemHandle, handle, anim.m_startTime);
                collector.animationSystemSetAnimationStopTime(animSystemHandle, handle, anim.m_stopTime);
            }
        }
    }
}
