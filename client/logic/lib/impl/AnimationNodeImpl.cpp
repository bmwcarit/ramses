//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/AnimationNodeImpl.h"
#include "impl/PropertyImpl.h"
#include "impl/DataArrayImpl.h"
#include "ramses-logic/EPropertyType.h"
#include "ramses-logic/Property.h"
#include "ramses-logic/DataArray.h"
#include "internals/EPropertySemantics.h"
#include "internals/ErrorReporting.h"
#include "generated/AnimationNodeGen.h"
#include "fmt/format.h"
#include <cmath>

namespace rlogic::internal
{
    AnimationNodeImpl::AnimationNodeImpl(AnimationChannels channels, bool exposeDataAsProperties, std::string_view name, uint64_t id) noexcept
        : LogicNodeImpl(name, id)
        , m_channels{ std::move(channels) }
        , m_hasChannelDataExposedViaProperties{ exposeDataAsProperties }
    {
        m_channelsWorkData.resize(m_channels.size());
        for (size_t i = 0u; i < m_channels.size(); ++i)
        {
            const auto& channel = m_channels[i];
            assert(channel.timeStamps && channel.keyframes);
            assert(channel.timeStamps->getNumElements() == channel.keyframes->getNumElements());
            assert(!channel.tangentsIn || channel.timeStamps->getNumElements() == channel.tangentsIn->getNumElements());
            assert(!channel.tangentsOut || channel.timeStamps->getNumElements() == channel.tangentsOut->getNumElements());
            assert(!exposeDataAsProperties || channel.keyframes->getDataType() != EPropertyType::Array);

            // extract basic channel data to work containers, update logic operates with these containers instead of original channel data
            // (for timestamps and keyframes at least), it also makes it possible to modify this data in runtime while keeping original data constant
            assert(m_channels[i].timeStamps->getDataType() == EPropertyType::Float && m_channels[i].timeStamps->getNumElements() > 0);
            m_channelsWorkData[i].timestamps = *m_channels[i].timeStamps->getData<float>();
            m_channelsWorkData[i].keyframes = m_channels[i].keyframes->m_impl.getDataVariant();

            // overall duration equals longest channel in animation
            m_maxChannelDuration = std::max(m_maxChannelDuration, channel.timeStamps->getData<float>()->back());
        }
    }

    void AnimationNodeImpl::createRootProperties()
    {
        HierarchicalTypeData inputs = MakeStruct("", {
            {"progress", EPropertyType::Float},   // EInputIdx_Progress
            });
        if (m_hasChannelDataExposedViaProperties)
        {
            std::vector<HierarchicalTypeData> channelsData;
            for (const auto& channel : m_channels)
            {
                assert(channel.timeStamps && channel.keyframes);
                assert(channel.timeStamps->getNumElements() == channel.keyframes->getNumElements());
                assert(channel.timeStamps->getNumElements() <= MaxArrayPropertySize);
                const std::initializer_list<HierarchicalTypeData> channelDataArrays =
                {
                    MakeArray("timestamps", channel.timeStamps->getNumElements(), EPropertyType::Float),
                    MakeArray("keyframes", channel.keyframes->getNumElements(), channel.keyframes->getDataType())
                };

                channelsData.push_back(HierarchicalTypeData({ std::string{ channel.name }, EPropertyType::Struct }, channelDataArrays));
            }
            inputs.children.push_back(HierarchicalTypeData({ "channelsData", EPropertyType::Struct }, channelsData)); // EInputIdx_ChannelsData
        }
        auto inputsImpl = std::make_unique<PropertyImpl>(std::move(inputs), EPropertySemantics::AnimationInput);

        HierarchicalTypeData outputs = MakeStruct("", {
            {"duration", EPropertyType::Float},   // EOutputIdx_Duration
            });
        for (const auto& channel : m_channels)
        {
            if (channel.keyframes->getDataType() == EPropertyType::Array)
            {
                const size_t elementArraySize = channel.keyframes->getData<std::vector<float>>()->front().size();
                assert(elementArraySize < MaxArrayPropertySize);
                outputs.children.push_back(MakeArray(std::string{ channel.name }, elementArraySize, EPropertyType::Float));
            }
            else
            {
                outputs.children.push_back(MakeType(std::string{ channel.name }, channel.keyframes->getDataType()));
            }
        }
        auto outputsImpl = std::make_unique<PropertyImpl>(std::move(outputs), EPropertySemantics::AnimationOutput);

        setRootProperties(std::make_unique<Property>(std::move(inputsImpl)), std::make_unique<Property>(std::move(outputsImpl)));

        if (m_hasChannelDataExposedViaProperties)
            initAnimationDataPropertyValues();

        // initialize duration property, no need to set every update as it can change only if timestamps are modified
        getOutputs()->getChild(EOutputIdx_Duration)->set(m_maxChannelDuration);
    }

