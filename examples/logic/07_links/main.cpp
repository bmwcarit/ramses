//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/Property.h"

#include <string>
#include <cassert>

/**
 * This example demonstrates how to link properties of different scripts to
 * create a network of logic nodes executed in a specific order
 */

int main()
{
    rlogic::LogicEngine logicEngine;

    // Create a simple script which increments an integer and prints the result
    const std::string_view scriptSrc = R"(
        function interface(IN,OUT)
            IN.script_name = Type:String()

            IN.number = Type:Int32()
            OUT.incremented_number = Type:Int32()
        end

        function run(IN,OUT)
            -- add 1 to the input and assign to the output
            OUT.incremented_number = IN.number + 1

            -- Print the name of the script currently being executed and the result
            print("Executing: " .. IN.script_name .. ". Result is: " .. OUT.incremented_number)
        end
    )";

    // Enable the Lua base library so the print() function will be available in the script
    rlogic::LuaConfig config;
    config.addStandardModuleDependency(rlogic::EStandardModule::Base);

    // Create two scripts using the Lua source code from above
    rlogic::LuaScript* script1 = logicEngine.createLuaScript(scriptSrc, config);
    rlogic::LuaScript* script2 = logicEngine.createLuaScript(scriptSrc, config);

    // Assign the scripts their names so that we can see their execution order
    script1->getInputs()->getChild("script_name")->set<std::string>("script 1");
    script2->getInputs()->getChild("script_name")->set<std::string>("script 2");

    // Scripts will be executed at arbitrary order (they are not linked yet!)
    logicEngine.update();

    // Both scripts will produce 1 as a result (add 1 to 0 -> results to 1)
    assert(1 == *script1->getOutputs()->getChild("incremented_number")->get<int32_t>());
    assert(1 == *script2->getOutputs()->getChild("incremented_number")->get<int32_t>());

    // Create a link between the output property (of script1) and the input property (of script2)
    logicEngine.link(
        *script1->getOutputs()->getChild("incremented_number"),     // Get data from this property...
        *script2->getInputs()->getChild("number"));                 // ... and provide it to this property

    // Let's initialize script1's input with a new number
    script1->getInputs()->getChild("number")->set<int32_t>(42);

    // Above link will cause script1 to be executed first now, its output's data
    // provided to the input of script2, then script2 will be executed
    logicEngine.update();

    // New results are different, based on the new input value of script1 and the link between the scripts
    assert(43 == *script1->getOutputs()->getChild("incremented_number")->get<int32_t>());
    assert(44 == *script2->getOutputs()->getChild("incremented_number")->get<int32_t>());

    return 0;
}
