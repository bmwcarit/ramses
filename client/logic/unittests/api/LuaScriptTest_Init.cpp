//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LuaScriptTest_Base.h"

#include "ramses-logic/Property.h"
#include "impl/LogicEngineImpl.h"
#include "internals/ApiObjects.h"
#include "LogTestUtils.h"
#include "WithTempDirectory.h"


namespace rlogic
{
    class ALuaScript_Init : public ALuaScript
    {
    };

    TEST_F(ALuaScript_Init, CreatesGlobals)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function init()
                GLOBAL.number = 5
                GLOBAL.string = "foo"
                GLOBAL.bool = false
            end

            function interface(IN,OUT)
                OUT.number = Type:Int32()
                OUT.string = Type:String()
                OUT.bool = Type:Bool()
            end

            function run(IN,OUT)
                OUT.number = GLOBAL.number
                OUT.string = GLOBAL.string
                OUT.bool = GLOBAL.bool
            end
        )");

        ASSERT_TRUE(m_logicEngine.update());
        EXPECT_EQ(5, *script->getOutputs()->getChild("number")->get<int32_t>());
        EXPECT_EQ("foo", *script->getOutputs()->getChild("string")->get<std::string>());
        EXPECT_EQ(false, *script->getOutputs()->getChild("bool")->get<bool>());
    }

    TEST_F(ALuaScript_Init, CanUseGlobalsInInterface)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function init()
                GLOBAL.inputNames = {"foo", "bar"}
            end

            function interface(IN,OUT)
                for key,value in pairs(GLOBAL.inputNames) do
                    OUT[value] = Type:Float()
                end
            end

            function run(IN,OUT)
                for key,value in pairs(GLOBAL.inputNames) do
                    OUT[value] = 4.2
                end
            end
        )", WithStdModules({EStandardModule::Base}));

        ASSERT_NE(nullptr, script);
        ASSERT_TRUE(m_logicEngine.update());
        EXPECT_FLOAT_EQ(4.2f, *script->getOutputs()->getChild("foo")->get<float>());
        EXPECT_FLOAT_EQ(4.2f, *script->getOutputs()->getChild("bar")->get<float>());
    }

    TEST_F(ALuaScript_Init, CanModifyGlobalsAsIfTheyWereGlobalVariables)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function init()
                GLOBAL.number = 5
            end

            function interface(IN,OUT)
                IN.setGlobal = Type:Int32()
                OUT.getGlobal = Type:Int32()
            end

            function run(IN,OUT)
                if IN.setGlobal ~= 0 then
                    GLOBAL.number = IN.setGlobal
                end
                OUT.getGlobal = GLOBAL.number
            end
        )");

        ASSERT_TRUE(m_logicEngine.update());
        EXPECT_EQ(5, *script->getOutputs()->getChild("getGlobal")->get<int32_t>());

        EXPECT_TRUE(script->getInputs()->getChild("setGlobal")->set<int32_t>(42));
        ASSERT_TRUE(m_logicEngine.update());
        EXPECT_EQ(42, *script->getOutputs()->getChild("getGlobal")->get<int32_t>());
    }

    TEST_F(ALuaScript_Init, CanDeclareFunctions)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function init()
                GLOBAL.fun = function () return 42 end
            end

            function interface(IN,OUT)
                OUT.getGlobal = Type:Int32()
            end

            function run(IN,OUT)
                OUT.getGlobal = GLOBAL.fun()
            end
        )");

        ASSERT_TRUE(m_logicEngine.update());
        EXPECT_EQ(42, *script->getOutputs()->getChild("getGlobal")->get<int32_t>());
    }

    TEST_F(ALuaScript_Init, CanUseStandardModules)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function init()
                GLOBAL.number = math.floor(4.2)
            end

            function interface(IN,OUT)
                OUT.getGlobal = Type:Int32()
            end

            function run(IN,OUT)
                OUT.getGlobal = GLOBAL.number
            end
        )", WithStdModules({ EStandardModule::Math }));

        ASSERT_TRUE(m_logicEngine.update());
        EXPECT_EQ(4, *script->getOutputs()->getChild("getGlobal")->get<int32_t>());
    }

    TEST_F(ALuaScript_Init, CanUseCustomModules)
    {
        const std::string_view moduleSourceCode = R"(
            local mymath = {}
            function mymath.add(a,b)
                return a+b
            end
            mymath.PI=3.1415
            return mymath
        )";

        LuaConfig config;
        config.addDependency("mymath", *m_logicEngine.createLuaModule(moduleSourceCode));

        auto* script = m_logicEngine.createLuaScript(R"(
            modules("mymath")

            function init()
                GLOBAL.number = mymath.add(5, mymath.PI)
            end
            function interface(IN,OUT)
                OUT.getGlobal = Type:Float()
            end
            function run(IN,OUT)
                OUT.getGlobal = GLOBAL.number
            end
        )", config);

        ASSERT_TRUE(m_logicEngine.update());
        EXPECT_FLOAT_EQ(8.1415f, *script->getOutputs()->getChild("getGlobal")->get<float>());
    }

    TEST_F(ALuaScript_Init, IssuesErrorWhenUsingUndeclaredStandardModule)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function init()
                GLOBAL.number = math.floor(4.2)
            end
            function interface(IN,OUT)
            end
            function run(IN,OUT)
            end
        )");

        EXPECT_EQ(script, nullptr);
        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr(
                "Trying to read global variable 'math' in the init() function!"));
    }

    TEST_F(ALuaScript_Init, InitializesAfterDeserilization)
    {
        WithTempDirectory tmpFolder;

        {
            LogicEngine tmpLogicEngine;
            auto* script = tmpLogicEngine.createLuaScript(R"(
                function init()
                    GLOBAL.number = 5
                end

                function interface(IN,OUT)
                    OUT.globalValueBefore = Type:Int32()
                    OUT.globalValueAfter = Type:Int32()
                end

                function run(IN,OUT)
                    OUT.globalValueBefore = GLOBAL.number
                    GLOBAL.number = 42
                    OUT.globalValueAfter = GLOBAL.number
                end
            )", WithStdModules({EStandardModule::Base}), "withGlobals");

            ASSERT_TRUE(tmpLogicEngine.update());
            ASSERT_EQ(5, *script->getOutputs()->getChild("globalValueBefore")->get<int32_t>());
            ASSERT_EQ(42, *script->getOutputs()->getChild("globalValueAfter")->get<int32_t>());
            ASSERT_TRUE(SaveToFileWithoutValidation(tmpLogicEngine, "withGlobals.bin"));
        }

        ASSERT_TRUE(m_logicEngine.loadFromFile("withGlobals.bin"));
        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_EQ(5, *m_logicEngine.findByName<LuaScript>("withGlobals")->getOutputs()->getChild("globalValueBefore")->get<int32_t>());
        EXPECT_EQ(42, *m_logicEngine.findByName<LuaScript>("withGlobals")->getOutputs()->getChild("globalValueAfter")->get<int32_t>());
    }

    TEST_F(ALuaScript_Init, DoesNotLeaveAnyLuaStackObjectsWhenLuaScriptDestroyed)
    {
        constexpr std::string_view scriptText = R"(
            function interface(IN,OUT)
            end
            function run(IN,OUT)
            end)";

        for (int i = 0; i < 100; ++i)
        {
            auto* script = m_logicEngine.createLuaScript(scriptText);
            ASSERT_TRUE(script != nullptr);
            ASSERT_TRUE(m_logicEngine.destroy(*script));
            EXPECT_EQ(0, m_logicEngine.m_impl->getApiObjects().getNumElementsInLuaStack());
        }
    }

    TEST_F(ALuaScript_Init, DoesNotLeaveAnyLuaStackObjectsWhenLuaScriptDestroyed_WithModule)
    {
        constexpr std::string_view moduleSourceCode = R"(
            local mymodule = {}
            function mymodule.colorType()
                return {
                    red = Type:Int32(),
                    blue = Type:Int32(),
                    green = Type:Int32()
                }
            end
            function mymodule.structWithArray()
                return {
                    value = Type:Int32(),
                    array = Type:Array(2, Type:Int32())
                }
            end
            mymodule.color = {
                red = 255,
                green = 128,
                blue = 72
            }

            return mymodule)";

        constexpr std::string_view scriptSourceCode = R"(
            modules("mymodule")
            function init()
                GLOBAL.number = 5
            end
            function interface(IN,OUT)
                IN.struct = mymodule.structWithArray()
                OUT.struct = mymodule.structWithArray()
                OUT.color = mymodule.colorType();
                OUT.value = Type:Int32()
            end
            function run(IN,OUT)
                OUT.struct = IN.struct
                OUT.color = mymodule.color
                OUT.value = GLOBAL.number
            end)";

        for (int i = 0; i < 100; ++i)
        {
            auto module = m_logicEngine.createLuaModule(moduleSourceCode);
            ASSERT_TRUE(module != nullptr);
            LuaConfig config;
            config.addDependency("mymodule", *module);
            auto script = m_logicEngine.createLuaScript(scriptSourceCode, config);
            ASSERT_TRUE(script != nullptr);
            ASSERT_TRUE(m_logicEngine.destroy(*script));
            ASSERT_TRUE(m_logicEngine.destroy(*module));
            EXPECT_EQ(0, m_logicEngine.m_impl->getApiObjects().getNumElementsInLuaStack());
        }
    }

    TEST_F(ALuaScript_Init, ScriptUsesInterfaceTypeDefinitionFromGlobal)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function init()
                GLOBAL.outputType = Type:String()
                GLOBAL.outputName = "name"
                GLOBAL.outputValue = "MrAnderson"
            end

            function interface(IN,OUT)
                OUT[GLOBAL.outputName] = GLOBAL.outputType
            end

            function run(IN,OUT)
                OUT[GLOBAL.outputName] = GLOBAL.outputValue
            end)");
        ASSERT_NE(nullptr, script);

        EXPECT_TRUE(m_logicEngine.update());

        EXPECT_STREQ("MrAnderson", script->getOutputs()->getChild("name")->get<std::string>()->c_str());
    }

    TEST_F(ALuaScript_Init, ScriptUsesInterfaceTypeDefinitionFromGlobal_Array)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function init()
                GLOBAL.outputType = Type:Array(2, Type:Int32())
            end

            function interface(IN,OUT)
                OUT.array = GLOBAL.outputType
            end

            function run(IN,OUT)
                OUT.array[2] = 42
            end)");
        ASSERT_NE(nullptr, script);

        EXPECT_TRUE(m_logicEngine.update());
        const auto arrayOutput = script->getOutputs()->getChild("array");
        ASSERT_NE(nullptr, arrayOutput);
        ASSERT_EQ(2u, arrayOutput->getChildCount());

        EXPECT_EQ(42, *arrayOutput->getChild(1u)->get<int32_t>());
    }

    TEST_F(ALuaScript_Init, ScriptUsesInterfaceStructDefinedInGlobal)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function init()
                GLOBAL.outputDefinition = { value = Type:Int32() }
            end

            function interface(IN,OUT)
                OUT.struct = GLOBAL.outputDefinition
            end

            function run(IN,OUT)
                OUT.struct.value = 666
            end)");
        ASSERT_NE(nullptr, script);

        EXPECT_TRUE(m_logicEngine.update());
        ASSERT_NE(nullptr, script->getOutputs()->getChild("struct"));
        ASSERT_NE(nullptr, script->getOutputs()->getChild("struct")->getChild("value"));
        EXPECT_EQ(666, *script->getOutputs()->getChild("struct")->getChild("value")->get<int32_t>());
    }

    TEST_F(ALuaScript_Init, ScriptUsesInterfaceStructDefinedInGlobal_WithArray)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function init()
                GLOBAL.outputDefinition = {
                    value = Type:Int32(),
                    array = Type:Array(2, Type:Int32())
                }
            end

            function interface(IN,OUT)
                OUT.struct = GLOBAL.outputDefinition
            end

            function run(IN,OUT)
                OUT.struct.value = 666
                OUT.struct.array[2] = 42
            end)");
        ASSERT_NE(nullptr, script);

        EXPECT_TRUE(m_logicEngine.update());
        ASSERT_NE(nullptr, script->getOutputs()->getChild("struct"));
        ASSERT_NE(nullptr, script->getOutputs()->getChild("struct")->getChild("value"));
        EXPECT_EQ(666, *script->getOutputs()->getChild("struct")->getChild("value")->get<int32_t>());
        const auto arrayOutput = script->getOutputs()->getChild("struct")->getChild("array");
        ASSERT_NE(nullptr, arrayOutput);
        ASSERT_EQ(2u, arrayOutput->getChildCount());
        EXPECT_EQ(42, *arrayOutput->getChild(1u)->get<int32_t>());
    }

    TEST_F(ALuaScript_Init, SaveAndLoadScriptUsingInterfaceStructDefinedInGlobalWithArray)
    {
        WithTempDirectory tmpFolder;
        {
            LogicEngine otherLogic;
            LuaScript* script = otherLogic.createLuaScript(R"(
                function init()
                    GLOBAL.outputDefinition = {
                        value = Type:Int32(),
                        array = Type:Array(2, Type:Int32())
                    }
                end

                function interface(IN,OUT)
                    OUT.struct = GLOBAL.outputDefinition
                end

                function run(IN,OUT)
                    OUT.struct.value = 666
                    OUT.struct.array[2] = 42
                end)", {}, "script");
            ASSERT_NE(nullptr, script);
            EXPECT_TRUE(otherLogic.update());
            EXPECT_TRUE(SaveToFileWithoutValidation(otherLogic, "intfInGlobal.bin"));
        }

        EXPECT_TRUE(m_logicEngine.loadFromFile("intfInGlobal.bin"));
        EXPECT_TRUE(m_logicEngine.update());

        const auto script = m_logicEngine.findByName<LuaScript>("script");
        ASSERT_NE(nullptr, script->getOutputs()->getChild("struct"));
        ASSERT_NE(nullptr, script->getOutputs()->getChild("struct")->getChild("value"));
        EXPECT_EQ(666, *script->getOutputs()->getChild("struct")->getChild("value")->get<int32_t>());
        const auto arrayOutput = script->getOutputs()->getChild("struct")->getChild("array");
        ASSERT_NE(nullptr, arrayOutput);
        ASSERT_EQ(2u, arrayOutput->getChildCount());
        EXPECT_EQ(42, *arrayOutput->getChild(1u)->get<int32_t>());
    }

    class ALuaScript_Init_Sandboxing : public ALuaScript_Init
    {
    };

    TEST_F(ALuaScript_Init_Sandboxing, ReportsErrorWhenTryingToReadUnknownGlobals)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function init()
                local t = someGlobalVariable
            end

            function interface(IN,OUT)
            end

            function run(IN,OUT)
            end)");
        ASSERT_EQ(nullptr, script);

        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr(
                "Trying to read global variable 'someGlobalVariable' in the init() function! This"
                " can cause undefined behavior and is forbidden! Use the GLOBAL table to read/write global data!"));
    }

    TEST_F(ALuaScript_Init_Sandboxing, ReportsErrorWhenTryingToDeclareUnknownGlobals)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function init()
                thisCausesError = 'bad'
            end

            function interface(IN,OUT)
            end

            function run(IN,OUT)
            end)");
        ASSERT_EQ(nullptr, script);

        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr("Unexpected global variable definition 'thisCausesError' in init()! Please use the GLOBAL table to declare global data and functions, or use modules!"));
    }

    TEST_F(ALuaScript_Init_Sandboxing, ReportsErrorWhenTryingToOverrideGlobals)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function init()
                GLOBAL = {}
            end

            function interface(IN,OUT)
            end

            function run(IN,OUT)
            end)");
        ASSERT_EQ(nullptr, script);

        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr(" Trying to override the GLOBAL table in init()! You can only add data, but not overwrite the table!"));
    }

    TEST_F(ALuaScript_Init_Sandboxing, ReportsErrorWhenTryingToOverrideTypeTable)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function init()
                Type = {}
            end

            function interface(IN,OUT)
            end

            function run(IN,OUT)
            end)");
        ASSERT_EQ(nullptr, script);

        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr(" Can't override the Type special table in init()"));
    }

    TEST_F(ALuaScript_Init_Sandboxing, ReportsErrorWhenTryingToDeclareInitFunctionTwice)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function init()
            end

            function init()
            end

            function interface(IN,OUT)
            end

            function run(IN,OUT)
            end)");
        ASSERT_EQ(nullptr, script);

        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr("Function 'init' can only be declared once!"));
    }

    TEST_F(ALuaScript_Init_Sandboxing, ForbidsCallingSpecialFunctionsFromInsideInit)
    {
        for (const auto& specialFunction  : std::vector<std::string>{ "init", "run", "interface" })
        {
            LuaScript* script = m_logicEngine.createLuaScript(fmt::format(R"(
                function interface(IN,OUT)
                end
                function run(IN,OUT)
                end

                function init()
                    {}()
                end
            )", specialFunction));

            ASSERT_EQ(nullptr, script);

            EXPECT_THAT(m_logicEngine.getErrors()[0].message,
                ::testing::HasSubstr(fmt::format("Trying to read global variable '{}' in the init() function! This can cause undefined behavior "
                    "and is forbidden! Use the GLOBAL table to read/write global data!", specialFunction)));
        }
    }
}
