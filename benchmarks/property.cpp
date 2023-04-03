//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "benchmark/benchmark.h"

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/Property.h"

#include "impl/LogicEngineImpl.h"
#include "fmt/format.h"

namespace rlogic
{
    static void BM_Property_SetIntValue(benchmark::State& state)
    {
        LogicEngine logicEngine;

        const int64_t propertyCount = state.range(0);

        const std::string scriptSrc = fmt::format(R"(
            function interface(IN,OUT)
                for i = 0,{},1 do
                    IN["param"..tostring(i)] = Type:Int32()
                end
            end
            function run(IN,OUT)
            end
        )", propertyCount);

        LuaConfig config;
        config.addStandardModuleDependency(EStandardModule::Base);
        LuaScript* script = logicEngine.createLuaScript(scriptSrc, config);
        Property* property = script->getInputs()->getChild("param0");
        // Need different value, otherwise triggers internal caching (can't disable value check)
        int32_t increasingValue = 0;

        for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores) False positive
        {
            property->set<int32_t>(increasingValue++);
        }
    }

    // Measures time to set the value of a property to script based on how many properties are there in the script's interface()
    // ARG: how many properties are in the script's interface
    BENCHMARK(BM_Property_SetIntValue)->Arg(10)->Arg(100)->Arg(1000);
}


