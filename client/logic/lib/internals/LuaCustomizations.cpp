//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internals/LuaCustomizations.h"

#include "ramses-logic/Property.h"
#include "ramses-logic/EPropertyType.h"

#include "impl/LoggerImpl.h"

#include "internals/SolHelper.h"
#include "internals/WrappedLuaProperty.h"
#include "internals/PropertyTypeExtractor.h"
#include "internals/LuaTypeConversions.h"
#include "internals/TypeUtils.h"

namespace rlogic::internal
{
    void LuaCustomizations::RegisterTypes(sol::state& state)
    {
        state["rl_len"] = rl_len;
        state["rl_next"] = rl_next;
        state["rl_pairs"] = rl_pairs;
        state["rl_ipairs"] = rl_ipairs;
    }

    void LuaCustomizations::MapToEnvironment(sol::state& state, sol::environment& env)
    {
        env["rl_len"] = state["rl_len"];
        env["rl_next"] = state["rl_next"];
        env["rl_pairs"] = state["rl_pairs"];
        env["rl_ipairs"] = state["rl_ipairs"];
    }

    void LuaCustomizations::MapDebugLogFunctions(sol::environment& env)
    {
        auto rl_logInfo = [](const std::string& msg)
        {
            LOG_INFO("LuaDebugLog: {}", msg);
        };
        auto rl_logWarn = [](const std::string& msg)
        {
            LOG_WARN("LuaDebugLog: {}", msg);
        };
        auto rl_logError = [](const std::string& msg)
        {
            LOG_ERROR("LuaDebugLog: {}", msg);
        };

        env["rl_logInfo"] = rl_logInfo;
        env["rl_logWarn"] = rl_logWarn;
        env["rl_logError"] = rl_logError;
    }

    size_t LuaCustomizations::rl_len(const sol::object& obj)
    {
        // Check if normal lua table, or read-only module table
        const std::optional<sol::lua_table> potentialTable = LuaTypeConversions::ExtractLuaTable(obj);
        if (potentialTable)
        {
            return potentialTable->size();
        }

        // Check for custom types (registered by logic engine)
        if (obj.get_type() == sol::type::userdata)
        {
            if (obj.is<WrappedLuaProperty>())
            {
                return obj.as<WrappedLuaProperty>().size();
            }

            if (obj.is<PropertyTypeExtractor>())
            {
                const auto& extractor = obj.as<PropertyTypeExtractor>();
                const EPropertyType rootType = extractor.getRootTypeData().type;
                if (TypeUtils::CanHaveChildren(rootType))
                    return extractor.getNestedExtractors().size();

                // all other custom types are not countable during type extraction
                sol_helper::throwSolException("rl_len() called on an unsupported type '{}'", GetLuaPrimitiveTypeName(rootType));
            }
        }

        // Other type (unsupported) -> report usage error
        sol_helper::throwSolException("rl_len() called on an unsupported type '{}'", sol_helper::GetSolTypeName(obj.get_type()));

        return 0u;
    }

