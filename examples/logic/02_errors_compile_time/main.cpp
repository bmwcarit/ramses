//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/ramses-client.h"
#include <cassert>
#include <iostream>

/**
 * This example shows how to deal with Lua compilation errors.
 */

int main()
{
    ramses::RamsesFramework framework{ ramses::RamsesFrameworkConfig{ ramses::EFeatureLevel_Latest } };
    ramses::RamsesClient* client = framework.createClient("client");
    ramses::Scene* scene = client->createScene(ramses::sceneId_t{ 123u });
    ramses::LogicEngine& logicEngine{ *scene->createLogicEngine() };

    /**
     * Try to compile a script which has invalid Lua syntax
     * Giving a name to the script helps identify the source of the issue
     */
    ramses::LuaScript* faultyScript = logicEngine.createLuaScript(R"(
        function interface(IN,OUT)
            this.does.not.compile
        end
        function run(IN,OUT)
        end
    )");

    // script is nullptr because of the compilation error
    if (nullptr == faultyScript)
    {
        /**
         * To get further information about the issue, fetch last error from RamsesFramework.
         * Note that the error will be reset once getLastError is called.
         */
        const auto lastError = framework.getLastError();
        assert(lastError.has_value());

        /**
        * Note that this error has no stack trace, because there is no stack - the script failed compiling
        * Furthermore, the source code line indication is also "Lua style" - it starts at 1, not 0
        */
        std::cout << lastError->message << std::endl;
    }

    return 0;
}