    float AnimationNodeImpl::getMaximumChannelDuration() const
    {
        return m_maxChannelDuration;
    }

    const AnimationChannels& AnimationNodeImpl::getChannels() const
    {
        return m_channels;
    }

    std::optional<LogicNodeRuntimeError> AnimationNodeImpl::update()
    {
        // propagate data from properties if this animation node has channel data properties
        if (m_hasChannelDataExposedViaProperties)
            updateAnimationDataFromProperties();

        const float progress = *getInputs()->getChild(EInputIdx_Progress)->get<float>();
        const float localAnimationTime = progress * m_maxChannelDuration;

        for (size_t i = 0u; i < m_channels.size(); ++i)
            updateChannel(i, localAnimationTime);

        return std::nullopt;
    }

    void AnimationNodeImpl::updateChannel(size_t channelIdx, float localAnimationTime)
    {
        const auto& channelWorkData = m_channelsWorkData[channelIdx];
        const auto& channel = m_channels[channelIdx];
        const auto& timeStamps = channelWorkData.timestamps;

        // find upper/lower timestamp neighbor of elapsed timestamp
        auto tsUpperIt = std::upper_bound(timeStamps.cbegin(), timeStamps.cend(), localAnimationTime);
        const auto tsLowerIt = (tsUpperIt == timeStamps.cbegin() ? timeStamps.cbegin() : tsUpperIt - 1);
        tsUpperIt = (tsUpperIt == timeStamps.cend() ? tsUpperIt - 1 : tsUpperIt);

        // get index into corresponding keyframes
        const auto lowerIdx = static_cast<size_t>(std::distance(timeStamps.cbegin(), tsLowerIt));
        const auto upperIdx = static_cast<size_t>(std::distance(timeStamps.cbegin(), tsUpperIt));
        assert(lowerIdx < channel.keyframes->getNumElements());
        assert(upperIdx < channel.keyframes->getNumElements());

        // calculate interpolation ratio between the elapsed time and timestamp neighbors [0.0, 1.0] (0.0=lower, 1.0=upper)
        float interpRatio = 0.f;
        float timeBetweenKeys = *tsUpperIt - *tsLowerIt;
        if (tsUpperIt != tsLowerIt)
            interpRatio = (localAnimationTime - *tsLowerIt) / timeBetweenKeys;
        // no clamping needed mathematically but to avoid float precision issues
        interpRatio = std::clamp(interpRatio, 0.f, 1.f);

        using DataVariant = std::variant<
            float,
            vec2f,
            vec3f,
            vec4f,
            int32_t,
            vec2i,
            vec3i,
            vec4i,
            std::vector<float>
        >;
        DataVariant interpolatedValue;
        std::visit([&](const auto& v) {
            switch (channel.interpolationType)
            {
            case EInterpolationType::Step:
                interpolatedValue = v[lowerIdx];
                break;
            case EInterpolationType::Linear:
            case EInterpolationType::Linear_Quaternions:
                interpolatedValue = interpolateKeyframes_linear(v[lowerIdx], v[upperIdx], interpRatio);
                break;
            case EInterpolationType::Cubic:
            case EInterpolationType::Cubic_Quaternions:
            {
                using ValueType = std::remove_const_t<std::remove_reference_t<decltype(v.front())>>;
                const auto& tIn = *channel.tangentsIn->getData<ValueType>();
                const auto& tOut = *channel.tangentsOut->getData<ValueType>();
                interpolatedValue = interpolateKeyframes_cubic(v[lowerIdx], v[upperIdx], tOut[lowerIdx], tIn[upperIdx], interpRatio, timeBetweenKeys);
                break;
            }
            }
        }, channelWorkData.keyframes);

        if (channel.interpolationType == EInterpolationType::Linear_Quaternions || channel.interpolationType == EInterpolationType::Cubic_Quaternions)
        {
            auto& asQuaternion = std::get<vec4f>(interpolatedValue);
            const float normalizationFactor = 1 / std::sqrt(
                asQuaternion[0] * asQuaternion[0] +
                asQuaternion[1] * asQuaternion[1] +
                asQuaternion[2] * asQuaternion[2] +
                asQuaternion[3] * asQuaternion[3]);

            asQuaternion[0] *= normalizationFactor;
            asQuaternion[1] *= normalizationFactor;
            asQuaternion[2] *= normalizationFactor;
            asQuaternion[3] *= normalizationFactor;
        }

        // 'progress' is at index 0, channel outputs are shifted by one
        auto outputValueProp = getOutputs()->getChild(channelIdx + EOutputIdx_ChannelsBegin);
        std::visit([outputValueProp](const auto& v) {
            using ValueType = std::remove_const_t<std::remove_reference_t<decltype(v)>>;
            if constexpr (std::is_same_v<ValueType, std::vector<float>>)
            {
                // array data type requires each array element to be set to individual output property
                for (size_t arrayIdx = 0u; arrayIdx < v.size(); ++arrayIdx)
                    outputValueProp->getChild(arrayIdx)->m_impl->setValue(v[arrayIdx]);
            }
            else
            {
                outputValueProp->m_impl->setValue(PropertyValue{ v });
            }
            }, interpolatedValue);
    }

