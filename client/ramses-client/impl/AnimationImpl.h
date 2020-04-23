//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONIMPL_H
#define RAMSES_ANIMATIONIMPL_H

// API
#include "ramses-client-api/AnimationTypes.h"

// internal
#include "AnimationObjectImpl.h"
#include "Animation/AnimationCommon.h"

namespace ramses
{
    class AnimatedProperty;
    class SplineImpl;
    class AnimatedPropertyImpl;

    class AnimationImpl final : public AnimationObjectImpl
    {
    public:
        explicit AnimationImpl(AnimationSystemImpl& animationSystem, const char* name);
        virtual ~AnimationImpl();

        void             initializeFrameworkData(const AnimatedPropertyImpl& animatedProperty, ramses_internal::SplineHandle splineHandle, ramses_internal::EInterpolationType interpolationType);
        virtual void     deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        virtual status_t validate(uint32_t indent, StatusObjectSet& visitedObjects) const override;

        status_t stop(timeMilliseconds_t delay);
        status_t stopAt(globalTimeStamp_t timeStamp);

        globalTimeStamp_t  getStartTime() const;
        globalTimeStamp_t  getStopTime() const;

        ramses_internal::AnimationInstanceHandle getAnimationInstanceHandle() const;
        ramses_internal::AnimationHandle         getAnimationHandle() const;

        const AnimatedPropertyImpl* findAnimatedProperty(ramses_internal::DataBindHandle handle) const;
        const SplineImpl*           findSpline(ramses_internal::SplineHandle handle) const;

    private:
        ramses_internal::AnimationInstanceHandle m_animationInstanceHandle;
        ramses_internal::AnimationHandle         m_animationHandle;
    };
}

#endif
