//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SPLINEIMPL_H
#define RAMSES_SPLINEIMPL_H

// API
#include "ramses-client-api/AnimationTypes.h"

// internal
#include "AnimationObjectImpl.h"
#include "Utils/DataTypeUtils.h"
#include "Animation/AnimationCommon.h"

namespace ramses_internal
{
    class SplineBase;
}

namespace ramses
{
    class SplineImpl final : public AnimationObjectImpl
    {
    public:
        explicit SplineImpl(AnimationSystemImpl& animationSystem, ERamsesObjectType type, const char* name);
        virtual ~SplineImpl() override;

        void             initializeFrameworkData(ramses_internal::EInterpolationType interpolationType, ramses_internal::EDataTypeID dataTypeID);
        virtual void     deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        virtual status_t validate() const override;

        status_t setSplineKeyStepBool(splineTimeStamp_t timeStamp, bool value);
        status_t setSplineKeyStepInt32(splineTimeStamp_t timeStamp, int32_t value);
        status_t setSplineKeyStepFloat(splineTimeStamp_t timeStamp, float value);
        status_t setSplineKeyStepVector2f(splineTimeStamp_t timeStamp, float x, float y);
        status_t setSplineKeyStepVector3f(splineTimeStamp_t timeStamp, float x, float y, float z);
        status_t setSplineKeyStepVector4f(splineTimeStamp_t timeStamp, float x, float y, float z, float w);
        status_t setSplineKeyStepVector2i(splineTimeStamp_t timeStamp, int32_t x, int32_t y);
        status_t setSplineKeyStepVector3i(splineTimeStamp_t timeStamp, int32_t x, int32_t y, int32_t z);
        status_t setSplineKeyStepVector4i(splineTimeStamp_t timeStamp, int32_t x, int32_t y, int32_t z, int32_t w);
        status_t setSplineKeyLinearInt32(splineTimeStamp_t timeStamp, int32_t value);
        status_t setSplineKeyLinearFloat(splineTimeStamp_t timeStamp, float value);
        status_t setSplineKeyLinearVector2f(splineTimeStamp_t timeStamp, float x, float y);
        status_t setSplineKeyLinearVector3f(splineTimeStamp_t timeStamp, float x, float y, float z);
        status_t setSplineKeyLinearVector4f(splineTimeStamp_t timeStamp, float x, float y, float z, float w);
        status_t setSplineKeyLinearVector2i(splineTimeStamp_t timeStamp, int32_t x, int32_t y);
        status_t setSplineKeyLinearVector3i(splineTimeStamp_t timeStamp, int32_t x, int32_t y, int32_t z);
        status_t setSplineKeyLinearVector4i(splineTimeStamp_t timeStamp, int32_t x, int32_t y, int32_t z, int32_t w);
        status_t setSplineKeyBezierInt32(splineTimeStamp_t timeStamp, int32_t value, float tanInX, float tanInY, float tanOutX, float tanOutY);
        status_t setSplineKeyBezierFloat(splineTimeStamp_t timeStamp, float value, float tanInX, float tanInY, float tanOutX, float tanOutY);
        status_t setSplineKeyBezierVector2f(splineTimeStamp_t timeStamp, float x, float y, float tanInX, float tanInY, float tanOutX, float tanOutY);
        status_t setSplineKeyBezierVector3f(splineTimeStamp_t timeStamp, float x, float y, float z, float tanInX, float tanInY, float tanOutX, float tanOutY);
        status_t setSplineKeyBezierVector4f(splineTimeStamp_t timeStamp, float x, float y, float z, float w, float tanInX, float tanInY, float tanOutX, float tanOutY);
        status_t setSplineKeyBezierVector2i(splineTimeStamp_t timeStamp, int32_t x, int32_t y, float tanInX, float tanInY, float tanOutX, float tanOutY);
        status_t setSplineKeyBezierVector3i(splineTimeStamp_t timeStamp, int32_t x, int32_t y, int32_t z, float tanInX, float tanInY, float tanOutX, float tanOutY);
        status_t setSplineKeyBezierVector4i(splineTimeStamp_t timeStamp, int32_t x, int32_t y, int32_t z, int32_t w, float tanInX, float tanInY, float tanOutX, float tanOutY);

        status_t getSplineKeyBool    (splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, bool& value) const;
        status_t getSplineKeyInt32   (splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, int32_t& value) const;
        status_t getSplineKeyFloat   (splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& value) const;
        status_t getSplineKeyVector2f(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& x, float& y) const;
        status_t getSplineKeyVector3f(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& x, float& y, float& z) const;
        status_t getSplineKeyVector4f(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& x, float& y, float& z, float& w) const;
        status_t getSplineKeyVector2i(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, int32_t& x, int32_t& y) const;
        status_t getSplineKeyVector3i(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, int32_t& x, int32_t& y, int32_t& z) const;
        status_t getSplineKeyVector4i(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, int32_t& x, int32_t& y, int32_t& z, int32_t& w) const;
        status_t getSplineKeyTangentsInt32(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, int32_t& value, float& tanInX, float& tanInY, float& tanOutX, float& tanOutY) const;
        status_t getSplineKeyTangentsFloat(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& value, float& tanInX, float& tanInY, float& tanOutX, float& tanOutY) const;
        status_t getSplineKeyTangentsVector2f(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& x, float& y, float& tanInX, float& tanInY, float& tanOutX, float& tanOutY) const;
        status_t getSplineKeyTangentsVector3f(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& x, float& y, float& z, float& tanInX, float& tanInY, float& tanOutX, float& tanOutY) const;
        status_t getSplineKeyTangentsVector4f(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& x, float& y, float& z, float& w, float& tanInX, float& tanInY, float& tanOutX, float& tanOutY) const;
        status_t getSplineKeyTangentsVector2i(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, int32_t& x, int32_t& y, float& tanInX, float& tanInY, float& tanOutX, float& tanOutY) const;
        status_t getSplineKeyTangentsVector3i(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, int32_t& x, int32_t& y, int32_t& z, float& tanInX, float& tanInY, float& tanOutX, float& tanOutY) const;
        status_t getSplineKeyTangentsVector4i(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, int32_t& x, int32_t& y, int32_t& z, int32_t& w, float& tanInX, float& tanInY, float& tanOutX, float& tanOutY) const;

        ramses_internal::SplineHandle       getSplineHandle() const;
        uint32_t                            getNumKeys() const;

        static ramses_internal::EDataTypeID        GetDataTypeForSplineType(ERamsesObjectType splineType);
        static ramses_internal::EInterpolationType GetInterpolationTypeForSplineType(ERamsesObjectType splineType);
        static ramses_internal::ESplineKeyType     GetKeyTypeForInterpolation(ramses_internal::EInterpolationType interpolationType);

    private:
        status_t checkIfKeyIndexValid(const ramses_internal::SplineBase* spline, ramses_internal::SplineKeyIndex keyIndex) const;

        template <typename EDataType>
        static const EDataType& getSplineKeyValue(const ramses_internal::SplineBase* spline, ramses_internal::SplineKeyIndex keyIndex);
        template <typename EDataType>
        static const EDataType& getSplineKeyTangentsValue(const ramses_internal::SplineBase* spline, ramses_internal::SplineKeyIndex keyIndex, float& tanInX, float& tanInY, float& tanOutX, float& tanOutY);

        ramses_internal::SplineHandle      m_splineHandle;
    };
}

#endif
