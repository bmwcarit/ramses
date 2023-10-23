//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/ramses-client.h"

#include <cassert>
#include <iostream>

/**
 * This example shows how to deal with runtime errors in Lua scripts.
 */
int main()
{
    ramses::RamsesFramework framework{ ramses::RamsesFrameworkConfig{ ramses::EFeatureLevel_Latest } };
    ramses::RamsesClient* client = framework.createClient("client");
    ramses::Scene* scene = client->createScene(ramses::sceneId_t{ 123u });
    ramses::LogicEngine& logicEngine{ *scene->createLogicEngine() };

    /**
     * This script contains a runtime error, i.e. from Lua point of view this is
     * valid syntax, but the type check of the Logic engine will fire a runtime
     * error for trying to assign a string to a Type:Vec4f() property
     */
    ramses::LuaScript* faultyScript = logicEngine.createLuaScript(R"(
        function interface(IN,OUT)
            OUT.vec4f = Type:Vec4f()
        end

        function run(IN,OUT)
            OUT.vec4f = "this is not a table with 4 floats and will trigger a runtime error!"
        end
    )", {}, "faultyScript");

    // Script is successfully created, as it is syntactically correct
    assert(faultyScript != nullptr);

    /**
     * Update the logic engine including the faulty script above.
     * Because there is a runtime error in the script, the execution will return "false" and print
     * the error alongside stacktrace in the logs.
     * The stack trace is coming from the Lua VM and has limited information on the error. See
     * https://ramses-logic.readthedocs.io/en/latest/lua_syntax.html#the-global-in-and-out-objects for more information
     * how to read the stack trace
     */
    const bool status = logicEngine.update();

    // Ramses keeps last error that occurred during some API call until the getLastError is called, then the error is cleared.
    // Use it to get the error message and a pointer to the object which caused the error.
    const auto issue = framework.getLastError();
    assert(
        !status &&
        issue.has_value() &&
        issue->object == faultyScript);

    logicEngine.destroy(*faultyScript);

    return status ? -1 : 0;
}
