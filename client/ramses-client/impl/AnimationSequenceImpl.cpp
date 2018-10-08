//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/Animation.h"
#include "ramses-client-api/AnimationSequence.h"
#include "AnimationSequenceImpl.h"
#include "AnimationSystemImpl.h"
#include "AnimationImpl.h"
#include "SceneImpl.h"
#include "RamsesObjectTypeUtils.h"
#include "RamsesObjectRegistryIterator.h"
#include "Animation/Animation.h"
#include "SerializationContext.h"
#include "Common/Cpp11Macros.h"

namespace ramses
{
    AnimationSequenceImpl::AnimationSequenceImpl(AnimationSystemImpl& animationSystem, const char* name)
        : AnimationObjectImpl(animationSystem, ERamsesObjectType_AnimationSequence, name)
        , m_playbackSpeed(1.f)
    {
    }

    AnimationSequenceImpl::~AnimationSequenceImpl()
    {
    }

    void AnimationSequenceImpl::deinitializeFrameworkData()
    {
    }

    status_t AnimationSequenceImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(AnimationObjectImpl::serialize(outStream, serializationContext));

        outStream << static_cast<uint32_t>(m_animations.count());
        for (const auto& item : m_animations)
        {
            outStream << item.key;
            outStream << item.value.startTime;
            outStream << item.value.stopTime;
            outStream << item.value.flags;
            outStream << item.value.loopDuration;
        }

        outStream << m_playbackSpeed;
        outStream << m_startTime.getTimeStamp();

