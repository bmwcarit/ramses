//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internals/InterfaceTypeInfo.h"
#include "internals/LuaTypeConversions.h"

namespace rlogic::internal
{
    // This class holds the functions used to declare types in Lua, e.g. Type:Int32(), Type:Array() etc.
    // The Lua syntax Table:Method(args) converts to Table.Method(Table, args) and therefore we have an
    // unused argument in the methods because Type is a singleton instance and we don't use it for anything currently
    class InterfaceTypeFunctions
    {
    public:
        [[nodiscard]] static sol::object CreateArray(
            sol::this_state state,
            const sol::object& /*unused*/,
            const sol::object& size,
            std::optional<sol::object> arrayType);

        [[nodiscard]] static sol::object CreateStruct(
            sol::this_state state,
            const sol::object& /*unused*/,
            std::optional<sol::object> structType);

        template <EPropertyType type>
        [[nodiscard]] static sol::object CreatePrimitiveType(sol::this_state state, const sol::object& /*unused*/)
        {
            return sol::object(state, sol::in_place_type<InterfaceTypeInfo>, InterfaceTypeInfo{ type, 0u, sol::nil });
        }
    };
}
