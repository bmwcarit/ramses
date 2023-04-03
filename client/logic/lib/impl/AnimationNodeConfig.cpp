//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/AnimationNodeConfig.h"
#include "impl/AnimationNodeConfigImpl.h"

namespace rlogic
{
    AnimationNodeConfig::AnimationNodeConfig() noexcept
        : m_impl(std::make_unique<internal::AnimationNodeConfigImpl>())
    {
    }

    AnimationNodeConfig::~AnimationNodeConfig() noexcept = default;

    AnimationNodeConfig& AnimationNodeConfig::operator=(const AnimationNodeConfig& other)
    {
        m_impl = std::make_unique<internal::AnimationNodeConfigImpl>(*other.m_impl);
        return *this;
    }

    AnimationNodeConfig::AnimationNodeConfig(const AnimationNodeConfig& other)
    {
        *this = other;
    }

    AnimationNodeConfig::AnimationNodeConfig(AnimationNodeConfig&&) noexcept = default;
    AnimationNodeConfig& AnimationNodeConfig::operator=(AnimationNodeConfig&&) noexcept = default;

    bool AnimationNodeConfig::addChannel(const AnimationChannel& channelData)
    {
        return m_impl->addChannel(channelData);
    }

    const AnimationChannels& AnimationNodeConfig::getChannels() const
    {
        return m_impl->getChannels();
    }

    bool AnimationNodeConfig::setExposingOfChannelDataAsProperties(bool enabled)
    {
        return m_impl->setExposingOfChannelDataAsProperties(enabled);
    }

    bool AnimationNodeConfig::getExposingOfChannelDataAsProperties() const
    {
        return m_impl->getExposingOfChannelDataAsProperties();
    }
}
