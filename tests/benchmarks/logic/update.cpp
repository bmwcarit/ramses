//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "benchmarksetup.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/Property.h"

#include "impl/logic/LogicEngineImpl.h"
#include "fmt/format.h"

namespace ramses
{
    static void BM_Update_AssignProperty(benchmark::State& state)
    {
        BenchmarkSetUp setup;
        auto& logicEngine = setup.m_logicEngine;

        const int64_t loopCount = state.range(0);

        const std::string scriptSrc = fmt::format(R"(
            function interface(IN,OUT)
                IN.param = Type:Int32()
                OUT.param = Type:Int32()
            end
            function run(IN,OUT)
                for i = 0,{},1 do
                    OUT.param = IN.param
                end
            end
        )", loopCount);

        logicEngine.createLuaScript(scriptSrc);

        logicEngine.impl().disableTrackingDirtyNodes();
        for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores) False positive
        {
            logicEngine.update();
        }
    }

    // Measures update() speed based on how many properties are assigned in the script's run() method
    // Dirty handling: off
    // ARG: how many assignments are made in run() to the same property
    BENCHMARK(BM_Update_AssignProperty)->Arg(1)->Arg(10)->Arg(100)->Arg(1000);

    static void BM_Update_AssignStruct(benchmark::State& state)
    {
        BenchmarkSetUp setup;
        auto& logicEngine = setup.m_logicEngine;

        const int64_t loopCount = state.range(0);

        const std::string scriptSrc = fmt::format(R"(
            function interface(IN,OUT)
                IN.struct = {{
                    int = Type:Int32(),
                    float = Type:Float(),
                    vec4f = Type:Vec4f(),
                    nested = {{
                        int = Type:Int32(),
                        float = Type:Float(),
                        vec4f = Type:Vec4f()
                    }}
                }}
                OUT.struct = {{
                    int = Type:Int32(),
                    float = Type:Float(),
                    vec4f = Type:Vec4f(),
                    nested = {{
                        int = Type:Int32(),
                        float = Type:Float(),
                        vec4f = Type:Vec4f()
                    }}
                }}
            end
            function run(IN,OUT)
                for i = 0,{},1 do
                    OUT.struct = IN.struct
                end
            end
        )", loopCount);

        logicEngine.createLuaScript(scriptSrc);

        logicEngine.impl().disableTrackingDirtyNodes();
        for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores) False positive
        {
            logicEngine.update();
        }
    }

    // Same as BM_Update_AssignProperty, but with structs
    BENCHMARK(BM_Update_AssignStruct)->Arg(1)->Arg(10)->Arg(100)->Arg(1000)->Unit(benchmark::kMicrosecond);

    static void BM_Update_AssignArray(benchmark::State& state)
    {
        BenchmarkSetUp setup;
        auto& logicEngine = setup.m_logicEngine;

        const int64_t loopCount = state.range(0);

        const std::string scriptSrc = fmt::format(R"(
            function interface(IN,OUT)
                IN.array = Type:Array(255, Type:Vec4f())
                OUT.array = Type:Array(255, Type:Vec4f())
            end
            function run(IN,OUT)
                for i = 0,{},1 do
                    OUT.array = IN.array
                end
            end
        )", loopCount);

        logicEngine.createLuaScript(scriptSrc);

        logicEngine.impl().disableTrackingDirtyNodes();
        for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores) False positive
        {
            logicEngine.update();
        }
    }

    // Same as BM_Update_AssignProperty, but with arrays
    BENCHMARK(BM_Update_AssignArray)->Arg(1)->Arg(10)->Arg(100)->Arg(1000)->Unit(benchmark::kMicrosecond);

    // Test that update is faster when fewer scripts have to be updated (i.e. the dirty handling works
    // as expected). This benchmark creates a static list of linearly linked scripts so that if the
    // first in the list has its 'dirty_trigger' value changed, all scripts in the change will be
    // triggered for re-execution, whereas setting the trigger on the last script will only have the
    // last script executed
    // Read the results like this: the higher the arg (0 .. 99) the faster the update should be, ideally
    // when the arg is close to 99, the time to update should be close to zero
    static void BM_Update_IsFasterWithFewerDirtyScripts(benchmark::State& state)
    {
        BenchmarkSetUp setup;
        auto& logicEngine = setup.m_logicEngine;

        const auto scriptToSetDirty = static_cast<std::size_t>(state.range(0));
        constexpr int64_t scriptCount = 100;

        const std::string scriptSrc = R"(
            function interface(IN,OUT)
                IN.dirty_trigger = Type:Int32()
                IN.lots_of_data = {}
                OUT.lots_of_data = {}
                for i = 0,10,1 do
                    IN.lots_of_data["arr"..tostring(i)] = Type:Array(10, Type:Int32())
                    OUT.lots_of_data["arr"..tostring(i)] = Type:Array(10, Type:Int32())
                end
            end
            function run(IN,OUT)
                -- heavy data operation (deep copy of struct of arrays)
                OUT.lots_of_data = IN.lots_of_data
                -- trigger the dirty mechanism by updating one of the values based on the 'dirty trigger'
                OUT.lots_of_data.arr0[1] = IN.lots_of_data.arr0[1] + IN.dirty_trigger
            end
        )";

        LuaConfig config;
        config.addStandardModuleDependency(EStandardModule::Base);

        // To make sure there were no API errors
        bool success = false;
        (void)success;
        std::vector<LuaScript*> scripts(scriptCount);
        for (std::size_t i = 0; i < scriptCount; ++i)
        {
            scripts[i] = logicEngine.createLuaScript(scriptSrc, config, fmt::format("script{}", i));

            if (i >= 1)
            {
                for (std::size_t link = 0; link < 10; ++link)
                {
                    auto target = scripts[i]->getInputs()->getChild("lots_of_data")->getChild(fmt::format("arr{}", link));
                    auto src = scripts[i - 1]->getOutputs()->getChild("lots_of_data")->getChild(fmt::format("arr{}", link));

                    for (std::size_t array_element = 0; array_element < 10; ++array_element)
                    {
                        success = logicEngine.link(*src->getChild(array_element), *target->getChild(array_element));
                        assert(success);
                    }
                }
            }
        }

        Property* dirtyTrigger = scripts[scriptToSetDirty]->getInputs()->getChild("dirty_trigger");
        int32_t valueForDirtyTriggering = 1;
        for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores) False positive
        {
            success = dirtyTrigger->set<int32_t>(valueForDirtyTriggering++);
            assert(success);
            success = logicEngine.update();
            assert(success);
        }
    }

    BENCHMARK(BM_Update_IsFasterWithFewerDirtyScripts)->Arg(0)->Arg(49)->Arg(99)->Unit(benchmark::kMillisecond);
}


