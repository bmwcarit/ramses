//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATION_ANIMATIONSYSTEM_H
#define RAMSES_ANIMATION_ANIMATIONSYSTEM_H

#include "AnimationAPI/IAnimationSystem.h"
#include "Animation/AnimationData.h"
#include "Animation/AnimationLogic.h"

namespace ramses_internal
{
    enum EAnimationSystemFlags
    {
        EAnimationSystemFlags_Default        = 0,
        EAnimationSystemFlags_FullProcessing = BIT(0),  ///< Full processing of animations is used. If not set only animations are processed only when finished.
        EAnimationSystemFlags_RealTime       = BIT(1)   ///< Hints the renderer to use system time for every frame updates. If not set animation system is fully controlled via setTime calls from client.
    };

    class AnimationSystem : public IAnimationSystem
    {
    public:
        AnimationSystem(UInt32 flags, const AnimationSystemSizeInformation& sizeInfo);
        virtual ~AnimationSystem();

        virtual void                         setHandle(AnimationSystemHandle handle) override;
        virtual void                         setTime(const AnimationTime& globalTime) override;
        virtual const AnimationTime&         getTime() const override;
        virtual UInt32                       getFlags() const override;
        virtual bool                         isRealTime() const override;

        virtual SplineHandle                 allocateSpline(ESplineKeyType keyType, EDataTypeID dataTypeID, SplineHandle handleRequest = SplineHandle::Invalid()) override;
        virtual DataBindHandle               allocateDataBinding(IScene& scene, TDataBindID dataBindID, MemoryHandle handle1, MemoryHandle handle2, DataBindHandle handleRequest = DataBindHandle::Invalid()) override;
        virtual AnimationInstanceHandle      allocateAnimationInstance(SplineHandle splineHandle, EInterpolationType interpolationType, EVectorComponent vectorComponent, AnimationInstanceHandle handleRequest = AnimationInstanceHandle::Invalid()) override;
        virtual AnimationHandle              allocateAnimation(AnimationInstanceHandle handle, AnimationHandle handleRequest = AnimationHandle::Invalid()) override;

        virtual void                         addDataBindingToAnimationInstance(AnimationInstanceHandle handle, DataBindHandle dataBindHandle) override;