        return StatusOK;
    }

    status_t AnimationSequenceImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(AnimationObjectImpl::deserialize(inStream, serializationContext));

        uint32_t count = 0u;
        inStream >> count;
        for (uint32_t i = 0u; i < count; ++i)
        {
            ramses_internal::AnimationHandle handle;
            inStream >> handle;

            SequenceItem item;
            inStream >> item.startTime;
            inStream >> item.stopTime;
            inStream >> item.flags;
            inStream >> item.loopDuration;

            m_animations.put(handle, item);
        }

        inStream >> m_playbackSpeed;

        ramses_internal::AnimationTime::TimeStamp time;
        inStream >> time;
        m_startTime = time;

        return StatusOK;
    }

    status_t AnimationSequenceImpl::validate(uint32_t indent) const
    {
        status_t status = AnimationObjectImpl::validate(indent);
        indent += IndentationStep;

        if (m_animations.count() == 0u)
        {
            addValidationMessage(EValidationSeverity_Warning, indent, "sequence does not contain any animations");
            status = getValidationErrorStatus();
        }

        ramses_foreach(m_animations, animationIt)
        {
            const AnimationImpl* anim = findAnimation(animationIt->key);
            if (anim == NULL)
            {
                addValidationMessage(EValidationSeverity_Error, indent, "animation contained in sequence does not exist anymore, was probably destroyed but still used by it");
                status = getValidationErrorStatus();
            }
            else if (addValidationOfDependentObject(indent, *anim) != StatusOK)
            {
                status = getValidationErrorStatus();
            }
        }

        RamsesObjectRegistryIterator iter(getAnimationSystemImpl().getObjectRegistry(), ERamsesObjectType_AnimationSequence);
        while (const AnimationSequence* sequence = iter.getNext<AnimationSequence>())
        {
            const AnimationSequenceImpl& sequenceImpl = sequence->impl;
            if (&sequenceImpl != this)
            {
                ramses_foreach(m_animations, animationIt)
                {
                    if (sequenceImpl.m_animations.contains(animationIt->key))
                    {
                        addValidationMessage(EValidationSeverity_Warning, indent, "animation seems to be contained in more than one sequence, this is fine only if usage of those sequences does not overlap");
                        status = getValidationErrorStatus();
                    }
                }
            }
        }

        return status;
    }

    status_t AnimationSequenceImpl::addAnimation(const Animation& animation, sequenceTimeStamp_t startTimeInSequence, sequenceTimeStamp_t stopTimeInSequence)
    {
        const ramses_internal::AnimationHandle handle = animation.impl.getAnimationHandle();

        if (stopTimeInSequence == 0u)
        {
            stopTimeInSequence = getAnimationStopTime(handle, startTimeInSequence, m_playbackSpeed);
        }

        SequenceItem item;
        item.startTime = startTimeInSequence;
        item.stopTime = stopTimeInSequence;
        item.flags = 0;
        item.loopDuration = 0;
        m_animations.put(handle, item);

        return StatusOK;
    }

    status_t AnimationSequenceImpl::removeAnimation(const Animation& animation)
    {
        m_animations.remove(animation.impl.getAnimationHandle());
        return StatusOK;
    }

    status_t AnimationSequenceImpl::start(timeMilliseconds_t offset)
    {
        const globalTimeStamp_t startTime = getIAnimationSystem().getTime().getTimeStamp() + offset;
        return startAt(startTime, false);
    }

    status_t AnimationSequenceImpl::startAt(globalTimeStamp_t timeStamp)
    {
        return startAt(timeStamp, false);
    }

    status_t AnimationSequenceImpl::startReverse(timeMilliseconds_t offset)
    {
        const globalTimeStamp_t startTime = getIAnimationSystem().getTime().getTimeStamp() + offset;
        return startAt(startTime, true);
    }

    status_t AnimationSequenceImpl::startReverseAt(globalTimeStamp_t timeStamp)
    {
        return startAt(timeStamp, true);
    }

    status_t AnimationSequenceImpl::startAt(globalTimeStamp_t timeStamp, bool reverse)
    {
        const ramses_internal::AnimationTime startTime(timeStamp);

        m_startTime = startTime;

        for (auto& item : m_animations)
        {
            if (reverse)
            {
                item.value.flags |= ramses_internal::Animation::EAnimationFlags_Reverse;
            }
            else
            {
                item.value.flags &= ~ramses_internal::Animation::EAnimationFlags_Reverse;
            }
        }

        return flushSequenceData(true);
    }

    status_t AnimationSequenceImpl::flushSequenceData(bool forceFlush)
    {
        if (forceFlush || isSequenceActive())
        {
            for (const auto& item : m_animations)
            {
                setAnimationData(item.key, item.value);
            }
        }

        return StatusOK;
    }

    void AnimationSequenceImpl::setAnimationData(ramses_internal::AnimationHandle handle, const SequenceItem& item)
    {
        getIAnimationSystem().setAnimationStartTime(handle, m_startTime + item.startTime);
        getIAnimationSystem().setAnimationStopTime(handle, m_startTime + item.stopTime);
        getIAnimationSystem().setAnimationProperties(
            handle,
            m_playbackSpeed,
            item.flags,
            item.loopDuration,
            getIAnimationSystem().getTime());
    }

    status_t AnimationSequenceImpl::stop(timeMilliseconds_t delay)
    {
        const timeMilliseconds_t stopTime = getIAnimationSystem().getTime().getTimeStamp() + delay;
        return stopAt(stopTime);
    }

    status_t AnimationSequenceImpl::stopAt(globalTimeStamp_t timeStamp)
    {
        if (isSequenceActive())
        {
            for (const auto& item : m_animations)
            {
                getIAnimationSystem().setAnimationStopTime(item.key, timeStamp);
            }
        }

        return StatusOK;
    }

    status_t AnimationSequenceImpl::setPlaybackSpeed(float playbackSpeed)
    {
        if (playbackSpeed <= 0.f)
        {
            return addErrorEntry("AnimationSequence::setPlaybackSpeed failed, argument must be greater than zero.");
        }

        if (m_playbackSpeed != playbackSpeed)
        {
            const float invPlaybackSpeed = m_playbackSpeed / playbackSpeed;
            m_playbackSpeed = playbackSpeed;

            // adjust start and stop times within sequence
            for (auto& item : m_animations)
            {
                item.value.startTime = static_cast<sequenceTimeStamp_t>(item.value.startTime * invPlaybackSpeed);
                item.value.stopTime = static_cast<sequenceTimeStamp_t>(item.value.stopTime  * invPlaybackSpeed);
            }

            if (isSequenceActive())
            {
                // adjust sequence start time if it is being played right now
                const ramses_internal::AnimationTime& currentGlobalTime = getIAnimationSystem().getTime();
                const ramses_internal::AnimationTime::Duration sinceSequenceStart = currentGlobalTime.getDurationSince(m_startTime);
                if (sinceSequenceStart != ramses_internal::AnimationTime::InvalidTimeStamp)
                {
                    const ramses_internal::AnimationTime::Duration newSinceSequenceStart = static_cast<ramses_internal::AnimationTime::Duration>(sinceSequenceStart * invPlaybackSpeed);
                    m_startTime = currentGlobalTime.getTimeStamp() - newSinceSequenceStart;
                }

                return flushSequenceData(true);
            }
        }

        return StatusOK;
    }

    float AnimationSequenceImpl::getPlaybackSpeed() const
    {
        return m_playbackSpeed;
    }

    sequenceTimeStamp_t AnimationSequenceImpl::getAnimationStopTime(
        ramses_internal::AnimationHandle handle,
        sequenceTimeStamp_t startTime,
        float playbackSpeed)
    {
        const ramses_internal::Animation& animation = getIAnimationSystem().getAnimation(handle);
        ramses_internal::AnimationTime::Duration animDuration = getIAnimationSystem().getAnimationDurationFromSpline(handle);
        animDuration = static_cast<sequenceTimeStamp_t>(animDuration / playbackSpeed);
        const bool looping = (animation.m_flags & ramses_internal::Animation::EAnimationFlags_Looping) != 0;
        if (looping)
        {
            animDuration <<= 20u;
        }
        const sequenceTimeStamp_t stopTimeInSequence = startTime + static_cast<sequenceTimeStamp_t>(animDuration);
        return stopTimeInSequence;
    }

    status_t AnimationSequenceImpl::setAnimationLooping(const Animation& animation, timeMilliseconds_t loopDuration)
    {
        SequenceItemMap::Iterator it = m_animations.find(animation.impl.getAnimationHandle());
        if (it != m_animations.end())
        {
            loopDuration = (loopDuration > 0 ? loopDuration : 0u);
            const bool loopEnabled = (it->value.flags & ramses_internal::Animation::EAnimationFlags_Looping) != 0;
            if (!loopEnabled || it->value.loopDuration != loopDuration)
            {
                it->value.loopDuration = loopDuration;
                it->value.flags |= ramses_internal::Animation::EAnimationFlags_Looping;
            }

            return flushSequenceData();
        }

        return addErrorEntry("AnimationSequence::setAnimationLooping failed, animation is not included in sequence");
    }

    bool AnimationSequenceImpl::isAnimationLooping(const Animation& animation) const
    {
        SequenceItemMap::ConstIterator it = m_animations.find(animation.impl.getAnimationHandle());
        if (it != m_animations.end())
        {
            return (it->value.flags & ramses_internal::Animation::EAnimationFlags_Looping) != 0;
        }

        return false;
    }

    timeMilliseconds_t AnimationSequenceImpl::getAnimationLoopDuration(const Animation& animation) const
    {
        SequenceItemMap::ConstIterator it = m_animations.find(animation.impl.getAnimationHandle());
        if (it != m_animations.end())
        {
            return it->value.loopDuration;
        }

        return 0;
    }

    status_t AnimationSequenceImpl::setAnimationRelative(const Animation& animation)
    {
        SequenceItemMap::Iterator it = m_animations.find(animation.impl.getAnimationHandle());
        if (it != m_animations.end())
        {
            if ((it->value.flags & ramses_internal::Animation::EAnimationFlags_Relative) == 0u)
            {
                it->value.flags |= ramses_internal::Animation::EAnimationFlags_Relative;
            }

            return flushSequenceData();
        }

        return addErrorEntry("AnimationSequence::setAnimationRelative failed, animation is not included in sequence");
    }

    status_t AnimationSequenceImpl::setAnimationAbsolute(const Animation& animation)
    {
        SequenceItemMap::Iterator it = m_animations.find(animation.impl.getAnimationHandle());
        if (it != m_animations.end())
        {
            if ((it->value.flags & ramses_internal::Animation::EAnimationFlags_Relative) != 0)
            {
                it->value.flags &= ~ramses_internal::Animation::EAnimationFlags_Relative;
            }

            return flushSequenceData();
        }

        return addErrorEntry("AnimationSequence::setAnimationAbsolute failed, animation is not included in sequence");
    }

    bool AnimationSequenceImpl::isAnimationRelative(const Animation& animation) const
    {
        SequenceItemMap::ConstIterator it = m_animations.find(animation.impl.getAnimationHandle());
        if (it != m_animations.end())
        {
            return (it->value.flags & ramses_internal::Animation::EAnimationFlags_Relative) != 0;
        }

        return false;
    }

    uint32_t AnimationSequenceImpl::getNumAnimations() const
    {
        return static_cast<uint32_t>(m_animations.count());
    }

    bool AnimationSequenceImpl::containsAnimation(const Animation& animation) const
    {
        return m_animations.contains(animation.impl.getAnimationHandle());
    }

    sequenceTimeStamp_t AnimationSequenceImpl::getAnimationStartTimeInSequence(const Animation& animation) const
    {
        SequenceItemMap::ConstIterator it = m_animations.find(animation.impl.getAnimationHandle());
        if (it != m_animations.end())
        {
            return it->value.startTime;
        }

        return InvalidSequenceTimeStamp;
    }

    sequenceTimeStamp_t AnimationSequenceImpl::getAnimationStopTimeInSequence(const Animation& animation) const
    {
        SequenceItemMap::ConstIterator it = m_animations.find(animation.impl.getAnimationHandle());
        if (it != m_animations.end())
        {
            return it->value.stopTime;
        }

        return InvalidSequenceTimeStamp;
    }

    bool AnimationSequenceImpl::isSequenceActive() const
    {
        if (!m_startTime.isValid())
        {
            return false;
        }

        const ramses_internal::AnimationTime& currentTime = getIAnimationSystem().getTime();
        for (const auto& anim : m_animations)
        {
            // if any animation is active, sequence is active
            const ramses_internal::Animation& animation = getIAnimationSystem().getAnimation(anim.key);
            if (currentTime.getTimeStamp() >= animation.m_startTime.getTimeStamp() &&
                currentTime.getTimeStamp() < animation.m_stopTime.getTimeStamp())
            {
                return true;
            }
        }

        return false;
    }

    const AnimationImpl* AnimationSequenceImpl::findAnimation(ramses_internal::AnimationHandle handle) const
    {
        RamsesObjectRegistryIterator iter(getAnimationSystemImpl().getObjectRegistry(), ERamsesObjectType_Animation);
        while (const Animation* anim = iter.getNext<Animation>())
        {
            if (anim->impl.getAnimationHandle() == handle)
            {
                return &anim->impl;
            }
        }

        return NULL;
    }

    sequenceTimeStamp_t AnimationSequenceImpl::getAnimationSequenceStopTime() const
    {
        sequenceTimeStamp_t sequenceTime = 0;
        for (const auto& item : m_animations)
        {
            const sequenceTimeStamp_t animationTime = item.value.stopTime;
            if (animationTime > sequenceTime)
            {
                sequenceTime = animationTime;
            }
        }

        return sequenceTime;
    }
}
