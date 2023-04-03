//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/Property.h"

#include <iostream>
#include <cassert>

/**
 * This example demonstrates more complex data structures and possible
 * ways to interact with them
 */

int main()
{
    rlogic::LogicEngine logicEngine;

    // Create a script with inputs and outputs of the same type (consists of nested structs)
    rlogic::LuaScript* script = logicEngine.createLuaScript(R"(
        function interface(IN,OUT)
            IN.struct = {
                nested = {
                    data1 = Type:String(),
                    data2 = Type:Int32()
                }
            }

            OUT.struct = {
                nested = {
                    data1 = Type:String(),
                    data2 = Type:Int32()
                }
            }
        end

        function run(IN,OUT)
            -- can assign whole structs if both sides are of compatible types
            OUT.struct = IN.struct

            -- Also possible to assign a 'nested' struct (recursively assigns all sub-fields)
            OUT.struct.nested = IN.struct.nested

            -- These assignments would result in error, because types don't match.
            -- OUT.struct = IN.struct.nested
            -- OUT.struct.nested = IN.struct
            -- OUT.struct.nested = IN
        end
    )");

    // Set some data on the inputs
    script->getInputs()->getChild("struct")->getChild("nested")->getChild("data1")->set<std::string>("hello world");
    script->getInputs()->getChild("struct")->getChild("nested")->getChild("data2")->set<int32_t>(42);

    // Update the logic engine including our script
    logicEngine.update();

    // Inspect the results of the script
    std::string data1 = *script->getOutputs()->getChild("struct")->getChild("nested")->getChild("data1")->get<std::string>();
    int32_t data2 = *script->getOutputs()->getChild("struct")->getChild("nested")->getChild("data2")->get<int32_t>();

    std::cout << "data1 value: " << data1 << "\n";
    std::cout << "data2 value: " << data2 << "\n";

    logicEngine.destroy(*script);

    return 0;
}
