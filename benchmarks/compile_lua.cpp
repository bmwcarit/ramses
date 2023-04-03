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
#include "fmt/format.h"

namespace rlogic
{
    static void CompileLua(LogicEngine& logicEngine, std::string_view src, const LuaConfig& config)
    {
        LuaScript* script = logicEngine.createLuaScript(src, config);
        logicEngine.destroy(*script);
    }

    static void BM_CompileLua_Interface(benchmark::State& state)
    {
        LogicEngine logicEngine;

        LuaConfig config;
        config.addStandardModuleDependency(EStandardModule::Base);

        const int64_t scriptSize = state.range(0);

        const std::string scriptSrc = fmt::format(R"(
            function interface(IN,OUT)
                for i = 0,{},1 do
                    IN["param"..tostring(i)] = Type:Int32()
                end
            end
            function run(IN,OUT)
            end
        )", scriptSize);

        for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores) False positive
        {
            CompileLua(logicEngine, scriptSrc, config);
        }
    }

    // Measures compilation times depending on the number of inputs in the interface
    // ARG: number of inputs in script's interface()
    BENCHMARK(BM_CompileLua_Interface)->Arg(1)->Arg(10)->Arg(100);
}


