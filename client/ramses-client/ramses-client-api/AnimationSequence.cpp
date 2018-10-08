//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/AnimationSequence.h"
#include "ramses-client-api/Animation.h"

// internal
#include "AnimationSequenceImpl.h"
#include "AnimationImpl.h"
#include "Animation/AnimationTime.h"
#include "Animation/Animation.h"
#include "PlatformAbstraction/PlatformTime.h"

namespace ramses
{
    AnimationSequence::AnimationSequence(AnimationSequenceImpl& pimpl)
        : AnimationObject(pimpl)
        , impl(pimpl)
    {
    }

    AnimationSequence::~AnimationSequence()
    {
    }

    status_t AnimationSequence::addAnimation(const Animation& animation, sequenceTimeStamp_t startTimeInSequence, sequenceTimeStamp_t stopTimeInSequence)
    {
        const status_t status = impl.addAnimation(animation, startTimeInSequence, stopTimeInSequence);
        LOG_HL_CLIENT_API3(status, LOG_API_RAMSESOBJECT_STRING(animation), startTimeInSequence, stopTimeInSequence)
        return status;
    }

    status_t AnimationSequence::removeAnimation(const Animation& animation)
    {
        const status_t status = impl.removeAnimation(animation);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(animation))
        return status;
    }

    status_t AnimationSequence::start(timeMilliseconds_t offset)
    {
        const status_t status = impl.start(offset);
        LOG_HL_CLIENT_API1(status, offset);
        return status;
    }

    status_t AnimationSequence::startReverse(timeMilliseconds_t offset)
    {
        const status_t status = impl.startReverse(offset);
        LOG_HL_CLIENT_API1(status, offset)
        return status;
    }

    status_t AnimationSequence::startAt(globalTimeStamp_t timeStamp)
    {
        const status_t status = impl.startAt(timeStamp);
        LOG_HL_CLIENT_API1(status, timeStamp)
        return status;
    }

    status_t AnimationSequence::startReverseAt(globalTimeStamp_t timeStamp)
    {
        const status_t status = impl.startReverseAt(timeStamp);
        LOG_HL_CLIENT_API1(status, timeStamp)
        return status;
    }

    status_t AnimationSequence::stop(timeMilliseconds_t delay)
    {
        const status_t status = impl.stop(delay);
        LOG_HL_CLIENT_API1(status, delay)
        return status;
    }

    status_t AnimationSequence::stopAt(globalTimeStamp_t timeStamp)
    {
        const status_t status = impl.stopAt(timeStamp);
        LOG_HL_CLIENT_API1(status, timeStamp)
        return status;
    }

    status_t AnimationSequence::setPlaybackSpeed(float playbackSpeed)
    {
        const status_t status = impl.setPlaybackSpeed(playbackSpeed);
        LOG_HL_CLIENT_API1(status, playbackSpeed)
        return status;
    }

    float AnimationSequence::getPlaybackSpeed() const
    {
        return impl.getPlaybackSpeed();
    }

    status_t AnimationSequence::setAnimationLooping(const Animation& animation, timeMilliseconds_t loopDuration)
    {
        const status_t status = impl.setAnimationLooping(animation, loopDuration);
        LOG_HL_CLIENT_API2(status, LOG_API_RAMSESOBJECT_STRING(animation), loopDuration)
        return status;
    }

    bool AnimationSequence::isAnimationLooping(const Animation& animation) const
    {
        return impl.isAnimationLooping(animation);
    }

    timeMilliseconds_t AnimationSequence::getAnimationLoopDuration(const Animation& animation) const
    {
        return impl.getAnimationLoopDuration(animation);
    }

    status_t AnimationSequence::setAnimationRelative(const Animation& animation)
    {
        const status_t status = impl.setAnimationRelative(animation);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(animation))
        return status;
    }

    status_t AnimationSequence::setAnimationAbsolute(const Animation& animation)
    {
        const status_t status = impl.setAnimationAbsolute(animation);
        LOG_HL_CLIENT_API1(status, LOG_API_RAMSESOBJECT_STRING(animation))
        return status;
    }

    bool AnimationSequence::isAnimationRelative(const Animation& animation) const
    {
        return impl.isAnimationRelative(animation);
    }

    uint32_t AnimationSequence::getNumberOfAnimations() const
    {
        return impl.getNumAnimations();
    }

    bool AnimationSequence::containsAnimation(const Animation& animation) const
    {
        return impl.containsAnimation(animation);
    }

    sequenceTimeStamp_t AnimationSequence::getAnimationStartTimeInSequence(const Animation& animation) const
    {
        return impl.getAnimationStartTimeInSequence(animation);
    }

    sequenceTimeStamp_t AnimationSequence::getAnimationStopTimeInSequence(const Animation& animation) const
    {
        return impl.getAnimationStopTimeInSequence(animation);
    }

    sequenceTimeStamp_t AnimationSequence::getAnimationSequenceStopTime() const
    {
        return impl.getAnimationSequenceStopTime();
    }
}
