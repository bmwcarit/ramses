//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/AnimationNodeConfigImpl.h"

#include "ramses-logic/EPropertyType.h"
#include "ramses-logic/DataArray.h"
#include "impl/LoggerImpl.h"

namespace rlogic::internal
{
    bool AnimationNodeConfigImpl::addChannel(const AnimationChannel& channelData)
    {
        if (!channelData.timeStamps || !channelData.keyframes)
        {
            LOG_ERROR("AnimationNodeConfig::addChannel: Cannot add channelData data '{}', missing timestamps and/or keyframes.", channelData.name);
            return false;
        }

        if (!CanPropertyTypeBeAnimated(channelData.keyframes->getDataType()))
        {
            LOG_ERROR("AnimationNodeConfig::addChannel: Cannot add channelData data '{}', keyframes data type cannot be animated.", channelData.name);
            return false;
        }

        if (channelData.timeStamps->getDataType() != EPropertyType::Float)
        {
            LOG_ERROR("AnimationNodeConfig::addChannel: Cannot add channelData data '{}', timestamps must be of type Float.", channelData.name);
            return false;
        }

        if (channelData.timeStamps->getNumElements() != channelData.keyframes->getNumElements())
        {
            LOG_ERROR("AnimationNodeConfig::addChannel: Cannot add channelData data '{}', number of keyframes must be same as number of timestamps.", channelData.name);
            return false;
        }

        const auto& timestamps = *channelData.timeStamps->getData<float>();
        if (std::adjacent_find(timestamps.cbegin(), timestamps.cend(), std::greater_equal<float>()) != timestamps.cend())
        {
            LOG_ERROR("AnimationNodeConfig::addChannel: Cannot add channelData data '{}', timestamps have to be strictly in ascending order.", channelData.name);
            return false;
        }

        if (channelData.keyframes->getDataType() == EPropertyType::Array)
        {
            const size_t elementArraySize = channelData.keyframes->getData<std::vector<float>>()->front().size();
            if (elementArraySize > MaxArrayPropertySize)
            {
                LOG_ERROR("AnimationNodeConfig::addChannel: Cannot add channelData data '{}',"
                    " when using elements of data type float array, the float array size ({}) cannot exceed {}.",
                    channelData.name, elementArraySize, MaxArrayPropertySize);
                return false;
            }
        }

        if ((channelData.interpolationType == EInterpolationType::Linear_Quaternions || channelData.interpolationType == EInterpolationType::Cubic_Quaternions) &&
            channelData.keyframes->getDataType() != EPropertyType::Vec4f)
        {
            LOG_ERROR("AnimationNodeConfig::addChannel: Cannot add channelData data '{}', quaternion animation requires the keyframes to be of type vec4f.", channelData.name);
            return false;
        }

        if (channelData.interpolationType == EInterpolationType::Cubic || channelData.interpolationType == EInterpolationType::Cubic_Quaternions)
        {
            if (!channelData.tangentsIn || !channelData.tangentsOut)
            {
                LOG_ERROR("AnimationNodeConfig::addChannel: Cannot add channelData data '{}', cubic interpolation requires tangents to be provided.", channelData.name);
                return false;
            }
            if (channelData.tangentsIn->getDataType() != channelData.keyframes->getDataType() ||
                channelData.tangentsOut->getDataType() != channelData.keyframes->getDataType())
            {
                LOG_ERROR("AnimationNodeConfig::addChannel: Cannot add channelData data '{}', tangents must be of same data type as keyframes.", channelData.name);
                return false;
            }
            if (channelData.tangentsIn->getNumElements() != channelData.keyframes->getNumElements() ||
                channelData.tangentsOut->getNumElements() != channelData.keyframes->getNumElements())
            {
                LOG_ERROR("AnimationNodeConfig::addChannel: Cannot add channelData data '{}', number of tangents in/out must be same as number of keyframes.", channelData.name);
                return false;
            }
            if (channelData.keyframes->getDataType() == EPropertyType::Array)
            {
                const size_t elementArraySize = channelData.keyframes->getData<std::vector<float>>()->front().size();
                if (channelData.tangentsIn->getData<std::vector<float>>()->front().size() != elementArraySize ||
                    channelData.tangentsOut->getData<std::vector<float>>()->front().size() != elementArraySize)
                {
                    LOG_ERROR("AnimationNodeConfig::addChannel: Cannot add channelData data '{}', tangents must have same array element size as keyframes.", channelData.name);
                    return false;
                }
            }
        }
        else if (channelData.tangentsIn || channelData.tangentsOut)
        {
            LOG_ERROR("AnimationNodeConfig::addChannel: Cannot add channelData data '{}', tangents were provided for other than cubic interpolation type.", channelData.name);
            return false;
        }

        if (m_exposeChannelDataAsProperties)
        {
            if (channelData.keyframes->getNumElements() > MaxArrayPropertySize)
            {
                LOG_ERROR("AnimationNodeConfig::addChannel: Cannot add channelData data '{}', number of keyframes ({}) cannot exceed {} when animation data exposed as properties.",
                    channelData.name, channelData.keyframes->getNumElements(), MaxArrayPropertySize);
                return false;
            }

            if (channelData.keyframes->getDataType() == EPropertyType::Array)
            {
                LOG_ERROR("AnimationNodeConfig::addChannel: Cannot add channelData data '{}', elements of data type float arrays cannot be exposed as properties.", channelData.name);
                return false;
            }
        }

        m_channels.push_back(channelData);
        return true;
    }

    const AnimationChannels& AnimationNodeConfigImpl::getChannels() const
    {
        return m_channels;
    }

    bool AnimationNodeConfigImpl::setExposingOfChannelDataAsProperties(bool enabled)
    {
        if (!enabled)
        {
            m_exposeChannelDataAsProperties = false;
            return true;
        }

        for (const auto& channelData : m_channels)
        {
            if (channelData.keyframes->getNumElements() > MaxArrayPropertySize)
            {
                LOG_ERROR("AnimationNodeConfig::setExposingOfChannelDataAsProperties: Cannot enable channel data properties for channel '{}',"
                    " number of keyframes ({}) cannot exceed {} when animation data exposed as properties.",
                    channelData.name, channelData.keyframes->getNumElements(), MaxArrayPropertySize);
                return false;
            }

            if (channelData.keyframes->getDataType() == EPropertyType::Array)
            {
                LOG_ERROR("AnimationNodeConfig::setExposingOfChannelDataAsProperties: Cannot enable channel data properties for channel '{}',"
                    " elements of data type float arrays cannot be exposed as properties.", channelData.name);
                return false;
            }
        }

        m_exposeChannelDataAsProperties = true;
        return true;
    }

    bool AnimationNodeConfigImpl::getExposingOfChannelDataAsProperties() const
    {
        return m_exposeChannelDataAsProperties;
    }
}
