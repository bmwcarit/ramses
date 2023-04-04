//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/LuaInterface.h"
#include "ramses-logic/Property.h"

#include <string>
#include <cassert>

/**
 * This example demonstrates how to create interfaces and link its properties to the properties of scripts
 */

int main()
{
    rlogic::LogicEngine logicEngine;

    // Create an interface which could represent a scene with a node that should be translated
    const std::string_view interfaceSrc = R"(
        function interface(inout_params)
            inout_params.translation_x = Type:Int32()
        end
    )";

    //create another script that could read the values of the interface and do some logic based on it
    const std::string_view wheelTranslationScriptSrc = R"(
        function interface(IN, OUT)
            IN.translation_x = Type:Int32()
            OUT.translation_vec = Type:Vec3i()
        end

        function run(IN, OUT)
            OUT.translation_vec = { IN.translation_x, 20, 30}
        end
    )";


    // Create a script and an interface using the Lua source code from above
    rlogic::LuaScript* wheelRotationScript = logicEngine.createLuaScript(wheelTranslationScriptSrc);
    rlogic::LuaInterface* intf = logicEngine.createLuaInterface(interfaceSrc, "TranslateInterface");

    // Create a link between the property of the interface and the input property of the wheel rotation script
    logicEngine.link(
        //NOTICE: the interface has 'translation_x' both as input and as output
        *intf->getOutputs()->getChild("translation_x"),                         // Get data from this property...
        *wheelRotationScript->getInputs()->getChild("translation_x"));          // ... and provide it to this property

    // Let's initialize the interface's input with some value
    intf->getInputs()->getChild("translation_x")->set<int32_t>(42);

    // Above links will cause the script to be executed, its output's data to be updated
    // and provided to the input of the interface
    logicEngine.update();

    // New results, based on the input value of the logic script and the IN/OUT links of the interface
    assert(42 == *intf->getOutputs()->getChild("translation_x")->get<int32_t>());
    assert(42 == *wheelRotationScript->getInputs()->getChild("translation_x")->get<int32_t>());
    assert(42 == (*wheelRotationScript->getOutputs()->getChild("translation_vec")->get<rlogic::vec3i>())[0]);

    return 0;
}
