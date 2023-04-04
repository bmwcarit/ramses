//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internals/SolWrapper.h"

namespace rlogic::internal
{
    class PropertyTypeExtractor;
    class WrappedLuaProperty;

    class LuaCustomizations
    {
    public:
        static void RegisterTypes(sol::state& state);
        static void MapToEnvironment(sol::state& state, sol::environment& env);
        static void MapDebugLogFunctions(sol::environment& env);

    private:
        [[nodiscard]] static size_t rl_len(const sol::object& obj);

        [[nodiscard]] static std::tuple<sol::object, sol::object> rl_next(sol::this_state state, const sol::object& container, const sol::object& indexObject);
        [[nodiscard]] static std::tuple<sol::object, sol::object> rl_next_runtime_array(sol::this_state state, const WrappedLuaProperty& wrappedArray, const sol::object& indexObject);
        [[nodiscard]] static std::tuple<sol::object, sol::object> rl_next_runtime_struct(sol::this_state state, const WrappedLuaProperty& wrappedStruct, const sol::object& indexObject);
        [[nodiscard]] static std::tuple<sol::object, sol::object> rl_next_runtime_vector(sol::this_state state, const WrappedLuaProperty& wrappedVec, const sol::object& indexObject);
        [[nodiscard]] static std::tuple<sol::object, sol::object> rl_next_array_extractor(sol::this_state state, const PropertyTypeExtractor& arrayExtractor, const sol::object& indexObject);
        [[nodiscard]] static std::tuple<sol::object, sol::object> rl_next_struct_extractor(sol::this_state state, const PropertyTypeExtractor& structExtractor, const sol::object& indexObject);

        [[nodiscard]] static std::tuple<sol::object, sol::object, sol::object> rl_pairs(sol::this_state s, sol::object iterableObject);
        [[nodiscard]] static std::tuple<sol::object, sol::object, sol::object> rl_ipairs(sol::this_state s, sol::object iterableObject);

        [[nodiscard]] static std::tuple<sol::object, sol::object> ResolveExtractorField(sol::this_state s, const PropertyTypeExtractor& typeExtractor, size_t fieldId);
    };
}
