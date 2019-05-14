//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Animation/AnimationData.h"
#include "Animation/AnimationProcessData.h"
#include "AnimationAPI/AnimationSystemSizeInformation.h"

namespace ramses_internal
{
    AnimationData::AnimationData(const AnimationSystemSizeInformation& sizeInfo)
        : m_splinePool(sizeInfo.splineCount)
        , m_dataBindPool(sizeInfo.dataBindCount)
        , m_animationInstancePool(sizeInfo.animationInstanceCount)
        , m_animationPool(sizeInfo.animationCount)
    {
    }

    AnimationData::~AnimationData()
    {
        for (SplineHandle handle(0u); handle < m_splinePool.getTotalCount(); ++handle)
        {
            if (m_splinePool.isAllocated(handle))
            {
                delete *m_splinePool.getMemory(handle);
            }
        }

        for (DataBindHandle handle(0u); handle < m_dataBindPool.getTotalCount(); ++handle)
        {
            if (m_dataBindPool.isAllocated(handle))
            {
                delete *m_dataBindPool.getMemory(handle);
            }
        }
    }

    UInt32 AnimationData::getTotalSplineCount() const
    {
        return m_splinePool.getTotalCount();
    }

    const SplineBase* AnimationData::getSpline(SplineHandle splineHandle) const
    {
        assert(m_splinePool.isAllocated(splineHandle));
        return *m_splinePool.getMemory(splineHandle);
    }

    Bool AnimationData::containsSpline(SplineHandle splineHandle) const
    {
        return m_splinePool.isAllocated(splineHandle);
    }

    void AnimationData::removeSplineKey(SplineHandle handle, SplineKeyIndex keyIndex)
    {
        SplineBase* spline = getSplineInternal(handle);
        assert(spline != NULL);
        spline->removeKey(keyIndex);
        notifySplineChanged(handle);
    }

    void AnimationData::removeSplineKeys(SplineHandle handle)
    {
        SplineBase* spline = getSplineInternal(handle);
        assert(spline != NULL);
        spline->removeAllKeys();
        notifySplineChanged(handle);
    }

    const AnimationDataBindBase* AnimationData::getDataBinding(DataBindHandle dataBindHandle) const
    {
        assert(m_dataBindPool.isAllocated(dataBindHandle));
        return *m_dataBindPool.getMemory(dataBindHandle);
    }

    AnimationDataBindBase* AnimationData::getDataBinding(DataBindHandle dataBindHandle)
    {
        // Use const version of getDataBinding to avoid code duplication
        return const_cast<AnimationDataBindBase*>((const_cast<const AnimationData&>(*this)).getDataBinding(dataBindHandle));
    }

    Bool AnimationData::containsDataBinding(DataBindHandle dataBindHandle) const
    {
        return m_dataBindPool.isAllocated(dataBindHandle);
    }

    UInt32 AnimationData::getTotalDataBindCount() const
    {
        return m_dataBindPool.getTotalCount();
    }

    AnimationInstanceHandle AnimationData::allocateAnimationInstance(SplineHandle splineHandle, EInterpolationType interpolationType, EVectorComponent vectorComponent, AnimationInstanceHandle handle)
    {
        assert(getSpline(splineHandle) != 0 && interpolationType != EInterpolationType_Invalid);
        const AnimationInstanceHandle handleActual = m_animationInstancePool.allocate(handle);
        AnimationInstance animationInstance(splineHandle, interpolationType, vectorComponent);
        *m_animationInstancePool.getMemory(handleActual) = animationInstance;

        return handleActual;
    }

    const AnimationInstance& AnimationData::getAnimationInstance(AnimationInstanceHandle instanceHandle) const
    {
        assert(containsAnimationInstance(instanceHandle));
        return *m_animationInstancePool.getMemory(instanceHandle);
    }

    Bool AnimationData::containsAnimationInstance(AnimationInstanceHandle instanceHandle) const
    {
        return m_animationInstancePool.isAllocated(instanceHandle);
    }

    UInt32 AnimationData::getTotalAnimationInstanceCount() const
    {
        return m_animationInstancePool.getTotalCount();
    }

