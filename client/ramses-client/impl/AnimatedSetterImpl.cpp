//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/AnimatedProperty.h"

// internal
#include "AnimationSystemImpl.h"
#include "AnimatedSetterImpl.h"
#include "AnimatedPropertyImpl.h"
#include "SerializationHelper.h"

// framework
#include "AnimationAPI/IAnimationSystem.h"
#include "Animation/SplineBase.h"
#include "Animation/Animation.h"
#include "Animation/AnimationInstance.h"
#include "SerializationContext.h"
#include "Math3d/Vector2.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector4.h"
#include "Math3d/Vector2i.h"
#include "Math3d/Vector3i.h"
#include "Math3d/Vector4i.h"


namespace ramses
{
    AnimatedSetterImpl::AnimatedSetterImpl(AnimationSystemImpl& animationSystem, const timeMilliseconds_t& delay, const char* name)
        : AnimationObjectImpl(animationSystem, ERamsesObjectType_AnimatedSetter, name)
        , m_animation(animationSystem, "AnimatedSetter")
        , m_splineDataType(ramses_internal::EDataTypeID_Invalid)
        , m_delay(delay)
    {
    }

    AnimatedSetterImpl::~AnimatedSetterImpl()
    {
    }

    status_t AnimatedSetterImpl::setValue(bool x)
    {
        return setValueInternal(x);
    }

    status_t AnimatedSetterImpl::setValue(int32_t x)
    {
        return setValueInternal(x);
    }

    status_t AnimatedSetterImpl::setValue(float x)
    {
        return setValueInternal(x);
    }

    status_t AnimatedSetterImpl::setValue(float x, float y)
    {
        const ramses_internal::Vector2 value(x, y);
        return setValueInternal(value);
    }

    status_t AnimatedSetterImpl::setValue(float x, float y, float z)
    {
        const ramses_internal::Vector3 value(x, y, z);
        return setValueInternal(value);
    }

    status_t AnimatedSetterImpl::setValue(float x, float y, float z, float w)
    {
        const ramses_internal::Vector4 value(x, y, z, w);
        return setValueInternal(value);
    }

    status_t AnimatedSetterImpl::setValue(int32_t x, int32_t y)
    {
        const ramses_internal::Vector2i value(x, y);
        return setValueInternal(value);
    }

    status_t AnimatedSetterImpl::setValue(int32_t x, int32_t y, int32_t z)
    {
        const ramses_internal::Vector3i value(x, y, z);
        return setValueInternal(value);
    }

    status_t AnimatedSetterImpl::setValue(int32_t x, int32_t y, int32_t z, int32_t w)
    {
        const ramses_internal::Vector4i value(x, y, z, w);
        return setValueInternal(value);
    }

    template <typename DATA>
    status_t AnimatedSetterImpl::setValueInternal(const DATA& value)
    {
        if (ramses_internal::DataTypeToDataIDSelector<DATA>::DataTypeID != m_splineDataType)
        {
            return addErrorEntry("AnimatedSetter::setValue failed: invalid data type used, animated property assigned to this AnimatedSetter has a different data type!");
        }

        const ramses_internal::AnimationTime currentTime = getIAnimationSystem().getTime();
        const ramses_internal::AnimationHandle animationHandle = m_animation.getAnimationHandle();
        const ramses_internal::Animation& animation = getIAnimationSystem().getAnimation(animationHandle);
        const ramses_internal::AnimationTime& stopTime = animation.m_stopTime;

        // first time a setValue is called
        const bool firstSet = (!stopTime.isValid());
        // setValue is called after a threshold, meaning the new value should be applied directly without interpolating from the previous one
        const bool setAfterStopped = (currentTime.getTimeStamp() > stopTime.getTimeStamp() - m_delay);

        if (firstSet || setAfterStopped)
        {
            // clear all spline keys if any
            const ramses_internal::SplineBase* spline = getIAnimationSystem().getSpline(m_splineHandle);
            while (spline->getNumKeys() > 0u)
            {
                getIAnimationSystem().removeSplineKey(m_splineHandle, 0u);
            }

            // set beginning of spline to new value
            setSplineKey<DATA>(0u, value);
            // start animation with a fixed delay
            getIAnimationSystem().setAnimationStartTime(animationHandle, currentTime + m_delay);
        }
        else
        {
            // update spline on the fly
            const ramses_internal::AnimationTime startTime = animation.m_startTime.getTimeStamp();
            const ramses_internal::AnimationTime::Duration durationSinceStart = (currentTime + m_delay).getDurationSince(startTime);
            // add new future key for new value
            setSplineKey<DATA>(static_cast<splineTimeStamp_t>(durationSinceStart), value);
        }

        // update stop time
        // animation will stop if no set comes within the fixed delay
        getIAnimationSystem().setAnimationStopTime(animationHandle, currentTime + 2u * m_delay);

        return StatusOK;
    }

    template <typename DATA>
    void AnimatedSetterImpl::setSplineKey(splineTimeStamp_t, const DATA&)
    {
        assert(false);
    }

    template <>
    void AnimatedSetterImpl::setSplineKey<bool>(splineTimeStamp_t keyTimeStamp, const bool& value)
    {
        getIAnimationSystem().setSplineKeyBasicBool(m_splineHandle, keyTimeStamp, value);
    }

