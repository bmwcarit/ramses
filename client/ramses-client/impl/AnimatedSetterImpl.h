//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATEDSETTERIMPL_H
#define RAMSES_ANIMATEDSETTERIMPL_H

#include "AnimationObjectImpl.h"
#include "AnimationImpl.h"
#include "Animation/AnimationCommon.h"
#include "Utils/DataTypeUtils.h"

namespace ramses_internal
{
    class Variant;
}

namespace ramses
{
    class AnimatedProperty;

    class AnimatedSetterImpl final : public AnimationObjectImpl
    {
    public:
        explicit AnimatedSetterImpl(AnimationSystemImpl& animationSystem, const timeMilliseconds_t& delay, const char* name);
        virtual ~AnimatedSetterImpl();

        void             initializeFrameworkData(const AnimatedPropertyImpl& animatedProperty);
        virtual void     deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        virtual status_t validate(uint32_t indent, StatusObjectSet& visitedObjects) const override;

        status_t setValue(bool x);
        status_t setValue(int32_t x);
        status_t setValue(float x);
        status_t setValue(float x, float y);
        status_t setValue(float x, float y, float z);
        status_t setValue(float x, float y, float z, float w);
        status_t setValue(int32_t x, int32_t y);
        status_t setValue(int32_t x, int32_t y, int32_t z);
        status_t setValue(int32_t x, int32_t y, int32_t z, int32_t w);

        status_t stop(timeMilliseconds_t delay);
        status_t stopAt(globalTimeStamp_t timeStamp);

        ramses_internal::SplineHandle            getSplineHandle() const;
        const AnimationImpl&                     getAnimation() const;

        template <typename DATA>
        status_t setValueInternal(const DATA& value);

    private:
        template <typename DATA>
        void setSplineKey(splineTimeStamp_t keyTimeStamp, const DATA& value);

        ramses::AnimationImpl                    m_animation;
        ramses_internal::SplineHandle            m_splineHandle;
        ramses_internal::EDataTypeID             m_splineDataType;
        const timeMilliseconds_t&                m_delay;
    };
}

#endif