    std::tuple<sol::object, sol::object> LuaCustomizations::rl_next(sol::this_state state, const sol::object& container, const sol::object& indexObject)
    {
        // Runtime property (checked first because performance-wise highest priority)
        if (container.is<WrappedLuaProperty>())
        {
            const WrappedLuaProperty& wrappedProperty = container.as<WrappedLuaProperty>();
            const EPropertyType propType = wrappedProperty.getWrappedProperty().getType();

            if (TypeUtils::CanHaveChildren(propType))
            {
                assert(propType == EPropertyType::Struct || propType == EPropertyType::Array);

                // Valid case for struct and array! If container empty, next() is required to return a tuple of nil's
                if (wrappedProperty.size() == 0u)
                    return std::make_tuple(sol::lua_nil, sol::lua_nil);

                if (propType == EPropertyType::Array)
                    return rl_next_runtime_array(state, wrappedProperty, indexObject);
                if (propType == EPropertyType::Struct)
                    return rl_next_runtime_struct(state, wrappedProperty, indexObject);
            }

            if (TypeUtils::IsPrimitiveVectorType(propType))
                return rl_next_runtime_vector(state, wrappedProperty, indexObject);

            sol_helper::throwSolException("rl_next() called on an unsupported type '{}'", GetLuaPrimitiveTypeName(propType));
        }

        // Standard Lua table or a read-only module
        std::optional<sol::lua_table> potentialModuleTable = LuaTypeConversions::ExtractLuaTable(container);
        if (potentialModuleTable)
        {
            sol::function stdNext = sol::state_view(state)["next"];
            std::tuple<sol::object, sol::object> resultOfStdNext = stdNext(*potentialModuleTable, indexObject);

            return resultOfStdNext;
        }

        // Property extractor - this is not executed during runtime, only during interface(), so it's ok to check last
        if (container.is<PropertyTypeExtractor>())
        {
            const PropertyTypeExtractor& typeExtractor = container.as<PropertyTypeExtractor>();
            const EPropertyType rootType = typeExtractor.getRootTypeData().type;

            // Not possible to get non-iteratable types during extraction, safe to assert here
            if (TypeUtils::CanHaveChildren(rootType))
            {
                // Valid case! If container empty, next() is required to return a tuple of nil's
                if (typeExtractor.getNestedExtractors().empty())
                    return std::make_tuple(sol::lua_nil, sol::lua_nil);

                if (rootType == EPropertyType::Array)
                    return rl_next_array_extractor(state, typeExtractor, indexObject);

                return rl_next_struct_extractor(state, typeExtractor, indexObject);
            }

            // all other custom types are not iteratable during type extraction
            sol_helper::throwSolException("rl_next() called on an unsupported type '{}'", GetLuaPrimitiveTypeName(rootType));
        }

        sol_helper::throwSolException("rl_next() called on an unsupported type '{}'", sol_helper::GetSolTypeName(container.get_type()));

        return std::make_tuple(sol::lua_nil, sol::lua_nil);
    }

    std::tuple<sol::object, sol::object> LuaCustomizations::rl_next_runtime_array(sol::this_state state, const WrappedLuaProperty& wrappedArray, const sol::object& indexObject)
    {
        assert(wrappedArray.getWrappedProperty().getType() == EPropertyType::Array);

        // If index is nil, return the first element
        if (indexObject.get_type() == sol::type::lua_nil)
        {
            // in Lua counting starts at 1; this returns the first tuple of index + value
            return std::make_tuple(sol::make_object(state, 1), wrappedArray.resolveChild(state, 0u));
        }

        const DataOrError<size_t> potentiallyIndex = LuaTypeConversions::ExtractSpecificType<size_t>(indexObject);

        if (potentiallyIndex.hasError())
        {
            sol_helper::throwSolException("Invalid key to rl_next() of type: {}", potentiallyIndex.getError());
        }

        const size_t index = potentiallyIndex.getData();

        if (index == 0 || index > wrappedArray.size())
        {
            sol_helper::throwSolException("Invalid key value '{}' for rl_next(). Expected a number in the range [1, {}]!", index, wrappedArray.size());
        }

        // This is valid - when index is the last element, the 'next' one is idx=nil, value=nil
        if (index == wrappedArray.size())
        {
            return std::make_tuple(sol::lua_nil, sol::lua_nil);
        }

        return std::make_tuple(sol::make_object(state, index + 1), wrappedArray.resolveChild(state, index));
    }

    std::tuple<sol::object, sol::object> LuaCustomizations::rl_next_runtime_struct(sol::this_state state, const WrappedLuaProperty& wrappedStruct, const sol::object& indexObject)
    {
        assert(wrappedStruct.getWrappedProperty().getType() == EPropertyType::Struct);

        // If index is nil, return the first element
        if (indexObject.get_type() == sol::type::lua_nil)
        {
            // return name of first element as key
            return std::make_tuple(sol::make_object(state, wrappedStruct.getWrappedProperty().getChild(0)->getName()), wrappedStruct.resolveChild(state, 0u));
        }

        // TODO Violin rework the error message here so that it's clear where the error comes from
        const size_t structFieldIndex = wrappedStruct.resolvePropertyIndex(indexObject);

        // This is valid - when index is the last element, the 'next' one is idx=nil, value=nil
        if (structFieldIndex == wrappedStruct.size() - 1)
        {
            return std::make_tuple(sol::lua_nil, sol::lua_nil);
        }

        return std::make_tuple(
            sol::make_object(state, wrappedStruct.getWrappedProperty().getChild(structFieldIndex + 1)->getName()),
            wrappedStruct.resolveChild(state, structFieldIndex + 1));
    }