    void AnimationData::addDataBindingToAnimationInstance(AnimationInstanceHandle handle, DataBindHandle dataBindHandle)
    {
        assert(containsAnimationInstance(handle));

        AnimationInstance& animInst = getAnimationInstanceInternal(handle);
        assert(isDataBindingCompatibleWithAnimationInstance(animInst, dataBindHandle));

        animInst.addDataBinding(dataBindHandle);
        notifyAnimationInstanceChanged(handle);
    }

    AnimationHandle AnimationData::allocateAnimation(AnimationInstanceHandle animationInstanceHandle, AnimationHandle handle)
    {
        assert(containsAnimationInstance(animationInstanceHandle));
        const AnimationHandle handleActual = m_animationPool.allocate(handle);
        m_animationPool.getMemory(handleActual)->m_animationInstanceHandle = animationInstanceHandle;
        return handleActual;
    }

    const Animation& AnimationData::getAnimation(AnimationHandle handle) const
    {
        assert(containsAnimation(handle));
        return *m_animationPool.getMemory(handle);
    }

    Bool AnimationData::containsAnimation(AnimationHandle handle) const
    {
        return m_animationPool.isAllocated(handle);
    }

    UInt32 AnimationData::getTotalAnimationCount() const
    {
        return m_animationPool.getTotalCount();
    }

    void AnimationData::getAnimationHandles(AnimationHandleVector& handles) const
    {
        handles.resize(getTotalAnimationCount());
        AnimationHandleVector::iterator it = handles.begin();

        for (AnimationHandle animHandle(0u); animHandle < m_animationPool.getTotalCount(); ++animHandle)
        {
            if (containsAnimation(animHandle))
            {
                *it = animHandle;
                ++it;
            }
        }
    }

    AnimationInstance& AnimationData::getAnimationInstanceInternal(AnimationInstanceHandle handle)
    {
        // Non-const version of getAnimationInstance cast to its const version to avoid duplicating code
        return const_cast<AnimationInstance&>((const_cast<const AnimationData&>(*this)).getAnimationInstance(handle));
    }

    void AnimationData::setAnimationTimeRange(AnimationHandle handle, const AnimationTime& startTime, const AnimationTime& stopTime)
    {
        assert(containsAnimation(handle));
        notifyPreAnimationTimeRangeChange(handle);
        Animation& animation = getAnimationInternal(handle);
        animation.m_startTime = startTime;
        animation.m_stopTime = stopTime;
        notifyAnimationTimeRangeChanged(handle);
    }

    void AnimationData::setAnimationPaused(AnimationHandle handle, Bool pause)
    {
        assert(containsAnimation(handle));
        Animation& animation = getAnimationInternal(handle);
        if (animation.m_paused != pause)
        {
            animation.m_paused = pause;
            notifyAnimationPauseChanged(handle, pause);
        }
    }

    void AnimationData::setAnimationProperties(AnimationHandle handle, Float playbackSpeed, Animation::Flags flags, AnimationTime::Duration loopDuration)
    {
        assert(containsAnimation(handle));
        Animation& animation = getAnimationInternal(handle);
        const Bool changed = (animation.m_playbackSpeed != playbackSpeed)
            || (animation.m_flags != flags)
            || (animation.m_loopDuration != loopDuration);

        if (changed)
        {
            animation.m_playbackSpeed = playbackSpeed;
            animation.m_flags = flags;
            animation.m_loopDuration = loopDuration;
            notifyAnimationPropertiesChanged(handle);
        }
    }

    void AnimationData::getAnimationProcessData(AnimationHandle handle, AnimationProcessData& processData) const
    {
        assert(containsAnimation(handle));
        const Animation& animation = getAnimation(handle);
        assert(containsAnimationInstance(animation.m_animationInstanceHandle));
        const AnimationInstance& animInst = getAnimationInstance(animation.m_animationInstanceHandle);

        processData.m_animation = animation;
        getDataBindingsForAnimationInstance(animation.m_animationInstanceHandle, processData.m_dataBinds);
        processData.m_spline = getSpline(animInst.getSplineHandle());
        assert(processData.m_spline != 0);
        processData.m_interpolationType = animInst.getInterpolationType();
        processData.m_dataComponent = animInst.getVectorComponentFlag();
    }

