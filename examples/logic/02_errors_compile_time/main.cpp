//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/LogicEngine.h"
#include <cassert>
#include <iostream>

/**
 * This example shows how to deal with Lua compilation errors.
 */

int main()
{
    rlogic::LogicEngine logicEngine;

    /**
     * Try to compile a script which has invalid Lua syntax
     * Giving a name to the script helps identify the source of the issue
     */
    rlogic::LuaScript* faultyScript = logicEngine.createLuaScript(R"(
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
         * To get further information about the issue, fetch errors from LogicEngine
         * Note that this list will be reset with the next API call to logicEngine!
         */
        auto errorList = logicEngine.getErrors();
        assert(!errorList.empty());

        // Print out the error information
        for (auto& error : errorList)
        {
            /**
             * Note that this error has no stack trace, because there is no stack - the script failed compiling
             * Furthermore, the source code line indication is also "Lua style" - it starts at 1, not 0
             */
            std::cout << error.message << std::endl;
        }
    }

    return 0;
}
