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
#include "impl/LogicEngineImpl.h"
#include "internals/SolWrapper.h"
#include "fmt/format.h"

namespace rlogic
{
    static void Run(benchmark::State& state, const std::string& src)
    {
        LogicEngine logicEngine;

        LuaConfig config;
        config.addStandardModuleDependency(EStandardModule::Base);

        logicEngine.createLuaScript(src, config);
        logicEngine.m_impl->disableTrackingDirtyNodes();

        while (state.KeepRunning())
        {
            if (!logicEngine.update())
            {
                state.SkipWithError("failure running update()");
            }
        }
    }

    static void BM_GetPropertyLocal(benchmark::State& state)
    {
        const int64_t scriptSize = state.range(0);
        const std::string scriptSrc = fmt::format(R"(
            function interface(IN,OUT)
                for i = 0,{},1 do
                    IN["param"..tostring(i)] = Type:Int32()
                end
                OUT.param = Type:Int32()
            end
            function run(IN,OUT)
                local result = 0
                local p0 = IN.param0
                local p1 = IN.param{}
                for i = 0,10000,1 do
                    result = result + p0 + p1
                end
                OUT.param = result
            end
        )", scriptSize, scriptSize);
        Run(state, scriptSrc);
    }

    static void BM_GetPropertyGlobal(benchmark::State& state)
    {
        const int64_t scriptSize = state.range(0);
        const std::string scriptSrc = fmt::format(R"(
            function interface(IN,OUT)
                for i = 0,{},1 do
                    IN["param"..tostring(i)] = Type:Int32()
                end
                OUT.param = Type:Int32()
            end
            function run(IN,OUT)
                local result = 0
                GLOBAL.p0 = IN.param0
                GLOBAL.p1 = IN.param{}
                for i = 0,10000,1 do
                    result = result + GLOBAL.p0 + GLOBAL.p1
                end
                OUT.param = result
            end
        )", scriptSize, scriptSize);
        Run(state, scriptSrc);
    }

    static void BM_GetProperty(benchmark::State& state)
    {
        const int64_t scriptSize = state.range(0);
        const std::string scriptSrc = fmt::format(R"(
            function interface(IN,OUT)
                for i = 0,{},1 do
                    IN["param"..tostring(i)] = Type:Int32()
                end
                OUT.param = Type:Int32()
            end
            function run(IN,OUT)
                local result = 0
                for i = 0,10000,1 do
                    result = result + IN.param0 + IN.param{}
                end
                OUT.param = result
            end
        )", scriptSize, scriptSize);
        Run(state, scriptSrc);
    }

    static void BM_GetPropertyNested(benchmark::State& state)
    {
        const int64_t scriptSize = state.range(0);
        const std::string scriptSrc = fmt::format(R"(
            function interface(IN,OUT)
                for i = 0,{},1 do
                    IN["param"..tostring(i)] = {{
                        deeply = {{
                            nested = {{
                                val = Type:Int32()
                            }}
                        }}
                    }}
                end
                OUT.param = Type:Int32()
            end
            function run(IN,OUT)
                local result = 0
                for i = 0,10000,1 do
                    result = result + IN.param0.deeply.nested.val + IN.param{}.deeply.nested.val
                end
                OUT.param = result
            end
        )", scriptSize, scriptSize);
        Run(state, scriptSrc);
    }

    struct Userdata
    {
        inline static const char* const name = "Userdata";
        int param = 0;
        int param0 = 0;
        int param1 = 1;
        int param2 = 2;
        int param3 = 3;
        int param4 = 4;
        int param5 = 5;
        int param6 = 6;
        int param7 = 7;
        int param8 = 8;
        int param9 = 9;
        int param10 = 10;
        int param100 = 100;

        int getValue(const std::string& strKey)
        {
            int retval = -1;
            if (strKey == "param0")
            {
                retval = param0;
            }
            else if (strKey == "param10")
            {
                retval = param10;
            }
            else if (strKey == "param100")
            {
                retval = param100;
            }
            return retval;
        }

        void setValue(const std::string& strKey, int value)
        {
            if (strKey == "param")
            {
                param = value;
            }
        }

        sol::object index(sol::stack_object key, sol::this_state L)
        {
            auto strKey = key.as<std::string>();
            return sol::object(L, sol::in_place, getValue(strKey));
        }

        void new_index(const sol::object& key, const sol::object& value)
        {
            auto strKey   = key.as<std::string>();
            auto intValue = value.as<int>();
            setValue(strKey, intValue);
        }

        static int lua_index(lua_State* L)
        {
            // 1st param (self)
            auto* userdata = static_cast<Userdata*>(luaL_checkudata(L, 1, Userdata::name));
            // 2nd param (key)
            const std::string key = luaL_checkstring(L, 2);
            // 1 return value
            lua_pushinteger(L, userdata->getValue(key));
            return 1;
        }

        static int lua_new_index(lua_State* L)
        {
            // 1st param (self)
            auto* userdata = static_cast<Userdata*>(luaL_checkudata(L, 1, Userdata::name));
            // 2nd param (key)
            const std::string key = luaL_checkstring(L, 2);
            // 3rd param (value)
            auto value = static_cast<int>(luaL_checkinteger(L, 3));
            userdata->setValue(key, value);
            return 0;
        }
    };

