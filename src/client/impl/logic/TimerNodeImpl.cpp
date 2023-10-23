//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/logic/TimerNodeImpl.h"
#include "ramses/client/logic/Property.h"
#include "impl/logic/PropertyImpl.h"
#include "impl/ErrorReporting.h"
#include "internal/logic/flatbuffers/generated/TimerNodeGen.h"
#include "flatbuffers/flatbuffers.h"
#include "fmt/format.h"

namespace ramses::internal
{
    TimerNodeImpl::TimerNodeImpl(SceneImpl& scene, std::string_view name, sceneObjectId_t id) noexcept
        : LogicNodeImpl{ scene, name, id }
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

        setRootProperties(std::move(inputsImpl), std::move(outputsImpl));
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

        getOutputs()->getChild(0u)->impl().setValue(outTicker_us);

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

        const PropertyImpl& property = timerNode.getOutputs()->getChild(0u)->impl();

        // 1. Cache current output values
        const PropertyValue outTickerTmp = property.getValue();

        // 2. Set to 0
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        const_cast<PropertyImpl&>(property).setValue(int64_t(0));

        // 3. Serialize
        const auto logicObject = LogicObjectImpl::Serialize(timerNode, builder);
        const auto inputPropertyObject = PropertyImpl::Serialize(timerNode.getInputs()->impl(), builder, serializationMap);
        const auto outputPropertyObject = PropertyImpl::Serialize(timerNode.getOutputs()->impl(), builder, serializationMap);
        auto timerNodeOffset = rlogic_serialization::CreateTimerNode(
            builder,
            logicObject,
            inputPropertyObject,
            outputPropertyObject
        );

        // 4. Restore values
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        const_cast<PropertyImpl&>(property).setValue(outTickerTmp);

        return timerNodeOffset;
    }

    std::unique_ptr<TimerNodeImpl> TimerNodeImpl::Deserialize(
        const rlogic_serialization::TimerNode& timerNodeFB,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap)
    {
        std::string name;
        sceneObjectId_t id{};
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(timerNodeFB.base(), name, id, userIdHigh, userIdLow, errorReporting) || !timerNodeFB.rootInput() || !timerNodeFB.rootOutput())
        {
            errorReporting.set("Fatal error during loading of TimerNode from serialized data: missing name, id or in/out property data!", nullptr);
            return nullptr;
        }

        auto deserialized = std::make_unique<TimerNodeImpl>(deserializationMap.getScene(), name, id);
        deserialized->setUserId(userIdHigh, userIdLow);

        // deserialize and overwrite constructor generated properties
        auto rootInProperty = PropertyImpl::Deserialize(*timerNodeFB.rootInput(), EPropertySemantics::ScriptInput, errorReporting, deserializationMap);
        auto rootOutProperty = PropertyImpl::Deserialize(*timerNodeFB.rootOutput(), EPropertySemantics::ScriptOutput, errorReporting, deserializationMap);
        if (rootInProperty->getChildCount() != 1u ||
            rootOutProperty->getChildCount() != 1u ||
            !rootInProperty->getChild(0u) || rootInProperty->getChild(0u)->getName() != "ticker_us" ||
            !rootOutProperty->getChild(0u) || rootOutProperty->getChild(0u)->getName() != "ticker_us")
        {
            errorReporting.set(fmt::format("Fatal error during loading of TimerNode '{}': missing or invalid properties!", name), nullptr);
            return nullptr;
        }
        deserialized->setRootProperties(std::move(rootInProperty), std::move(rootOutProperty));

        return deserialized;
    }
}
