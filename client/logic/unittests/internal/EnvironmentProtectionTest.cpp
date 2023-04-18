//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"

#include "internals/SolState.h"
#include "internals/EnvironmentProtection.h"

namespace rlogic::internal
{
    class AEnvironmentProtection : public ::testing::Test
    {
    protected:

        sol::table getInternalEnvironment()
        {
            return EnvironmentProtection::GetProtectedEnvironmentTable(m_protEnv);
        }

        SolState m_solState;
        sol::environment m_protEnv{ m_solState.createEnvironment({}, {}, false) };
    };

    TEST_F(AEnvironmentProtection, ForbidsNotStringKeyGlobalAccessOfAnySort)
    {
        const std::vector<std::string> invalidStatements = {
            "_G[0] = 42",
            "local l=_G[0]",
            "_G[{}] = 42",
            "local l=_G[{}]",
            "_G[_G] = 42",
            "local l=_G[_G]",
            "_G[true] = 42",
            "local l=_G[true]",
        };

        const std::vector<EEnvProtectionFlag> protectionFlags = {
            EEnvProtectionFlag::LoadScript,
            EEnvProtectionFlag::InitFunction,
            EEnvProtectionFlag::InterfaceFunctionInScript,
            EEnvProtectionFlag::RunFunction,
        };

        for (const auto pFlag : protectionFlags)
        {
            EnvironmentProtection::SetEnvironmentProtectionLevel(m_protEnv, pFlag);
            for (const auto& statement : invalidStatements)
            {
                sol::protected_function loadedScript = m_solState.loadScript(statement, "test script");
                m_protEnv.set_on(loadedScript);

                sol::protected_function_result result = loadedScript();
                ASSERT_FALSE(result.valid());
                sol::error error = result;
                EXPECT_THAT(error.what(), ::testing::HasSubstr("Assigning global variables with a non-string index is prohibited! (key type used"));
            }
        }
    }

    class AEnvironmentProtection_LoadScript : public AEnvironmentProtection
    {
    protected:
        AEnvironmentProtection_LoadScript()
        {
            EnvironmentProtection::SetEnvironmentProtectionLevel(m_protEnv, EEnvProtectionFlag::LoadScript);
        }
    };

    TEST_F(AEnvironmentProtection_LoadScript, AllowsDeclaringWhitelistedFunctions)
    {
        const std::string_view script = R"(
            function init()
                return 5
            end
            function interface(IN,OUT)
                return 6
            end
            function run(IN,OUT)
                return 7
            end
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_TRUE(result.valid());

        sol::protected_function init = getInternalEnvironment()["init"];
        sol::protected_function iface = getInternalEnvironment()["interface"];
        sol::protected_function run = getInternalEnvironment()["run"];

        const int initResult = init();
        const int ifaceResult = iface();
        const int runResult = run();

        EXPECT_EQ(5, initResult);
        EXPECT_EQ(6, ifaceResult);
        EXPECT_EQ(7, runResult);
    }

    TEST_F(AEnvironmentProtection_LoadScript, AllowsDeclaringWhitelistedFunctions_ExoticSyntax)
    {
        const std::string_view script = R"(
            -- This is a somewhat exotic way to create a global function, must should have the same level of protection
            _G["run"] = function () return 5 end
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        sol::protected_function run = getInternalEnvironment()["run"];
        const int runResult = run();
        EXPECT_EQ(5, runResult);
    }

    TEST_F(AEnvironmentProtection_LoadScript, ForbidsDeclaringUnknownFunctions)
    {
        const std::string_view script = R"(
            function thisIsNotAllowed()
                return 5
            end
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Unexpected function name 'thisIsNotAllowed'! Allowed names: 'init', 'interface', 'run'"));
    }

    TEST_F(AEnvironmentProtection_LoadScript, ForbidsDeclaringUnknownFunctions_ExoticSyntax)
    {
        const std::string_view script = R"(
            -- This is a somewhat exotic way to create a global function, must be handled as error just as the normal case above
            _G["thisIsNotAllowed"] = function () return 5 end
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Unexpected function name 'thisIsNotAllowed'! Allowed names: 'init', 'interface', 'run'"));
    }

