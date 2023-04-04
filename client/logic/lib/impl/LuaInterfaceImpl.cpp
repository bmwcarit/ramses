//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/LuaInterfaceImpl.h"

#include "internals/ErrorReporting.h"
#include "internals/TypeUtils.h"

#include "generated/LuaInterfaceGen.h"

namespace rlogic::internal
{
    LuaInterfaceImpl::LuaInterfaceImpl(LuaCompiledInterface compiledInterface, std::string_view name, uint64_t id)
        : LogicNodeImpl(name, id)
    {
        setRootProperties(std::move(compiledInterface.rootProperty), nullptr);
    }

    flatbuffers::Offset<rlogic_serialization::LuaInterface> LuaInterfaceImpl::Serialize(
        const LuaInterfaceImpl& luaInterface,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap,
        EFeatureLevel /*featureLevel*/)
    {
        const auto logicObject = LogicObjectImpl::Serialize(luaInterface, builder);
        const auto propertyObject = PropertyImpl::Serialize(*luaInterface.getInputs()->m_impl, builder, serializationMap);
        auto intf = rlogic_serialization::CreateLuaInterface(builder, logicObject, propertyObject);
        builder.Finish(intf);

        return intf;
    }

    std::unique_ptr<LuaInterfaceImpl> LuaInterfaceImpl::Deserialize(
        const rlogic_serialization::LuaInterface& luaInterface,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap)
    {
        std::string name;
        uint64_t id = 0u;
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(luaInterface.base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.add("Fatal error during loading of LuaInterface from serialized data: missing name and/or ID!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (name.length() == 0)
        {
            errorReporting.add("Fatal error during loading of LuaInterface from serialized data: empty name!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!luaInterface.rootProperty())
        {
            errorReporting.add("Fatal error during loading of LuaInterface from serialized data: missing root property!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        std::unique_ptr<PropertyImpl> rootProperty = PropertyImpl::Deserialize(*luaInterface.rootProperty(), EPropertySemantics::Interface, errorReporting, deserializationMap);
        if (!rootProperty)
        {
            return nullptr;
        }

        if (rootProperty->getType() != EPropertyType::Struct)
        {
            errorReporting.add("Fatal error during loading of LuaScript from serialized data: root property has unexpected type!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        auto root = std::make_unique<Property>(std::move(rootProperty));

        auto deserialized = std::make_unique<LuaInterfaceImpl>(
            LuaCompiledInterface{
                std::move(root)
            },
            name, id);
        deserialized->setUserId(userIdHigh, userIdLow);

        return deserialized;
    }

    std::vector<const Property*> LuaInterfaceImpl::collectUnlinkedProperties() const
    {
        std::vector<const Property*> outputProperties = getOutputs()->m_impl->collectLeafChildren();

        // filter for unlinked properties by removing linked ones
        auto it = std::remove_if(outputProperties.begin(), outputProperties.end(), [](const auto* node) { return node->isLinked(); });
        outputProperties.erase(it, outputProperties.end());

        return outputProperties;
    }

    std::optional<LogicNodeRuntimeError> LuaInterfaceImpl::update()
    {
        // No-op, interfaces don't need any logic, they just hold proxy properties which are linked to other objects
        return std::nullopt;
    }

    void LuaInterfaceImpl::createRootProperties()
    {
        // unlike other logic objects, lua interface properties created outside of it (from script or deserialized)
    }

    Property* LuaInterfaceImpl::getOutputs()
    {
        // For an interface, intputs == outputs
        return getInputs();
    }

    const Property* LuaInterfaceImpl::getOutputs() const
    {
        // For an interface, intputs == outputs
        return getInputs();
    }

}
