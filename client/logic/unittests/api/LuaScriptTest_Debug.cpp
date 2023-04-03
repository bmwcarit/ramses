//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LuaScriptTest_Base.h"
#include "WithTempDirectory.h"

#include "ramses-logic/LuaScript.h"
#include "ramses-logic/Property.h"
#include "impl/LuaScriptImpl.h"
#include "impl/LuaModuleImpl.h"
#include "LogTestUtils.h"

#include <fstream>

namespace rlogic
{
    class ALuaScript_Debug : public ALuaScript
    {
    protected:
        std::string_view m_scriptWithInterfaceError = R"(
            function interface(IN,OUT)
                IN.prop = nil
            end
            function run(IN,OUT)
            end
        )";

        std::string_view m_scriptWithRuntimeError = R"(
            function interface(IN,OUT)
            end
            function run(IN,OUT)
                IN.prop = nil
            end
        )";

        // Silence logs, unless explicitly enabled, to reduce spam and speed up tests
        ScopedLogContextLevel m_silenceLogs{ ELogMessageType::Off };
    };

    TEST_F(ALuaScript_Debug, ProducesErrorWithFullStackTrace_WhenErrorsInInterface)
    {
        LuaScript* script = m_logicEngine.createLuaScript(m_scriptWithInterfaceError, {}, "errorscript");

        ASSERT_EQ(nullptr, script);
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr(
                    "[errorscript] Error while loading script. Lua stack trace:\n"
                    "lua: error: Invalid type of field 'prop'! Expected Type:T() syntax where T=Float,Int32,... Found a value of type 'nil' instead"));
        // nullptr because no LogicNode was created
        EXPECT_EQ(nullptr, m_logicEngine.getErrors()[0].object);
    }

    TEST_F(ALuaScript_Debug, ProducesErrorWithFullStackTrace_WhenRuntimeErrors)
    {
        LuaScript* script = m_logicEngine.createLuaScript(m_scriptWithRuntimeError, {}, "errorscript");

        ASSERT_NE(nullptr, script);
        m_logicEngine.update();
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr(
            "lua: error: Tried to access undefined struct property 'prop'"));
        EXPECT_EQ(script, m_logicEngine.getErrors()[0].object);
    }

    TEST_F(ALuaScript_Debug, ErrorStackTraceContainsScriptName_WhenScriptWasNotLoadedFromFile)
    {
        // Script loaded from string, not file
        LuaScript* script = m_logicEngine.createLuaScript(m_scriptWithInterfaceError, {}, "errorscript");

        // Error message contains script name in the stack (file not known)
        EXPECT_EQ(nullptr, script);
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr(
            "[errorscript] Error while loading script. Lua stack trace:\n"
            "lua: error: Invalid type of field 'prop'! Expected Type:T() syntax where T=Float,Int32,... Found a value of type 'nil' instead"));
    }

    TEST_F(ALuaScript_Debug, LogsDebugMessage)
    {
        ScopedLogContextLevel enableLogs{ ELogMessageType::Info };

        constexpr std::string_view src = R"(
            function init()
                rl_logInfo("test1")
            end
            function interface(IN,OUT)
            end
            function run(IN,OUT)
                rl_logInfo("test2")
                rl_logWarn('x'..3)
                rl_logError('x'..'error')
            end
        )";

        LuaConfig config;
        config.enableDebugLogFunctions();
        const LuaScript* script = m_logicEngine.createLuaScript(src, config);
        ASSERT_NE(nullptr, script);
        EXPECT_TRUE(script->m_script.hasDebugLogFunctions());
        EXPECT_TRUE(m_logicEngine.update());
    }

    TEST_F(ALuaScript_Debug, LogsDebugMessageUsingModule)
    {
        ScopedLogContextLevel enableLogs{ ELogMessageType::Info };

        constexpr std::string_view moduleSrc = R"(
            local mymod = {}
            function mymod.concat(a,b)
                rl_logWarn(a..b)
            end
            return mymod
        )";

        constexpr std::string_view src = R"(
            modules("mymod")

            function init()
                mymod.concat("init1", "init2")
            end
            function interface(IN,OUT)
                mymod.concat("intf1", "intf2")
            end
            function run(IN,OUT)
                mymod.concat("run", "x"..3)
            end
        )";

        LuaConfig configModule;
        configModule.enableDebugLogFunctions();
        LuaModule* mod = m_logicEngine.createLuaModule(moduleSrc, configModule);
        ASSERT_NE(nullptr, mod);
        EXPECT_TRUE(mod->m_impl.hasDebugLogFunctions());

        LuaConfig config;
        config.addDependency("mymod", *mod);
        const LuaScript* script = m_logicEngine.createLuaScript(src, config);
        ASSERT_NE(nullptr, script);
        EXPECT_FALSE(script->m_script.hasDebugLogFunctions());

        EXPECT_TRUE(m_logicEngine.update());
    }

    TEST_F(ALuaScript_Debug, FailsToCompileScriptWhenUsingLogDebugFunctionWithoutEnabling)
    {
        constexpr std::string_view src = R"(
            function init()
                rl_logInfo("test1")
            end
            function interface(IN,OUT)
            end
            function run(IN,OUT)
                rl_logInfo("test2")
                rl_logWarn('x'..3)
                rl_logError('x'..'error')
            end
        )";

        const LuaScript* script = m_logicEngine.createLuaScript(src);
        EXPECT_EQ(nullptr, script);
    }

    TEST_F(ALuaScript_Debug, FailsToCompileScriptWhenUsingLogDebugFunctionInModuleWithoutEnabling)
    {
        constexpr std::string_view moduleSrc = R"(
            local mymod = {}
            function mymod.concat(a,b)
                rl_logWarn(a..b)
            end
            return mymod
        )";

        constexpr std::string_view src = R"(
            modules("mymod")

            function init()
                mymod.concat("init1", "init2")
            end
            function interface(IN,OUT)
                mymod.concat("intf1", "intf2")
            end
            function run(IN,OUT)
                mymod.concat("run", "x"..3)
            end
        )";

        LuaModule* mod = m_logicEngine.createLuaModule(moduleSrc);
        ASSERT_NE(nullptr, mod);

        LuaConfig config;
        config.addDependency("mymod", *mod);
        const LuaScript* script = m_logicEngine.createLuaScript(src, config);
        EXPECT_EQ(nullptr, script);
    }

    TEST_F(ALuaScript_Debug, FailsToSaveToFileIfScriptHasDebugLogFunctions)
    {
        WithTempDirectory tempDir;

        LuaConfig config;
        config.enableDebugLogFunctions();
        LuaScript* script = m_logicEngine.createLuaScript(m_minimalScript, config, "script");
        ASSERT_NE(nullptr, script);
        EXPECT_TRUE(script->m_script.hasDebugLogFunctions());

        EXPECT_FALSE(m_logicEngine.saveToFile("willNotSave.tmp"));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_EQ(m_logicEngine.getErrors().front().message, "Cannot save to file, Lua script 'script [Id=1]' has enabled debug log functions, remove this script before saving.");
        EXPECT_EQ(m_logicEngine.getErrors().front().object, script);

        // can save after removal
        EXPECT_TRUE(m_logicEngine.destroy(*script));
        EXPECT_TRUE(m_logicEngine.saveToFile("willSave.tmp"));
    }

    TEST_F(ALuaScript_Debug, FailsToSaveToFileIfModuleHasDebugLogFunctions)
    {
        WithTempDirectory tempDir;

        constexpr std::string_view moduleSrc = R"(
            local mymod = {}
            function mymod.concat(a,b)
                rl_logWarn(a..b)
            end
            return mymod
        )";

        LuaConfig config;
        config.enableDebugLogFunctions();
        LuaModule* mod = m_logicEngine.createLuaModule(moduleSrc, config, "mod");
        ASSERT_NE(nullptr, mod);
        EXPECT_TRUE(mod->m_impl.hasDebugLogFunctions());

        EXPECT_FALSE(m_logicEngine.saveToFile("willNotSave.tmp"));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_EQ(m_logicEngine.getErrors().front().message, "Cannot save to file, Lua module 'mod [Id=1]' has enabled debug log functions, remove this module before saving.");
        EXPECT_EQ(m_logicEngine.getErrors().front().object, mod);

        // can save after removal
        EXPECT_TRUE(m_logicEngine.destroy(*mod));
        EXPECT_TRUE(m_logicEngine.saveToFile("willSave.tmp"));
    }
}