    static void BM_GetSolUserdataBind(benchmark::State& state)
    {
        const int64_t scriptSize = state.range(0);
        const std::string scriptSrc = fmt::format(R"(
            function interface(IN,OUT)
                for i = 0,{},1 do
                    IN["param"..tostring(i)] = Type:Int32()
                end
                OUT.param = Type:Int32()
            end
            function run(IN,OUT)
                local result = 0
                for i = 0,10000,1 do
                    result = result + IN.param0 + IN.param{}
                end
                OUT.param = result
            end
        )", scriptSize, scriptSize);


        Userdata userdata;
        auto sol = sol::state();
        sol.new_usertype<Userdata>(
            Userdata::name,
            sol::no_constructor,
            "param",
            &Userdata::param,
            "param0",
            &Userdata::param0,
            "param1",
            &Userdata::param1,
            "param2",
            &Userdata::param2,
            "param3",
            &Userdata::param3,
            "param4",
            &Userdata::param4,
            "param5",
            &Userdata::param5,
            "param6",
            &Userdata::param6,
            "param7",
            &Userdata::param7,
            "param8",
            &Userdata::param8,
            "param9",
            &Userdata::param9,
            "param10",
            &Userdata::param10,
            "param100",
            &Userdata::param100
            );

        auto result = sol.script(scriptSrc);
        if (!result.valid())
        {
            sol::error err = result;
            state.SkipWithError(err.what());
        }

        while (state.KeepRunning())
        {
            result = sol["run"](std::ref(userdata), std::ref(userdata));
            if (!result.valid())
            {
                sol::error err = result;
                state.SkipWithError(err.what());
            }
        }
    }

    static void BM_GetSolUserdataIndex(benchmark::State& state)
    {
        const int64_t scriptSize = state.range(0);
        const std::string scriptSrc = fmt::format(R"(
            function interface(IN,OUT)
                for i = 0,{},1 do
                    IN["param"..tostring(i)] = Type:Int32()
                end
                OUT.param = Type:Int32()
            end
            function run(IN,OUT)
                local result = 0
                for i = 0,10000,1 do
                    result = result + IN.param0 + IN.param{}
                end
                OUT.param = result
            end
        )", scriptSize, scriptSize);


        Userdata userdata;
        auto sol = sol::state();
        sol.new_usertype<Userdata>(
            Userdata::name,
            sol::no_constructor,
            sol::meta_function::index,
            &Userdata::index,
            sol::meta_function::new_index,
            &Userdata::new_index
            );

        auto result = sol.script(scriptSrc);
        if (!result.valid())
        {
            sol::error err = result;
            state.SkipWithError(err.what());
        }

        while (state.KeepRunning())
        {
            result = sol["run"](std::ref(userdata), std::ref(userdata));
            if (!result.valid())
            {
                sol::error err = result;
                state.SkipWithError(err.what());
            }
        }
    }

    static void BM_GetLuaUserdataIndex(benchmark::State& state)
    {
        const int64_t scriptSize = state.range(0);
        const std::string scriptSrc = fmt::format(R"(
            function interface()
                for i = 0,{},1 do
                    IN["param"..tostring(i)] = Type:Int32()
                end
                OUT.param = Type:Int32()
            end
            function run()
                local result = 0
                for i = 0,10000,1 do
                    result = result + IN.param0 + IN.param{}
                end
                OUT.param = result
            end
        )", scriptSize, scriptSize);

        auto sol = sol::state();
        auto L = sol.lua_state();
        {
            // Register Userdata type
            // NOLINTNEXTLINE(modernize-avoid-c-arrays) C array for C API
            static struct luaL_Reg userdata_methods[] = {{"__index", Userdata::lua_index}, {"__newindex", Userdata::lua_new_index}, {nullptr, nullptr}};
            luaL_newmetatable(L, Userdata::name);
            lua_pushvalue(L, -1);
            lua_setfield(L, -2, "__index");
            luaL_setfuncs(L, userdata_methods, 0);
            lua_pop(L, 1);
        }
        {
            // sol["IN"]
            void* buf = lua_newuserdata(L, sizeof(Userdata));
            new (buf) Userdata();
            luaL_getmetatable(L, "Userdata");
            lua_setmetatable(L, -2);
            lua_setglobal(L, "IN");
        }
        {
            // sol["OUT"]
            void* buf = lua_newuserdata(L, sizeof(Userdata));
            new (buf) Userdata();
            luaL_getmetatable(L, "Userdata");
            lua_setmetatable(L, -2);
            lua_setglobal(L, "OUT");
        }

        auto result = sol.script(scriptSrc);
        if (!result.valid())
        {
            sol::error err = result;
            state.SkipWithError(err.what());
        }

        while (state.KeepRunning())
        {
            result = sol["run"]();
            if (!result.valid())
            {
                sol::error err = result;
                state.SkipWithError(err.what());
            }
        }
    }

    // Compares direct access to properties vs. access to a local or global lua variable
    // ARG: number of inputs in script's interface()
    BENCHMARK(BM_GetPropertyLocal)->Arg(1)->Arg(10)->Arg(100)->Unit(benchmark::TimeUnit::kMillisecond);
    BENCHMARK(BM_GetPropertyGlobal)->Arg(1)->Arg(10)->Arg(100)->Unit(benchmark::TimeUnit::kMillisecond);
    BENCHMARK(BM_GetProperty)->Arg(1)->Arg(10)->Arg(100)->Unit(benchmark::TimeUnit::kMillisecond);
    BENCHMARK(BM_GetPropertyNested)->Arg(1)->Arg(10)->Arg(100)->Unit(benchmark::TimeUnit::kMillisecond);
    // for comparison: Simple userdata with pure sol
    BENCHMARK(BM_GetSolUserdataIndex)->Arg(1)->Arg(10)->Arg(100)->Unit(benchmark::TimeUnit::kMillisecond);
    BENCHMARK(BM_GetSolUserdataBind)->Arg(1)->Arg(10)->Arg(100)->Unit(benchmark::TimeUnit::kMillisecond);
    BENCHMARK(BM_GetLuaUserdataIndex)->Arg(1)->Arg(10)->Arg(100)->Unit(benchmark::TimeUnit::kMillisecond);
}


