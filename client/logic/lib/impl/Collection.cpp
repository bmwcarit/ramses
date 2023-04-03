//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/Collection.h"
#include "ramses-logic/LuaScript.h"
#include <type_traits>

namespace rlogic
{
    // Make sure collections have no hidden side effects
    // Excluded on purpose: default-construct, direct assignment
    static_assert(std::is_trivially_copy_assignable_v<Collection<LuaScript>>);
    static_assert(std::is_trivially_copy_constructible_v<Collection<LuaScript>>);
    static_assert(std::is_trivially_copyable_v<Collection<LuaScript>>);
    static_assert(std::is_trivially_destructible_v<Collection<LuaScript>>);
    static_assert(std::is_trivially_move_assignable_v<Collection<LuaScript>>);
    static_assert(std::is_trivially_move_constructible_v<Collection<LuaScript>>);
}
