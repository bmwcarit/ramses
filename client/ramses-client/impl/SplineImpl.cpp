//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SplineImpl.h"
#include "AnimationSystemImpl.h"
#include "Math3d/Vector2.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector4.h"
#include "Math3d/Vector2i.h"
#include "Math3d/Vector3i.h"
#include "Math3d/Vector4i.h"
#include "SerializationContext.h"
#include "Animation/SplineBase.h"
#include "Animation/Spline.h"
#include "Animation/SplineKey.h"
#include "Animation/SplineKeyTangents.h"
#include "AnimationAPI/IAnimationSystem.h"

namespace ramses
{
    SplineImpl::SplineImpl(AnimationSystemImpl& animationSystem, ERamsesObjectType type, const char* name)
        : AnimationObjectImpl(animationSystem, type, name)
    {
    }

    SplineImpl::~SplineImpl()
    {
    }

    status_t SplineImpl::setSplineKeyStepBool(splineTimeStamp_t timeStamp, bool value)
    {
        getIAnimationSystem().setSplineKeyBasicBool(m_splineHandle, timeStamp, value);
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyStepInt32(splineTimeStamp_t timeStamp, int32_t value)
    {
        getIAnimationSystem().setSplineKeyBasicInt32(m_splineHandle, timeStamp, value);
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyStepFloat(splineTimeStamp_t timeStamp, float value)
    {
        getIAnimationSystem().setSplineKeyBasicFloat(m_splineHandle, timeStamp, value);
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyStepVector2f(splineTimeStamp_t timeStamp, float x, float y)
    {
        getIAnimationSystem().setSplineKeyBasicVector2f(m_splineHandle, timeStamp, ramses_internal::Vector2(x, y));
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyStepVector3f(splineTimeStamp_t timeStamp, float x, float y, float z)
    {
        getIAnimationSystem().setSplineKeyBasicVector3f(m_splineHandle, timeStamp, ramses_internal::Vector3(x, y, z));
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyStepVector4f(splineTimeStamp_t timeStamp, float x, float y, float z, float w)
    {
        getIAnimationSystem().setSplineKeyBasicVector4f(m_splineHandle, timeStamp, ramses_internal::Vector4(x, y, z, w));
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyStepVector2i(splineTimeStamp_t timeStamp, int32_t x, int32_t y)
    {
        getIAnimationSystem().setSplineKeyBasicVector2i(m_splineHandle, timeStamp, ramses_internal::Vector2i(x, y));
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyStepVector3i(splineTimeStamp_t timeStamp, int32_t x, int32_t y, int32_t z)
    {
        getIAnimationSystem().setSplineKeyBasicVector3i(m_splineHandle, timeStamp, ramses_internal::Vector3i(x, y, z));
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyStepVector4i(splineTimeStamp_t timeStamp, int32_t x, int32_t y, int32_t z, int32_t w)
    {
        getIAnimationSystem().setSplineKeyBasicVector4i(m_splineHandle, timeStamp, ramses_internal::Vector4i(x, y, z, w));
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyLinearInt32(splineTimeStamp_t timeStamp, int32_t value)
    {
        getIAnimationSystem().setSplineKeyBasicInt32(m_splineHandle, timeStamp, value);
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyLinearFloat(splineTimeStamp_t timeStamp, float value)
    {
        getIAnimationSystem().setSplineKeyBasicFloat(m_splineHandle, timeStamp, value);
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyLinearVector2f(splineTimeStamp_t timeStamp, float x, float y)
    {
        getIAnimationSystem().setSplineKeyBasicVector2f(m_splineHandle, timeStamp, ramses_internal::Vector2(x, y));
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyLinearVector3f(splineTimeStamp_t timeStamp, float x, float y, float z)
    {
        getIAnimationSystem().setSplineKeyBasicVector3f(m_splineHandle, timeStamp, ramses_internal::Vector3(x, y, z));
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyLinearVector4f(splineTimeStamp_t timeStamp, float x, float y, float z, float w)
    {
        getIAnimationSystem().setSplineKeyBasicVector4f(m_splineHandle, timeStamp, ramses_internal::Vector4(x, y, z, w));
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyLinearVector2i(splineTimeStamp_t timeStamp, int32_t x, int32_t y)
    {
        getIAnimationSystem().setSplineKeyBasicVector2i(m_splineHandle, timeStamp, ramses_internal::Vector2i(x, y));
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyLinearVector3i(splineTimeStamp_t timeStamp, int32_t x, int32_t y, int32_t z)
    {
        getIAnimationSystem().setSplineKeyBasicVector3i(m_splineHandle, timeStamp, ramses_internal::Vector3i(x, y, z));
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyLinearVector4i(splineTimeStamp_t timeStamp, int32_t x, int32_t y, int32_t z, int32_t w)
    {
        getIAnimationSystem().setSplineKeyBasicVector4i(m_splineHandle, timeStamp, ramses_internal::Vector4i(x, y, z, w));
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyBezierInt32(splineTimeStamp_t timeStamp, int32_t value, float tanInX, float tanInY, float tanOutX, float tanOutY)
    {
        getIAnimationSystem().setSplineKeyTangentsInt32(m_splineHandle, timeStamp, value, ramses_internal::Vector2(tanInX, tanInY), ramses_internal::Vector2(tanOutX, tanOutY));
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyBezierFloat(splineTimeStamp_t timeStamp, float value, float tanInX, float tanInY, float tanOutX, float tanOutY)
    {
        getIAnimationSystem().setSplineKeyTangentsFloat(m_splineHandle, timeStamp, value, ramses_internal::Vector2(tanInX, tanInY), ramses_internal::Vector2(tanOutX, tanOutY));
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyBezierVector2f(splineTimeStamp_t timeStamp, float x, float y, float tanInX, float tanInY, float tanOutX, float tanOutY)
    {
        getIAnimationSystem().setSplineKeyTangentsVector2f(m_splineHandle, timeStamp, ramses_internal::Vector2(x, y), ramses_internal::Vector2(tanInX, tanInY), ramses_internal::Vector2(tanOutX, tanOutY));
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyBezierVector3f(splineTimeStamp_t timeStamp, float x, float y, float z, float tanInX, float tanInY, float tanOutX, float tanOutY)
    {
        getIAnimationSystem().setSplineKeyTangentsVector3f(m_splineHandle, timeStamp, ramses_internal::Vector3(x, y, z), ramses_internal::Vector2(tanInX, tanInY), ramses_internal::Vector2(tanOutX, tanOutY));
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyBezierVector4f(splineTimeStamp_t timeStamp, float x, float y, float z, float w, float tanInX, float tanInY, float tanOutX, float tanOutY)
    {
        getIAnimationSystem().setSplineKeyTangentsVector4f(m_splineHandle, timeStamp, ramses_internal::Vector4(x, y, z, w), ramses_internal::Vector2(tanInX, tanInY), ramses_internal::Vector2(tanOutX, tanOutY));
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyBezierVector2i(splineTimeStamp_t timeStamp, int32_t x, int32_t y, float tanInX, float tanInY, float tanOutX, float tanOutY)
    {
        getIAnimationSystem().setSplineKeyTangentsVector2i(m_splineHandle, timeStamp, ramses_internal::Vector2i(x, y), ramses_internal::Vector2(tanInX, tanInY), ramses_internal::Vector2(tanOutX, tanOutY));
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyBezierVector3i(splineTimeStamp_t timeStamp, int32_t x, int32_t y, int32_t z, float tanInX, float tanInY, float tanOutX, float tanOutY)
    {
        getIAnimationSystem().setSplineKeyTangentsVector3i(m_splineHandle, timeStamp, ramses_internal::Vector3i(x, y, z), ramses_internal::Vector2(tanInX, tanInY), ramses_internal::Vector2(tanOutX, tanOutY));
        return StatusOK;
    }

    status_t SplineImpl::setSplineKeyBezierVector4i(splineTimeStamp_t timeStamp, int32_t x, int32_t y, int32_t z, int32_t w, float tanInX, float tanInY, float tanOutX, float tanOutY)
    {
        getIAnimationSystem().setSplineKeyTangentsVector4i(m_splineHandle, timeStamp, ramses_internal::Vector4i(x, y, z, w), ramses_internal::Vector2(tanInX, tanInY), ramses_internal::Vector2(tanOutX, tanOutY));
        return StatusOK;
    }

    status_t SplineImpl::getSplineKeyBool(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, bool& value) const
    {
        const ramses_internal::SplineBase* spline = getIAnimationSystem().getSpline(m_splineHandle);
        CHECK_RETURN_ERR(checkIfKeyIndexValid(spline, keyIndex));
        value = getSplineKeyValue<ramses_internal::Bool>(spline, keyIndex);
        timeStamp = spline->getTimeStamp(keyIndex);

        return StatusOK;
    }

    status_t SplineImpl::getSplineKeyInt32(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, int32_t& value) const
    {
        const ramses_internal::SplineBase* spline = getIAnimationSystem().getSpline(m_splineHandle);
        CHECK_RETURN_ERR(checkIfKeyIndexValid(spline, keyIndex));
        value = getSplineKeyValue<ramses_internal::Int32>(spline, keyIndex);
        timeStamp = spline->getTimeStamp(keyIndex);

        return StatusOK;
    }

    status_t SplineImpl::getSplineKeyFloat(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& value) const
    {
        const ramses_internal::SplineBase* spline = getIAnimationSystem().getSpline(m_splineHandle);
        CHECK_RETURN_ERR(checkIfKeyIndexValid(spline, keyIndex));
        value = getSplineKeyValue<ramses_internal::Float>(spline, keyIndex);
        timeStamp = spline->getTimeStamp(keyIndex);

        return StatusOK;
    }

    status_t SplineImpl::getSplineKeyVector2f(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& x, float& y) const
    {
        const ramses_internal::SplineBase* spline = getIAnimationSystem().getSpline(m_splineHandle);
        CHECK_RETURN_ERR(checkIfKeyIndexValid(spline, keyIndex));
        const ramses_internal::Vector2& value = getSplineKeyValue<ramses_internal::Vector2>(spline, keyIndex);
        x = value.x;
        y = value.y;
        timeStamp = spline->getTimeStamp(keyIndex);

        return StatusOK;
    }

    status_t SplineImpl::getSplineKeyVector3f(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& x, float& y, float& z) const
    {
        const ramses_internal::SplineBase* spline = getIAnimationSystem().getSpline(m_splineHandle);
        CHECK_RETURN_ERR(checkIfKeyIndexValid(spline, keyIndex));
        const ramses_internal::Vector3& value = getSplineKeyValue<ramses_internal::Vector3>(spline, keyIndex);
        x = value.x;
        y = value.y;
        z = value.z;
        timeStamp = spline->getTimeStamp(keyIndex);

        return StatusOK;
    }

    status_t SplineImpl::getSplineKeyVector4f(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& x, float& y, float& z, float& w) const
    {
        const ramses_internal::SplineBase* spline = getIAnimationSystem().getSpline(m_splineHandle);
        CHECK_RETURN_ERR(checkIfKeyIndexValid(spline, keyIndex));
        const ramses_internal::Vector4& value = getSplineKeyValue<ramses_internal::Vector4>(spline, keyIndex);
        x = value.x;
        y = value.y;
        z = value.z;
        w = value.w;
        timeStamp = spline->getTimeStamp(keyIndex);

        return StatusOK;
    }

    status_t SplineImpl::getSplineKeyVector2i(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, int32_t& x, int32_t& y) const
    {
        const ramses_internal::SplineBase* spline = getIAnimationSystem().getSpline(m_splineHandle);
        CHECK_RETURN_ERR(checkIfKeyIndexValid(spline, keyIndex));
        const ramses_internal::Vector2i& value = getSplineKeyValue<ramses_internal::Vector2i>(spline, keyIndex);
        x = value.x;
        y = value.y;
        timeStamp = spline->getTimeStamp(keyIndex);

        return StatusOK;
    }

    status_t SplineImpl::getSplineKeyVector3i(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, int32_t& x, int32_t& y, int32_t& z) const
    {
        const ramses_internal::SplineBase* spline = getIAnimationSystem().getSpline(m_splineHandle);
        CHECK_RETURN_ERR(checkIfKeyIndexValid(spline, keyIndex));
        const ramses_internal::Vector3i& value = getSplineKeyValue<ramses_internal::Vector3i>(spline, keyIndex);
        x = value.x;
        y = value.y;
        z = value.z;
        timeStamp = spline->getTimeStamp(keyIndex);

        return StatusOK;
    }

    status_t SplineImpl::getSplineKeyVector4i(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, int32_t& x, int32_t& y, int32_t& z, int32_t& w) const
    {
        const ramses_internal::SplineBase* spline = getIAnimationSystem().getSpline(m_splineHandle);
        CHECK_RETURN_ERR(checkIfKeyIndexValid(spline, keyIndex));
        const ramses_internal::Vector4i& value = getSplineKeyValue<ramses_internal::Vector4i>(spline, keyIndex);
        x = value.x;
        y = value.y;
        z = value.z;
        w = value.w;
        timeStamp = spline->getTimeStamp(keyIndex);

        return StatusOK;
    }

    status_t SplineImpl::getSplineKeyTangentsInt32(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, int32_t& value, float& tanInX, float& tanInY, float& tanOutX, float& tanOutY) const
    {
        const ramses_internal::SplineBase* spline = getIAnimationSystem().getSpline(m_splineHandle);
        CHECK_RETURN_ERR(checkIfKeyIndexValid(spline, keyIndex));
        value = getSplineKeyTangentsValue<ramses_internal::Int32>(spline, keyIndex, tanInX, tanInY, tanOutX, tanOutY);
        timeStamp = spline->getTimeStamp(keyIndex);

        return StatusOK;
    }

    status_t SplineImpl::getSplineKeyTangentsFloat(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& value, float& tanInX, float& tanInY, float& tanOutX, float& tanOutY) const
    {
        const ramses_internal::SplineBase* spline = getIAnimationSystem().getSpline(m_splineHandle);
        CHECK_RETURN_ERR(checkIfKeyIndexValid(spline, keyIndex));
        value = getSplineKeyTangentsValue<ramses_internal::Float>(spline, keyIndex, tanInX, tanInY, tanOutX, tanOutY);
        timeStamp = spline->getTimeStamp(keyIndex);

        return StatusOK;
    }

    status_t SplineImpl::getSplineKeyTangentsVector2f(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& x, float& y, float& tanInX, float& tanInY, float& tanOutX, float& tanOutY) const
    {
        const ramses_internal::SplineBase* spline = getIAnimationSystem().getSpline(m_splineHandle);
        CHECK_RETURN_ERR(checkIfKeyIndexValid(spline, keyIndex));
        const ramses_internal::Vector2& value = getSplineKeyTangentsValue<ramses_internal::Vector2>(spline, keyIndex, tanInX, tanInY, tanOutX, tanOutY);
        x = value.x;
        y = value.y;
        timeStamp = spline->getTimeStamp(keyIndex);

        return StatusOK;
    }

    status_t SplineImpl::getSplineKeyTangentsVector3f(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& x, float& y, float& z, float& tanInX, float& tanInY, float& tanOutX, float& tanOutY) const
    {
        const ramses_internal::SplineBase* spline = getIAnimationSystem().getSpline(m_splineHandle);
        CHECK_RETURN_ERR(checkIfKeyIndexValid(spline, keyIndex));
        const ramses_internal::Vector3& value = getSplineKeyTangentsValue<ramses_internal::Vector3>(spline, keyIndex, tanInX, tanInY, tanOutX, tanOutY);
        x = value.x;
        y = value.y;
        z = value.z;
        timeStamp = spline->getTimeStamp(keyIndex);

        return StatusOK;
    }

    status_t SplineImpl::getSplineKeyTangentsVector4f(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& x, float& y, float& z, float& w, float& tanInX, float& tanInY, float& tanOutX, float& tanOutY) const
    {
        const ramses_internal::SplineBase* spline = getIAnimationSystem().getSpline(m_splineHandle);
        CHECK_RETURN_ERR(checkIfKeyIndexValid(spline, keyIndex));
        const ramses_internal::Vector4& value = getSplineKeyTangentsValue<ramses_internal::Vector4>(spline, keyIndex, tanInX, tanInY, tanOutX, tanOutY);
        x = value.x;
        y = value.y;
        z = value.z;
        w = value.w;
        timeStamp = spline->getTimeStamp(keyIndex);

        return StatusOK;
    }

    status_t SplineImpl::getSplineKeyTangentsVector2i(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, int32_t& x, int32_t& y, float& tanInX, float& tanInY, float& tanOutX, float& tanOutY) const
    {
        const ramses_internal::SplineBase* spline = getIAnimationSystem().getSpline(m_splineHandle);
        CHECK_RETURN_ERR(checkIfKeyIndexValid(spline, keyIndex));
        const ramses_internal::Vector2i& value = getSplineKeyTangentsValue<ramses_internal::Vector2i>(spline, keyIndex, tanInX, tanInY, tanOutX, tanOutY);
        x = value.x;
        y = value.y;
        timeStamp = spline->getTimeStamp(keyIndex);

        return StatusOK;
    }

    status_t SplineImpl::getSplineKeyTangentsVector3i(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, int32_t& x, int32_t& y, int32_t& z, float& tanInX, float& tanInY, float& tanOutX, float& tanOutY) const
    {
        const ramses_internal::SplineBase* spline = getIAnimationSystem().getSpline(m_splineHandle);
        CHECK_RETURN_ERR(checkIfKeyIndexValid(spline, keyIndex));
        const ramses_internal::Vector3i& value = getSplineKeyTangentsValue<ramses_internal::Vector3i>(spline, keyIndex, tanInX, tanInY, tanOutX, tanOutY);
        x = value.x;
        y = value.y;
        z = value.z;
        timeStamp = spline->getTimeStamp(keyIndex);

        return StatusOK;
    }

    status_t SplineImpl::getSplineKeyTangentsVector4i(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, int32_t& x, int32_t& y, int32_t& z, int32_t& w, float& tanInX, float& tanInY, float& tanOutX, float& tanOutY) const
    {
        const ramses_internal::SplineBase* spline = getIAnimationSystem().getSpline(m_splineHandle);
        CHECK_RETURN_ERR(checkIfKeyIndexValid(spline, keyIndex));
        const ramses_internal::Vector4i& value = getSplineKeyTangentsValue<ramses_internal::Vector4i>(spline, keyIndex, tanInX, tanInY, tanOutX, tanOutY);
        x = value.x;
        y = value.y;
        z = value.z;
        w = value.w;
        timeStamp = spline->getTimeStamp(keyIndex);

        return StatusOK;
    }

    status_t SplineImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(AnimationObjectImpl::serialize(outStream, serializationContext));

        outStream << m_splineHandle;

        return StatusOK;
    }

    status_t SplineImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(AnimationObjectImpl::deserialize(inStream, serializationContext));

        inStream >> m_splineHandle;

        return StatusOK;
    }

    status_t SplineImpl::validate(uint32_t indent, StatusObjectSet& visitedObjects) const
    {
        status_t status = AnimationObjectImpl::validate(indent, visitedObjects);
        indent += IndentationStep;

        const ramses_internal::SplineBase& spline = *getIAnimationSystem().getSpline(m_splineHandle);
        if (spline.getNumKeys() == 0u)
        {
            addValidationMessage(EValidationSeverity_Warning, indent, "spline has no keys assigned");
            status = getValidationErrorStatus();
        }

        return status;
    }

    void SplineImpl::initializeFrameworkData(ramses_internal::EInterpolationType interpolationType, ramses_internal::EDataTypeID dataTypeID)
    {
        const ramses_internal::ESplineKeyType keyType = GetKeyTypeForInterpolation(interpolationType);
        m_splineHandle = getIAnimationSystem().allocateSpline(keyType, dataTypeID);
    }

    void SplineImpl::deinitializeFrameworkData()
    {
        getIAnimationSystem().removeSpline(m_splineHandle);
        m_splineHandle = ramses_internal::SplineHandle::Invalid();
    }

    ramses_internal::SplineHandle SplineImpl::getSplineHandle() const
    {
        return m_splineHandle;
    }

    uint32_t SplineImpl::getNumKeys() const
    {
        const ramses_internal::SplineBase* spline = getIAnimationSystem().getSpline(m_splineHandle);
        if (spline != nullptr)
        {
            return spline->getNumKeys();
        }

        return 0u;
    }

    ramses_internal::EDataTypeID SplineImpl::GetDataTypeForSplineType(ERamsesObjectType splineType)
    {
        switch (splineType)
        {
        case ramses::ERamsesObjectType_SplineStepBool:
            return ramses_internal::EDataTypeID_Boolean;
        case ramses::ERamsesObjectType_SplineStepFloat:
        case ramses::ERamsesObjectType_SplineLinearFloat:
        case ramses::ERamsesObjectType_SplineBezierFloat:
            return ramses_internal::EDataTypeID_Float;
        case ramses::ERamsesObjectType_SplineStepInt32:
        case ramses::ERamsesObjectType_SplineLinearInt32:
        case ramses::ERamsesObjectType_SplineBezierInt32:
            return ramses_internal::EDataTypeID_Int32;
        case ramses::ERamsesObjectType_SplineStepVector2f:
        case ramses::ERamsesObjectType_SplineLinearVector2f:
        case ramses::ERamsesObjectType_SplineBezierVector2f:
            return ramses_internal::EDataTypeID_Vector2f;
        case ramses::ERamsesObjectType_SplineStepVector3f:
        case ramses::ERamsesObjectType_SplineLinearVector3f:
        case ramses::ERamsesObjectType_SplineBezierVector3f:
            return ramses_internal::EDataTypeID_Vector3f;
        case ramses::ERamsesObjectType_SplineStepVector4f:
        case ramses::ERamsesObjectType_SplineLinearVector4f:
        case ramses::ERamsesObjectType_SplineBezierVector4f:
            return ramses_internal::EDataTypeID_Vector4f;
        case ramses::ERamsesObjectType_SplineStepVector2i:
        case ramses::ERamsesObjectType_SplineLinearVector2i:
        case ramses::ERamsesObjectType_SplineBezierVector2i:
            return ramses_internal::EDataTypeID_Vector2i;
        case ramses::ERamsesObjectType_SplineStepVector3i:
        case ramses::ERamsesObjectType_SplineLinearVector3i:
        case ramses::ERamsesObjectType_SplineBezierVector3i:
            return ramses_internal::EDataTypeID_Vector3i;
        case ramses::ERamsesObjectType_SplineStepVector4i:
        case ramses::ERamsesObjectType_SplineLinearVector4i:
        case ramses::ERamsesObjectType_SplineBezierVector4i:
            return ramses_internal::EDataTypeID_Vector4i;
        default:
            assert(false);
            return ramses_internal::EDataTypeID_Invalid;
        }
    }

    ramses_internal::EInterpolationType SplineImpl::GetInterpolationTypeForSplineType(ERamsesObjectType splineType)
    {
        switch (splineType)
        {
        case ramses::ERamsesObjectType_SplineStepBool:
        case ramses::ERamsesObjectType_SplineStepFloat:
        case ramses::ERamsesObjectType_SplineStepInt32:
        case ramses::ERamsesObjectType_SplineStepVector2f:
        case ramses::ERamsesObjectType_SplineStepVector3f:
        case ramses::ERamsesObjectType_SplineStepVector4f:
        case ramses::ERamsesObjectType_SplineStepVector2i:
        case ramses::ERamsesObjectType_SplineStepVector3i:
        case ramses::ERamsesObjectType_SplineStepVector4i:
            return ramses_internal::EInterpolationType_Step;
        case ramses::ERamsesObjectType_SplineLinearFloat:
        case ramses::ERamsesObjectType_SplineLinearInt32:
        case ramses::ERamsesObjectType_SplineLinearVector2f:
        case ramses::ERamsesObjectType_SplineLinearVector3f:
        case ramses::ERamsesObjectType_SplineLinearVector4f:
        case ramses::ERamsesObjectType_SplineLinearVector2i:
        case ramses::ERamsesObjectType_SplineLinearVector3i:
        case ramses::ERamsesObjectType_SplineLinearVector4i:
            return ramses_internal::EInterpolationType_Linear;
        case ramses::ERamsesObjectType_SplineBezierFloat:
        case ramses::ERamsesObjectType_SplineBezierInt32:
        case ramses::ERamsesObjectType_SplineBezierVector2f:
        case ramses::ERamsesObjectType_SplineBezierVector3f:
        case ramses::ERamsesObjectType_SplineBezierVector4f:
        case ramses::ERamsesObjectType_SplineBezierVector2i:
        case ramses::ERamsesObjectType_SplineBezierVector3i:
        case ramses::ERamsesObjectType_SplineBezierVector4i:
            return ramses_internal::EInterpolationType_Bezier;
        default:
            assert(false);
            return ramses_internal::EInterpolationType_Invalid;
        }
    }

    ramses_internal::ESplineKeyType SplineImpl::GetKeyTypeForInterpolation(ramses_internal::EInterpolationType interpolationType)
    {
        switch (interpolationType)
        {
        case ramses_internal::EInterpolationType_Step:
        case ramses_internal::EInterpolationType_Linear:
            return ramses_internal::ESplineKeyType_Basic;
        case ramses_internal::EInterpolationType_Bezier:
            return ramses_internal::ESplineKeyType_Tangents;
        default:
            assert(false);
            return ramses_internal::ESplineKeyType_Invalid;
        }
    }

    status_t SplineImpl::checkIfKeyIndexValid(const ramses_internal::SplineBase* spline, ramses_internal::SplineKeyIndex keyIndex) const
    {
        assert(spline != nullptr);
        if (keyIndex >= spline->getNumKeys())
        {
            return addErrorEntry("Spline::getKey failed, invalid spline key index.");
        }

        return StatusOK;
    }

    template <typename EDataType>
    const EDataType& SplineImpl::getSplineKeyValue(const ramses_internal::SplineBase* spline, ramses_internal::SplineKeyIndex keyIndex)
    {
        typedef ramses_internal::SplineKey<EDataType> SplineKeyType;
        using SplineType = ramses_internal::Spline<ramses_internal::SplineKey, EDataType>;

        assert(spline != nullptr);
        assert(spline->getKeyType() == ramses_internal::ESplineKeyType_Basic);
        assert(spline->getDataType() == ramses_internal::DataTypeToDataIDSelector<EDataType>::DataTypeID);

        const SplineType& splineTyped = static_cast<const SplineType&>(*spline);
        const SplineKeyType& key = splineTyped.getKey(keyIndex);
        return key.m_value;
    }

    template <typename EDataType>
    const EDataType& SplineImpl::getSplineKeyTangentsValue(const ramses_internal::SplineBase* spline, ramses_internal::SplineKeyIndex keyIndex, float& tanInX, float& tanInY, float& tanOutX, float& tanOutY)
    {
        using SplineKeyType = ramses_internal::SplineKeyTangents<EDataType>;
        using SplineType = ramses_internal::Spline<ramses_internal::SplineKeyTangents, EDataType>;

        assert(spline != nullptr);
        assert(spline->getKeyType() == ramses_internal::ESplineKeyType_Tangents);
        assert(spline->getDataType() == ramses_internal::DataTypeToDataIDSelector<EDataType>::DataTypeID);

        const SplineType& splineTyped = static_cast<const SplineType&>(*spline);
        const SplineKeyType& key = splineTyped.getKey(keyIndex);

        tanInX = key.m_tangentIn.x;
        tanInY = key.m_tangentIn.y;
        tanOutX = key.m_tangentOut.x;
        tanOutY = key.m_tangentOut.y;

        return key.m_value;
    }
}
