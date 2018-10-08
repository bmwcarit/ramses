//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Animation/ActionCollectingAnimationSystem.h"

namespace ramses_internal
{
    ActionCollectingAnimationSystem::ActionCollectingAnimationSystem(UInt32 flags, SceneActionCollection& actionCollector,
                                        const AnimationSystemSizeInformation& sizeInfo)
        : AnimationSystem(flags, sizeInfo)
        , m_sceneActionsCollector(actionCollector)
        , m_creator(m_sceneActionsCollector)
    {
    }

    void ActionCollectingAnimationSystem::setTime(const AnimationTime& globalTime)
    {
        AnimationSystem::setTime(globalTime);
        m_creator.animationSystemSetTime(getHandle(), globalTime);
    }

    SplineHandle ActionCollectingAnimationSystem::allocateSpline(ESplineKeyType keyType, EDataTypeID dataTypeID, SplineHandle handleRequest)
    {
        const SplineHandle splineHandle = AnimationSystem::allocateSpline(keyType, dataTypeID, handleRequest);
        m_creator.animationSystemAllocateSpline(getHandle(), keyType, dataTypeID, splineHandle);

        return splineHandle;
    }

    DataBindHandle ActionCollectingAnimationSystem::allocateDataBinding(IScene& scene, TDataBindID dataBindID, MemoryHandle handle1, MemoryHandle handle2, DataBindHandle handleRequest)
    {
        const DataBindHandle handle = AnimationSystem::allocateDataBinding(scene, dataBindID, handle1, handle2, handleRequest);
        m_creator.animationSystemAllocateDataBinding(getHandle(), dataBindID, handle1, handle2, handle);

        return handle;
    }

    AnimationInstanceHandle ActionCollectingAnimationSystem::allocateAnimationInstance(SplineHandle splineHandle, EInterpolationType interpolationType, EVectorComponent vectorComponent, AnimationInstanceHandle handleRequest)
    {
        const AnimationInstanceHandle handle = AnimationSystem::allocateAnimationInstance(splineHandle, interpolationType, vectorComponent, handleRequest);
        m_creator.animationSystemAllocateAnimationInstance(getHandle(), splineHandle, interpolationType, vectorComponent, handle);

        return handle;
    }

    AnimationHandle ActionCollectingAnimationSystem::allocateAnimation(AnimationInstanceHandle handle, AnimationHandle handleRequest)
    {
        const AnimationHandle animHandle = AnimationSystem::allocateAnimation(handle, handleRequest);
        m_creator.animationSystemAllocateAnimation(getHandle(), handle, animHandle);

        return animHandle;
    }

    void ActionCollectingAnimationSystem::addDataBindingToAnimationInstance(AnimationInstanceHandle handle, DataBindHandle dataBindHandle)
    {
        AnimationSystem::addDataBindingToAnimationInstance(handle, dataBindHandle);
        m_creator.animationSystemAddDataBindingToAnimationInstance(getHandle(), handle, dataBindHandle);
    }

    void ActionCollectingAnimationSystem::setSplineKeyBasicBool(SplineHandle splineHandle, SplineTimeStamp timeStamp, Bool value)
    {
        AnimationSystem::setSplineKeyBasicBool(splineHandle, timeStamp, value);
        m_creator.animationSystemSetSplineKeyBasicBool(getHandle(), splineHandle, timeStamp, value);
    }

    void ActionCollectingAnimationSystem::setSplineKeyBasicInt32(SplineHandle splineHandle, SplineTimeStamp timeStamp, Int32 value)
    {
        AnimationSystem::setSplineKeyBasicInt32(splineHandle, timeStamp, value);
        m_creator.animationSystemSetSplineKeyBasicInt32(getHandle(), splineHandle, timeStamp, value);
    }

    void ActionCollectingAnimationSystem::setSplineKeyBasicFloat(SplineHandle splineHandle, SplineTimeStamp timeStamp, Float value)
    {
        AnimationSystem::setSplineKeyBasicFloat(splineHandle, timeStamp, value);
        m_creator.animationSystemSetSplineKeyBasicFloat(getHandle(), splineHandle, timeStamp, value);
    }

