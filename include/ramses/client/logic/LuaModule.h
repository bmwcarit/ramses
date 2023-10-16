//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/logic/LogicObject.h"

namespace ramses::internal
{
    class LuaModuleImpl;
}

namespace ramses
{
    /**
    * #LuaModule represents Lua source code in form of a reusable Lua module
    * which can be used in Lua scripts of one or more #ramses::LuaScript instances. Lua modules can
    * be also declared as dependency to other modules.
    * Lua modules are expected to follow these guidelines https://www.tutorialspoint.com/lua/lua_modules.htm
    * and have some limitations:
    *     - the name of the module given when creating it is only used to differentiate between #LuaModule
    *       instances on Ramses logic API, not to access them from a script or to resolve a file on a filesystem
    *     - the module is not supposed to be resolved with the 'require' keyword in Lua scripts where used
    *       (due to security concerns) but must be provided explicitly when creating #ramses::LuaScript instance
    *     - modules are read-only in order to prevent data races when accessed from different scripts with undefined
    *       order of execution
    * #ramses::LuaModule source code is loaded into its own Lua environment and is accessible in other #ramses::LuaScript
    * and/or #LuaModule instances in their own environments under the alias name given when creating those.
    *
    * #LuaModule can also be used to help provide property types for #ramses::LuaScript interface declarations,
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
    class RAMSES_API LuaModule : public LogicObject
    {
    public:
        /**
         * Get the internal data for implementation specifics of #LuaModule.
         */
        [[nodiscard]] internal::LuaModuleImpl& impl();

        /**
         * Get the internal data for implementation specifics of #LuaModule.
         */
        [[nodiscard]] const internal::LuaModuleImpl& impl() const;

    protected:
        /**
        * Internal constructor of #LuaModule. Use #ramses::LogicEngine::createLuaModule for user code.
        *
        * @param impl implementation details of the #LuaModule
        */
        explicit LuaModule(std::unique_ptr<internal::LuaModuleImpl> impl) noexcept;

        /**
         * Implementation detail of #LuaModule
         */
        internal::LuaModuleImpl& m_impl;

        friend class internal::ApiObjects;
    };
}
