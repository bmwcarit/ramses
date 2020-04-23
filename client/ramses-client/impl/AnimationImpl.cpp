//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/AnimatedProperty.h"
#include "ramses-client-api/Spline.h"

// internal
#include "AnimationSystemImpl.h"
#include "AnimationImpl.h"
#include "SplineImpl.h"
#include "AnimatedPropertyImpl.h"
#include "RamsesObjectRegistryIterator.h"
#include "RamsesObjectTypeUtils.h"

// framework
#include "SerializationContext.h"
#include "Animation/Animation.h"
#include "Animation/AnimationDataBind.h"
#include "Animation/AnimationInstance.h"


namespace ramses
{
    AnimationImpl::AnimationImpl(AnimationSystemImpl& animationSystem, const char* name)
        : AnimationObjectImpl(animationSystem, ERamsesObjectType_Animation, name)
    {
    }

    AnimationImpl::~AnimationImpl()
    {
    }

    status_t AnimationImpl::stop(timeMilliseconds_t delay)
    {
        const timeMilliseconds_t stopTime = getIAnimationSystem().getTime().getTimeStamp() + delay;
        return stopAt(stopTime);
    }

    status_t AnimationImpl::stopAt(globalTimeStamp_t timeStamp)
    {
        const ramses_internal::AnimationTime stopTime(timeStamp);
        getIAnimationSystem().setAnimationStopTime(m_animationHandle, stopTime);
        return StatusOK;
    }

    status_t AnimationImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(AnimationObjectImpl::serialize(outStream, serializationContext));

        outStream << m_animationInstanceHandle;
        outStream << m_animationHandle;

        return StatusOK;
    }

    status_t AnimationImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(AnimationObjectImpl::deserialize(inStream, serializationContext));

        inStream >> m_animationInstanceHandle;
        inStream >> m_animationHandle;

        return StatusOK;
    }

    status_t AnimationImpl::validate(uint32_t indent, StatusObjectSet& visitedObjects) const
    {
        status_t status = AnimationObjectImpl::validate(indent, visitedObjects);
        indent += IndentationStep;

        const ramses_internal::AnimationInstance& animInstance = getIAnimationSystem().getAnimationInstance(m_animationInstanceHandle);

        // validate assigned databinding / AnimatedProperty
        const ramses_internal::DataBindHandleVector& dataBinds = animInstance.getDataBindings();
        for(const auto& dataBind : dataBinds)
        {
            const AnimatedPropertyImpl* animProperty = findAnimatedProperty(dataBind);
            if (animProperty == nullptr)
            {
                addValidationMessage(EValidationSeverity_Error, indent, "assigned AnimatedProperty does not exist anymore, was probably destroyed but still used by Animation");
                status = getValidationErrorStatus();
            }
            else if (addValidationOfDependentObject(indent, *animProperty, visitedObjects) != StatusOK)
            {
                status = getValidationErrorStatus();
            }
        }

        // validate assigned spline
        const SplineImpl* spline = findSpline(animInstance.getSplineHandle());
        if (spline == nullptr)
        {
            addValidationMessage(EValidationSeverity_Error, indent, "assigned Spline does not exist anymore, was probably destroyed but still used by Animation");
            status = getValidationErrorStatus();
        }
        else if (addValidationOfDependentObject(indent, *spline, visitedObjects) != StatusOK)
        {
            status = getValidationErrorStatus();
        }

        return status;
    }

    void AnimationImpl::initializeFrameworkData(const AnimatedPropertyImpl& animatedProperty, ramses_internal::SplineHandle splineHandle, ramses_internal::EInterpolationType interpolationType)
    {
        m_animationInstanceHandle = getIAnimationSystem().allocateAnimationInstance(splineHandle, interpolationType, animatedProperty.getVectorComponent());
        getIAnimationSystem().addDataBindingToAnimationInstance(m_animationInstanceHandle, animatedProperty.getDataBindHandle());

        m_animationHandle = getIAnimationSystem().allocateAnimation(m_animationInstanceHandle);
    }

    void AnimationImpl::deinitializeFrameworkData()
    {
        getIAnimationSystem().removeAnimation(m_animationHandle);
        getIAnimationSystem().removeAnimationInstance(m_animationInstanceHandle);

        m_animationHandle = ramses_internal::AnimationHandle::Invalid();
        m_animationInstanceHandle = ramses_internal::AnimationInstanceHandle::Invalid();
    }

    globalTimeStamp_t AnimationImpl::getStartTime() const
    {
        const ramses_internal::Animation& animation = getIAnimationSystem().getAnimation(m_animationHandle);
        return animation.m_startTime.getTimeStamp();
    }

    globalTimeStamp_t AnimationImpl::getStopTime() const
    {
        const ramses_internal::Animation& animation = getIAnimationSystem().getAnimation(m_animationHandle);
        return animation.m_stopTime.getTimeStamp();
    }

    ramses_internal::AnimationInstanceHandle AnimationImpl::getAnimationInstanceHandle() const
    {
        return m_animationInstanceHandle;
    }

    ramses_internal::AnimationHandle AnimationImpl::getAnimationHandle() const
    {
        return m_animationHandle;
    }

    const AnimatedPropertyImpl* AnimationImpl::findAnimatedProperty(ramses_internal::DataBindHandle handle) const
    {
        RamsesObjectRegistryIterator iter(getAnimationSystemImpl().getObjectRegistry(), ERamsesObjectType_AnimatedProperty);
        while (const AnimatedProperty* animProperty = iter.getNext<AnimatedProperty>())
        {
            if (animProperty->impl.getDataBindHandle() == handle)
            {
                return &animProperty->impl;
            }
        }

        return nullptr;
    }

    const SplineImpl* AnimationImpl::findSpline(ramses_internal::SplineHandle handle) const
    {
        RamsesObjectVector splines;
        getAnimationSystemImpl().getObjectRegistry().getObjectsOfType(splines, ERamsesObjectType_Spline);

        for (const auto it : splines)
        {
            const Spline& spline = RamsesObjectTypeUtils::ConvertTo<Spline>(*it);
            if (spline.impl.getSplineHandle() == handle)
            {
                return &spline.impl;
            }
        }

        return nullptr;
    }
}