    TEST_F(AEnvironmentProtection_LoadScript, CatchesWritingToGlobalsAsError)
    {
        const std::string_view script = R"(
            global="this generates error"
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Declaring global variables is forbidden (exceptions: the functions 'init', 'interface' and 'run')!"));
    }

    TEST_F(AEnvironmentProtection_LoadScript, CatchesReadingGlobalsAsError)
    {
        const std::string_view script = R"(
            local t=_G["this generates error"]
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Trying to read global variable 'this generates error' outside the scope of init(), interface() and run() functions! This can cause undefined behavior and is forbidden!"));
    }

    TEST_F(AEnvironmentProtection_LoadScript, ForbidsOverwritingSpecialFunctions)
    {
        const std::string_view script = R"(
            function init()
                return 5
            end
            function init()
                return 6
            end
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();

        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Function 'init' can only be declared once!"));

        sol::protected_function init = getInternalEnvironment()["init"];
        int val = init();
        EXPECT_EQ(val, 5);
    }

    TEST_F(AEnvironmentProtection_LoadScript, ForbidsOverwritingTheGlobalTable)
    {
        const std::string_view script = R"(
            GLOBAL = {}
        )";

        getInternalEnvironment()["GLOBAL"] = m_solState.createTable();
        getInternalEnvironment()["GLOBAL"]["data"] = 5;

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();

        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Declaring global variables is forbidden (exceptions: the functions 'init', 'interface' and 'run')! (found value of type 'table')"));