    template <typename T>
    T AnimationNodeImpl::interpolateKeyframes_linear(T lowerVal, T upperVal, float interpRatio)
    {
        // this will be compiled for variant visitor but must not be executed
        if constexpr (std::is_same_v<T, bool>)
            assert(false);

        if constexpr (std::is_floating_point_v<T>)
        {
            return lowerVal + interpRatio * (upperVal - lowerVal);
        }
        else if constexpr (std::is_integral_v<T>)
        {
            return lowerVal + std::lround(interpRatio * static_cast<float>(upperVal - lowerVal));
        }
        else
        {
            T val = lowerVal;
            auto valIt = val.begin();
            auto lowerValIt = lowerVal.cbegin();
            auto upperValIt = upperVal.cbegin();

            // decompose vecXy and interpolate each component separately
            for (; valIt != val.cend(); ++valIt, ++lowerValIt, ++upperValIt)
                *valIt = interpolateKeyframes_linear(*lowerValIt, *upperValIt, interpRatio);

            return val;
        }
    }

    template <typename T>
    T AnimationNodeImpl::interpolateKeyframes_cubic(T lowerVal, T upperVal, T lowerTangentOut, T upperTangentIn, float interpRatio, float timeBetweenKeys)
    {
        // this will be compiled for variant visitor but must not be executed
        if constexpr (std::is_same_v<T, bool>)
            assert(false);

        if constexpr (std::is_floating_point_v<T>)
        {
            // GLTF v2 Appendix C (https://github.com/KhronosGroup/glTF/tree/master/specification/2.0?ts=4#appendix-c-spline-interpolation)
            const float t = interpRatio;
            const float t2 = t * t;
            const float t3 = t2 * t;
            const T p0 = lowerVal;
            const T p1 = upperVal;
            const T m0 = timeBetweenKeys * lowerTangentOut;
            const T m1 = timeBetweenKeys * upperTangentIn;
            return (2.f*t3 - 3.f*t2 + 1.f) * p0 + (t3 - 2.f*t2 + t) * m0 + (-2.f*t3 + 3.f*t2) * p1 + (t3 - t2) * m1;
        }
        else if constexpr (std::is_integral_v<T>)
        {
            return std::lround(interpolateKeyframes_cubic(
                static_cast<float>(lowerVal),
                static_cast<float>(upperVal),
                static_cast<float>(lowerTangentOut),
                static_cast<float>(upperTangentIn),
                interpRatio,
                timeBetweenKeys));
        }
        else
        {
            T val = lowerVal;
            auto valIt = val.begin();
            auto lowerValIt = lowerVal.cbegin();
            auto upperValIt = upperVal.cbegin();
            auto lowerTangentOutIt = lowerTangentOut.cbegin();
            auto upperTangentInIt = upperTangentIn.cbegin();
            assert(lowerVal.size() == lowerTangentOut.size());
            assert(lowerVal.size() == upperTangentIn.size());

            // decompose vecXy and interpolate each component separately
            for (; valIt != val.cend(); ++valIt, ++lowerValIt, ++upperValIt, ++lowerTangentOutIt, ++upperTangentInIt)
                *valIt = interpolateKeyframes_cubic(*lowerValIt, *upperValIt, *lowerTangentOutIt, *upperTangentInIt, interpRatio, timeBetweenKeys);

            return val;
        }
    }

