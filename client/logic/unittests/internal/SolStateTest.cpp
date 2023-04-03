//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"

#include "internals/SolState.h"
#include "internals/SolWrapper.h"
#include "internals/LuaCompilationUtils.h"
#include "internals/ErrorReporting.h"

namespace rlogic::internal
{
    class ASolState : public ::testing::Test
    {
        protected:
            SolState m_solState;

            const std::string_view m_valid_empty_script = R"(
                function interface(IN,OUT)
                end
                function run(IN,OUT)
                end
            )";
    };

    TEST_F(ASolState, DoesNotHaveErrorsAfterLoadingEmptyScript)
    {

        auto load_result = m_solState.loadScript("", "emptyScript");
        EXPECT_TRUE(load_result.valid());
    }

    TEST_F(ASolState, HasNoErrorsAfterLoadingValidScript)
    {
        auto load_result = m_solState.loadScript(m_valid_empty_script, "validEmptryScript");
        EXPECT_TRUE(load_result.valid());
    }

    TEST_F(ASolState, DoesNotLoadAScriptWithErrors)
    {
        auto load_result = m_solState.loadScript("this.does.not.compile", "cantCompileScript");
        EXPECT_FALSE(load_result.valid());
        sol::error error = load_result;
        EXPECT_THAT(error.what(), ::testing::HasSubstr("'<name>' expected near 'not'"));
    }

    TEST_F(ASolState, CreatesNewEnvironment)
    {
        sol::environment env = m_solState.createEnvironment({}, {}, false);
        EXPECT_TRUE(env.valid());
    }

    TEST_F(ASolState, NewEnvironment_DoesNotExposeTypeSymbols)
    {
        sol::environment env = m_solState.createEnvironment({}, {}, false);
        ASSERT_TRUE(env.valid());

        EXPECT_FALSE(env["Type"].valid());
    }

    TEST_F(ASolState, CreatesCustomMethods)
    {
        sol::environment env = m_solState.createEnvironment({}, {}, false);
        ASSERT_TRUE(env.valid());

        EXPECT_TRUE(env["modules"].valid());
        EXPECT_TRUE(env["rl_len"].valid());
    }

    TEST_F(ASolState, NewEnvironment_DoesNotExposeDebugLogFunctions)
    {
        sol::environment env = m_solState.createEnvironment({}, {}, false);
        ASSERT_TRUE(env.valid());

        EXPECT_FALSE(env["rl_logInfo"].valid());
        EXPECT_FALSE(env["rl_logWarn"].valid());
        EXPECT_FALSE(env["rl_logError"].valid());
    }

    TEST_F(ASolState, NewEnvironment_ExposesDebugLogFunctions)
    {
        sol::environment env = m_solState.createEnvironment({}, {}, true);
        ASSERT_TRUE(env.valid());

        EXPECT_TRUE(env["rl_logInfo"].valid());
        EXPECT_TRUE(env["rl_logWarn"].valid());
        EXPECT_TRUE(env["rl_logError"].valid());
    }

    class ASolState_Environment : public ASolState
    {
    protected:
        sol::environment m_env {m_solState.createEnvironment({}, {}, false)};
    };

    TEST_F(ASolState_Environment, HidesGlobalStandardModulesByDefault)
    {
        EXPECT_FALSE(m_env["print"].valid());
        EXPECT_FALSE(m_env["debug"].valid());
        EXPECT_FALSE(m_env["string"].valid());
        EXPECT_FALSE(m_env["table"].valid());
        EXPECT_FALSE(m_env["error"].valid());
        EXPECT_FALSE(m_env["math"].valid());
    }

    TEST_F(ASolState_Environment, ExposesOnlyRequestedGlobalStandardModules)
    {
        sol::environment env = m_solState.createEnvironment({EStandardModule::Math}, {}, false);
        ASSERT_TRUE(env.valid());

        EXPECT_TRUE(env["math"].valid());

        EXPECT_FALSE(env["print"].valid());
        EXPECT_FALSE(env["debug"].valid());
        EXPECT_FALSE(env["string"].valid());
        EXPECT_FALSE(env["table"].valid());
        EXPECT_FALSE(env["error"].valid());
    }

    TEST_F(ASolState_Environment, ExposesRequestedGlobalStandardModules_TwoModules)
    {
        sol::environment env = m_solState.createEnvironment({ EStandardModule::String, EStandardModule::Table }, {}, false);
        ASSERT_TRUE(env.valid());

        EXPECT_TRUE(env["string"].valid());
        EXPECT_TRUE(env["table"].valid());

        EXPECT_FALSE(env["math"].valid());
        EXPECT_FALSE(env["print"].valid());
        EXPECT_FALSE(env["debug"].valid());
        EXPECT_FALSE(env["error"].valid());
    }

    TEST_F(ASolState_Environment, ExposesRequestedGlobalStandardModules_BaseLib)
    {
        sol::environment env = m_solState.createEnvironment({ EStandardModule::Base }, {}, false);
        ASSERT_TRUE(env.valid());

        EXPECT_TRUE(env["error"].valid());
        EXPECT_TRUE(env["tostring"].valid());
        EXPECT_TRUE(env["print"].valid());

        EXPECT_FALSE(env["table"].valid());
        EXPECT_FALSE(env["math"].valid());
        EXPECT_FALSE(env["debug"].valid());
        EXPECT_FALSE(env["string"].valid());
    }

    TEST_F(ASolState_Environment, HasNoFunctionsExpectedByUserScript)
    {
        EXPECT_FALSE(m_env["interface"].valid());
        EXPECT_FALSE(m_env["run"].valid());
    }

    TEST_F(ASolState_Environment, NewEnvironment_TwoEnvironmentsShareNoData)
    {
        sol::environment env2 = m_solState.createEnvironment({}, {}, false);
        ASSERT_TRUE(env2.valid());

        m_env["thisBelongsTo"] = "m_env";
        env2["thisBelongsTo"] = "env2";

        const std::string data1 = m_env["thisBelongsTo"];
        const std::string data2 = env2["thisBelongsTo"];

        EXPECT_EQ(data1, "m_env");
        EXPECT_EQ(data2, "env2");
    }

    TEST_F(ASolState_Environment, HasNoAccessToPreviouslyDeclaredGlobalSymbols)
    {
        const std::string_view script = R"(
            global= "this is global"
            function func()
                return global
            end
            return func
        )";

        // Execute the script and obtain the func pointer 'func'
        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        sol::function func = loadedScript();

        // Apply fresh environment to func
        sol::environment newEnv = m_solState.createEnvironment({}, {}, false);
        ASSERT_TRUE(newEnv.valid());
        newEnv.set_on(func);

        // Func has no access to 'global' because it was defined _before_ applying the new environment
        sol::object result = func();
        EXPECT_EQ(result, sol::nil);
    }

    // Similar to NewlyCreatedEnvironment_HasNoAccessToPreviouslyDeclaredGlobalSymbols
    // But here the environment is applied before global symbols are declared -> access to those is available
    TEST_F(ASolState_Environment, HasAccessToGlobalSymbols_DeclaredAfterApplyingTheEnvironment)
    {
        const std::string_view script = R"(
            global = "this is global"
            function func()
                return global
            end
            return func
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");

        // Apply a fresh environments to loaded script _before_ executing it
        m_env.set_on(loadedScript);
        sol::function func = loadedScript();

        // Can access global symbol, because it lives in the new environment
        const std::string result = func();
        EXPECT_EQ(result, "this is global");
    }

    TEST_F(ASolState_Environment, OverridesEnvironmentOfScript_AfterAppliedOnIt)
    {
        const std::string_view reportData = R"(
                if data ~= nil then
                    return "data: " .. data
                else
                    return "no data"
                end
            )";

        sol::protected_function script = m_solState.loadScript(reportData, "test script");

        std::string dataStatus = script();
        EXPECT_EQ(dataStatus, "no data");

        m_env["data"] = "a lot of data!";

        m_env.set_on(script);

        dataStatus = script();
        EXPECT_EQ(dataStatus, "data: a lot of data!");
    }
}
