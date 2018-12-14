//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONSEQUENCE_H
#define RAMSES_ANIMATIONSEQUENCE_H

#include "ramses-client-api/AnimationObject.h"
#include "ramses-client-api/AnimationTypes.h"

namespace ramses
{
    class Animation;

    /**
    * @brief The AnimationSequence is a container for multiple animations.
    * AnimationSequence has its own virtual time line where all its animations
    * are put onto with given offsets. The sequence of animations can then be
    * started/stopped altogether.
    */
    class RAMSES_API AnimationSequence : public AnimationObject
    {
    public:
        /**
        * @brief Add animation to the sequence.
        * Animation will be placed on to sequence time line at given time stamp if provided.
        * The animation must not be added to multiple sequences, otherwise this will result in an undefined behavior
        *
        * @param animation Animation to be added to sequence. If animation already exists in sequence, only its start/stop time is updated.
        * @param startTimeInSequence Time stamp for animation to start within sequence time line. By default animation is added to the beginning of sequence.
        * @param stopTimeInSequence Time stamp for animation to stop within sequence time line. By default animation stops when its last spline key is reached.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t addAnimation(const Animation& animation, sequenceTimeStamp_t startTimeInSequence = 0u, sequenceTimeStamp_t stopTimeInSequence = 0u);

        /**
        * @brief Remove animation from sequence.
        *
        * @param animation Animation to be removed from sequence.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t removeAnimation(const Animation& animation);

        /**
        * @brief Starts the sequence of animations.
        * Offset can be used to postpone start if positive or start sequence
        * from a certain point if negative.
        *
        * @param offset Offset start in milliseconds.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t start(timeMilliseconds_t offset = 0);

        /**
        * @brief Starts the sequence of animations at given time.
        *
        * @param timeStamp Global time stamp for sequence to start.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t startAt(globalTimeStamp_t timeStamp);

        /**
        * @brief Starts the sequence of animations in reverse.
        * Offset can be used to postpone start if positive or start sequence
        * from a certain point if negative.
        *
        * @param offset Offset start in milliseconds.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t startReverse(timeMilliseconds_t offset = 0);

        /**
        * @brief Starts the sequence of animations at given time in reverse.
        *
        * @param timeStamp Global time stamp for sequence to start in reverse.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t startReverseAt(globalTimeStamp_t timeStamp);

        /**
        * @brief Stops the sequence of animations.
        * Delay can be used to postpone the stop.
        *
        * @param delay Delay stop in milliseconds.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t stop(timeMilliseconds_t delay = 0);

        /**
        * @brief Stops the sequence of animations at given time.
        *
        * @param timeStamp Global time stamp for sequence to stop.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t stopAt(globalTimeStamp_t timeStamp);

        /**
        * @brief Sets sequence playback speed affecting all animations within the sequence.
        * Default sequence playback speed is 1, higher than 1 means faster (2 is double speed),
        * lower than 1 means slower (0.5 is half speed).
        * Zero and negative values are not allowed.
        *
        * @param playbackSpeed Playback speed multiplier.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setPlaybackSpeed(float playbackSpeed);

        /**
        * @brief Gets sequence current playback speed.
        * See setPlaybackSpeed() for meanings of the values.
        *
        * @return Playback speed.
        */
        float getPlaybackSpeed() const;

        /**
        * @brief Sets animation relative.
        * By default animation is absolute, ie. data values from spline keys are directly
        * assigned to animated property.
        * Relative animation takes value of the animated property at the point when animation starts
        * and adds spline key value to it.
        *
        * Note: Changing from absolute to relative animation during playback will cause undefined results.
        *
        * @param animation Animation in sequence for which relative is to be set
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setAnimationRelative(const Animation& animation);

        /**
        * @brief Sets animation absolute.
        * By default animation is absolute, ie. data values from spline keys are directly
        * assigned to animated property.
        *
        * Note: Changing from relative to absolute animation during playback will cause undefined results.
        *
        * @param animation Animation in sequence for which absolute is to be set
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setAnimationAbsolute(const Animation& animation);

        /**
        * @brief Enables animation looping.
        * When animation reaches last spline key (or loopDuration passes) it begins playback from start.
        * Looping animation can only be stopped by calling stop().
        *
        * @param animation Animation in sequence for which looping is to be set
        * @param loopDuration Duration of one loop iteration. If 0 one loop iteration equals duration
        * of whole animation (determined from last spline key).
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setAnimationLooping(const Animation& animation, timeMilliseconds_t loopDuration = 0);

        /**
        * @brief Returns true if animation is set to relative.
        * See setAnimationRelative().
        *
        * @param animation Animation in sequence for which relative state has to be returned
        * @return True if animation is relative, false otherwise.
        */
        bool isAnimationRelative(const Animation& animation) const;

        /**
        * @brief Returns true if animation is set to loop.
        *
        * @param animation Animation in sequence for which looping state has to be returned
        * @return True if animation is set to loop, false otherwise.
        */
        bool isAnimationLooping(const Animation& animation) const;

        /**
        * @brief Gets loop duration for animation.
        * See setAnimationLooping() for meaning of the value.
        *
        * @param animation Animation in sequence for which looping duration has to be returned
        * @return Loop duration of animation.
        */
        timeMilliseconds_t getAnimationLoopDuration(const Animation& animation) const;

        /**
        * @brief Returns number of animations within the sequence.
        *
        * @return Number of animations.
        */
        uint32_t getNumberOfAnimations() const;

        /**
        * @brief Checks if given animation is in the AnimationSequence.
        *
        * @param animation Animation to check.
        * @return True if animation is contained by the AnimationSequence, false otherwise.
        */
        bool containsAnimation(const Animation& animation) const;

        /**
        * @brief Gives animation start time within the AnimationSequence.
        *
        * @param animation Animation to check.
        * @return Animation start time stamp in the AnimationSequence time line.
        *         InvalidSequenceTimeStamp if animation is not in this AnimationSequence.
        */
        sequenceTimeStamp_t getAnimationStartTimeInSequence(const Animation& animation) const;

        /**
        * @brief Gives animation stop time within the AnimationSequence.
        *
        * @param animation Animation to check.
        * @return Animation stop time stamp in the AnimationSequence time line.
        *         InvalidSequenceTimeStamp if animation is not in this AnimationSequence.
        */
        sequenceTimeStamp_t getAnimationStopTimeInSequence(const Animation& animation) const;

        /**
        * @brief Gets the maximum timestamp of all animations in this sequence
        *
        * @return max time stamp
        */
        sequenceTimeStamp_t getAnimationSequenceStopTime() const;

        /**
        * @brief Stores internal data for implementation specifics of AnimationSequence.
        */
        class AnimationSequenceImpl& impl;

    protected:
        /**
        * @brief AnimationSystemData is the factory for creating AnimationSequence.
        */
        friend class AnimationSystemData;

        /**
        * @brief Constructor of the AnimationSequence.
        *
        * @param[in] pimpl Internal data for implementation specifics of AnimationSequence (sink - instance becomes owner)
        */
        explicit AnimationSequence(AnimationSequenceImpl& pimpl);

        /**
        * @brief Copy constructor of AnimationSequence.
        *
        * @param[in] other Other instance of AnimationSequence class
        */
        AnimationSequence(const AnimationSequence& other);

        /**
        * @brief Assignment operator of AnimationSequence.
        *
        * @param[in] other Other instance of AnimationSequence class
        * @return This instance after assignment
        */
        AnimationSequence& operator=(const AnimationSequence& other);

        /**
        * @brief Destructor of the AnimationSequence.
        */
        virtual ~AnimationSequence();

    };
}

#endif
