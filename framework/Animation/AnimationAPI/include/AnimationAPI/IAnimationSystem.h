//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IANIMATIONSYSTEM_H
#define RAMSES_IANIMATIONSYSTEM_H

#include "AnimationAPI/AnimationSystemSizeInformation.h"
#include "Animation/AnimationTime.h"
#include "Collections/Guid.h"
#include "Utils/DataTypeUtils.h"
#include "Utils/DataBindCommon.h"
#include "SceneAPI/Handles.h"

namespace ramses_internal
{
    class IScene;
    class Vector2;
    class Vector3;
    class Vector4;
    class SplineBase;
    class AnimationDataBindBase;
    class AnimationInstance;
    class Animation;
    class AnimationLogicListener;

    class IAnimationSystem
    {
    public:
        virtual ~IAnimationSystem()
        {
        }

        virtual void                         setHandle(AnimationSystemHandle handle) = 0;
        virtual void                         setTime(const AnimationTime& globalTime) = 0;
        virtual const AnimationTime&         getTime() const = 0;
        virtual UInt32                       getFlags() const = 0;
        virtual Bool                         isRealTime() const = 0;

        virtual SplineHandle                 allocateSpline(ESplineKeyType keyType, EDataTypeID dataTypeID, SplineHandle handleRequest = SplineHandle::Invalid()) = 0;
        virtual DataBindHandle               allocateDataBinding(IScene& scene, TDataBindID dataBindID, MemoryHandle handle1, MemoryHandle handle2, DataBindHandle handleRequest = DataBindHandle::Invalid()) = 0;
        virtual AnimationInstanceHandle      allocateAnimationInstance(SplineHandle splineHandle, EInterpolationType interpolationType, EVectorComponent vectorComponent, AnimationInstanceHandle handleRequest = AnimationInstanceHandle::Invalid()) = 0;
        virtual AnimationHandle              allocateAnimation(AnimationInstanceHandle handle, AnimationHandle handleRequest = AnimationHandle::Invalid()) = 0;

        virtual void                         addDataBindingToAnimationInstance(AnimationInstanceHandle handle, DataBindHandle dataBindHandle) = 0;

        virtual void                         setSplineKeyBasicBool(SplineHandle splineHandle, SplineTimeStamp timeStamp, Bool value) = 0;
        virtual void                         setSplineKeyBasicInt32(SplineHandle splineHandle, SplineTimeStamp timeStamp, Int32 value) = 0;
        virtual void                         setSplineKeyBasicFloat(SplineHandle splineHandle, SplineTimeStamp timeStamp, Float value) = 0;
        virtual void                         setSplineKeyBasicVector2f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2& value) = 0;
        virtual void                         setSplineKeyBasicVector3f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3& value) = 0;
        virtual void                         setSplineKeyBasicVector4f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4& value) = 0;
        virtual void                         setSplineKeyBasicVector2i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2i& value) = 0;
        virtual void                         setSplineKeyBasicVector3i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3i& value) = 0;
        virtual void                         setSplineKeyBasicVector4i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4i& value) = 0;
        virtual void                         setSplineKeyTangentsInt32(SplineHandle splineHandle, SplineTimeStamp timeStamp, Int32 value, const Vector2& tanIn, const Vector2& tanOut) = 0;
        virtual void                         setSplineKeyTangentsFloat(SplineHandle splineHandle, SplineTimeStamp timeStamp, Float value, const Vector2& tanIn, const Vector2& tanOut) = 0;
        virtual void                         setSplineKeyTangentsVector2f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2& value, const Vector2& tanIn, const Vector2& tanOut) = 0;
        virtual void                         setSplineKeyTangentsVector3f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3& value, const Vector2& tanIn, const Vector2& tanOut) = 0;
        virtual void                         setSplineKeyTangentsVector4f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4& value, const Vector2& tanIn, const Vector2& tanOut) = 0;
        virtual void                         setSplineKeyTangentsVector2i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2i& value, const Vector2& tanIn, const Vector2& tanOut) = 0;
        virtual void                         setSplineKeyTangentsVector3i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3i& value, const Vector2& tanIn, const Vector2& tanOut) = 0;
        virtual void                         setSplineKeyTangentsVector4i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4i& value, const Vector2& tanIn, const Vector2& tanOut) = 0;
        virtual void                         removeSplineKey(SplineHandle splineHandle, SplineKeyIndex keyIndex) = 0;

        virtual void                         setAnimationStartTime(AnimationHandle handle, const AnimationTime& timeStamp) = 0;
        virtual void                         setAnimationStopTime(AnimationHandle handle, const AnimationTime& timeStamp) = 0;
        virtual void                         setAnimationProperties(AnimationHandle handle, Float playbackSpeed, UInt32 flags, AnimationTime::Duration loopDuration, const AnimationTime& timeStamp) = 0;
        virtual void                         stopAnimationAndRollback(AnimationHandle handle) = 0;

        virtual AnimationTime::Duration      getAnimationDurationFromSpline(AnimationHandle handle) const = 0;
        virtual Bool                         hasActiveAnimations() const = 0;

        virtual const SplineBase*            getSpline(SplineHandle handle) const = 0;
        virtual UInt32                       getTotalSplineCount() const = 0;
        virtual Bool                         containsSpline(SplineHandle handle) const = 0;
        virtual const AnimationDataBindBase* getDataBinding(DataBindHandle handle) const = 0;
        virtual UInt32                       getTotalDataBindCount() const = 0;
        virtual Bool                         containsDataBinding(DataBindHandle handle) const = 0;
        virtual const AnimationInstance&     getAnimationInstance(AnimationInstanceHandle handle) const = 0;
        virtual UInt32                       getTotalAnimationInstanceCount() const = 0;
        virtual Bool                         containsAnimationInstance(AnimationInstanceHandle handle) const = 0;
        virtual const Animation&             getAnimation(AnimationHandle handle) const = 0;
        virtual UInt32                       getTotalAnimationCount() const = 0;
        virtual Bool                         containsAnimation(AnimationHandle handle) const = 0;

        virtual void                         removeSpline(SplineHandle handle) = 0;
        virtual void                         removeDataBinding(DataBindHandle handle) = 0;
        virtual void                         removeAnimationInstance(AnimationInstanceHandle handle) = 0;
        virtual void                         removeAnimation(AnimationHandle handle) = 0;

        virtual AnimationSystemSizeInformation getTotalSizeInformation() const = 0;

        virtual void                         registerAnimationLogicListener(AnimationLogicListener* listener) = 0;
        virtual void                         unregisterAnimationLogicListener(AnimationLogicListener* listener) = 0;
    };
}

#endif