    void ActionCollectingAnimationSystem::setSplineKeyBasicVector2f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2& value)
    {
        AnimationSystem::setSplineKeyBasicVector2f(splineHandle, timeStamp, value);
        m_creator.animationSystemSetSplineKeyBasicVector2f(getHandle(), splineHandle, timeStamp, value);
    }

    void ActionCollectingAnimationSystem::setSplineKeyBasicVector3f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3& value)
    {
        AnimationSystem::setSplineKeyBasicVector3f(splineHandle, timeStamp, value);
        m_creator.animationSystemSetSplineKeyBasicVector3f(getHandle(), splineHandle, timeStamp, value);
    }

    void ActionCollectingAnimationSystem::setSplineKeyBasicVector4f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4& value)
    {
        AnimationSystem::setSplineKeyBasicVector4f(splineHandle, timeStamp, value);
        m_creator.animationSystemSetSplineKeyBasicVector4f(getHandle(), splineHandle, timeStamp, value);
    }

    void ActionCollectingAnimationSystem::setSplineKeyBasicVector2i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2i& value)
    {
        AnimationSystem::setSplineKeyBasicVector2i(splineHandle, timeStamp, value);
        m_creator.animationSystemSetSplineKeyBasicVector2i(getHandle(), splineHandle, timeStamp, value);
    }

    void ActionCollectingAnimationSystem::setSplineKeyBasicVector3i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3i& value)
    {
        AnimationSystem::setSplineKeyBasicVector3i(splineHandle, timeStamp, value);
        m_creator.animationSystemSetSplineKeyBasicVector3i(getHandle(), splineHandle, timeStamp, value);
    }

    void ActionCollectingAnimationSystem::setSplineKeyBasicVector4i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4i& value)
    {
        AnimationSystem::setSplineKeyBasicVector4i(splineHandle, timeStamp, value);
        m_creator.animationSystemSetSplineKeyBasicVector4i(getHandle(), splineHandle, timeStamp, value);
    }

    void ActionCollectingAnimationSystem::setSplineKeyTangentsInt32(SplineHandle splineHandle, SplineTimeStamp timeStamp, Int32 value, const Vector2& tanIn, const Vector2& tanOut)
    {
        AnimationSystem::setSplineKeyTangentsInt32(splineHandle, timeStamp, value, tanIn, tanOut);
        m_creator.animationSystemSetSplineKeyTangentsInt32(getHandle(), splineHandle, timeStamp, value, tanIn, tanOut);
    }

    void ActionCollectingAnimationSystem::setSplineKeyTangentsFloat(SplineHandle splineHandle, SplineTimeStamp timeStamp, Float value, const Vector2& tanIn, const Vector2& tanOut)
    {
        AnimationSystem::setSplineKeyTangentsFloat(splineHandle, timeStamp, value, tanIn, tanOut);
        m_creator.animationSystemSetSplineKeyTangentsFloat(getHandle(), splineHandle, timeStamp, value, tanIn, tanOut);
    }

    void ActionCollectingAnimationSystem::setSplineKeyTangentsVector2f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2& value, const Vector2& tanIn, const Vector2& tanOut)
    {
        AnimationSystem::setSplineKeyTangentsVector2f(splineHandle, timeStamp, value, tanIn, tanOut);
        m_creator.animationSystemSetSplineKeyTangentsVector2f(getHandle(), splineHandle, timeStamp, value, tanIn, tanOut);
    }

    void ActionCollectingAnimationSystem::setSplineKeyTangentsVector3f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3& value, const Vector2& tanIn, const Vector2& tanOut)
    {
        AnimationSystem::setSplineKeyTangentsVector3f(splineHandle, timeStamp, value, tanIn, tanOut);
        m_creator.animationSystemSetSplineKeyTangentsVector3f(getHandle(), splineHandle, timeStamp, value, tanIn, tanOut);
    }

    void ActionCollectingAnimationSystem::setSplineKeyTangentsVector4f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4& value, const Vector2& tanIn, const Vector2& tanOut)
    {
        AnimationSystem::setSplineKeyTangentsVector4f(splineHandle, timeStamp, value, tanIn, tanOut);
        m_creator.animationSystemSetSplineKeyTangentsVector4f(getHandle(), splineHandle, timeStamp, value, tanIn, tanOut);
    }

    void ActionCollectingAnimationSystem::setSplineKeyTangentsVector2i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2i& value, const Vector2& tanIn, const Vector2& tanOut)
    {
        AnimationSystem::setSplineKeyTangentsVector2i(splineHandle, timeStamp, value, tanIn, tanOut);
        m_creator.animationSystemSetSplineKeyTangentsVector2i(getHandle(), splineHandle, timeStamp, value, tanIn, tanOut);
    }

    void ActionCollectingAnimationSystem::setSplineKeyTangentsVector3i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3i& value, const Vector2& tanIn, const Vector2& tanOut)
    {
        AnimationSystem::setSplineKeyTangentsVector3i(splineHandle, timeStamp, value, tanIn, tanOut);
        m_creator.animationSystemSetSplineKeyTangentsVector3i(getHandle(), splineHandle, timeStamp, value, tanIn, tanOut);
    }

    void ActionCollectingAnimationSystem::setSplineKeyTangentsVector4i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4i& value, const Vector2& tanIn, const Vector2& tanOut)
    {
        AnimationSystem::setSplineKeyTangentsVector4i(splineHandle, timeStamp, value, tanIn, tanOut);
        m_creator.animationSystemSetSplineKeyTangentsVector4i(getHandle(), splineHandle, timeStamp, value, tanIn, tanOut);
    }

    void ActionCollectingAnimationSystem::removeSplineKey(SplineHandle splineHandle, SplineKeyIndex keyIndex)
    {
        AnimationSystem::removeSplineKey(splineHandle, keyIndex);
        m_creator.animationSystemRemoveSplineKey(getHandle(), splineHandle, keyIndex);
    }

    void ActionCollectingAnimationSystem::setAnimationStartTime(AnimationHandle handle, const AnimationTime& timeStamp)
    {
        AnimationSystem::setAnimationStartTime(handle, timeStamp);
        m_creator.animationSystemSetAnimationStartTime(getHandle(), handle, timeStamp);
    }

    void ActionCollectingAnimationSystem::setAnimationStopTime(AnimationHandle handle, const AnimationTime& timeStamp)
    {
        AnimationSystem::setAnimationStopTime(handle, timeStamp);
        m_creator.animationSystemSetAnimationStopTime(getHandle(), handle, timeStamp);
    }

    void ActionCollectingAnimationSystem::setAnimationProperties(AnimationHandle handle, Float playbackSpeed, UInt32 flags, AnimationTime::Duration loopDuration, const AnimationTime& timeStamp)
    {
        AnimationSystem::setAnimationProperties(handle, playbackSpeed, flags, loopDuration, timeStamp);
        m_creator.animationSystemSetAnimationProperties(getHandle(), handle, playbackSpeed, flags, loopDuration, timeStamp);
    }

    void ActionCollectingAnimationSystem::stopAnimationAndRollback(AnimationHandle handle)
    {
        AnimationSystem::stopAnimationAndRollback(handle);
        m_creator.animationSystemStopAnimationAndRollback(getHandle(), handle);
    }

    void ActionCollectingAnimationSystem::removeSpline(SplineHandle handle)
    {
        AnimationSystem::removeSpline(handle);
        m_creator.animationSystemRemoveSpline(getHandle(), handle);
    }

    void ActionCollectingAnimationSystem::removeDataBinding(DataBindHandle handle)
    {
        AnimationSystem::removeDataBinding(handle);
        m_creator.animationSystemRemoveDataBinding(getHandle(), handle);
    }

    void ActionCollectingAnimationSystem::removeAnimationInstance(AnimationInstanceHandle handle)
    {
        AnimationSystem::removeAnimationInstance(handle);
        m_creator.animationSystemRemoveAnimationInstance(getHandle(), handle);
    }

    void ActionCollectingAnimationSystem::removeAnimation(AnimationHandle handle)
    {
        AnimationSystem::removeAnimation(handle);
        m_creator.animationSystemRemoveAnimation(getHandle(), handle);
    }
}