    flatbuffers::Offset<rlogic_serialization::AnimationNode> AnimationNodeImpl::Serialize(
        const AnimationNodeImpl& animNode,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap)
    {
        std::vector<flatbuffers::Offset<rlogic_serialization::Channel>> channelsFB;
        channelsFB.reserve(animNode.m_channels.size());
        for (const auto& channel : animNode.m_channels)
        {
            rlogic_serialization::EInterpolationType interpTypeFB = rlogic_serialization::EInterpolationType::MAX;
            switch (channel.interpolationType)
            {
            case EInterpolationType::Step:
                interpTypeFB = rlogic_serialization::EInterpolationType::Step;
                break;
            case EInterpolationType::Linear:
                interpTypeFB = rlogic_serialization::EInterpolationType::Linear;
                break;
            case EInterpolationType::Cubic:
                interpTypeFB = rlogic_serialization::EInterpolationType::Cubic;
                break;
            case EInterpolationType::Linear_Quaternions:
                interpTypeFB = rlogic_serialization::EInterpolationType::Linear_Quaternions;
                break;
            case EInterpolationType::Cubic_Quaternions:
                interpTypeFB = rlogic_serialization::EInterpolationType::Cubic_Quaternions;
                break;
            }

            channelsFB.push_back(rlogic_serialization::CreateChannel(
                builder,
                builder.CreateString(channel.name),
                serializationMap.resolveDataArrayOffset(channel.timeStamps->getId()),
                serializationMap.resolveDataArrayOffset(channel.keyframes->getId()),
                interpTypeFB,
                channel.tangentsIn ? serializationMap.resolveDataArrayOffset(channel.tangentsIn->getId()) : 0,
                channel.tangentsOut ? serializationMap.resolveDataArrayOffset(channel.tangentsOut->getId()) : 0
                ));
        }

        const auto logicObject = LogicObjectImpl::Serialize(animNode, builder);
        const auto inputPropertyObject = PropertyImpl::Serialize(*animNode.getInputs()->m_impl, builder, serializationMap);
        const auto ouputPropertyObject = PropertyImpl::Serialize(*animNode.getOutputs()->m_impl, builder, serializationMap);
        return rlogic_serialization::CreateAnimationNode(
            builder,
            logicObject,
            builder.CreateVector(channelsFB),
            animNode.m_hasChannelDataExposedViaProperties,
            inputPropertyObject,
            ouputPropertyObject
        );
    }

