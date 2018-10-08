//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ACTIONCOLLECTINGANIMATIONSYSTEM_H
#define RAMSES_ACTIONCOLLECTINGANIMATIONSYSTEM_H

#include "Animation/AnimationSystem.h"
#include "SceneAPI/SceneId.h"
#include "Scene/SceneActionCollectionCreator.h"

namespace ramses_internal
{
    class SceneActionCollection;

    class ActionCollectingAnimationSystem final : public AnimationSystem
    {
    public:
        ActionCollectingAnimationSystem(UInt32 flags, SceneActionCollection& actionCollector, const AnimationSystemSizeInformation& sizeInfo);

        virtual void                        setTime(const AnimationTime& globalTime) override;

        virtual SplineHandle                allocateSpline(ESplineKeyType keyType, EDataTypeID dataTypeID, SplineHandle handleRequest = SplineHandle::Invalid()) override;
        virtual DataBindHandle              allocateDataBinding(IScene& scene, TDataBindID dataBindID, MemoryHandle handle1, MemoryHandle handle2, DataBindHandle handleRequest = DataBindHandle::Invalid()) override;
        virtual AnimationInstanceHandle     allocateAnimationInstance(SplineHandle splineHandle, EInterpolationType interpolationType, EVectorComponent vectorComponent, AnimationInstanceHandle handleRequest = AnimationInstanceHandle::Invalid()) override;
        virtual AnimationHandle             allocateAnimation(AnimationInstanceHandle handle, AnimationHandle handleRequest = AnimationHandle::Invalid()) override;

        virtual void                        addDataBindingToAnimationInstance(AnimationInstanceHandle handle, DataBindHandle dataBindHandle) override;

        virtual void                        setSplineKeyBasicBool(SplineHandle splineHandle, SplineTimeStamp timeStamp, Bool value) override;
        virtual void                        setSplineKeyBasicInt32(SplineHandle splineHandle, SplineTimeStamp timeStamp, Int32 value) override;
        virtual void                        setSplineKeyBasicFloat(SplineHandle splineHandle, SplineTimeStamp timeStamp, Float value) override;
        virtual void                        setSplineKeyBasicVector2f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2& value) override;
        virtual void                        setSplineKeyBasicVector3f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3& value) override;
        virtual void                        setSplineKeyBasicVector4f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4& value) override;
        virtual void                        setSplineKeyBasicVector2i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2i& value) override;
        virtual void                        setSplineKeyBasicVector3i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3i& value) override;
        virtual void                        setSplineKeyBasicVector4i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4i& value) override;
        virtual void                        setSplineKeyTangentsInt32(SplineHandle splineHandle, SplineTimeStamp timeStamp, Int32 value, const Vector2& tanIn, const Vector2& tanOut) override;
        virtual void                        setSplineKeyTangentsFloat(SplineHandle splineHandle, SplineTimeStamp timeStamp, Float value, const Vector2& tanIn, const Vector2& tanOut) override;
        virtual void                        setSplineKeyTangentsVector2f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2& value, const Vector2& tanIn, const Vector2& tanOut) override;
        virtual void                        setSplineKeyTangentsVector3f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3& value, const Vector2& tanIn, const Vector2& tanOut) override;
        virtual void                        setSplineKeyTangentsVector4f(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4& value, const Vector2& tanIn, const Vector2& tanOut) override;
        virtual void                        setSplineKeyTangentsVector2i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector2i& value, const Vector2& tanIn, const Vector2& tanOut) override;
        virtual void                        setSplineKeyTangentsVector3i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector3i& value, const Vector2& tanIn, const Vector2& tanOut) override;
        virtual void                        setSplineKeyTangentsVector4i(SplineHandle splineHandle, SplineTimeStamp timeStamp, const Vector4i& value, const Vector2& tanIn, const Vector2& tanOut) override;
        virtual void                        removeSplineKey(SplineHandle splineHandle, SplineKeyIndex keyIndex) override;

        virtual void                        setAnimationStartTime(AnimationHandle handle, const AnimationTime& timeStamp) override;
        virtual void                        setAnimationStopTime(AnimationHandle handle, const AnimationTime& timeStamp) override;
        virtual void                        setAnimationProperties(AnimationHandle handle, Float playbackSpeed, UInt32 flags, AnimationTime::Duration loopDuration, const AnimationTime& timeStamp) override;
        virtual void                        stopAnimationAndRollback(AnimationHandle handle) override;

        virtual void                        removeSpline(SplineHandle handle) override;
        virtual void                        removeDataBinding(DataBindHandle handle) override;
        virtual void                        removeAnimationInstance(AnimationInstanceHandle handle) override;
        virtual void                        removeAnimation(AnimationHandle handle) override;

    protected:
        SceneActionCollection&               m_sceneActionsCollector;
        SceneActionCollectionCreator         m_creator;
    };
}

#endif
