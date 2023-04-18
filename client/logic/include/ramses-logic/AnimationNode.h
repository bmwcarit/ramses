//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/LogicNode.h"
#include "ramses-logic/AnimationTypes.h"
#include "ramses-logic/EPropertyType.h"
#include <string>
#include <memory>

namespace rlogic::internal
{
    class AnimationNodeImpl;
}

namespace rlogic
{
    /**
    * Animation node can be used to animate properties in logic network.
    * Animation node itself is a logic node and has a set of input and output properties:
    * - Fixed inputs:
    *     - progress (float)  - point within [0;1] normalized range of where to jump to in the animation
    *                         - in this range 0 marks the time 0 (regardless of timestamp of the first keyframe),
    *                           and 1 marks the end of the animation determined by the duration of the animation
    *                         - values outside of the [0;1] range are accepted and will be clamped
    * - Fixed outputs:
    *     - duration (float)  - total duration of the animation, determined by the highest timestamp from all its channels
    *                         - is affected by modifications to timestamp data via properties
    *                           (see #rlogic::AnimationNodeConfig::setExposingOfChannelDataAsProperties)
    *
    * - Channel outputs: Each animation channel provided at creation time (#rlogic::LogicEngine::createAnimationNode)
    *                    will be represented as output property with name of the channel (#rlogic::AnimationChannel::name)
    *                    and a value of type matching element in #rlogic::AnimationChannel::keyframes.
    *                    If the data type of keyframes is #rlogic::EPropertyType::Array (i.e. each keyframe is represented
    *                    by an array of floats), the output property is also of array type and contains a corresponding
    *                    number of children properties of type #rlogic::EPropertyType::Float.
    *                    Channel value output is a result of keyframes interpolation based on the 'progress' input above,
    *                    it can be linked to another logic node input to use the animation result.
    *
    * - Channel data inputs (only if created with #rlogic::AnimationNodeConfig::setExposingOfChannelDataAsProperties enabled):
    *     - channelsData (struct) - contains all channels and their data in a hierarchy. For each channel:
    *         - [channelName] (struct)
    *             - timestamps (array of float) - each element represents a timestamp value
    *             - keyframes (array of T) - each element represents a keyframe value
    *                                      - type T is data type matching this channel original keyframes
    *
    * During update when 'progress' input is set the following logic is executed:
    *     - calculate local animation time based on progress
    *     - for each channel:
    *         - lookup closest previous and next timestamp/keyframe pair according to the local animation time,
    *         - interpolate between them according to the interpolation type of that channel,
    *         - and finally set this value to the channel's output property.
    *
    * Note that all channel outputs will always have a value determined by corresponding keyframes, this includes
    * also when the time falls outside of the first/last animation timestamps:
    *     - channel output value equals first keyframe for any time at or before the first keyframe timestamp
    *     - channel output value equals last keyframe for any time at or after the last keyframe timestamp
    * This can be useful for example when needing to initialize the outputs before playing the animation yet,
    * when updating the animation node with progress 0, the logic will execute and update outputs to their
    * first keyframes.
    */
    class AnimationNode : public LogicNode
    {
    public:
        /**
        * Constructor of AnimationNode. User is not supposed to call this - AnimationNodes are created by other factory classes
        *
        * @param impl implementation details of the AnimationNode
        */
        explicit AnimationNode(std::unique_ptr<internal::AnimationNodeImpl> impl) noexcept;

        /**
        * Destructor of AnimationNode.
        */
        ~AnimationNode() noexcept override;

        /**
        * Returns channel data used in this animation (as provided at creation time #rlogic::LogicEngine::createAnimationNode).
        *
        * Note that the retrieved data is not affected by any modifications via channel data input properties
        * (see #rlogic::AnimationNodeConfig::setExposingOfChannelDataAsProperties). If modifications were made, only corresponding
        * properties hold the actual values used during animation.
        *
        * @return animation channels used in this animation.
        */
        [[nodiscard]] RAMSES_API const AnimationChannels& getChannels() const;

        /**
        * Copy Constructor of AnimationNode is deleted because AnimationNodes are not supposed to be copied
        *
        * @param other AnimationNodes to copy from
        */
        AnimationNode(const AnimationNode& other) = delete;

        /**
        * Move Constructor of AnimationNode is deleted because AnimationNodes are not supposed to be moved
        *
        * @param other AnimationNodes to move from
        */
        AnimationNode(AnimationNode&& other) = delete;

        /**
        * Assignment operator of AnimationNode is deleted because AnimationNodes are not supposed to be copied
        *
        * @param other AnimationNodes to assign from
        */
        AnimationNode& operator=(const AnimationNode& other) = delete;

        /**
        * Move assignment operator of AnimationNode is deleted because AnimationNodes are not supposed to be moved
        *
        * @param other AnimationNodes to assign from
        */
        AnimationNode& operator=(AnimationNode&& other) = delete;

        /**
        * Implementation of AnimationNode
        */
        internal::AnimationNodeImpl& m_animationNodeImpl;
    };
}
