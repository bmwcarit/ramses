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
 * This example demonstrates how indexed properties work (vectors and arrays)
 */

int main()
{
    rlogic::LogicEngine logicEngine;

    // A script which demonstrates how to access vector and array properties
    rlogic::LuaScript* script = logicEngine.createLuaScript(R"(
        function interface(IN,OUT)
            IN.vec3f = Type:Vec3f()

            OUT.array = Type:Array(9, Type:Float())
            OUT.vec4f = Type:Vec4f()
        end

        function run(IN,OUT)
            -- Can access vec3f components by index. Beware Lua indexing convention starts at 1!
            print("From inside Lua: vec3f = [" .. IN.vec3f[1] .. ", " .. IN.vec3f[2] .. ", " .. IN.vec3f[3] .. "]")

            -- Can assign array values as a Lua table
            -- Must specify all array values, otherwise expect runtime errors!
            OUT.array = {1, 2, 3, 4, 5, 6, 7, 8, 9}

            -- Can assign vec4f as if it was an array of size 4
            -- Note: you can't assign a single vecXY component - you have to set all of them atomically
            -- When using this notation, you can reorder indices, but ultimately all vecNt types must have
            -- exactly N[2|3|4] components of type t[i|f] distributed over indices 1..N
            OUT.vec4f = { 11.0, 12.0, 13.0, 14.0 }

            -- This is equivalent to the above statement, but with shuffled indices
            OUT.vec4f = {
                [4] = 14.0,
                [1] = 11.0,
                [2] = 12.0,
                [3] = 13.0
            }
        end
    )");

    /**
     * Set some data on the inputs
     * Note that with this notation (using C++ initializer lists) it is possible to accidentally
     * provide fewer entries than the vector expects. This will result in zeros filling the unspecified slots
     */
    script->getInputs()->getChild("vec3f")->set<rlogic::vec3f>({ 0.1f, 0.2f, 0.3f });

    // Update the logic engine including our script
    logicEngine.update();

    // Inspect the results of the script
    const rlogic::Property* arrayProperty = script->getOutputs()->getChild("array");
    const rlogic::vec4f vec4f = *script->getOutputs()->getChild("vec4f")->get<rlogic::vec4f>();

    for (size_t i = 0; i < arrayProperty->getChildCount(); ++i)
    {
        std::cout << "array[" << i << "] == " << *arrayProperty->getChild(i)->get<float>() << "\n";
    }

    std::cout << "vec4f = ["
        << vec4f[0] << ", "
        << vec4f[1] << ", "
        << vec4f[2] << ", "
        << vec4f[3] << "]\n";

    logicEngine.destroy(*script);

    return 0;
}
