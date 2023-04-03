//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/LuaModule.h"
#include "ramses-logic/Property.h"

#include <iostream>
#include <cassert>

/**
 * This example demonstrates how to create reusable modules and embed them to scripts
 */

int main()
{
    rlogic::LogicEngine logicEngine;

    // Create a LuaConfig object which we use to configure how the module will be built.
    // In this example, we use the 'print' method, so we add the 'Base' standard Lua library
    rlogic::LuaConfig moduleConfig;
    moduleConfig.addStandardModuleDependency(rlogic::EStandardModule::Base);

    // Create a module which wraps Lua's print method and prints the name of the caller
    rlogic::LuaModule* myPrint = logicEngine.createLuaModule(R"(
        local myPrint = {}

        function myPrint.print(name)
            print("Hello, " .. name .. "!")
        end

        return myPrint
    )", moduleConfig);

    // Create a LuaConfig object which we use to configure how the module
    // shall be mapped to the script later (under the alias 'PrintModule')
    rlogic::LuaConfig scriptConfig;
    scriptConfig.addDependency("PrintModule", *myPrint);

    // Create a script which uses the custom print module. Notice that the script
    // declares its dependency to a PrintModule via the modules() function
    rlogic::LuaScript* script = logicEngine.createLuaScript(R"(
        -- The script must declare the modules it depends on
        -- The name here must match the alias provided in the LuaConfig above!
        modules('PrintModule')

        function interface(IN,OUT)
            IN.name = Type:String()
        end

        function run(IN,OUT)
            -- Calls the 'print' function packaged in the PrintModule
            PrintModule.print(IN.name)
        end
    )", scriptConfig, "ScriptWithModule");

    // Set the name input
    script->getInputs()->getChild("name")->set<std::string>("MrAnderson");

    // Update the logic engine (will print 'Hello MrAnderson')
    logicEngine.update();

    // Modules have to be destroyed after all scripts that reference them have been destroyed
    logicEngine.destroy(*script);
    logicEngine.destroy(*myPrint);

    return 0;
}