        virtual void                         setSplineKeyBasicBool(SplineHandle splineHandle, SplineTimeStamp timeStamp, bool value) override;
        virtual void                         setSplineKeyBasicInt32(SplineHandle splineHandle, SplineTimeStamp timeStamp, Int32 value) override;
        virtual void                         setSplineKeyBasicFloat(SplineHandle splineHandle, SplineTimeStamp timeStamp, Float value) override;
        virtual void                         setSplineKeyBasicVector2f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2& value) override;
        virtual void                         setSplineKeyBasicVector3f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3& value) override;
        virtual void                         setSplineKeyBasicVector4f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4& value) override;
        virtual void                         setSplineKeyBasicVector2i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2i& value) override;
        virtual void                         setSplineKeyBasicVector3i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3i& value) override;
        virtual void                         setSplineKeyBasicVector4i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4i& value) override;
        virtual void                         setSplineKeyTangentsInt32(SplineHandle splineHandle, SplineTimeStamp timeStamp, Int32 value, const Vector2& tanIn, const Vector2& tanOut) override;
        virtual void                         setSplineKeyTangentsFloat(SplineHandle splineHandle, SplineTimeStamp timeStamp, Float value, const Vector2& tanIn, const Vector2& tanOut) override;
        virtual void                         setSplineKeyTangentsVector2f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2& value, const Vector2& tanIn, const Vector2& tanOut) override;
        virtual void                         setSplineKeyTangentsVector3f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3& value, const Vector2& tanIn, const Vector2& tanOut) override;
        virtual void                         setSplineKeyTangentsVector4f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4& value, const Vector2& tanIn, const Vector2& tanOut) override;
        virtual void                         setSplineKeyTangentsVector2i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2i& value, const Vector2& tanIn, const Vector2& tanOut) override;
        virtual void                         setSplineKeyTangentsVector3i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3i& value, const Vector2& tanIn, const Vector2& tanOut) override;
        virtual void                         setSplineKeyTangentsVector4i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4i& value, const Vector2& tanIn, const Vector2& tanOut) override;
        virtual void                         removeSplineKey(SplineHandle splineHandle, SplineKeyIndex keyIndex) override;

        virtual void                         setAnimationStartTime(AnimationHandle handle, const AnimationTime& timeStamp) override;
        virtual void                         setAnimationStopTime(AnimationHandle handle, const AnimationTime& timeStamp) override;
        virtual void                         setAnimationProperties(AnimationHandle handle, Float playbackSpeed, UInt32 flags, AnimationTime::Duration loopDuration, const AnimationTime& timeStamp) override;
        virtual void                         stopAnimationAndRollback(AnimationHandle handle) override;

        virtual AnimationTime::Duration      getAnimationDurationFromSpline(AnimationHandle handle) const override;
        virtual bool                         hasActiveAnimations() const override;

        virtual const SplineBase*            getSpline(SplineHandle handle) const override;
        virtual UInt32                       getTotalSplineCount() const override;
        virtual bool                         containsSpline(SplineHandle handle) const override;
        virtual const AnimationDataBindBase* getDataBinding(DataBindHandle handle) const override;
        virtual bool                         containsDataBinding(DataBindHandle handle) const override;
        virtual UInt32                       getTotalDataBindCount() const override;
        virtual const AnimationInstance&     getAnimationInstance(AnimationInstanceHandle handle) const override;
        virtual UInt32                       getTotalAnimationInstanceCount() const override;
        virtual bool                         containsAnimationInstance(AnimationInstanceHandle handle) const override;
        virtual const Animation&             getAnimation(AnimationHandle handle) const override;
        virtual UInt32                       getTotalAnimationCount() const override;
        virtual bool                         containsAnimation(AnimationHandle handle) const override;

        virtual void                         removeSpline(SplineHandle handle) override;
        virtual void                         removeDataBinding(DataBindHandle handle) override;
        virtual void                         removeAnimationInstance(AnimationInstanceHandle handle) override;
        virtual void                         removeAnimation(AnimationHandle handle) override;

        virtual AnimationSystemSizeInformation getTotalSizeInformation() const override;

        virtual void                         registerAnimationLogicListener(AnimationLogicListener* listener) override;
        virtual void                         unregisterAnimationLogicListener(AnimationLogicListener* listener) override;

    protected:
        SplineHandle                         allocateSplineBasic(EDataTypeID dataTypeID, SplineHandle handleRequest);
        SplineHandle                         allocateSplineTangents(EDataTypeID dataTypeID, SplineHandle handleRequest);
        DataBindHandle                       allocateDataBinding_0Handle(IScene& scene, TDataBindID dataBindID, EDataTypeID dataTypeID, DataBindHandle handleRequest);
        DataBindHandle                       allocateDataBinding_1Handle(IScene& scene, TDataBindID dataBindID, EDataTypeID dataTypeID, MemoryHandle handle1, DataBindHandle handleRequest);
        DataBindHandle                       allocateDataBinding_2Handle(IScene& scene, TDataBindID dataBindID, EDataTypeID dataTypeID, MemoryHandle handle1, MemoryHandle handle2, DataBindHandle handleRequest);
        void                                 setAnimationPlaybackSpeed(AnimationHandle handle, Float playbackSpeed, const Animation& animation, const AnimationTime& timeStamp);
        void                                 setAnimationFlags(AnimationHandle handle, Animation::Flags flags, const Animation& animation);
        void                                 setAnimationLoopDuration(AnimationHandle handle, AnimationTime::Duration duration, const Animation& animation);
        AnimationSystemHandle                getHandle() const;

        AnimationData            m_animationData;
        AnimationLogic           m_animationLogic;
        AnimationLogicListener*  m_animationProcessing;
        const UInt32             m_flags;

        static const UInt64 LoopingLengthMultiplier = 1u << 20;
    private:
        AnimationSystemHandle    m_handle;
    };
}

#endif