    std::tuple<sol::object, sol::object> LuaCustomizations::rl_next_runtime_vector(sol::this_state state, const WrappedLuaProperty& wrappedVec, const sol::object& indexObject)
    {
        assert(TypeUtils::IsPrimitiveVectorType(wrappedVec.getWrappedProperty().getType()));

        // If index is nil, return the first element
        if (indexObject.get_type() == sol::type::lua_nil)
            return std::make_tuple(sol::make_object(state, 1u), wrappedVec.resolveVectorElement(state, 1u));

        const size_t elementIndex = wrappedVec.resolvePropertyIndex(indexObject);
        const size_t numElements = TypeUtils::ComponentsSizeForPropertyType(wrappedVec.getWrappedProperty().getType());

        // This is valid - when index is the last element, the 'next' one is idx=nil, value=nil
        if (elementIndex == numElements)
            return std::make_tuple(sol::lua_nil, sol::lua_nil);

        return std::make_tuple(sol::make_object(state, elementIndex + 1), wrappedVec.resolveVectorElement(state, elementIndex + 1));
    }

    std::tuple<sol::object, sol::object> LuaCustomizations::rl_next_array_extractor(sol::this_state state, const PropertyTypeExtractor& arrayExtractor, const sol::object& indexObject)
    {
        // If index is nil, return the first element
        if (indexObject.get_type() == sol::type::lua_nil)
        {
            // in Lua counting starts at 1; this returns the first tuple of index + value
            return ResolveExtractorField(state, arrayExtractor, 0u);
        }

        const DataOrError<size_t> potentiallyIndex = LuaTypeConversions::ExtractSpecificType<size_t>(indexObject);

        if (potentiallyIndex.hasError())
        {
            sol_helper::throwSolException("Invalid key to rl_next() of type: {}", potentiallyIndex.getError());
        }

        const size_t arrayElementCount = arrayExtractor.getNestedExtractors().size();

        const size_t index = potentiallyIndex.getData();

        if (index == 0 || index > arrayElementCount)
        {
            sol_helper::throwSolException("Invalid key value '{}' for rl_next(). Expected a number in the range [1, {}]!", index, arrayElementCount);
        }

        // This is valid - when index is the last element, the 'next' one is idx=nil, value=nil
        if (index == arrayElementCount)
        {
            return std::make_tuple(sol::lua_nil, sol::lua_nil);
        }

        // Return next element which is 'index + 1', but because Lua starts at 1, it's just 'index'
        return ResolveExtractorField(state, arrayExtractor, index);
    }

    std::tuple<sol::object, sol::object> LuaCustomizations::rl_next_struct_extractor(sol::this_state state, const PropertyTypeExtractor& structExtractor, const sol::object& indexObject)
    {
        // If index is nil, return the first element
        if (indexObject.get_type() == sol::type::lua_nil)
        {
            return ResolveExtractorField(state, structExtractor, 0);
        }

        const DataOrError<std::string_view> potentiallyStrIndex = LuaTypeConversions::ExtractSpecificType<std::string_view>(indexObject);

        if (potentiallyStrIndex.hasError())
        {
            sol_helper::throwSolException("Invalid key to rl_next(): {}", potentiallyStrIndex.getError());
        }

        const std::string_view strIndex = potentiallyStrIndex.getData();

        const std::vector<PropertyTypeExtractor>& fields = structExtractor.getNestedExtractors();
        const auto fieldIter = std::find_if(fields.cbegin(), fields.cend(),
            [&strIndex](const PropertyTypeExtractor& field) {
                return field.getRootTypeData().name == strIndex;
            });

        if (fieldIter == fields.cend())
        {
            sol_helper::throwSolException("Could not find field named '{}' in struct object '{}'!",
                strIndex, structExtractor.getRootTypeData().name);
        }

        const size_t structFieldIndex = fieldIter - fields.cbegin();

        // This is valid - when index is the last element, the 'next' one is idx=nil, value=nil
        if (structFieldIndex == fields.size() - 1)
        {
            return std::make_tuple(sol::lua_nil, sol::lua_nil);
        }

        return ResolveExtractorField(state, structExtractor, structFieldIndex + 1);
    }

