//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/ramses-client.h"

#include <iostream>
#include <cassert>

/**
 * This example demonstrates how to use global variables in scripts
 */

int main()
{
    ramses::RamsesFramework framework{ ramses::RamsesFrameworkConfig{ ramses::EFeatureLevel_Latest } };
    ramses::RamsesClient* client = framework.createClient("client");
    ramses::Scene* scene = client->createScene(ramses::sceneId_t{ 123u });
    ramses::LogicEngine& logicEngine{ *scene->createLogicEngine() };

    // Create a script which initializes global variables in its init() method
    // They are used during the interface definition as well as at runtime
    ramses::LuaScript* script = logicEngine.createLuaScript(R"(
        function init()
            GLOBAL.outputType = Type:String()
            GLOBAL.outputName = "name"
            GLOBAL.outputValue = "MrAnderson"
        end

        function interface(IN,OUT)
            OUT[GLOBAL.outputName] = GLOBAL.outputType
        end

        function run(IN,OUT)
            OUT[GLOBAL.outputName] = GLOBAL.outputValue
        end
    )");

    // Update the logic engine (sets OUT.name = "MrAnderson")
    logicEngine.update();

    // Value comes from global variable
    assert(std::string("MrAnderson") == *script->getOutputs()->getChild("name")->get<std::string>());

    logicEngine.destroy(*script);

    return 0;
}
