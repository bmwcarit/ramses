//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "benchmarksetup.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/Property.h"
#include "internal/Core/Utils/RamsesLogger.h"

#include "fmt/format.h"
#include <fstream>

namespace ramses
{
    static auto CreateLargeLogicEngineBuffer(std::string_view fileName, std::size_t scriptCount)
    {
        ramses::internal::GetRamsesLogger().setConsoleLogLevel(ELogLevel::Off);

        BenchmarkSetUp setup;
        auto& logicEngine = setup.m_logicEngine;

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

        std::ignore = setup.m_scene.saveToFile(fileName, {});

        std::ifstream fileStream(std::string(fileName), std::ifstream::binary);
        fileStream.seekg(0, std::ios::end);

        const auto size = static_cast<size_t>(fileStream.tellg());
        std::unique_ptr<std::byte[], void(*)(const std::byte*)> byteBuffer(new std::byte[size], [](const auto* ptr) { delete[] ptr; });
        fileStream.seekg(0, std::ios::beg);
        fileStream.read(reinterpret_cast<char*>(byteBuffer.get()), static_cast<std::streamsize>(size));
        return std::pair{std::move(byteBuffer), size};
    }

    static void BM_LoadFromBuffer_WithVerifier(benchmark::State& state)
    {
        ramses::internal::GetRamsesLogger().setConsoleLogLevel(ELogLevel::Off);

        const auto scriptCount = static_cast<std::size_t>(state.range(0));

        auto buffer = CreateLargeLogicEngineBuffer("largeFile.bin", scriptCount);

        BenchmarkSetUp setup;
        SceneConfig config;
        config.setMemoryVerificationEnabled(true);
        for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores) False positive
        {
            setup.m_client.loadSceneFromMemory(std::move(buffer.first), buffer.second, config);
        }
    }

    // ARG: script count
    BENCHMARK(BM_LoadFromBuffer_WithVerifier)->Arg(8)->Arg(32)->Arg(128)->Unit(benchmark::kMicrosecond);

    static void BM_LoadFromBuffer_WithoutVerifier(benchmark::State& state)
    {
        ramses::internal::GetRamsesLogger().setConsoleLogLevel(ELogLevel::Off);

        const auto scriptCount = static_cast<std::size_t>(state.range(0));

        auto buffer = CreateLargeLogicEngineBuffer("largeFile.bin", scriptCount);

        BenchmarkSetUp setup;
        SceneConfig config;
        config.setMemoryVerificationEnabled(false);

        for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores) False positive
        {
            setup.m_client.loadSceneFromMemory(std::move(buffer.first), buffer.second, config);
        }
    }

    // ARG: script count
    BENCHMARK(BM_LoadFromBuffer_WithoutVerifier)->Arg(8)->Arg(32)->Arg(128)->Unit(benchmark::kMicrosecond);
}

