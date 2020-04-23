//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONSEQUENCEIMPL_H
#define RAMSES_ANIMATIONSEQUENCEIMPL_H

// API
#include "ramses-client-api/AnimationTypes.h"

// internal
#include "AnimationObjectImpl.h"

// RAMSES framework
#include "AnimationAPI/IAnimationSystem.h"
#include "Collections/HashMap.h"

namespace ramses
{
    class Animation;
    class SceneImpl;

    struct SequenceItem
    {
        sequenceTimeStamp_t              startTime;
        sequenceTimeStamp_t              stopTime;
        timeMilliseconds_t               loopDuration;
        ramses_internal::UInt32          flags;
    };

    class AnimationSequenceImpl final : public AnimationObjectImpl
    {
    public:
        explicit AnimationSequenceImpl(AnimationSystemImpl& animationSystem, const char* name);
        virtual ~AnimationSequenceImpl();

        virtual void     deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        virtual status_t validate(uint32_t indent, StatusObjectSet& visitedObjects) const override;

        status_t addAnimation(const Animation& animation, sequenceTimeStamp_t startTimeInSequence, sequenceTimeStamp_t stopTimeInSequence);
        status_t removeAnimation(const Animation& animation);
        status_t start(timeMilliseconds_t offset);
        status_t startAt(globalTimeStamp_t timeStamp);
        status_t startReverse(timeMilliseconds_t offset);
        status_t startReverseAt(globalTimeStamp_t timeStamp);
        status_t stop(timeMilliseconds_t delay);
        status_t stopAt(globalTimeStamp_t timeStamp);
        status_t setPlaybackSpeed(float playbackSpeed);
        float getPlaybackSpeed() const;

        status_t setAnimationRelative(const Animation& animation);
        status_t setAnimationAbsolute(const Animation& animation);
        bool isAnimationRelative(const Animation& animation) const;

        status_t setAnimationLooping(const Animation& animation, timeMilliseconds_t loopDuration);
        bool isAnimationLooping(const Animation& animation) const;
        timeMilliseconds_t getAnimationLoopDuration(const Animation& animation) const;

        uint32_t getNumAnimations() const;
        bool   containsAnimation(const Animation& animation) const;
        sequenceTimeStamp_t getAnimationStartTimeInSequence(const Animation& animation) const;
        sequenceTimeStamp_t getAnimationStopTimeInSequence(const Animation& animation) const;
        sequenceTimeStamp_t getAnimationSequenceStopTime() const;

    private:
        status_t startAt(globalTimeStamp_t timeStamp, bool reverse);
        status_t flushSequenceData(bool forceFlush = false);
        void setAnimationData(ramses_internal::AnimationHandle handle, const SequenceItem& item);
        sequenceTimeStamp_t getAnimationStopTime(
            ramses_internal::AnimationHandle handle,
            sequenceTimeStamp_t startTime,
            float playbackSpeed);
        bool isSequenceActive() const;
        const AnimationImpl* findAnimation(ramses_internal::AnimationHandle handle) const;

        typedef ramses_internal::HashMap<ramses_internal::AnimationHandle, SequenceItem> SequenceItemMap;

        SequenceItemMap                          m_animations;
        float                                    m_playbackSpeed;
        ramses_internal::AnimationTime           m_startTime;
    };
}

#endif