    std::tuple<sol::object, sol::object, sol::object> LuaCustomizations::rl_ipairs(sol::this_state s, sol::object iterableObject)
    {
        if (iterableObject.get_type() == sol::type::userdata)
        {
            auto wrappedProperty = iterableObject.as<sol::optional<const WrappedLuaProperty&>>();
            auto propertyExtractor = iterableObject.as<sol::optional<const PropertyTypeExtractor&>>();
            // Assert that one of the types were found
            assert(wrappedProperty || propertyExtractor);

            const EPropertyType propertyType = (wrappedProperty ? wrappedProperty->getWrappedProperty().getType() : propertyExtractor->getRootTypeData().type);
            if (propertyType == EPropertyType::Array || // Array types can be iterated both in runtime and extraction
                (TypeUtils::IsPrimitiveVectorType(propertyType) && wrappedProperty)) // VEC types can be iterated only in runtime, not during property extraction
            {
                return std::make_tuple(sol::state_view(s)["rl_next"], std::move(iterableObject), sol::nil);
            }

            // no other custom types can be iterated using rl_ipairs
            sol_helper::throwSolException("rl_ipairs() called on an unsupported type '{}'. Use only with array-like built-in types or modules!", GetLuaPrimitiveTypeName(propertyType));
        }

        std::optional<sol::lua_table> potentialModuleTable = LuaTypeConversions::ExtractLuaTable(iterableObject);
        if (potentialModuleTable)
        {
            return std::make_tuple(sol::state_view(s)["next"], std::move(*potentialModuleTable), sol::nil);
        }

        sol_helper::throwSolException("rl_ipairs() called on an unsupported type '{}'. Use only with user types like IN/OUT, modules etc.!", sol_helper::GetSolTypeName(iterableObject.get_type()));
        return std::make_tuple(sol::nil, sol::nil, sol::nil);
    }

    std::tuple<sol::object, sol::object, sol::object> LuaCustomizations::rl_pairs(sol::this_state s, sol::object iterableObject)
    {
        if (iterableObject.get_type() == sol::type::userdata)
        {
            return std::make_tuple(sol::state_view(s)["rl_next"], std::move(iterableObject), sol::nil);
        }

        std::optional<sol::lua_table> potentialModuleTable = LuaTypeConversions::ExtractLuaTable(iterableObject);
        if (potentialModuleTable)
        {
            return std::make_tuple(sol::state_view(s)["next"], std::move(*potentialModuleTable), sol::nil);
        }

        sol_helper::throwSolException("rl_pairs() called on an unsupported type '{}'. Use only with user types like IN/OUT, modules etc.!", sol_helper::GetSolTypeName(iterableObject.get_type()));
        return std::make_tuple(sol::nil, sol::nil, sol::nil);
    }

    std::tuple<sol::object, sol::object> LuaCustomizations::ResolveExtractorField(sol::this_state s, const PropertyTypeExtractor& typeExtractor, size_t fieldId)
    {
        const EPropertyType rootType = typeExtractor.getRootTypeData().type;
        std::reference_wrapper<const PropertyTypeExtractor> field = typeExtractor.getChildReference(fieldId);
        const EPropertyType fieldType = field.get().getRootTypeData().type;

        // Provide name as key for structs, index for arrays
        sol::object key =
            rootType == EPropertyType::Struct ?
            sol::make_object(s, field.get().getRootTypeData().name) :
            sol::make_object(s, fieldId + 1);   // Convert to Lua numeric convention (starts at 1)

        // Cast to label, e.g. EPropertyType::Int32, EPropertyType:Float etc., for primitive types; Otherwise, return the container object for further iteration if needed
        sol::object value =
            TypeUtils::IsPrimitiveType(fieldType) ?
            sol::make_object(s, static_cast<int>(fieldType)) :
            sol::make_object(s, field);

        return std::make_tuple(std::move(key), std::move(value));
    }
}