    Bool AnimationData::isDataBindingCompatibleWithAnimationInstance(const AnimationInstance& animationInstance, DataBindHandle dataBindHandle) const
    {
        const SplineBase* const pSpline = getSpline(animationInstance.getSplineHandle());
        assert(pSpline != 0);
        const EDataTypeID splineDataType = pSpline->getDataType();

        const AnimationDataBindBase* const pDataBind = getDataBinding(dataBindHandle);
        assert(pDataBind != 0);
        const EDataTypeID dataBindDataType = pDataBind->getDataType();

        return CheckDataTypeCompatibility(splineDataType, dataBindDataType, animationInstance.getVectorComponentFlag());
    }

    Animation& AnimationData::getAnimationInternal(AnimationHandle handle)
    {
        // Non-const version of getAnimation cast to its const version to avoid duplicating code
        return const_cast<Animation&>((const_cast<const AnimationData&>(*this)).getAnimation(handle));
    }

    void AnimationData::removeAnimation(AnimationHandle handle)
    {
        assert(containsAnimation(handle));
        m_animationPool.release(handle);
    }

    void AnimationData::removeAnimationInstance(AnimationInstanceHandle handle)
    {
        assert(containsAnimationInstance(handle));
        m_animationInstancePool.release(handle);
    }

    void AnimationData::removeDataBinding(DataBindHandle handle)
    {
        assert(m_dataBindPool.isAllocated(handle));
        delete *m_dataBindPool.getMemory(handle);
        m_dataBindPool.release(handle);
    }

    void AnimationData::removeSpline(SplineHandle handle)
    {
        assert(m_splinePool.isAllocated(handle));
        delete *m_splinePool.getMemory(handle);
        m_splinePool.release(handle);
    }

    SplineBase* AnimationData::getSplineInternal(SplineHandle handle)
    {
        // Non-const version of getSpline cast to its const version to avoid duplicating code
        return const_cast<SplineBase*>((const_cast<const AnimationData&>(*this)).getSpline(handle));
    }

    void AnimationData::getDataBindingsForAnimationInstance(AnimationInstanceHandle instanceHandle, ConstDataBindVector& dataBinds) const
    {
        const AnimationInstance& animInst = getAnimationInstance(instanceHandle);
        const DataBindHandleVector& dataBindHandles = animInst.getDataBindings();
        const UInt numDataBinds = dataBindHandles.size();
        dataBinds.resize(numDataBinds);
        for (UInt i = 0u; i < numDataBinds; ++i)
        {
            const AnimationDataBindBase* const pDataBind = getDataBinding(dataBindHandles[i]);
            assert(pDataBind != 0);
            dataBinds[i] = pDataBind;
        }
    }

    AnimationSystemSizeInformation AnimationData::getTotalSizeInformation() const
    {
        AnimationSystemSizeInformation sizeInfo;
        sizeInfo.splineCount            = getTotalSplineCount();
        sizeInfo.dataBindCount          = getTotalDataBindCount();
        sizeInfo.animationInstanceCount = getTotalAnimationInstanceCount();
        sizeInfo.animationCount         = getTotalAnimationCount();
        return sizeInfo;
    }

    Bool AnimationData::CheckDataTypeCompatibility(EDataTypeID dataTypeID, EDataTypeID accessorDataTypeID, EVectorComponent accessorVectorComponent)
    {
        const Bool singleComponentBinding = (accessorVectorComponent != EVectorComponent_All);
        if (singleComponentBinding)
        {
            if (!isTypeMatchingComponentType(dataTypeID, accessorDataTypeID))
            {
                return false;
            }
        }
        else if (dataTypeID != accessorDataTypeID)
        {
            return false;
        }

        return true;
    }

    Bool AnimationData::isTypeMatchingComponentType(EDataTypeID compType, EDataTypeID type)
    {
        switch (type)
        {
        case ramses_internal::EDataTypeID_Vector2f:
        case ramses_internal::EDataTypeID_Vector3f:
        case ramses_internal::EDataTypeID_Vector4f:
            return compType == EDataTypeID_Float;
        case ramses_internal::EDataTypeID_Vector2i:
        case ramses_internal::EDataTypeID_Vector3i:
        case ramses_internal::EDataTypeID_Vector4i:
            return compType == EDataTypeID_Int32;
        default:
            return compType == type;
        }
    }
}
