//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <iostream>

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/Property.h"
#include "ramses-logic/EPropertyType.h"

int main()
{
    std::cout << "Start ramses-logic-submodule-check\n";
    rlogic::LogicEngine logicEngine;
    rlogic::LuaScript* script = logicEngine.createLuaScript(R"(
        function interface(IN,OUT)
            IN.int = Type:Int32()
            OUT.float = Type:Float()
        end

        function run(IN,OUT)
            OUT.float = IN.int + 0.5
        end
    )");

    script->getInputs()->getChild("int")->set<int32_t>(5);

    logicEngine.update();

    std::cout << "Result of script was: " << *script->getOutputs()->getChild("float")->get<float>() << "\n";
    std::cout << "Type of script input 'IN.int' is: " << GetLuaPrimitiveTypeName(rlogic::PropertyTypeToEnum<int>::TYPE) << "\n";

    printf("End ramses-logic-submodule-check\n");
    return 0;
}
