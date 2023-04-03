//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <iostream>
#include <vector>

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/DataArray.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/Property.h"
#include "ramses-logic/EPropertyType.h"

int main()
{
    std::cout << "Start ramses-logic-shared-lib-check\n";
    rlogic::LogicEngine logicEngine;
    logicEngine.createLuaScript(R"(
        function interface(IN,OUT)
            IN.int = Type:Int32()
            OUT.float = Type:Float()
        end

        function run(IN,OUT)
            OUT.float = IN.int + 0.5
        end
    )", {}, "aScript");

    // Test that type cast works
    auto script = logicEngine.findByName<rlogic::LogicObject>("aScript")->as<rlogic::LuaScript>();

    script->getInputs()->getChild("int")->set<int32_t>(5);
    const auto dataArray = logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f }, "dataarray");
    const auto arrayData = dataArray->getData<float>();
    (void) arrayData;

    logicEngine.update();

    std::cout << "Result of script was: " << *script->getOutputs()->getChild("float")->get<float>() << "\n";
    std::cout << "Type of script input 'IN.int' is: " << GetLuaPrimitiveTypeName(rlogic::PropertyTypeToEnum<int>::TYPE) << "\n";

    printf("End ramses-logic-shared-lib-check\n");
    return 0;
}
