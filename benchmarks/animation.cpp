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
#include "ramses-logic/AnimationNode.h"
#include "ramses-logic/AnimationNodeConfig.h"
#include "ramses-logic/AnimationTypes.h"
#include "fmt/format.h"

namespace rlogic
{
    const auto  animationIterations = 100;

    static void RunAnimation(LogicEngine& logicEngine, benchmark::State& state, Property* progressProp)
    {
        while (state.KeepRunning())
        {
            for (int i = 0; i < animationIterations; ++i)
            {
                progressProp->set(float(i) / animationIterations);
                if (!logicEngine.update())
                    state.SkipWithError("failure running update()");
            }
        }
    }

    static void RunScript(benchmark::State& state, const std::string& src)
    {
        LogicEngine logicEngine;
        LuaConfig config;
        config.addStandardModuleDependency(EStandardModule::Base);
        auto* script = logicEngine.createLuaScript(src, config);
        if (!script)
        {
            state.SkipWithError("Script creation failed");
            return;
        }
        auto* timeProp = script->getInputs()->getChild("time");

        while (state.KeepRunning())
        {
            auto i = animationIterations;
            float time = 0.0f;
            while ((i--) != 0)
            {
                time += 0.015f;
                timeProp->set(time);
                if (!logicEngine.update())
                {
                    state.SkipWithError("failure running update()");
                }
            }
        }
    }

    static void BM_AnimationScriptLinear(benchmark::State& state)
    {
        const std::string src = fmt::format(R"(
            function interface(IN,OUT)
                IN.time = Type:Float()
                OUT.value = Type:Vec3f()
            end
            function run(IN,OUT)
                local z = 0
                for i = 0,{},1 do
                    z = 360 * IN.time / 1.5
                    OUT.value = {{0, 0, z}}
                end
            end
        )", state.range(0));
        RunScript(state, src);
    }

    static void BM_AnimationScriptKeyframes(benchmark::State& state)
    {
        const std::string src = fmt::format(R"(
            function interface(IN,OUT)
                IN.time = Type:Float()
                OUT.value = Type:Vec3f()
            end
            function run(IN,OUT)
                local z = 0
                for i = 0,{},1 do
                    GLOBAL.pos = IN.time
                    if GLOBAL.pos < 0.5 then
                        z = 360 * GLOBAL.pos / 0.5
                    elseif GLOBAL.pos < 1.0 then
                        z = 180 - 80 * (GLOBAL.pos - 0.5) / 1.0
                    elseif GLOBAL.pos < 1.5 then
                        z = 100 + 260 * (GLOBAL.pos - 1) / 0.5
                    end
                    OUT.value = {{0, 0, z}}
                end
            end
        )", state.range(0));
        RunScript(state, src);
    }

    static void BM_AnimationLinear(benchmark::State& state)
    {
        LogicEngine logicEngine;
        const auto* animTimestamps = logicEngine.createDataArray(std::vector<float>{ 0.f, 1.5f }); // will be interpreted as seconds
        const auto* animKeyframes = logicEngine.createDataArray(std::vector<rlogic::vec3f>{ {0.f, 0.f, 0.f}, {0.f, 0.f, 360.f} });
        const rlogic::AnimationChannel channelLinear { "rotationZ", animTimestamps, animKeyframes, rlogic::EInterpolationType::Linear };

        AnimationNodeConfig config;
        for (int64_t i = 0; i < state.range(0); ++i)
            config.addChannel(channelLinear);
        auto* node = logicEngine.createAnimationNode(config);
        auto* progressProp = node->getInputs()->getChild("progress");

        RunAnimation(logicEngine, state, progressProp);
    }

    static void BM_AnimationKeyframes(benchmark::State& state)
    {
        LogicEngine logicEngine;
        const auto* animTimestamps = logicEngine.createDataArray(std::vector<float>{ 0.f, 0.5f, 1.f, 1.5f }); // will be interpreted as seconds
        const auto* animKeyframes = logicEngine.createDataArray(std::vector<rlogic::vec3f>{ {0.f, 0.f, 0.f}, {0.f, 0.f, 180.f}, {0.f, 0.f, 100.f}, {0.f, 0.f, 360.f} });
        const rlogic::AnimationChannel channelLinear { "rotationZ", animTimestamps, animKeyframes, rlogic::EInterpolationType::Linear };

        AnimationNodeConfig config;
        for (int64_t i = 0; i < state.range(0); ++i)
            config.addChannel(channelLinear);
        auto* node = logicEngine.createAnimationNode(config);
        auto* progressProp = node->getInputs()->getChild("progress");

        RunAnimation(logicEngine, state, progressProp);
    }

    static void BM_AnimationKeyframesCubic(benchmark::State& state)
    {
        LogicEngine logicEngine;
        const auto* animTimestamps = logicEngine.createDataArray(std::vector<float>{ 0.f, 0.5f, 1.f, 1.5f }); // will be interpreted as seconds
        const auto* animKeyframes = logicEngine.createDataArray(std::vector<rlogic::vec3f>{ {0.f, 0.f, 0.f}, {0.f, 0.f, 180.f}, {0.f, 0.f, 100.f}, {0.f, 0.f, 360.f} });
        const auto* cubicAnimTangentsIn = logicEngine.createDataArray(std::vector<rlogic::vec3f>{ {0.f, 0.f, 0.f}, { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f } });
        const auto* cubicAnimTangentsOut = logicEngine.createDataArray(std::vector<rlogic::vec3f>{ {0.f, 0.f, 0.f}, { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f } });
        const rlogic::AnimationChannel channelCubic { "rotationZcubic", animTimestamps, animKeyframes, rlogic::EInterpolationType::Cubic, cubicAnimTangentsIn, cubicAnimTangentsOut };

        AnimationNodeConfig config;
        for (int64_t i = 0; i < state.range(0); ++i)
            config.addChannel(channelCubic);
        auto* node = logicEngine.createAnimationNode(config);
        auto* progressProp = node->getInputs()->getChild("progress");

        RunAnimation(logicEngine, state, progressProp);
    }

    // Compares animation objects with animations done in lua
    // ARG: number of animation channels
    BENCHMARK(BM_AnimationScriptLinear)->Arg(1)->Arg(10);
    BENCHMARK(BM_AnimationScriptKeyframes)->Arg(1)->Arg(10);
    BENCHMARK(BM_AnimationLinear)->Arg(1)->Arg(10);
    BENCHMARK(BM_AnimationKeyframes)->Arg(1)->Arg(10);
    BENCHMARK(BM_AnimationKeyframesCubic)->Arg(1)->Arg(10);
}

