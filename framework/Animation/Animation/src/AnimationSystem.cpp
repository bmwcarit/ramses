//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Animation/AnimationSystem.h"
#include "Animation/SplineKey.h"
#include "Animation/SplineKeyTangents.h"
#include "Animation/AnimationProcessing.h"
#include "Scene/SceneDataBinding.h"

namespace ramses_internal
{
    AnimationSystem::AnimationSystem(UInt32 flags, const AnimationSystemSizeInformation& sizeInfo)
        : m_animationData(sizeInfo)
        , m_animationLogic(m_animationData)
        , m_animationProcessing(nullptr)
        , m_flags(flags)
        , m_handle(AnimationSystemHandle::Invalid())
    {
        m_animationData.addListener(&m_animationLogic);

        const bool fullProcessing = (flags & EAnimationSystemFlags_FullProcessing) != 0u;
        if (fullProcessing)
        {
            // Animation system with full data processing
            m_animationProcessing = new AnimationProcessing(m_animationData);
        }
        else
        {
            // Animation system without data processing
            m_animationProcessing = new AnimationProcessingFinished(m_animationData);
        }

        m_animationLogic.addListener(m_animationProcessing);
    }

    AnimationSystem::~AnimationSystem()
    {
        m_animationLogic.removeListener(m_animationProcessing);
        m_animationData.removeListener(&m_animationLogic);
        delete m_animationProcessing;
    }

    AnimationSystemHandle AnimationSystem::getHandle() const
    {
        assert(m_handle.isValid());
        return m_handle;
    }

    void AnimationSystem::setHandle(AnimationSystemHandle handle)
    {
        assert(!m_handle.isValid());
        assert(handle.isValid());
        m_handle = handle;
    }

    void AnimationSystem::setTime(const AnimationTime& globalTime)
    {
        m_animationLogic.setTime(globalTime);
    }

    const AnimationTime& AnimationSystem::getTime() const
    {
        return m_animationLogic.getTime();
    }

    UInt32 AnimationSystem::getFlags() const
    {
        return m_flags;
    }

    bool AnimationSystem::isRealTime() const
    {
        return (m_flags & EAnimationSystemFlags_RealTime) != 0;
    }

    SplineHandle AnimationSystem::allocateSpline(ESplineKeyType keyType, EDataTypeID dataTypeID, SplineHandle handleRequest)
    {
        switch (keyType)
        {
        case ESplineKeyType_Basic:
            return allocateSplineBasic(dataTypeID, handleRequest);
        case ESplineKeyType_Tangents:
            return allocateSplineTangents(dataTypeID, handleRequest);
        default:
            assert(false);
            return SplineHandle::Invalid();
        }
    }

    DataBindHandle AnimationSystem::allocateDataBinding(IScene& scene, TDataBindID dataBindID, MemoryHandle handle1, MemoryHandle handle2, DataBindHandle handleRequest)
    {
        const EDataTypeID dataTypeID = DataBindContainerToTraitsSelector<IScene>::ContainerTraitsClassType::m_dataBindTraits[dataBindID].m_eDataTypeID;
        if (handle1 == ramses_internal::InvalidMemoryHandle)
        {
            assert(handle2 == InvalidMemoryHandle);
            return allocateDataBinding_0Handle(scene, dataBindID, dataTypeID, handleRequest);
        }
        else if (handle2 == ramses_internal::InvalidMemoryHandle)
        {
            return allocateDataBinding_1Handle(scene, dataBindID, dataTypeID, handle1, handleRequest);
        }
        else
        {
            return allocateDataBinding_2Handle(scene, dataBindID, dataTypeID, handle1, handle2, handleRequest);
        }
    }

    AnimationInstanceHandle AnimationSystem::allocateAnimationInstance(SplineHandle splineHandle, EInterpolationType interpolationType, EVectorComponent vectorComponent, AnimationInstanceHandle handleRequest)
    {
        return m_animationData.allocateAnimationInstance(splineHandle, interpolationType, vectorComponent, handleRequest);
    }

