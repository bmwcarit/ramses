//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/TimerNodeImpl.h"
#include "ramses-logic/Property.h"
#include "impl/PropertyImpl.h"
#include "internals/ErrorReporting.h"
#include "generated/TimerNodeGen.h"
#include "flatbuffers/flatbuffers.h"
#include "fmt/format.h"

namespace rlogic::internal
{
    TimerNodeImpl::TimerNodeImpl(std::string_view name, uint64_t id) noexcept
        : LogicNodeImpl(name, id)
    {
    }

    void TimerNodeImpl::createRootProperties()
    {
        HierarchicalTypeData inputs = MakeStruct("", {
            {"ticker_us", EPropertyType::Int64}
            });
        auto inputsImpl = std::make_unique<PropertyImpl>(std::move(inputs), EPropertySemantics::ScriptInput);

        HierarchicalTypeData outputs = MakeStruct("", {
            {"ticker_us", EPropertyType::Int64}
            });
        auto outputsImpl = std::make_unique<PropertyImpl>(std::move(outputs), EPropertySemantics::ScriptOutput);

        setRootProperties(std::make_unique<Property>(std::move(inputsImpl)), std::make_unique<Property>(std::move(outputsImpl)));
    }

    std::optional<LogicNodeRuntimeError> TimerNodeImpl::update()
    {
        const int64_t ticker = *getInputs()->getChild(0u)->get<int64_t>();

        int64_t outTicker_us = 0;
        if (ticker == 0) // built-in ticker using system clock
        {
            outTicker_us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        }
        else // user provided ticker
        {
            outTicker_us = ticker;
        }

        getOutputs()->getChild(0u)->m_impl->setValue(outTicker_us);

        return std::nullopt;
    }

    flatbuffers::Offset<rlogic_serialization::TimerNode> TimerNodeImpl::Serialize(
        const TimerNodeImpl& timerNode,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap)
    {
        // Timer nodes require special serialization logic. We don't want to store system time
        // in the files (this makes their content undeterministic). Instead, we write zeroes, and
        // cache the current values and restore them again after serialization is done

        // 1. Cache current output values
        const PropertyValue outTickerTmp = timerNode.getOutputs()->getChild(0u)->m_impl->getValue();

        // 2. Set to 0
        timerNode.getOutputs()->getChild(0u)->m_impl->setValue(int64_t(0));

        // 3. Serialize
        const auto logicObject = LogicObjectImpl::Serialize(timerNode, builder);
        const auto inputPropertyObject = PropertyImpl::Serialize(*timerNode.getInputs()->m_impl, builder, serializationMap);
        const auto outputPropertyObject = PropertyImpl::Serialize(*timerNode.getOutputs()->m_impl, builder, serializationMap);
        auto timerNodeOffset = rlogic_serialization::CreateTimerNode(
            builder,
            logicObject,
            inputPropertyObject,
            outputPropertyObject
        );

        // 4. Restore values
        timerNode.getOutputs()->getChild(0u)->m_impl->setValue(outTickerTmp);

        return timerNodeOffset;
    }

    std::unique_ptr<TimerNodeImpl> TimerNodeImpl::Deserialize(
        const rlogic_serialization::TimerNode& timerNodeFB,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap)
    {
        std::string name;
        uint64_t id = 0u;
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(timerNodeFB.base(), name, id, userIdHigh, userIdLow, errorReporting) || !timerNodeFB.rootInput() || !timerNodeFB.rootOutput())
        {
            errorReporting.add("Fatal error during loading of TimerNode from serialized data: missing name, id or in/out property data!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        auto deserialized = std::make_unique<TimerNodeImpl>(name, id);
        deserialized->setUserId(userIdHigh, userIdLow);

        // deserialize and overwrite constructor generated properties
        auto rootInProperty = PropertyImpl::Deserialize(*timerNodeFB.rootInput(), EPropertySemantics::ScriptInput, errorReporting, deserializationMap);
        auto rootOutProperty = PropertyImpl::Deserialize(*timerNodeFB.rootOutput(), EPropertySemantics::ScriptOutput, errorReporting, deserializationMap);
        if (rootInProperty->getChildCount() != 1u ||
            rootOutProperty->getChildCount() != 1u ||
            !rootInProperty->getChild(0u) || rootInProperty->getChild(0u)->getName() != "ticker_us" ||
            !rootOutProperty->getChild(0u) || rootOutProperty->getChild(0u)->getName() != "ticker_us")
        {
            errorReporting.add(fmt::format("Fatal error during loading of TimerNode '{}': missing or invalid properties!", name), nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }
        deserialized->setRootProperties(std::make_unique<Property>(std::move(rootInProperty)), std::make_unique<Property>(std::move(rootOutProperty)));

        return deserialized;
    }
}