        const int val = getInternalEnvironment()["GLOBAL"]["data"];
        EXPECT_EQ(val, 5);
    }

    class AEnvironmentProtection_LoadInterface : public AEnvironmentProtection
    {
    protected:
        AEnvironmentProtection_LoadInterface()
        {
            EnvironmentProtection::SetEnvironmentProtectionLevel(m_protEnv, EEnvProtectionFlag::LoadInterface);
        }
    };

    TEST_F(AEnvironmentProtection_LoadInterface, AllowsDeclaringInterfaceFunction)
    {
        const std::string_view script = R"(
            function interface(IN,OUT)
                return 6
            end
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_TRUE(result.valid());

        sol::protected_function iface = getInternalEnvironment()["interface"];

        const int ifaceResult = iface();
        EXPECT_EQ(6, ifaceResult);
    }

    TEST_F(AEnvironmentProtection_LoadInterface, ForbidsDeclaringUnknownFunctions)
    {
        const std::string_view script = R"(
            function thisIsNotAllowed()
                return 5
            end
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Unexpected function name 'thisIsNotAllowed'! Only 'interface' function can be declared!"));
    }

    TEST_F(AEnvironmentProtection_LoadInterface, CatchesWritingToGlobalsAsError)
    {
        const std::string_view script = R"(
            global="this generates error"
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Declaring global variables is forbidden (exception: the 'interface' function)!"));
    }

    TEST_F(AEnvironmentProtection_LoadInterface, CatchesReadingGlobalsAsError)
    {
        const std::string_view script = R"(
            local t=_G["this generates error"]
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Trying to read global variable 'this generates error' in an interface!"));
    }

    TEST_F(AEnvironmentProtection_LoadInterface, ForbidsOverwritingInterfaceFunction)
    {
        const std::string_view script = R"(
            function interface(IN,OUT)
                return 5
            end
            function interface(IN,OUT)
                return 6
            end
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();

        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Function 'interface' can only be declared once!"));
    }

    TEST_F(AEnvironmentProtection_LoadInterface, ForbidsOverwritingTheGlobalTable)
    {
        const std::string_view script = R"(
            GLOBAL = {}
        )";

        getInternalEnvironment()["GLOBAL"] = m_solState.createTable();
        getInternalEnvironment()["GLOBAL"]["data"] = 5;

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();

        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Declaring global variables is forbidden (exception: the 'interface' function)! (found value of type 'table')"));

        const int val = getInternalEnvironment()["GLOBAL"]["data"];
        EXPECT_EQ(val, 5);
    }

    class AEnvironmentProtection_InitFunction: public AEnvironmentProtection
    {
    protected:
        AEnvironmentProtection_InitFunction()
        {
            EnvironmentProtection::SetEnvironmentProtectionLevel(m_protEnv, EEnvProtectionFlag::InitFunction);
        }
    };

    TEST_F(AEnvironmentProtection_InitFunction, AllowsDeclaringLocalFunctions)
    {
        const std::string_view script = R"(
            local fun = function ()
                return 5
            end
            return fun
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_TRUE(result.valid());

        sol::protected_function fun = result;

        const int funcResult = fun();

        EXPECT_EQ(5, funcResult);
    }

    TEST_F(AEnvironmentProtection_InitFunction, ForbidsDeclaringGlobalFunctions)
    {
        const std::string_view script = R"(
            function thisIsNotAllowed()
                return 5
            end
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Unexpected global variable definition 'thisIsNotAllowed' in init()! Please use the GLOBAL table to declare global data and functions, or use modules!"));
    }

    TEST_F(AEnvironmentProtection_InitFunction, CatchesWritingToGlobalsAsError)
    {
        const std::string_view script = R"(
            global="this generates error"
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(), ::testing::HasSubstr("Unexpected global variable definition 'global' in init()! Please use the GLOBAL table to declare global data and functions, or use modules!"));
    }

    TEST_F(AEnvironmentProtection_InitFunction, CatchesReadingGlobalsAsError)
    {
        const std::string_view script = R"(
            local t=_G["this generates error"]
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr(
                "Trying to read global variable 'this generates error' in the init() function! "
                "This can cause undefined behavior and is forbidden! Use the GLOBAL table to read/write global data!"));
    }

    TEST_F(AEnvironmentProtection_InitFunction, AllowsReadingPredefinedGlobalsTable)
    {
        const std::string_view script = R"(
            return GLOBAL.data
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        getInternalEnvironment()["GLOBAL"] = m_solState.createTable();
        getInternalEnvironment()["GLOBAL"]["data"] = 5;

        sol::protected_function_result result = loadedScript();
        ASSERT_TRUE(result.valid());
        const int resultData = result;
        EXPECT_EQ(resultData, 5);
    }

    TEST_F(AEnvironmentProtection_InitFunction, ForbidsOverwritingGlobalTable)
    {
        const std::string_view script = R"(
            GLOBAL = {data = 42}
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        getInternalEnvironment()["GLOBAL"] = m_solState.createTable();
        getInternalEnvironment()["GLOBAL"]["data"] = 5;

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Trying to override the GLOBAL table in init()! You can only add data, but not overwrite the table!"));
    }

    TEST_F(AEnvironmentProtection_InitFunction, ForbidsDeletingGlobalTable)
    {
        getInternalEnvironment()["GLOBAL"] = m_solState.createTable();
        getInternalEnvironment()["GLOBAL"]["data"] = 5;

        const std::string_view script = R"(
            GLOBAL = nil
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();

        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Trying to override the GLOBAL table in init()! You can only add data, but not overwrite the table!"));
    }

    TEST_F(AEnvironmentProtection_InitFunction, AllowsAddingDataToGlobalTable_DoesNotOverwriteExistingData)
    {
        const std::string_view script = R"(
            GLOBAL.moreData = 15
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        m_protEnv.raw_set("GLOBAL", m_solState.createTable());
        m_protEnv["GLOBAL"]["data"] = 5;

        loadedScript();
        const int existingData = m_protEnv["GLOBAL"]["data"];
        const int moreData = m_protEnv["GLOBAL"]["moreData"];
        EXPECT_EQ(existingData, 5);
        EXPECT_EQ(moreData, 15);
    }

    class AEnvironmentProtection_InterfaceFunctionInScript : public AEnvironmentProtection
    {
    protected:
        AEnvironmentProtection_InterfaceFunctionInScript()
        {
            EnvironmentProtection::SetEnvironmentProtectionLevel(m_protEnv, EEnvProtectionFlag::InterfaceFunctionInScript);
        }
    };

    TEST_F(AEnvironmentProtection_InterfaceFunctionInScript, ForbidsDeclaringGlobalFunctions)
    {
        const std::string_view script = R"(
            function thisIsNotAllowed()
                return 5
            end
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr(
                "Unexpected global variable definition 'thisIsNotAllowed' in interface()! "
                "Use the GLOBAL table inside the init() function to declare global data and functions, or use modules!"));
    }

    TEST_F(AEnvironmentProtection_InterfaceFunctionInScript, CatchesWritingToGlobalsAsError)
    {
        const std::string_view script = R"(
            global="this generates error"
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr(
                "Unexpected global variable definition 'global' in interface()! "
                "Use the GLOBAL table inside the init() function to declare global data and functions, or use modules!"));
    }

    TEST_F(AEnvironmentProtection_InterfaceFunctionInScript, CatchesReadingGlobalsAsError)
    {
        const std::string_view script = R"(
            local t=_G["this generates error"]
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Unexpected global access to key 'this generates error' in interface()! Only 'GLOBAL' and 'Type' are allowed as a key"));
    }

    TEST_F(AEnvironmentProtection_InterfaceFunctionInScript, AllowsReadingPredefinedGlobalsTable)
    {
        const std::string_view script = R"(
            return GLOBAL.data
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        getInternalEnvironment()["GLOBAL"] = m_solState.createTable();
        getInternalEnvironment()["GLOBAL"]["data"] = 5;

        sol::protected_function_result result = loadedScript();
        ASSERT_TRUE(result.valid());
        const int resultData = result;
        EXPECT_EQ(resultData, 5);
    }

    class AEnvironmentProtection_InterfaceFunctionInInterface : public AEnvironmentProtection
    {
    protected:
        AEnvironmentProtection_InterfaceFunctionInInterface()
        {
            EnvironmentProtection::SetEnvironmentProtectionLevel(m_protEnv, EEnvProtectionFlag::InterfaceFunctionInInterface);
        }
    };

    TEST_F(AEnvironmentProtection_InterfaceFunctionInInterface, ForbidsDeclaringGlobalFunctions)
    {
        const std::string_view script = R"(
            function thisIsNotAllowed()
                return 5
            end
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Unexpected variable definition 'thisIsNotAllowed' in interface()!"));
    }

    TEST_F(AEnvironmentProtection_InterfaceFunctionInInterface, CatchesWritingToGlobalsAsError)
    {
        const std::string_view script = R"(
            global="this generates error"
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Unexpected variable definition 'global' in interface()!"));
    }

    TEST_F(AEnvironmentProtection_InterfaceFunctionInInterface, CatchesReadingGlobalsAsError)
    {
        const std::string_view script = R"(
            local t=_G["this generates error"]
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Unexpected global access to key 'this generates error' in interface()!"));
    }

    TEST_F(AEnvironmentProtection_InterfaceFunctionInInterface, ForbidsReadingPredefinedGlobalsTable)
    {
        const std::string_view script = R"(
            return GLOBAL.data
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Unexpected global access to key 'GLOBAL' in interface()!"));
    }

    class AEnvironmentProtection_RunFunction : public AEnvironmentProtection
    {
    protected:
        AEnvironmentProtection_RunFunction()
        {
            EnvironmentProtection::SetEnvironmentProtectionLevel(m_protEnv, EEnvProtectionFlag::RunFunction);
        }
    };

    TEST_F(AEnvironmentProtection_RunFunction, ForbidsDeclaringGlobalFunctions)
    {
        const std::string_view script = R"(
            function thisIsNotAllowed()
                return 5
            end
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Unexpected global variable definition 'thisIsNotAllowed' in run()! Use the init() function to declare global data and functions, or use modules!"));
    }

    TEST_F(AEnvironmentProtection_RunFunction, CatchesWritingToGlobalsAsError)
    {
        const std::string_view script = R"(
            global="this generates error"
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Unexpected global variable definition 'global' in run()! Use the init() function to declare global data and functions, or use modules!"));
    }

    TEST_F(AEnvironmentProtection_RunFunction, CatchesReadingGlobalsAsError)
    {
        const std::string_view script = R"(
            local t=_G["this generates error"]
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Unexpected global access to key 'this generates error' in run()! Only 'GLOBAL' is allowed as a key"));
    }

    TEST_F(AEnvironmentProtection_RunFunction, AllowsReadingPredefinedGlobalsTable)
    {
        const std::string_view script = R"(
            return GLOBAL.data
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        getInternalEnvironment()["GLOBAL"] = m_solState.createTable();
        getInternalEnvironment()["GLOBAL"]["data"] = 5;

        sol::protected_function_result result = loadedScript();
        ASSERT_TRUE(result.valid());
        const int resultData = result;
        EXPECT_EQ(resultData, 5);
    }

    TEST_F(AEnvironmentProtection_RunFunction, ForbidsOverwritingGlobalTable)
    {
        const std::string_view script = R"(
            GLOBAL = {data = 42}
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        getInternalEnvironment()["GLOBAL"] = m_solState.createTable();
        getInternalEnvironment()["GLOBAL"]["data"] = 5;

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Trying to override the GLOBAL table in run()! You can only read data, but not overwrite the table!"));
    }

    class AEnvironmentProtection_Module: public AEnvironmentProtection
    {
    protected:
        AEnvironmentProtection_Module()
        {
            EnvironmentProtection::SetEnvironmentProtectionLevel(m_protEnv, EEnvProtectionFlag::Module);
        }
    };

    TEST_F(AEnvironmentProtection_Module, ForbidsDeclaringGlobalFunctions)
    {
        const std::string_view script = R"(
            function thisIsNotAllowed()
                return 5
            end
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Declaring global variables is forbidden in modules! (found value of type 'function' assigned to variable 'thisIsNotAllowed')"));
    }

    TEST_F(AEnvironmentProtection_Module, CatchesWritingToGlobalsAsError)
    {
        const std::string_view script = R"(
            global="this generates error"
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Declaring global variables is forbidden in modules! (found value of type 'string' assigned to variable 'global')"));
    }

    TEST_F(AEnvironmentProtection_Module, CatchesReadingGlobalsAsError)
    {
        const std::string_view script = R"(
            local t=_G["this generates error"]
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Trying to read global variable 'this generates error' in module! This can cause undefined behavior and is forbidden!"));
    }

    // This never happens in practice, because modules live in different environment than the
    // script which has the GLOBAL table. Still, this test makes sure that even if the symbol leaked
    // somehow, it's still not accessible in the module
    TEST_F(AEnvironmentProtection_Module, DoesNotAllowReadingPredefinedGlobalsTable)
    {
        const std::string_view script = R"(
            return GLOBAL.data
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        getInternalEnvironment()["GLOBAL"] = m_solState.createTable();
        getInternalEnvironment()["GLOBAL"]["data"] = 5;

        sol::protected_function_result result = loadedScript();
        ASSERT_TRUE(result.valid());
        const int resultData = result;
        EXPECT_EQ(resultData, 5);
    }

    TEST_F(AEnvironmentProtection_Module, ForbidsOverwritingTypeTable)
    {
        const std::string_view script = R"(
            Type = {}
        )";

        sol::protected_function loadedScript = m_solState.loadScript(script, "test script");
        m_protEnv.set_on(loadedScript);

        getInternalEnvironment()["Type"] = m_solState.createTable();
        getInternalEnvironment()["Type"]["Int"] = [](){return 5;};

        sol::protected_function_result result = loadedScript();
        ASSERT_FALSE(result.valid());
        sol::error error = result;
        EXPECT_THAT(error.what(),
            ::testing::HasSubstr("Special global 'Type' symbol should not be overwritten in modules!"));
    }
}
