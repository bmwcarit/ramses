//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cstdint>

namespace rlogic
{
    /**
    * Enum which represents the different Lua standard modules. Used in #rlogic::LuaConfig::addStandardModuleDependency
    */
    enum class EStandardModule : int32_t
    {
        Base=0,     //< Basic functionality mapped to global space, e.g. ipairs(), pcall(), error(), assert()
        String,     //< The String module mapped to the Lua environment as a table named 'string'
        Table,      //< The Table module mapped to the Lua environment as a table named 'table'
        Math,       //< The Math module mapped to the Lua environment as a table named 'math'
        Debug,      //< The Debug module mapped to the Lua environment as a table named 'debug'
        All,        //< Use this to load all standard modules
    };
}
