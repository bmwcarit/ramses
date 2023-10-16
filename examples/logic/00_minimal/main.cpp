//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <iostream>

#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/ramses-client.h"

/**
 * This example is designed to be minimal, but still provide insight into
 * the concepts and function of the Ramses logic library.
 */
int main()
{
    /**
     * Create an instance of the LogicEngine class. This holds all data
     * and offers methods to load and execute scripts among other things.
     * We need to instantiate the basic components of Ramses ecosystem first,
     * start with framework, then client and then scene which will be the owner
     * of the logic instance.
     */
    ramses::RamsesFramework framework{ ramses::RamsesFrameworkConfig{ ramses::EFeatureLevel_Latest } };
    ramses::RamsesClient* client = framework.createClient("client");
    ramses::Scene* scene = client->createScene(ramses::sceneId_t{ 123u });
    ramses::LogicEngine& logicEngine{ *scene->createLogicEngine() };

    /**
     * Create a script by providing the source code of the script with a string.
     * Each script needs an "interface" and "run" function
     * Inside the "interface" function, you can define all inputs and
     * outputs of the script.
     * The run function contains the real code, which is executed during runtime
     */
    ramses::LuaScript* script = logicEngine.createLuaScript(R"(
        function interface(IN,OUT)
            IN.rotate_x = Type:Float()
            OUT.rotation = Type:Vec3f()
        end
        function run(IN,OUT)
            OUT.rotation = {IN.rotate_x, 90, 180}
        end
    )");

    /**
     * Set the input of a script to some value
     */
    script->getInputs()->getChild("rotate_x")->set<float>(30.0f);

    /**
     * Update the state of the logic engine. There is a single script which
     * will be executed by invoking its run() method
     */
    logicEngine.update();

    /**
     * Inspect the result of the script by getting the value of its single output
     */
    std::cout << "Script ran successfully! The result is: ["
        << (*script->getOutputs()->getChild("rotation")->get<ramses::vec3f>())[0] << ", "
        << (*script->getOutputs()->getChild("rotation")->get<ramses::vec3f>())[1] << ", "
        << (*script->getOutputs()->getChild("rotation")->get<ramses::vec3f>())[2] << "]";

    /**
     * Destroy all created objects, in this case a single script. In this example
     * the script would be destroyed anyway with the logicEngine going out of scope,
     * but in a real-world application we should always take care of object lifecycles.
     */
    logicEngine.destroy(*script);

    return 0;
}
