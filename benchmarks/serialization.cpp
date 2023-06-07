//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "benchmark/benchmark.h"

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/Property.h"
#include "ramses-logic/Logger.h"

#include "fmt/format.h"
#include <fstream>

namespace ramses
{
    static std::vector<char> CreateLargeLogicEngineBuffer(std::string_view fileName, std::size_t scriptCount)
    {
        Logger::SetLogVerbosityLimit(ELogLevel::Off);

        LogicEngine logicEngine;

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
                    auto src = scripts[i - 1]->getOutputs()->getChild(fmt::format("src{}", link));
                    logicEngine.link(*src, *target);
                }
            }
        }

        SaveFileConfig configNoValidation;
        configNoValidation.setValidationEnabled(false);
        logicEngine.saveToFile(fileName, configNoValidation);

        std::ifstream fileStream(std::string(fileName), std::ifstream::binary);
        fileStream.seekg(0, std::ios::end);
        std::vector<char> byteBuffer(static_cast<size_t>(fileStream.tellg()));
        fileStream.seekg(0, std::ios::beg);
        fileStream.read(byteBuffer.data(), static_cast<std::streamsize>(byteBuffer.size()));
        return byteBuffer;
    }

    static void BM_LoadFromBuffer_WithVerifier(benchmark::State& state)
    {
        Logger::SetLogVerbosityLimit(ELogLevel::Off);

        const std::size_t scriptCount = static_cast<std::size_t>(state.range(0));

        const std::vector<char> buffer = CreateLargeLogicEngineBuffer("largeFile.bin", scriptCount);

        for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores) False positive
        {
            LogicEngine logicEngine;
            logicEngine.loadFromBuffer(buffer.data(), buffer.size(), nullptr, true);
        }
    }

    // ARG: script count
    BENCHMARK(BM_LoadFromBuffer_WithVerifier)->Arg(8)->Arg(32)->Arg(128)->Unit(benchmark::kMicrosecond);

    static void BM_LoadFromBuffer_WithoutVerifier(benchmark::State& state)
    {
        Logger::SetLogVerbosityLimit(ELogLevel::Off);

        const std::size_t scriptCount = static_cast<std::size_t>(state.range(0));

        const std::vector<char> buffer = CreateLargeLogicEngineBuffer("largeFile.bin", scriptCount);

        for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores) False positive
        {
            LogicEngine logicEngine;
            logicEngine.loadFromBuffer(buffer.data(), buffer.size(), nullptr, false);
        }
    }

    // ARG: script count
    BENCHMARK(BM_LoadFromBuffer_WithoutVerifier)->Arg(8)->Arg(32)->Arg(128)->Unit(benchmark::kMicrosecond);
}