    AnimationHandle AnimationSystem::allocateAnimation(AnimationInstanceHandle handle, AnimationHandle handleRequest)
    {
        return m_animationData.allocateAnimation(handle, handleRequest);
    }

    void AnimationSystem::addDataBindingToAnimationInstance(AnimationInstanceHandle handle, DataBindHandle dataBindHandle)
    {
        m_animationData.addDataBindingToAnimationInstance(handle, dataBindHandle);
    }

    void AnimationSystem::setSplineKeyBasicBool(SplineHandle splineHandle, SplineTimeStamp timeStamp, bool value)
    {
        SplineKey<bool> key(value);
        m_animationData.setSplineKey(splineHandle, timeStamp, key);
    }

    void AnimationSystem::setSplineKeyBasicInt32(SplineHandle splineHandle, SplineTimeStamp timeStamp, Int32 value)
    {
        SplineKey<Int32> key(value);
        m_animationData.setSplineKey(splineHandle, timeStamp, key);
    }

    void AnimationSystem::setSplineKeyBasicFloat(SplineHandle splineHandle, SplineTimeStamp timeStamp, Float value)
    {
        SplineKey<Float> key(value);
        m_animationData.setSplineKey(splineHandle, timeStamp, key);
    }

    void AnimationSystem::setSplineKeyBasicVector2f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2& value)
    {
        SplineKey<Vector2> key(value);
        m_animationData.setSplineKey(splineHandle, timeStamp, key);
    }

    void AnimationSystem::setSplineKeyBasicVector3f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3& value)
    {
        SplineKey<Vector3> key(value);
        m_animationData.setSplineKey(splineHandle, timeStamp, key);
    }

    void AnimationSystem::setSplineKeyBasicVector4f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4& value)
    {
        SplineKey<Vector4> key(value);
        m_animationData.setSplineKey(splineHandle, timeStamp, key);
    }

    void AnimationSystem::setSplineKeyBasicVector2i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2i& value)
    {
        SplineKey<Vector2i> key(value);
        m_animationData.setSplineKey(splineHandle, timeStamp, key);
    }

    void AnimationSystem::setSplineKeyBasicVector3i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3i& value)
    {
        SplineKey<Vector3i> key(value);
        m_animationData.setSplineKey(splineHandle, timeStamp, key);
    }

    void AnimationSystem::setSplineKeyBasicVector4i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4i& value)
    {
        SplineKey<Vector4i> key(value);
        m_animationData.setSplineKey(splineHandle, timeStamp, key);
    }

    void AnimationSystem::setSplineKeyTangentsInt32(SplineHandle splineHandle, SplineTimeStamp timeStamp, Int32 value, const Vector2& tanIn, const Vector2& tanOut)
    {
        SplineKeyTangents<Int32> key(value, tanIn, tanOut);
        m_animationData.setSplineKey(splineHandle, timeStamp, key);
    }

    void AnimationSystem::setSplineKeyTangentsFloat(SplineHandle splineHandle, SplineTimeStamp timeStamp, Float value, const Vector2& tanIn, const Vector2& tanOut)
    {
        SplineKeyTangents<Float> key(value, tanIn, tanOut);
        m_animationData.setSplineKey(splineHandle, timeStamp, key);
    }

    void AnimationSystem::setSplineKeyTangentsVector2f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2& value, const Vector2& tanIn, const Vector2& tanOut)
    {
        SplineKeyTangents<Vector2> key(value, tanIn, tanOut);
        m_animationData.setSplineKey(splineHandle, timeStamp, key);
    }

    void AnimationSystem::setSplineKeyTangentsVector3f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3& value, const Vector2& tanIn, const Vector2& tanOut)
    {
        SplineKeyTangents<Vector3> key(value, tanIn, tanOut);
        m_animationData.setSplineKey(splineHandle, timeStamp, key);
    }

    void AnimationSystem::setSplineKeyTangentsVector4f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4& value, const Vector2& tanIn, const Vector2& tanOut)
    {
        SplineKeyTangents<Vector4> key(value, tanIn, tanOut);
        m_animationData.setSplineKey(splineHandle, timeStamp, key);
    }

    void AnimationSystem::setSplineKeyTangentsVector2i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2i& value, const Vector2& tanIn, const Vector2& tanOut)
    {
        SplineKeyTangents<Vector2i> key(value, tanIn, tanOut);
        m_animationData.setSplineKey(splineHandle, timeStamp, key);
    }

    void AnimationSystem::setSplineKeyTangentsVector3i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3i& value, const Vector2& tanIn, const Vector2& tanOut)
    {
        SplineKeyTangents<Vector3i> key(value, tanIn, tanOut);
        m_animationData.setSplineKey(splineHandle, timeStamp, key);
    }

    void AnimationSystem::setSplineKeyTangentsVector4i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4i& value, const Vector2& tanIn, const Vector2& tanOut)
    {
        SplineKeyTangents<Vector4i> key(value, tanIn, tanOut);
        m_animationData.setSplineKey(splineHandle, timeStamp, key);
    }

    void AnimationSystem::removeSplineKey(SplineHandle splineHandle, SplineKeyIndex keyIndex)
    {
        m_animationData.removeSplineKey(splineHandle, keyIndex);
    }

    void AnimationSystem::setAnimationStartTime(AnimationHandle handle, const AnimationTime& timeStamp)
    {
        const Animation& animation = m_animationData.getAnimation(handle);
        AnimationTime stopTime;

        if ((animation.m_flags & Animation::EAnimationFlags_Looping) != 0)
        {
            stopTime = timeStamp + animation.m_loopDuration * LoopingLengthMultiplier;
        }
        else
        {
            stopTime = timeStamp + getAnimationDurationFromSpline(handle);
        }

        m_animationData.setAnimationTimeRange(handle, timeStamp, stopTime);
    }

    void AnimationSystem::setAnimationStopTime(AnimationHandle handle, const AnimationTime& timeStamp)
    {
        const Animation& animation = m_animationData.getAnimation(handle);
        if (animation.m_startTime.isValid())
        {
            m_animationData.setAnimationTimeRange(handle, animation.m_startTime, timeStamp);
        }
    }

    void AnimationSystem::setAnimationProperties(AnimationHandle handle, Float playbackSpeed, UInt32 flags, AnimationTime::Duration loopDuration, const AnimationTime& timeStamp)
    {
        const Animation& animation = m_animationData.getAnimation(handle);
        if (playbackSpeed != animation.m_playbackSpeed)
        {
            setAnimationPlaybackSpeed(handle, playbackSpeed, animation, timeStamp);
        }

        const bool loopEnabled = ((flags ^ animation.m_flags) & Animation::EAnimationFlags_Looping) != 0;
        if (loopEnabled || (loopDuration != animation.m_loopDuration))
        {
            setAnimationLoopDuration(handle, loopDuration, animation);
        }

        if (flags != animation.m_flags)
        {
            setAnimationFlags(handle, flags, animation);
        }
    }

    void AnimationSystem::stopAnimationAndRollback(AnimationHandle handle)
    {
        if (m_animationLogic.isAnimationActive(handle))
        {
            const Animation& animation = m_animationData.getAnimation(handle);
            const Animation::Flags flags = animation.m_flags;
            m_animationData.setAnimationProperties(handle, animation.m_playbackSpeed, flags | ramses_internal::Animation::EAnimationFlags_ApplyInitialValue, animation.m_loopDuration);
            m_animationData.setAnimationTimeRange(handle, animation.m_startTime, m_animationLogic.getTime());
            m_animationData.setAnimationProperties(handle, animation.m_playbackSpeed, flags, animation.m_loopDuration);
        }
    }

    const SplineBase* AnimationSystem::getSpline(SplineHandle handle) const
    {
        return m_animationData.getSpline(handle);
    }

    UInt32 AnimationSystem::getTotalSplineCount() const
    {
        return m_animationData.getTotalSplineCount();
    }

    bool AnimationSystem::containsSpline(SplineHandle handle) const
    {
        return m_animationData.containsSpline(handle);
    }

    const AnimationDataBindBase* AnimationSystem::getDataBinding(DataBindHandle handle) const
    {
        return m_animationData.getDataBinding(handle);
    }

    UInt32 AnimationSystem::getTotalDataBindCount() const
    {
        return m_animationData.getTotalDataBindCount();
    }

    bool AnimationSystem::containsDataBinding(DataBindHandle handle) const
    {
        return m_animationData.containsDataBinding(handle);
    }

    const AnimationInstance& AnimationSystem::getAnimationInstance(AnimationInstanceHandle handle) const
    {
        return m_animationData.getAnimationInstance(handle);
    }

    UInt32 AnimationSystem::getTotalAnimationInstanceCount() const
    {
        return m_animationData.getTotalAnimationInstanceCount();
    }

    bool AnimationSystem::containsAnimationInstance(AnimationInstanceHandle handle) const
    {
        return m_animationData.containsAnimationInstance(handle);
    }

    const Animation& AnimationSystem::getAnimation(AnimationHandle handle) const
    {
        return m_animationData.getAnimation(handle);
    }

    UInt32 AnimationSystem::getTotalAnimationCount() const
    {
        return m_animationData.getTotalAnimationCount();
    }

    bool AnimationSystem::containsAnimation(AnimationHandle handle) const
    {
        return m_animationData.containsAnimation(handle);
    }

    AnimationSystemSizeInformation AnimationSystem::getTotalSizeInformation() const
    {
        return m_animationData.getTotalSizeInformation();
    }

    void AnimationSystem::removeSpline(SplineHandle handle)
    {
        m_animationData.removeSpline(handle);
    }

    void AnimationSystem::removeDataBinding(DataBindHandle handle)
    {
        m_animationData.removeDataBinding(handle);
    }

    void AnimationSystem::removeAnimationInstance(AnimationInstanceHandle handle)
    {
        m_animationData.removeAnimationInstance(handle);
    }

    void AnimationSystem::removeAnimation(AnimationHandle handle)
    {
        if (m_animationLogic.isAnimationActive(handle))
        {
            m_animationLogic.dequeueAnimation(handle);
        }
        m_animationData.removeAnimation(handle);
    }

    SplineHandle AnimationSystem::allocateSplineBasic(EDataTypeID dataTypeID, SplineHandle handleRequest)
    {
        switch (dataTypeID)
        {
        case EDataTypeID_Boolean:
            return m_animationData.allocateSpline<SplineKey, bool>(handleRequest);
        case EDataTypeID_Int32:
            return m_animationData.allocateSpline<SplineKey, Int32>(handleRequest);
        case EDataTypeID_Float:
            return m_animationData.allocateSpline<SplineKey, Float>(handleRequest);
        case EDataTypeID_Vector2f:
            return m_animationData.allocateSpline<SplineKey, Vector2>(handleRequest);
        case EDataTypeID_Vector3f:
            return m_animationData.allocateSpline<SplineKey, Vector3>(handleRequest);
        case EDataTypeID_Vector4f:
            return m_animationData.allocateSpline<SplineKey, Vector4>(handleRequest);
        case EDataTypeID_Vector2i:
            return m_animationData.allocateSpline<SplineKey, Vector2i>(handleRequest);
        case EDataTypeID_Vector3i:
            return m_animationData.allocateSpline<SplineKey, Vector3i>(handleRequest);
        case EDataTypeID_Vector4i:
            return m_animationData.allocateSpline<SplineKey, Vector4i>(handleRequest);
        default:
            return SplineHandle::Invalid();
        }
    }

    SplineHandle AnimationSystem::allocateSplineTangents(EDataTypeID dataTypeID, SplineHandle handleRequest)
    {
        switch (dataTypeID)
        {
        case EDataTypeID_Boolean:
            return m_animationData.allocateSpline<SplineKeyTangents, bool>(handleRequest);
        case EDataTypeID_Int32:
            return m_animationData.allocateSpline<SplineKeyTangents, Int32>(handleRequest);
        case EDataTypeID_Float:
            return m_animationData.allocateSpline<SplineKeyTangents, Float>(handleRequest);
        case EDataTypeID_Vector2f:
            return m_animationData.allocateSpline<SplineKeyTangents, Vector2>(handleRequest);
        case EDataTypeID_Vector3f:
            return m_animationData.allocateSpline<SplineKeyTangents, Vector3>(handleRequest);
        case EDataTypeID_Vector4f:
            return m_animationData.allocateSpline<SplineKeyTangents, Vector4>(handleRequest);
        case EDataTypeID_Vector2i:
            return m_animationData.allocateSpline<SplineKeyTangents, Vector2i>(handleRequest);
        case EDataTypeID_Vector3i:
            return m_animationData.allocateSpline<SplineKeyTangents, Vector3i>(handleRequest);
        case EDataTypeID_Vector4i:
            return m_animationData.allocateSpline<SplineKeyTangents, Vector4i>(handleRequest);
        default:
            return SplineHandle::Invalid();
        }
    }

    DataBindHandle AnimationSystem::allocateDataBinding_0Handle(IScene& scene, TDataBindID dataBindID, EDataTypeID dataTypeID, DataBindHandle handleRequest)
    {
        switch (dataTypeID)
        {
        case EDataTypeID_Boolean:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, bool>(scene, dataBindID), handleRequest);
        case EDataTypeID_Int32:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Int32>(scene, dataBindID), handleRequest);
        case EDataTypeID_Float:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Float>(scene, dataBindID), handleRequest);
        case EDataTypeID_Vector2f:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Vector2>(scene, dataBindID), handleRequest);
        case EDataTypeID_Vector3f:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Vector3>(scene, dataBindID), handleRequest);
        case EDataTypeID_Vector4f:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Vector4>(scene, dataBindID), handleRequest);
        case EDataTypeID_Vector2i:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Vector2i>(scene, dataBindID), handleRequest);
        case EDataTypeID_Vector3i:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Vector3i>(scene, dataBindID), handleRequest);
        case EDataTypeID_Vector4i:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Vector4i>(scene, dataBindID), handleRequest);
        default:
            return DataBindHandle::Invalid();
        }
    }

    DataBindHandle AnimationSystem::allocateDataBinding_1Handle(IScene& scene, TDataBindID dataBindID, EDataTypeID dataTypeID, MemoryHandle handle1, DataBindHandle handleRequest)
    {
        switch (dataTypeID)
        {
        case EDataTypeID_Boolean:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, bool, MemoryHandle>(scene, handle1, dataBindID), handleRequest);
        case EDataTypeID_Int32:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Int32, MemoryHandle>(scene, handle1, dataBindID), handleRequest);
        case EDataTypeID_Float:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Float, MemoryHandle>(scene, handle1, dataBindID), handleRequest);
        case EDataTypeID_Vector2f:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Vector2, MemoryHandle>(scene, handle1, dataBindID), handleRequest);
        case EDataTypeID_Vector3f:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Vector3, MemoryHandle>(scene, handle1, dataBindID), handleRequest);
        case EDataTypeID_Vector4f:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Vector4, MemoryHandle>(scene, handle1, dataBindID), handleRequest);
        case EDataTypeID_Vector2i:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Vector2i, MemoryHandle>(scene, handle1, dataBindID), handleRequest);
        case EDataTypeID_Vector3i:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Vector3i, MemoryHandle>(scene, handle1, dataBindID), handleRequest);
        case EDataTypeID_Vector4i:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Vector4i, MemoryHandle>(scene, handle1, dataBindID), handleRequest);
        default:
            return DataBindHandle::Invalid();
        }
    }

    DataBindHandle AnimationSystem::allocateDataBinding_2Handle(IScene& scene, TDataBindID dataBindID, EDataTypeID dataTypeID, MemoryHandle handle1, MemoryHandle handle2, DataBindHandle handleRequest)
    {
        switch (dataTypeID)
        {
        case EDataTypeID_Boolean:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, bool, MemoryHandle, MemoryHandle>(scene, handle1, handle2, dataBindID), handleRequest);
        case EDataTypeID_Int32:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Int32, MemoryHandle, MemoryHandle>(scene, handle1, handle2, dataBindID), handleRequest);
        case EDataTypeID_Float:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Float, MemoryHandle, MemoryHandle>(scene, handle1, handle2, dataBindID), handleRequest);
        case EDataTypeID_Vector2f:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Vector2, MemoryHandle, MemoryHandle>(scene, handle1, handle2, dataBindID), handleRequest);
        case EDataTypeID_Vector3f:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Vector3, MemoryHandle, MemoryHandle>(scene, handle1, handle2, dataBindID), handleRequest);
        case EDataTypeID_Vector4f:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Vector4, MemoryHandle, MemoryHandle>(scene, handle1, handle2, dataBindID), handleRequest);
        case EDataTypeID_Vector2i:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Vector2i, MemoryHandle, MemoryHandle>(scene, handle1, handle2, dataBindID), handleRequest);
        case EDataTypeID_Vector3i:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Vector3i, MemoryHandle, MemoryHandle>(scene, handle1, handle2, dataBindID), handleRequest);
        case EDataTypeID_Vector4i:
            return m_animationData.allocateDataBinding(AnimationDataBind<IScene, Vector4i, MemoryHandle, MemoryHandle>(scene, handle1, handle2, dataBindID), handleRequest);
        default:
            return DataBindHandle::Invalid();
        }
    }

    void AnimationSystem::setAnimationPlaybackSpeed(AnimationHandle handle, Float playbackSpeed, const Animation& animation, const AnimationTime& /*timeStamp*/)
    {
        m_animationData.setAnimationProperties(handle, playbackSpeed, animation.m_flags, animation.m_loopDuration);
    }

    void AnimationSystem::setAnimationFlags(AnimationHandle handle, Animation::Flags flags, const Animation& animation)
    {
        m_animationData.setAnimationProperties(handle, animation.m_playbackSpeed, flags, animation.m_loopDuration);
    }

    void AnimationSystem::setAnimationLoopDuration(AnimationHandle handle, AnimationTime::Duration duration, const Animation& animation)
    {
        if (duration == 0u)
        {
            duration = getAnimationDurationFromSpline(handle);
        }

        m_animationData.setAnimationProperties(handle, animation.m_playbackSpeed, animation.m_flags | Animation::EAnimationFlags_Looping, duration);

        if (animation.m_stopTime.isValid())
        {
            m_animationData.setAnimationTimeRange(handle, animation.m_startTime, animation.m_startTime + duration * LoopingLengthMultiplier);
        }
    }

    AnimationTime::Duration AnimationSystem::getAnimationDurationFromSpline(AnimationHandle handle) const
    {
        const Animation& animation = m_animationData.getAnimation(handle);
        const AnimationInstance& animationInstance = m_animationData.getAnimationInstance(animation.m_animationInstanceHandle);
        AnimationTime::Duration animDuration = 0u;

        const SplineBase* const spline = m_animationData.getSpline(animationInstance.getSplineHandle());
        if (spline != nullptr)
        {
            const UInt32 numKeys = spline->getNumKeys();
            if (numKeys > 0u)
            {
                const SplineTimeStamp lastKeyTimeStamp = spline->getTimeStamp(numKeys - 1u);
                animDuration = lastKeyTimeStamp;
            }
        }

        return animDuration;
    }

    bool AnimationSystem::hasActiveAnimations() const
    {
        return m_animationLogic.getNumActiveAnimations() > 0u;
    }

    void AnimationSystem::registerAnimationLogicListener(AnimationLogicListener* listener)
    {
        m_animationLogic.addListener(listener);
    }

    void AnimationSystem::unregisterAnimationLogicListener(AnimationLogicListener* listener)
    {
        m_animationLogic.removeListener(listener);
    }
}
