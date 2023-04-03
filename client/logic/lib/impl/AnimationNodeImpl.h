//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/AnimationTypes.h"

#include "impl/LogicNodeImpl.h"
#include "impl/DataArrayImpl.h"
#include <memory>

namespace rlogic_serialization
{
    struct AnimationNode;
}

namespace flatbuffers
{
    template<typename T> struct Offset;
    class FlatBufferBuilder;
}

namespace rlogic::internal
{
    class SerializationMap;
    class DeserializationMap;
    class ErrorReporting;

    class AnimationNodeImpl : public LogicNodeImpl
    {
    public:
        AnimationNodeImpl(AnimationChannels channels, bool exposeDataAsProperties, std::string_view name, uint64_t id) noexcept;

        [[nodiscard]] float getMaximumChannelDuration() const;
        [[nodiscard]] const AnimationChannels& getChannels() const;

        std::optional<LogicNodeRuntimeError> update() override;

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::AnimationNode> Serialize(
            const AnimationNodeImpl& animNode,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap);
        [[nodiscard]] static std::unique_ptr<AnimationNodeImpl> Deserialize(
            const rlogic_serialization::AnimationNode& animNodeFB,
            ErrorReporting& errorReporting,
            DeserializationMap& deserializationMap);

        void createRootProperties() final;

    private:
        void updateChannel(size_t channelIdx, float localAnimationTime);

        template <typename T>
        T interpolateKeyframes_linear(T lowerVal, T upperVal, float interpRatio);
        template <typename T>
        T interpolateKeyframes_cubic(T lowerVal, T upperVal, T lowerTangentOut, T upperTangentIn, float interpRatio, float timeBetweenKeys);

        void initAnimationDataPropertyValues();
        void updateAnimationDataFromProperties();

        // original channel data provided by user
        AnimationChannels m_channels;

        // work data (extracted copy of subset of original data)
        struct ChannelWorkData
        {
            std::vector<float> timestamps;
            DataArrayImpl::DataArrayVariant keyframes;
        };
        std::vector<ChannelWorkData> m_channelsWorkData;

        float m_maxChannelDuration = 0.f;

        bool m_hasChannelDataExposedViaProperties = false;

        enum EInputIdx
        {
            EInputIdx_Progress = 0,
            EInputIdx_ChannelsData    // optional property for animation nodes with exposed channel data - should be always last!
        };

        enum EOutputIdx
        {
            EOutputIdx_Duration = 0,
            EOutputIdx_ChannelsBegin // must be last
        };
    };
}
