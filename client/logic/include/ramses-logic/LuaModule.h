//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/LogicObject.h"

namespace rlogic::internal
{
    class LuaModuleImpl;
}

namespace rlogic
{
    /**
    * #LuaModule represents Lua source code in form of a reusable Lua module
    * which can be used in Lua scripts of one or more #rlogic::LuaScript instances. Lua modules can
    * be also declared as dependency to other modules.
    * Lua modules are expected to follow these guidelines https://www.tutorialspoint.com/lua/lua_modules.htm
    * and have some limitations:
    *     - the name of the module given when creating it is only used to differentiate between #LuaModule
    *       instances on Ramses logic API, not to access them from a script or to resolve a file on a filesystem
    *     - the module is not supposed to be resolved with the 'require' keyword in Lua scripts where used
    *       (due to security concerns) but must be provided explicitly when creating #rlogic::LuaScript instance
    *     - modules are read-only in order to prevent data races when accessed from different scripts with undefined
    *       order of execution
    * #rlogic::LuaModule source code is loaded into its own Lua environment and is accessible in other #rlogic::LuaScript
    * and/or #LuaModule instances in their own environments under the alias name given when creating those.
    *
    * #LuaModule can also be used to help provide property types for #rlogic::LuaScript interface declarations,
    * for example a module with a 'struct' type:
    *   \code{.lua}
    *       local mytypes = {}
    *       function mytypes.mystruct()
    *           return Type:Struct({
    *               name = Type:String(),
    *               address = Type:Struct({
    *                   street = Type:String(),
    *                   number = Type:Int32()
    *               })
    *           })
    *       end
    *       return mytypes
    *   \endcode
    * And script using above module to define its interface:
    *   \code{.lua}
    *       modules("mytypes")  -- must declare the dependency explicitly
    *       function interface(IN,OUT)
    *           IN.input_struct = mytypes.mystruct()
    *           OUT.output_struct = mytypes.mystruct()
    *       end
    *   \endcode
    *
    * The label 'Type' is a reserved keyword and must not be overwritten for other purposes (e.g. name of variable or function).
    */
    class LuaModule : public LogicObject
    {
    public:
        /**
        * Destructor of #LuaModule
        */
        ~LuaModule() noexcept override;

        /**
        * Copy Constructor of #LuaModule is deleted because #LuaModule is not supposed to be copied
        */
        LuaModule(const LuaModule&) = delete;

        /**
        * Move Constructor of #LuaModule is deleted because #LuaModule is not supposed to be moved
        */
        LuaModule(LuaModule&&) = delete;

        /**
        * Assignment operator of #LuaModule is deleted because #LuaModule is not supposed to be copied
        */
        LuaModule& operator=(const LuaModule&) = delete;

        /**
        * Move assignment operator of #LuaModule is deleted because #LuaModule is not supposed to be moved
        */
        LuaModule& operator=(LuaModule&&) = delete;

        /**
        * Implementation detail of #LuaModule
        */
        internal::LuaModuleImpl& m_impl;

        /**
        * Internal constructor of #LuaModule. Use #rlogic::LogicEngine::createLuaModule for user code.
        *
        * @param impl implementation details of the #LuaModule
        */
        explicit LuaModule(std::unique_ptr<internal::LuaModuleImpl> impl) noexcept;
    };
}
