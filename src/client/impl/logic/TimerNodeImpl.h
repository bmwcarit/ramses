//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/logic/LogicNodeImpl.h"
#include <string>
#include <optional>
#include <chrono>

namespace rlogic_serialization
{
    struct TimerNode;
}

namespace flatbuffers
{
    template<typename T> struct Offset;
    template<bool> class FlatBufferBuilderImpl;
    using FlatBufferBuilder = FlatBufferBuilderImpl<false>;
}

namespace ramses::internal
{
    class SerializationMap;
    class DeserializationMap;
    class ErrorReporting;

    class TimerNodeImpl : public LogicNodeImpl
    {
    public:
        TimerNodeImpl(SceneImpl& scene, std::string_view name, sceneObjectId_t id) noexcept;

        std::optional<LogicNodeRuntimeError> update() override;

        void createRootProperties() final;

        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::TimerNode> Serialize(
            const TimerNodeImpl& timerNode,
            flatbuffers::FlatBufferBuilder& builder,
            SerializationMap& serializationMap);

        [[nodiscard]] static std::unique_ptr<TimerNodeImpl> Deserialize(
            const rlogic_serialization::TimerNode& timerNodeFB,
            ErrorReporting& errorReporting,
            DeserializationMap& deserializationMap);
    };
}
