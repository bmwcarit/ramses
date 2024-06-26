//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/logic/LuaInterfaceImpl.h"

#include "impl/ErrorReporting.h"
#include "internal/logic/TypeUtils.h"

#include "internal/logic/flatbuffers/generated/LuaInterfaceGen.h"

namespace ramses::internal
{
    LuaInterfaceImpl::LuaInterfaceImpl(SceneImpl& scene, LuaCompiledInterface compiledInterface, std::string_view name, sceneObjectId_t id)
        : LogicNodeImpl{ scene, name, id }
    {
        setRootProperties(std::move(compiledInterface.rootProperty), nullptr);
    }

    flatbuffers::Offset<rlogic_serialization::LuaInterface> LuaInterfaceImpl::Serialize(
        const LuaInterfaceImpl& luaInterface,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap)
    {
        const auto logicObject = LogicObjectImpl::Serialize(luaInterface, builder);
        const auto propertyObject = PropertyImpl::Serialize(luaInterface.getInputs()->impl(), builder, serializationMap);
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
        sceneObjectId_t id{};
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(luaInterface.base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.set("Fatal error during loading of LuaInterface from serialized data: missing name and/or ID!", nullptr);
            return nullptr;
        }

        if (name.length() == 0)
        {
            errorReporting.set("Fatal error during loading of LuaInterface from serialized data: empty name!", nullptr);
            return nullptr;
        }

        if (!luaInterface.rootProperty())
        {
            errorReporting.set("Fatal error during loading of LuaInterface from serialized data: missing root property!", nullptr);
            return nullptr;
        }

        std::unique_ptr<PropertyImpl> rootProperty = PropertyImpl::Deserialize(*luaInterface.rootProperty(), EPropertySemantics::Interface, errorReporting, deserializationMap);
        if (!rootProperty)
        {
            return nullptr;
        }

        if (rootProperty->getType() != EPropertyType::Struct)
        {
            errorReporting.set("Fatal error during loading of LuaScript from serialized data: root property has unexpected type!", nullptr);
            return nullptr;
        }

        auto deserialized = std::make_unique<LuaInterfaceImpl>(
            deserializationMap.getScene(),
            LuaCompiledInterface{ std::move(rootProperty) },
            name, id);
        deserialized->setUserId(userIdHigh, userIdLow);

        return deserialized;
    }

    std::vector<const Property*> LuaInterfaceImpl::collectUnlinkedProperties() const
    {
        std::vector<const Property*> outputProperties = getOutputs()->impl().collectLeafChildren();

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
