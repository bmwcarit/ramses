//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-framework-api/APIExport.h"
#include "ramses-logic/AnimationTypes.h"
#include <memory>

namespace ramses::internal
{
    class AnimationNodeConfigImpl;
}

namespace ramses
{
    /**
    * Holds data and settings for #ramses::AnimationNode creation using #ramses::LogicEngine::createAnimationNode.
    */
    class AnimationNodeConfig
    {
    public:
        RAMSES_API AnimationNodeConfig() noexcept;

        /**
        * Adds a data to animate. #ramses::AnimationNode can use multiple animation channels to animate,
        * see #ramses::AnimationNode and #ramses::AnimationChannel for details.
        * This call can fail, see #ramses::AnimationChannel for rules that channel data must follow.
        *
        * @param channelData channel data to add to configuration
        * @return \c true if the channel was added successfully, \c false otherwise.
        *         In case of an error, check the logs.
        */
        RAMSES_API bool addChannel(const AnimationChannel& channelData);

        /**
        * Returns all channels data added to this #AnimationNodeConfig so far.
        *
        * @return animation channels in this #AnimationNodeConfig.
        */
        [[nodiscard]] RAMSES_API const AnimationChannels& getChannels() const;

        /**
        * If enabled, the created #ramses::AnimationNode will expose its basic channel data (timestamps and keyframes) in form of logic node properties.
        * These properties can be set and linked allowing for control over actual animation data to be animated. The data can be adjusted at any point in time,
        * also during playing the animation.
        *
        * This functionality comes at a price - there is performance overhead and strict limit of how many keyframes the animation can have.
        * All timestamp/keyframe values are represented as child properties within an array, per channel, refer to #ramses::AnimationNode
        * for the exact structure of these inputs. Processing the animation data in this form is not cheap compared to a regular animation node
        * and there is a limit of 255 timestamps/keyframes any channel can have. Always try to minimize the amount of animation data when using
        * this type of animation node.
        *
        * Note that all modifications to the animation data are considered temporary and will NOT be preserved when saving to a file.
        * All values and corresponding properties will be reset to original values (provided in this #AnimationNodeConfig from when the #ramses::AnimationNode
        * was created) when loading from a file.
        *
        * By default this feature is disabled.
        * This feature cannot be enabled (this call will fail) if the #AnimationNodeConfig contains a channel with #ramses::DataArray using data type #ramses::EPropertyType::Array.
        *
        * @param enabled flag to enable or disable exposing of channels data as properties.
        * @return \c true if enabled successfully, \c false otherwise.
        *         In case of an error, check the logs.
        */
        RAMSES_API bool setExposingOfChannelDataAsProperties(bool enabled);

        /**
        * Returns the currently set state of the exposing of channel data as properties.
        *
        * @return the currently set state of the exposing of channel data as properties.
        */
        [[nodiscard]] RAMSES_API bool getExposingOfChannelDataAsProperties() const;

        /**
        * Destructor of #AnimationNodeConfig
        */
        RAMSES_API ~AnimationNodeConfig() noexcept;

        /**
        * Copy Constructor of #AnimationNodeConfig
        * @param other the other #AnimationNodeConfig to copy from
        */
        RAMSES_API AnimationNodeConfig(const AnimationNodeConfig& other);

        /**
        * Move Constructor of #AnimationNodeConfig
        * @param other the other #AnimationNodeConfig to move from
        */
        RAMSES_API AnimationNodeConfig(AnimationNodeConfig&& other) noexcept;

        /**
        * Assignment operator of #AnimationNodeConfig
        * @param other the other #AnimationNodeConfig to copy from
        * @return self
        */
        RAMSES_API AnimationNodeConfig& operator=(const AnimationNodeConfig& other);

        /**
        * Move assignment operator of #AnimationNodeConfig
        * @param other the other #AnimationNodeConfig to move from
        * @return self
        */
        RAMSES_API AnimationNodeConfig& operator=(AnimationNodeConfig&& other) noexcept;

        /**
        * Implementation detail of #AnimationNodeConfig
        */
        std::unique_ptr<internal::AnimationNodeConfigImpl> m_impl;
    };
}
