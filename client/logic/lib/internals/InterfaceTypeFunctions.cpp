//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internals/InterfaceTypeFunctions.h"
#include "internals/SolHelper.h"

namespace rlogic::internal
{

    sol::object InterfaceTypeFunctions::CreateArray(sol::this_state state, const sol::object& /*unused*/, const sol::object& size, std::optional<sol::object> arrayType)
    {
        const DataOrError<size_t> potentialUInt = LuaTypeConversions::ExtractSpecificType<size_t>(size);
        if (potentialUInt.hasError())
        {
            sol_helper::throwSolException("Type:Array(N, T) invoked with bad size argument! {}", potentialUInt.getError());
        }
        // TODO Violin revisit max array size
        // Putting a "sane" number here, but maybe worth reconsidering the limit again
        const size_t arraySize = potentialUInt.getData();
        if (arraySize == 0u || arraySize > MaxArrayPropertySize)
        {
            sol_helper::throwSolException("Type:Array(N, T) invoked with invalid size parameter N={} (must be in the range [1, {}])!", arraySize, MaxArrayPropertySize);
        }
        if (!arrayType)
        {
            sol_helper::throwSolException("Type:Array(N, T) invoked with invalid type parameter T!");
        }
        return sol::object(state, sol::in_place_type<InterfaceTypeInfo>, InterfaceTypeInfo{ EPropertyType::Array, arraySize, *arrayType });
    }

    sol::object InterfaceTypeFunctions::CreateStruct(sol::this_state state, const sol::object& /*unused*/, std::optional<sol::object> structType)
    {
        if (!structType)
        {
            sol_helper::throwSolException("Type:Struct(T) invoked with invalid type parameter T!");
        }
        return sol::object(state, sol::in_place_type<InterfaceTypeInfo>, InterfaceTypeInfo{ EPropertyType::Struct, 0, *structType });
    }
}