    template <>
    void AnimatedSetterImpl::setSplineKey<int32_t>(splineTimeStamp_t keyTimeStamp, const int32_t& value)
    {
        getIAnimationSystem().setSplineKeyBasicInt32(m_splineHandle, keyTimeStamp, value);
    }

    template <>
    void AnimatedSetterImpl::setSplineKey<float>(splineTimeStamp_t keyTimeStamp, const float& value)
    {
        getIAnimationSystem().setSplineKeyBasicFloat(m_splineHandle, keyTimeStamp, value);
    }

    template <>
    void AnimatedSetterImpl::setSplineKey<ramses_internal::Vector2>(splineTimeStamp_t keyTimeStamp, const ramses_internal::Vector2& value)
    {
        getIAnimationSystem().setSplineKeyBasicVector2f(m_splineHandle, keyTimeStamp, value);
    }

    template <>
    void AnimatedSetterImpl::setSplineKey<ramses_internal::Vector3>(splineTimeStamp_t keyTimeStamp, const ramses_internal::Vector3& value)
    {
        getIAnimationSystem().setSplineKeyBasicVector3f(m_splineHandle, keyTimeStamp, value);
    }

    template <>
    void AnimatedSetterImpl::setSplineKey<ramses_internal::Vector4>(splineTimeStamp_t keyTimeStamp, const ramses_internal::Vector4& value)
    {
        getIAnimationSystem().setSplineKeyBasicVector4f(m_splineHandle, keyTimeStamp, value);
    }

    template <>
    void AnimatedSetterImpl::setSplineKey<ramses_internal::Vector2i>(splineTimeStamp_t keyTimeStamp, const ramses_internal::Vector2i& value)
    {
        getIAnimationSystem().setSplineKeyBasicVector2i(m_splineHandle, keyTimeStamp, value);
    }

    template <>
    void AnimatedSetterImpl::setSplineKey<ramses_internal::Vector3i>(splineTimeStamp_t keyTimeStamp, const ramses_internal::Vector3i& value)
    {
        getIAnimationSystem().setSplineKeyBasicVector3i(m_splineHandle, keyTimeStamp, value);
    }

    template <>
    void AnimatedSetterImpl::setSplineKey<ramses_internal::Vector4i>(splineTimeStamp_t keyTimeStamp, const ramses_internal::Vector4i& value)
    {
        getIAnimationSystem().setSplineKeyBasicVector4i(m_splineHandle, keyTimeStamp, value);
    }

    status_t AnimatedSetterImpl::stop(timeMilliseconds_t delay)
    {
        return m_animation.stop(delay);
    }

    status_t AnimatedSetterImpl::stopAt(globalTimeStamp_t timeStamp)
    {
        return m_animation.stopAt(timeStamp);
    }

    status_t AnimatedSetterImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(AnimationObjectImpl::serialize(outStream, serializationContext));

        outStream << m_splineHandle;

        return m_animation.serialize(outStream, serializationContext);
    }

    status_t AnimatedSetterImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(AnimationObjectImpl::deserialize(inStream, serializationContext));

        inStream >> m_splineHandle;

        ObjectIDType objectID = DeserializationContext::GetObjectIDNull();
        return SerializationHelper::DeserializeObjectImpl(inStream, serializationContext, m_animation, objectID);
    }

    void AnimatedSetterImpl::initializeFrameworkData(const AnimatedPropertyImpl& animatedProperty)
    {
        assert(!m_splineHandle.isValid());
        m_splineDataType = (animatedProperty.getVectorComponent() == ramses_internal::EVectorComponent_All ? animatedProperty.getDataTypeID() : ramses_internal::EDataTypeID_Float);
        m_splineHandle = getIAnimationSystem().allocateSpline(ramses_internal::ESplineKeyType_Basic, m_splineDataType);

        m_animation.initializeFrameworkData(animatedProperty, m_splineHandle, ramses_internal::EInterpolationType_Linear);
    }

    void AnimatedSetterImpl::deinitializeFrameworkData()
    {
        m_animation.deinitializeFrameworkData();

        if (m_splineHandle.isValid())
        {
            getIAnimationSystem().removeSpline(m_splineHandle);
            m_splineHandle = ramses_internal::SplineHandle::Invalid();
        }
    }

    status_t AnimatedSetterImpl::validate(uint32_t indent, StatusObjectSet& visitedObjects) const
    {
        status_t status = AnimationObjectImpl::validate(indent, visitedObjects);
        indent += IndentationStep;

        const ramses_internal::AnimationInstance& animInstance = getIAnimationSystem().getAnimationInstance(m_animation.getAnimationInstanceHandle());

        // validate assigned databinding / AnimatedProperty
        const ramses_internal::DataBindHandleVector& dataBinds = animInstance.getDataBindings();
        for(const auto& dataBind : dataBinds)
        {
            const AnimatedPropertyImpl* animProperty = m_animation.findAnimatedProperty(dataBind);
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

        return status;
    }

    ramses_internal::SplineHandle AnimatedSetterImpl::getSplineHandle() const
    {
        return m_splineHandle;
    }

    const AnimationImpl& AnimatedSetterImpl::getAnimation() const
    {
        return m_animation;
    }
}
