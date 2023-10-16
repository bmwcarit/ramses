//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LuaScriptTest_Base.h"
#include "WithTempDirectory.h"

#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/Property.h"
#include "impl/logic/LuaScriptImpl.h"
#include "impl/logic/LuaModuleImpl.h"
#include "LogTestUtils.h"

#include <fstream>

namespace ramses::internal
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
        ScopedLogContextLevel m_silenceLogs{ CONTEXT_CLIENT, ELogLevel::Off };
    };

    TEST_F(ALuaScript_Debug, ProducesErrorWithFullStackTrace_WhenErrorsInInterface)
    {
        LuaScript* script = m_logicEngine->createLuaScript(m_scriptWithInterfaceError, {}, "errorscript");

        ASSERT_EQ(nullptr, script);
        expectErrorSubstring(
            "[errorscript] Error while loading script. Lua stack trace:\n"
            "lua: error: Invalid type of field 'prop'! Expected Type:T() syntax where T=Float,Int32,... Found a value of type 'nil' instead");
    }

    TEST_F(ALuaScript_Debug, ProducesErrorWithFullStackTrace_WhenRuntimeErrors)
    {
        LuaScript* script = m_logicEngine->createLuaScript(m_scriptWithRuntimeError, {}, "errorscript");

        ASSERT_NE(nullptr, script);
        m_logicEngine->update();
        expectErrorSubstring("lua: error: Tried to access undefined struct property 'prop'", script);
    }

    TEST_F(ALuaScript_Debug, ErrorStackTraceContainsScriptName_WhenScriptWasNotLoadedFromFile)
    {
        // Script loaded from string, not file
        LuaScript* script = m_logicEngine->createLuaScript(m_scriptWithInterfaceError, {}, "errorscript");

        // Error message contains script name in the stack (file not known)
        EXPECT_EQ(nullptr, script);
        EXPECT_THAT(getLastErrorMessage(), ::testing::HasSubstr(
            "[errorscript] Error while loading script. Lua stack trace:\n"
            "lua: error: Invalid type of field 'prop'! Expected Type:T() syntax where T=Float,Int32,... Found a value of type 'nil' instead"));
    }

    TEST_F(ALuaScript_Debug, LogsDebugMessage)
    {
        ScopedLogContextLevel enableLogs{ CONTEXT_CLIENT, ELogLevel::Info };

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
        const LuaScript* script = m_logicEngine->createLuaScript(src, config);
        ASSERT_NE(nullptr, script);
        EXPECT_TRUE(script->impl().hasDebugLogFunctions());
        EXPECT_TRUE(m_logicEngine->update());
    }

    TEST_F(ALuaScript_Debug, LogsDebugMessageUsingModule)
    {
        ScopedLogContextLevel enableLogs{ CONTEXT_CLIENT, ELogLevel::Info };

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
        LuaModule* mod = m_logicEngine->createLuaModule(moduleSrc, configModule);
        ASSERT_NE(nullptr, mod);
        EXPECT_TRUE(mod->impl().hasDebugLogFunctions());

        LuaConfig config;
        config.addDependency("mymod", *mod);
        const LuaScript* script = m_logicEngine->createLuaScript(src, config);
        ASSERT_NE(nullptr, script);
        EXPECT_FALSE(script->impl().hasDebugLogFunctions());

        EXPECT_TRUE(m_logicEngine->update());
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

        const LuaScript* script = m_logicEngine->createLuaScript(src);
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

        LuaModule* mod = m_logicEngine->createLuaModule(moduleSrc);
        ASSERT_NE(nullptr, mod);

        LuaConfig config;
        config.addDependency("mymod", *mod);
        const LuaScript* script = m_logicEngine->createLuaScript(src, config);
        EXPECT_EQ(nullptr, script);
    }

    TEST_F(ALuaScript_Debug, FailsToSaveToFileIfScriptHasDebugLogFunctions)
    {
        WithTempDirectory tempDir;

        LuaConfig config;
        config.enableDebugLogFunctions();
        LuaScript* script = m_logicEngine->createLuaScript(m_minimalScript, config, "script");
        ASSERT_NE(nullptr, script);
        EXPECT_TRUE(script->impl().hasDebugLogFunctions());

        EXPECT_FALSE(saveToFileWithoutValidation("willNotSave.tmp"));
        expectError("Cannot save to file, Lua script 'script [LogicObject ScnObjId=9]' has enabled debug log functions, remove this script before saving.", script);

        // can save after removal
        EXPECT_TRUE(m_logicEngine->destroy(*script));
        EXPECT_TRUE(saveToFileWithoutValidation("willSave.tmp"));
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
        LuaModule* mod = m_logicEngine->createLuaModule(moduleSrc, config, "mod");
        ASSERT_NE(nullptr, mod);
        EXPECT_TRUE(mod->impl().hasDebugLogFunctions());

        EXPECT_FALSE(saveToFileWithoutValidation("willNotSave.tmp"));
        expectError("Cannot save to file, Lua module 'mod [LogicObject ScnObjId=9]' has enabled debug log functions, remove this module before saving.", mod);

        // can save after removal
        EXPECT_TRUE(m_logicEngine->destroy(*mod));
        EXPECT_TRUE(saveToFileWithoutValidation("willSave.tmp"));
    }
}
