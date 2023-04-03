//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/Iterator.h"
#include "ramses-logic/LuaScript.h"

#include <vector>

namespace rlogic
{
    using IteratorTypeForStaticAsserts = Iterator<LuaScript, std::vector<std::unique_ptr<LuaScript>>, false>;
    using ConstIteratorTypeForStaticAsserts = Iterator<LuaScript, std::vector<std::unique_ptr<LuaScript>>, true>;

    // Assert that const type != non-const type
    static_assert(!std::is_same_v<IteratorTypeForStaticAsserts, ConstIteratorTypeForStaticAsserts>);

    // Check standard properties of non-const iterator
    static_assert(std::is_copy_assignable_v<IteratorTypeForStaticAsserts>);
    static_assert(std::is_copy_constructible_v<IteratorTypeForStaticAsserts>);
    static_assert(std::is_destructible_v<IteratorTypeForStaticAsserts>);
    static_assert(std::is_move_assignable_v<IteratorTypeForStaticAsserts>);
    static_assert(std::is_move_constructible_v<IteratorTypeForStaticAsserts>);

    // Check standard properties of const iterator
    static_assert(std::is_copy_assignable_v<ConstIteratorTypeForStaticAsserts>);
    static_assert(std::is_copy_constructible_v<ConstIteratorTypeForStaticAsserts>);
    static_assert(std::is_destructible_v<ConstIteratorTypeForStaticAsserts>);
    static_assert(std::is_move_assignable_v<ConstIteratorTypeForStaticAsserts>);
    static_assert(std::is_move_constructible_v<ConstIteratorTypeForStaticAsserts>);
}