    std::unique_ptr<AnimationNodeImpl> AnimationNodeImpl::Deserialize(
        const rlogic_serialization::AnimationNode& animNodeFB,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap)
    {
        std::string name;
        uint64_t id = 0u;
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(animNodeFB.base(), name, id, userIdHigh, userIdLow, errorReporting) || !animNodeFB.channels() || !animNodeFB.rootInput() || !animNodeFB.rootOutput())
        {
            errorReporting.add("Fatal error during loading of AnimationNode from serialized data: missing name, id, channels or in/out property data!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        AnimationChannels channels;
        channels.reserve(animNodeFB.channels()->size());
        for (const auto* channelFB : *animNodeFB.channels())
        {
            if (!channelFB->name() ||
                !channelFB->timestamps() ||
                !channelFB->keyframes())
            {
                errorReporting.add(fmt::format("Fatal error during loading of AnimationNode '{}' channel data: missing name, timestamps or keyframes!", name), nullptr, EErrorType::BinaryVersionMismatch);
                return nullptr;
            }

            AnimationChannel channel;
            channel.name = channelFB->name()->string_view();
            channel.timeStamps = &deserializationMap.resolveDataArray(*channelFB->timestamps());
            channel.keyframes = &deserializationMap.resolveDataArray(*channelFB->keyframes());

            switch (channelFB->interpolationType())
            {
            case rlogic_serialization::EInterpolationType::Step:
                channel.interpolationType = EInterpolationType::Step;
                break;
            case rlogic_serialization::EInterpolationType::Linear:
                channel.interpolationType = EInterpolationType::Linear;
                break;
            case rlogic_serialization::EInterpolationType::Cubic:
                channel.interpolationType = EInterpolationType::Cubic;
                break;
            case rlogic_serialization::EInterpolationType::Linear_Quaternions:
                channel.interpolationType = EInterpolationType::Linear_Quaternions;
                break;
            case rlogic_serialization::EInterpolationType::Cubic_Quaternions:
                channel.interpolationType = EInterpolationType::Cubic_Quaternions;
                break;
            default:
                errorReporting.add(fmt::format("Fatal error during loading of AnimationNode '{}' channel '{}' data: missing or invalid interpolation type!", name, channel.name), nullptr, EErrorType::BinaryVersionMismatch);
                return nullptr;
            }

            if (channel.interpolationType == EInterpolationType::Cubic || channel.interpolationType == EInterpolationType::Cubic_Quaternions)
            {
                if (!channelFB->tangentsIn() ||
                    !channelFB->tangentsOut())
                {
                    errorReporting.add(fmt::format("Fatal error during loading of AnimationNode '{}' channel '{}' data: missing tangents!", name, channel.name), nullptr, EErrorType::BinaryVersionMismatch);
                    return nullptr;
                }

                channel.tangentsIn = &deserializationMap.resolveDataArray(*channelFB->tangentsIn());
                channel.tangentsOut = &deserializationMap.resolveDataArray(*channelFB->tangentsOut());
            }

            channels.push_back(std::move(channel));
        }

        const bool hasChannelDataProperties = animNodeFB.channelsAsProperties();

        // deserialize and overwrite constructor generated properties
        auto rootInProperty = PropertyImpl::Deserialize(*animNodeFB.rootInput(), EPropertySemantics::AnimationInput, errorReporting, deserializationMap);
        auto rootOutProperty = PropertyImpl::Deserialize(*animNodeFB.rootOutput(), EPropertySemantics::AnimationOutput, errorReporting, deserializationMap);

        auto deserialized = std::make_unique<AnimationNodeImpl>(std::move(channels), hasChannelDataProperties, name, id);
        deserialized->setUserId(userIdHigh, userIdLow);

        if (!rootInProperty->getChild(EInputIdx_Progress) || rootInProperty->getChild(EInputIdx_Progress)->getName() != "progress" ||
            rootOutProperty->getChildCount() != deserialized->getChannels().size() + EOutputIdx_ChannelsBegin ||
            !rootOutProperty->getChild(EOutputIdx_Duration) || rootOutProperty->getChild(EOutputIdx_Duration)->getName() != "duration")
        {
            errorReporting.add(fmt::format("Fatal error during loading of AnimationNode '{}': missing or invalid properties!", name), nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (hasChannelDataProperties && (!rootInProperty->getChild(EInputIdx_ChannelsData) || rootInProperty->getChild(EInputIdx_ChannelsData)->getName() != "channelsData"))
        {
            errorReporting.add(fmt::format("Fatal error during loading of AnimationNode '{}': missing or invalid channels data property!", name), nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        deserialized->setRootProperties(std::make_unique<Property>(std::move(rootInProperty)), std::make_unique<Property>(std::move(rootOutProperty)));

        // reset property values of all channel data to original channel data provided at creation time
        if (hasChannelDataProperties)
            deserialized->initAnimationDataPropertyValues();

        return deserialized;
    }

    void AnimationNodeImpl::initAnimationDataPropertyValues()
    {
        Property* channelsData = getInputs()->getChild("channelsData");
        assert(channelsData && channelsData->getChildCount() == m_channelsWorkData.size());
        for (size_t channelIdx = 0u; channelIdx < channelsData->getChildCount(); ++channelIdx)
        {
            Property* channelDataProp = channelsData->getChild(channelIdx);
            Property* timestampsProp = channelDataProp->getChild("timestamps");
            Property* keyframesProp = channelDataProp->getChild("keyframes");
            assert(timestampsProp && keyframesProp);
            const auto& channelData = m_channelsWorkData[channelIdx];
            assert(timestampsProp->getChildCount() == channelData.timestamps.size());
            assert(keyframesProp->getChildCount() == timestampsProp->getChildCount());

            const auto& timestamps = channelData.timestamps;
            for (size_t i = 0u; i < timestamps.size(); ++i)
                timestampsProp->getChild(i)->m_impl->setValue(timestamps[i]);

            std::visit([&](const auto& keyframes) {
                using ValueType = std::remove_const_t<std::remove_reference_t<decltype(keyframes.front())>>;
                // array data type requires each array element of each keyframe element to be set to individual input property
                if constexpr (std::is_same_v<ValueType, std::vector<float>>)
                {
                    assert(!"not supported");
                }
                else
                {
                    for (size_t i = 0u; i < keyframes.size(); ++i)
                        keyframesProp->getChild(i)->m_impl->setValue(keyframes[i]);
                }
            }, channelData.keyframes);
        }
    }

    void AnimationNodeImpl::updateAnimationDataFromProperties()
    {
        const auto channelsDataProp = getInputs()->getChild(EInputIdx_ChannelsData);
        for (size_t ch = 0u; ch < channelsDataProp->getChildCount(); ++ch)
        {
            const auto channelDataProp = channelsDataProp->getChild(ch);

            const auto timestampsProp = channelDataProp->getChild(0u);
            auto& timestamps = m_channelsWorkData[ch].timestamps;
            assert(timestamps.size() == timestampsProp->getChildCount());
            for (size_t i = 0u; i < timestamps.size(); ++i)
            {
                timestamps[i] = *timestampsProp->getChild(i)->get<float>();
                m_maxChannelDuration = std::max(m_maxChannelDuration, timestamps[i]);
            }

            const auto keyframesProp = channelDataProp->getChild(1u);
            auto& keyframesVariant = m_channelsWorkData[ch].keyframes;
            std::visit([&](auto& keyframes) {
                using ValueType = std::remove_const_t<std::remove_reference_t<decltype(keyframes.front())>>;
                // array data type requires each array element of each keyframe element to be read from individual input property
                if constexpr (std::is_same_v<ValueType, std::vector<float>>)
                {
                    assert(!"not supported");
                }
                else
                {
                    for (size_t i = 0u; i < keyframes.size(); ++i)
                        keyframes[i] = *keyframesProp->getChild(i)->get<ValueType>();
                }
            }, keyframesVariant);
        }

        getOutputs()->getChild(EOutputIdx_Duration)->set(m_maxChannelDuration);
    }
}
