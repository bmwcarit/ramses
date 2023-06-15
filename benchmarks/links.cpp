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
#include "impl/LogicEngineImpl.h"
#include "ramses-logic/Property.h"

#include "fmt/format.h"

namespace ramses
{
    static void BM_Links_CreateDestroyLink(benchmark::State& state)
    {
        LogicEngine logicEngine;

        const int64_t propertyCount = state.range(0);

        const std::string scriptSrc = fmt::format(R"(
            function interface(IN,OUT)
                for i = 0,{},1 do
                    IN["target"..tostring(i)] = Type:Int32()
                    OUT["src"..tostring(i)] = Type:Int32()
                end
            end
            function run(IN,OUT)
            end
        )", propertyCount);

        LuaConfig config;
        config.addStandardModuleDependency(EStandardModule::Base);

        LuaScript* srcScript = logicEngine.createLuaScript(scriptSrc, config);
        LuaScript* destScript = logicEngine.createLuaScript(scriptSrc, config);
        const Property* from = srcScript->getOutputs()->getChild("src0");
        Property* to = destScript->getInputs()->getChild("target0");

        logicEngine.m_impl->disableTrackingDirtyNodes();
        for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores) False positive
        {
            logicEngine.link(*from, *to);
            logicEngine.unlink(*from, *to);
            logicEngine.update();
        }
    }

    // Measures time to create and immediately destroy a link between two scripts.
    // The benchmark does both (create AND destroy) to ensure that the context is the same for the
    // entire measurement, as opposed to when creating new links and incrementing the total link count
    // ARG: property count in the script (measures decrease in link time depending on number of properties per script)
    BENCHMARK(BM_Links_CreateDestroyLink)->Arg(1)->Arg(10)->Arg(100)->Arg(1000);

    static void BM_Links_CreateDestroyLink_ManyScripts(benchmark::State& state)
    {
        LogicEngine logicEngine;

        const std::size_t scriptCount = static_cast<std::size_t>(state.range(0));

        const std::string scriptSrc = R"(
            function interface(IN,OUT)
                for i = 0,20,1 do
                    IN["dest"..tostring(i)] = Type:Int32()
                    OUT["src"..tostring(i)] = Type:Int32()
                end
            end
            function run(IN,OUT)
            end
        )";

        LuaConfig config;
        config.addStandardModuleDependency(EStandardModule::Base);

        std::vector<LuaScript*> scripts(scriptCount);
        for (std::size_t i = 0; i < scriptCount; ++i)
        {
            scripts[i] = logicEngine.createLuaScript(scriptSrc, config);

            if (i >= 1)
            {
                for (std::size_t link = 0; link < 20; ++link)
                {
                    auto target = scripts[i]->getInputs()->getChild(fmt::format("dest{}", link));
                    auto src = scripts[i-1]->getOutputs()->getChild(fmt::format("src{}", link));
                    logicEngine.link(*src, *target);
                }
            }
        }

        const std::size_t halfwayIndex = scriptCount/2;
        const Property* srcPropertyInTheMiddle  = scripts[halfwayIndex]->getOutputs()->getChild("src10");
        const Property* destPropertyInTheMiddle = scripts[halfwayIndex+1]->getInputs()->getChild("dest10");

        logicEngine.m_impl->disableTrackingDirtyNodes();
        for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores) False positive
        {
            logicEngine.unlink(*srcPropertyInTheMiddle, *destPropertyInTheMiddle);
            logicEngine.link(*srcPropertyInTheMiddle, *destPropertyInTheMiddle);
            logicEngine.update();
        }
    }

    // Same as BM_Links_CreateDestroyLink, but tests with many scripts (how fast is link (re)creation depending on scripts count)
    // ARG: script count
    BENCHMARK(BM_Links_CreateDestroyLink_ManyScripts)->Arg(8)->Arg(32)->Arg(128);
}

