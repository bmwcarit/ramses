//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/LogicNode.h"

#include <string>
#include <memory>
#include <functional>

namespace rlogic::internal
{
    class LuaScriptImpl;
}

namespace rlogic
{
    class Property;

    using LuaPrintFunction = std::function<void(std::string_view scriptName, std::string_view message)>;

    /**
    * The LuaScript class is the cornerstone of RAMSES Logic as it encapsulates
    * a Lua script and the associated with it Lua environment. LuaScript instances are created
    * by the #rlogic::LogicEngine class.
    *
    * A LuaScript can be created from Lua source code which must fulfill following requirements:
    *
    * - valid Lua 5.1 syntax
    * - contains two global functions - interface(IN,OUT) and run(IN,OUT) with exactly two parameters (for inputs and outputs respectively) and no return values
    * - declares its inputs and outputs in the interface(IN,OUT) function, and its logic in the run(IN,OUT) function - you can use different names than IN/OUT
    * - the interface(IN,OUT) function declares zero or more inputs and outputs to the IN and OUT variables passed to the function
    * - inputs and outputs are declared like this:
    *   \code{.lua}
    *       function interface(IN,OUT)
    *           IN.input_name = Type:T()
    *           OUT.output_name = Type:T()
    *       end
    *   \endcode
    * - T is one of [Int32|Int64|Float|Bool|String|Vec2f|Vec3f|Vec4f|Vec2i|Vec3i|Vec4i|Struct|Array]
    * - For T=Struct:
    *       - The first and only argument to the function must be a table
    *       - The table keys must be strings, and the table values must be again declared with Type:T()
    * - For T=Array, the function declaration is of the form Type:Array(n, E) where:
    *     - n is a positive integer
    *     - E must be declared with Type:T()
    *     - E can be a Type:Struct(), i.e. arrays of structs are supported
    * - Each property must have a name (string) - other types like number, bool etc. are not supported as keys
    * - T can also be defined in a module, see #rlogic::LuaModule for details
    * - as a convenience abbreviation, you can use {...} instead of Type:Struct({...}) to declare structs
    * - You can optionally declare an init() function with no parameters
    *       - init() is allowed to write data or functions to a predefined GLOBAL table
    *       - init() will be executed exactly once after loading the script (also when loading from binaries)
    *       - the data from GLOBAL will be available to the interface() and run() functions
    *       - no other global data can be read or written in any of the functions or in the global scope of the script
    * - you may declare module dependencies using the modules() function
    *       - modules() accepts a vararg of strings for each module
    *       - the string provided will be the name under which the module will be available in other functions
    *       - See also #rlogic::LuaModule for more info on modules and their syntax
    *
    * Violating any of these requirements will result in errors, which can be obtained by calling
    * #rlogic::LogicEngine::getErrors().
    *
    * See also the full documentation at https://ramses-logic.readthedocs.io/en/latest/api.html for more details on Lua and
    * its interaction with C++.
    */
    class LuaScript : public LogicNode
    {
    public:

        /**
        * Constructor of LuaScript. User is not supposed to call this - script are created by other factory classes
        *
        * @param impl implementation details of the script
        */
        explicit LuaScript(std::unique_ptr<internal::LuaScriptImpl> impl) noexcept;

        /**
        * Destructor of LuaScript
        */
        ~LuaScript() noexcept override;

        /**
        * Copy Constructor of LuaScript is deleted because scripts are not supposed to be copied
        *
        * @param other script to copy from
        */
        LuaScript(const LuaScript& other) = delete;

        /**
        * Move Constructor of LuaScript is deleted because scripts are not supposed to be moved
        *
        * @param other script to move from
        */
        LuaScript(LuaScript&& other) = delete;

        /**
        * Assignment operator of LuaScript is deleted because scripts are not supposed to be copied
        *
        * @param other script to assign from
        */
        LuaScript& operator=(const LuaScript& other) = delete;

        /**
        * Move assignment operator of LuaScript is deleted because scripts are not supposed to be moved
        *
        * @param other script to move from
        */
        LuaScript& operator=(LuaScript&& other) = delete;

        /**
        * Implementation detail of LuaScript
        */
        internal::LuaScriptImpl& m_script;
    };
}
