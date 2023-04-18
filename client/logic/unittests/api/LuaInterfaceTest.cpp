//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "WithTempDirectory.h"

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/LuaInterface.h"
#include "impl/LuaInterfaceImpl.h"
#include "impl/LogicEngineImpl.h"
#include "impl/PropertyImpl.h"
#include "internals/ErrorReporting.h"

#include "generated/LuaInterfaceGen.h"

#include "SerializationTestUtils.h"

namespace rlogic::internal
{
    class ALuaInterface : public ::testing::Test
    {
    protected:
        LuaInterface* createTestInterface(std::string_view source, std::string_view interfaceName = "")
        {
            return m_logicEngine.createLuaInterface(source, interfaceName);
        }

        void createTestInterfaceAndExpectFailure(std::string_view source, std::string_view interfaceName = "")
        {
            const auto intf = m_logicEngine.createLuaInterface(source, interfaceName);
            EXPECT_EQ(intf, nullptr);
        }

        const std::string_view m_minimalInterface = R"(
            function interface(inputs)

                inputs.param1 = Type:Int32()
                inputs.param2 = Type:Float()

            end
        )";

        LogicEngine m_logicEngine{ ramses::EFeatureLevel_Latest };
    };

    TEST_F(ALuaInterface, CanCompileLuaInterface)
    {
        const LuaInterface* intf = createTestInterface(m_minimalInterface, "intf name");
        ASSERT_NE(nullptr, intf);
        EXPECT_STREQ("intf name", intf->getName().data());
    }

    TEST_F(ALuaInterface, CanExtractInputsFromLuaInterface)
    {
        const LuaInterface* intf = createTestInterface(m_minimalInterface, "intf name");
        ASSERT_NE(nullptr, intf);

        EXPECT_EQ(2u, intf->getInputs()->getChildCount());
        EXPECT_EQ("", intf->getInputs()->getName());

        EXPECT_EQ("param1", intf->getInputs()->getChild(0)->getName());
        EXPECT_EQ(rlogic::EPropertyType::Int32, intf->getInputs()->getChild(0)->getType());

        EXPECT_EQ("param2", intf->getInputs()->getChild(1)->getName());
        EXPECT_EQ(rlogic::EPropertyType::Float, intf->getInputs()->getChild(1)->getType());
    }

    TEST_F(ALuaInterface, ReturnsSameResultForOutputsAsInputs)
    {
        const LuaInterface* intf = createTestInterface(m_minimalInterface, "intf name");
        ASSERT_NE(nullptr, intf);

        EXPECT_EQ(2u, intf->getOutputs()->getChildCount());
        EXPECT_EQ("", intf->getOutputs()->getName());

        EXPECT_EQ("param1", intf->getInputs()->getChild(0)->getName());
        EXPECT_EQ(rlogic::EPropertyType::Int32, intf->getOutputs()->getChild(0)->getType());

        EXPECT_EQ("param2", intf->getOutputs()->getChild(1)->getName());
        EXPECT_EQ(rlogic::EPropertyType::Float, intf->getOutputs()->getChild(1)->getType());
    }

    TEST_F(ALuaInterface, FailsIfNameEmpty)
    {
        createTestInterfaceAndExpectFailure(R"SCRIPT(
            function interface(inputs)
            end
        )SCRIPT", "");

        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr("Can't create interface with empty name!"));
    }

    TEST_F(ALuaInterface, UpdatingInputsLeadsToUpdatingOutputs)
    {
        LuaInterface* intf = createTestInterface(m_minimalInterface, "intf name");
        ASSERT_NE(nullptr, intf);

        ASSERT_EQ(intf->getInputs()->getChild(0)->get<int32_t>(), intf->getOutputs()->getChild(0)->get<int32_t>());

        intf->getInputs()->getChild(0)->set<int32_t>(123);
        ASSERT_EQ(intf->getInputs()->getChild(0)->get<int32_t>(), intf->getOutputs()->getChild(0)->get<int32_t>());

        intf->m_impl.update();
        EXPECT_EQ(123, intf->getOutputs()->getChild(0)->get<int32_t>());
    }

    TEST_F(ALuaInterface, InterfaceFunctionIsExecutedOnlyOnce)
    {
        LuaInterface* intf = createTestInterface(R"SCRIPT(
            local firstExecution = true

            function interface(inputs)
                if not firstExecution then
                    error("a problem happened")
                end

                firstExecution = false
                inputs.param1 = Type:Int32()
                inputs.param2 = Type:Float()
            end

        )SCRIPT", "intf name");

        ASSERT_NE(nullptr, intf);
        EXPECT_STREQ("intf name", intf->getName().data());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 0u);
    }

    TEST_F(ALuaInterface, ReportsErrorIfInterfaceDidNotCompile)
    {
        createTestInterfaceAndExpectFailure(R"SCRIPT(
            function interface(inputs)
                not.a.valid.lua.syntax
            end

        )SCRIPT", "intf name");

        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message,
            ::testing::HasSubstr("[intf name] Error while loading interface. Lua stack trace:\n[string \"intf name\"]:3: unexpected symbol near 'not'"));
    }

    TEST_F(ALuaInterface, ReportsErrorIfNoInterfaceFunctionDefined)
    {
        createTestInterfaceAndExpectFailure(R"SCRIPT()SCRIPT", "intf name");
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_STREQ("[intf name] No 'interface' function defined!", m_logicEngine.getErrors()[0].message.c_str());
    }

    TEST_F(ALuaInterface, Sandboxing_ReportsErrorIfInitFunctionDefined)
    {
        createTestInterfaceAndExpectFailure(R"SCRIPT(

            function interface(inputs)
                inputs.param1 = Type:Int32()
                inputs.param2 = Type:Float()
            end

            function init()
            end

        )SCRIPT", "intf name");

        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr("Unexpected function name 'init'! Only 'interface' function can be declared!"));
    }

    TEST_F(ALuaInterface, Sandboxing_ReportsErrorIfRunFunctionDefined)
    {
        createTestInterfaceAndExpectFailure(R"SCRIPT(

            function interface(inputs)

                inputs.param1 = Type:Int32()
                inputs.param2 = Type:Float()
            end

            function run(IN,OUT)
            end

        )SCRIPT", "intf name");

        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr("Unexpected function name 'run'! Only 'interface' function can be declared!"));
    }

    TEST_F(ALuaInterface, Sandboxing_ReportsErrorIfGlobalSpecialVariableAccessed)
    {
        createTestInterfaceAndExpectFailure(R"SCRIPT(

            function interface(inputs)

                inputs.param1 = Type:Int32()
                inputs.param2 = Type:Float()

                GLOBAL.param3 = Type:Float()
            end

        )SCRIPT", "intf name");

        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr("Unexpected global access to key 'GLOBAL' in interface()!"));
    }

    TEST_F(ALuaInterface, Sandboxing_ReportsErrorIfLuaGlobalVariablesDefined)
    {
        createTestInterfaceAndExpectFailure(R"SCRIPT(

            someGlobal = 10

            function interface(inputs)

                inputs.param1 = Type:Int32()
                inputs.param2 = Type:Float()
            end

        )SCRIPT", "intf name");

        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr("Declaring global variables is forbidden (exception: the 'interface' function)! (found value of type 'number')"));
    }

    TEST_F(ALuaInterface, Sandboxing_ReportsErrorWhenTryingToReadUnknownGlobals)
    {
        createTestInterfaceAndExpectFailure(R"SCRIPT(

            function interface(inputs)
                local t = IN
            end

        )SCRIPT", "intf name");

        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr("Unexpected global access to key 'IN' in interface()!"));
    }

    TEST_F(ALuaInterface, Sandboxing_ReportsErrorWhenAccessingGlobalsOutsideInterfaceFunction)
    {
        createTestInterfaceAndExpectFailure(R"SCRIPT(

            table.getn(_G)
            function interface(inputs)
            end

        )SCRIPT", "intf name");

        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr("Trying to read global variable 'table' in an interface!"));
    }

    TEST_F(ALuaInterface, Sandboxing_ReportsErrorWhenSettingGlobals)
    {
        createTestInterfaceAndExpectFailure(R"SCRIPT(

            function interface(inputs)
                thisCausesError = 'bad'
            end

        )SCRIPT", "intf name");

        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr("Unexpected variable definition 'thisCausesError' in interface()!"));
    }

    TEST_F(ALuaInterface, Sandboxing_ReportsErrorWhenTryingToOverrideSpecialGlobalVariable)
    {
        createTestInterfaceAndExpectFailure(R"SCRIPT(

            function interface(inputs)
                GLOBAL = {}
            end

        )SCRIPT", "intf name");

        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr("Unexpected variable definition 'GLOBAL' in interface()!"));
    }

    TEST_F(ALuaInterface, Sandboxing_CanDeclareLocalVariables)
    {
        LuaInterface* intf = createTestInterface(R"SCRIPT(

            function interface(inputs)
                local multiplexersAreAwesomeIfYouLearnThem = 12
                inputs.param = Type:Int32()
            end

        )SCRIPT", "intf name");

        EXPECT_NE(nullptr, intf);
        EXPECT_EQ(m_logicEngine.getErrors().size(), 0u);
    }

    TEST_F(ALuaInterface, Sandboxing_ReportsErrorIfUnknownFunctionDefined)
    {
        createTestInterfaceAndExpectFailure(R"SCRIPT(

            function interface(inputs)

                inputs.param1 = Type:Int32()
                inputs.param2 = Type:Float()

            end

            function HackToCatchDeadlineCozNobodyChecksDeliveries()
            end

        )SCRIPT", "intf name");

        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr("Unexpected function name 'HackToCatchDeadlineCozNobodyChecksDeliveries'! Only 'interface' function can be declared!"));
    }

    TEST_F(ALuaInterface, Sandboxing_ReportsErrorWhenTryingToDeclareInterfaceFunctionTwice)
    {
        createTestInterfaceAndExpectFailure(R"SCRIPT(

            function interface(inputs)
            end

            function interface(inputs)
            end

        )SCRIPT", "intf name");

        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr("Function 'interface' can only be declared once!"));
    }

    TEST_F(ALuaInterface, Sandboxing_ForbidsCallingSpecialFunctionsFromInsideInterface)
    {
        for (const auto& specialFunction : std::vector<std::string>{ "init", "run", "interface" })
        {
            createTestInterfaceAndExpectFailure(fmt::format(R"SCRIPT(

                function interface(inputs)
                    {}()
                end
            )SCRIPT", specialFunction), "intf name");

            EXPECT_THAT(m_logicEngine.getErrors()[0].message,
                ::testing::HasSubstr(fmt::format("Unexpected global access to key '{}' in interface()!", specialFunction)));
        }
    }

    TEST_F(ALuaInterface, CanCreateInterfaceWithComplexTypes)
    {
        const std::string_view interfaceScript = R"(
            function interface(inputs)

                inputs.array_int = Type:Array(2, Type:Int32())
                inputs.array_struct = Type:Array(3, {a=Type:Int32(), b=Type:Float()})
                inputs.struct = {a=Type:Int32(), b={c = Type:Int32(), d=Type:Float()}}

            end
        )";
        LuaInterface* intf = createTestInterface(interfaceScript, "intf name");
        ASSERT_NE(nullptr, intf);

        ASSERT_EQ(intf->getInputs()->getChildCount(), 3u);
        ASSERT_EQ(intf->getOutputs()->getChildCount(), 3u);

        ASSERT_STREQ(intf->getInputs()->getChild(0)->getName().data(), "array_int");
        EXPECT_EQ(intf->getInputs()->getChild(0)->getType(), EPropertyType::Array);
        EXPECT_EQ(intf->getOutputs()->getChild(0)->getType(), EPropertyType::Array);
        EXPECT_EQ(intf->getOutputs()->getChild(0)->getChild(0)->getType(), EPropertyType::Int32);

        ASSERT_STREQ(intf->getInputs()->getChild(1)->getName().data(), "array_struct");
        EXPECT_EQ(intf->getInputs()->getChild(1)->getType(), EPropertyType::Array);
        EXPECT_EQ(intf->getOutputs()->getChild(1)->getType(), EPropertyType::Array);
        EXPECT_EQ(intf->getOutputs()->getChild(1)->getChild(0)->getType(), EPropertyType::Struct);

        ASSERT_STREQ(intf->getInputs()->getChild(2)->getName().data(), "struct");
        EXPECT_EQ(intf->getInputs()->getChild(2)->getType(), EPropertyType::Struct);
        EXPECT_EQ(intf->getOutputs()->getChild(2)->getType(), EPropertyType::Struct);
        EXPECT_EQ(intf->getOutputs()->getChild(2)->getChild(0)->getType(), EPropertyType::Int32);
        EXPECT_EQ(intf->getOutputs()->getChild(2)->getChild(1)->getType(), EPropertyType::Struct);
    }

    TEST_F(ALuaInterface, CanUpdateInterfaceValuesWithComplexTypes)
    {
        const std::string_view interfaceScript = R"(
            function interface(inputs)

                inputs.array_int = Type:Array(2, Type:Int32())
                inputs.array_struct = Type:Array(3, {a=Type:Int32(), b=Type:Float()})
                inputs.struct = {a=Type:Int32(), b={c = Type:Int32(), d=Type:Float()}}

            end
        )";
        LuaInterface* intf = createTestInterface(interfaceScript, "intf name");
        ASSERT_NE(nullptr, intf);

        intf->getInputs()->getChild(0)->getChild(0)->set<int32_t>(123);
        intf->getInputs()->getChild(2)->getChild(0)->set<int32_t>(456);

        ASSERT_EQ(intf->getOutputs()->getChild(0)->getChild(0)->get<int32_t>(), 123);
        ASSERT_EQ(intf->getOutputs()->getChild(2)->getChild(0)->get<int32_t>(), 456);

        intf->m_impl.update();
        EXPECT_EQ(123, intf->getOutputs()->getChild(0)->getChild(0)->get<int32_t>());
        EXPECT_EQ(456, intf->getOutputs()->getChild(2)->getChild(0)->get<int32_t>());
    }

    TEST_F(ALuaInterface, CanCheckIfOutputsAreLinked)
    {
        LuaInterface* intf = createTestInterface(R"(
            function interface(IN,OUT)

                IN.param1 = Type:Int32()
                IN.param2 = {a=Type:Float(), b=Type:Int32()}

            end
        )", "intf name");

        const auto& intfImpl = intf->m_interface;

        const auto* output1 = intf->getOutputs()->getChild(0);
        const auto* output21 = intf->getOutputs()->getChild(1)->getChild(0);
        const auto* output22 = intf->getOutputs()->getChild(1)->getChild(1);

        auto unlinkedOutputs = intfImpl.collectUnlinkedProperties();
        EXPECT_THAT(unlinkedOutputs, ::testing::UnorderedElementsAre(output1, output21, output22));

        LuaScript* inputsScript = m_logicEngine.createLuaScript(R"LUA_SCRIPT(
        function interface(IN,OUT)

            IN.param1 = Type:Int32()
            IN.param21 = Type:Float()
            IN.param22 = Type:Int32()

        end

        function run(IN,OUT)
        end
        )LUA_SCRIPT", {});

        //link 1 output
        m_logicEngine.link(*output1, *inputsScript->getInputs()->getChild(0));
        unlinkedOutputs = intfImpl.collectUnlinkedProperties();
        EXPECT_THAT(unlinkedOutputs, ::testing::UnorderedElementsAre(output21, output22));

        //link 2nd output
        m_logicEngine.link(*output21, *inputsScript->getInputs()->getChild(1));
        unlinkedOutputs = intfImpl.collectUnlinkedProperties();
        EXPECT_THAT(unlinkedOutputs, ::testing::UnorderedElementsAre(output22));

        m_logicEngine.link(*output22, *inputsScript->getInputs()->getChild(2));
        unlinkedOutputs = intfImpl.collectUnlinkedProperties();
        EXPECT_TRUE(unlinkedOutputs.empty());
    }

    class ALuaInterfaceWithModule : public ALuaInterface
    {
    protected:
        const std::string_view m_moduleSrc1 = R"(
            local mymodule = {}
            function mymodule.colorType()
                return {
                    red = Type:Int32(),
                    blue = Type:Int32(),
                    green = Type:Int32()
                }
            end

            return mymodule
        )";

        const std::string_view m_moduleSrc2 = R"(
            local mymodule = {}
            function mymodule.otherType()
                return Type:Float()
            end

            return mymodule
        )";
    };

    TEST_F(ALuaInterfaceWithModule, UsesModule)
    {
        constexpr std::string_view intfSrc = R"(
            modules("mymod")
            function interface(IN)
                IN.color = mymod.colorType()
            end
        )";

        LuaModule* mod = m_logicEngine.createLuaModule(m_moduleSrc1);
        LuaConfig config;
        config.addDependency("mymod", *mod);

        const auto intf = m_logicEngine.createLuaInterface(intfSrc, "intf", config);
        ASSERT_NE(intf, nullptr);

        const auto colorProp = intf->getInputs()->getChild(0);
        ASSERT_TRUE(colorProp);
        EXPECT_EQ("color", colorProp->getName());
        ASSERT_EQ(3u, colorProp->getChildCount());
        ASSERT_TRUE(colorProp->getChild("red"));
        ASSERT_TRUE(colorProp->getChild("blue"));
        ASSERT_TRUE(colorProp->getChild("green"));
        EXPECT_EQ(EPropertyType::Int32, colorProp->getChild("red")->getType());
        EXPECT_EQ(EPropertyType::Int32, colorProp->getChild("blue")->getType());
        EXPECT_EQ(EPropertyType::Int32, colorProp->getChild("green")->getType());
    }

    TEST_F(ALuaInterfaceWithModule, UsesTwoModules)
    {
        constexpr std::string_view intfSrc = R"(
            modules("mymod1", "mymod2")
            function interface(IN)
                IN.color = mymod1.colorType()
                IN.val = mymod2.otherType()
            end
        )";

        const LuaModule* mod1 = m_logicEngine.createLuaModule(m_moduleSrc1);
        const LuaModule* mod2 = m_logicEngine.createLuaModule(m_moduleSrc2);
        LuaConfig config;
        config.addDependency("mymod1", *mod1);
        config.addDependency("mymod2", *mod2);

        const auto intf = m_logicEngine.createLuaInterface(intfSrc, "intf", config);
        ASSERT_NE(intf, nullptr);

        const auto colorProp = intf->getInputs()->getChild(0);
        ASSERT_TRUE(colorProp);
        ASSERT_EQ(3u, colorProp->getChildCount());
        ASSERT_TRUE(colorProp->getChild("red"));
        ASSERT_TRUE(colorProp->getChild("blue"));
        ASSERT_TRUE(colorProp->getChild("green"));
        EXPECT_EQ(EPropertyType::Int32, colorProp->getChild("red")->getType());
        EXPECT_EQ(EPropertyType::Int32, colorProp->getChild("blue")->getType());
        EXPECT_EQ(EPropertyType::Int32, colorProp->getChild("green")->getType());

        const auto valProp = intf->getInputs()->getChild(1);
        ASSERT_TRUE(valProp);
        EXPECT_EQ(EPropertyType::Float, valProp->getType());
    }

    TEST_F(ALuaInterfaceWithModule, UsesSameModuleUnderMultipleNames)
    {
        constexpr std::string_view intfSrc = R"(
            modules("mymod1", "mymod2")
            function interface(IN)
                IN.color1 = mymod1.colorType()
                IN.color2 = mymod2.colorType()
            end
        )";

        const LuaModule* mod = m_logicEngine.createLuaModule(m_moduleSrc1);
        LuaConfig config;
        config.addDependency("mymod1", *mod);
        config.addDependency("mymod2", *mod);

        const auto intf = m_logicEngine.createLuaInterface(intfSrc, "intf", config);
        ASSERT_NE(intf, nullptr);

        for (size_t i = 0u; i < 2; ++i)
        {
            const auto colorProp = intf->getInputs()->getChild(i);
            ASSERT_TRUE(colorProp);
            ASSERT_EQ(3u, colorProp->getChildCount());
            ASSERT_TRUE(colorProp->getChild("red"));
            ASSERT_TRUE(colorProp->getChild("blue"));
            ASSERT_TRUE(colorProp->getChild("green"));
            EXPECT_EQ(EPropertyType::Int32, colorProp->getChild("red")->getType());
            EXPECT_EQ(EPropertyType::Int32, colorProp->getChild("blue")->getType());
            EXPECT_EQ(EPropertyType::Int32, colorProp->getChild("green")->getType());
        }
    }

    TEST_F(ALuaInterfaceWithModule, UsesTypeFromModule_StructAsTableField)
    {
        constexpr std::string_view moduleSrc = R"(
            local mymodule = {}
            mymodule.colorType = {
                    red = Type:Int32(),
                    blue = Type:Int32(),
                    green = Type:Int32()
            }
            return mymodule
        )";

        constexpr std::string_view intfSrc = R"(
            modules("mymod")
            function interface(IN)
                IN.color = mymod.colorType
            end
        )";

        LuaModule* mod = m_logicEngine.createLuaModule(moduleSrc);
        LuaConfig config;
        config.addDependency("mymod", *mod);

        const auto intf = m_logicEngine.createLuaInterface(intfSrc, "intf", config);
        ASSERT_NE(intf, nullptr);

        const auto colorProp = intf->getInputs()->getChild(0);
        ASSERT_TRUE(colorProp);
        EXPECT_EQ("color", colorProp->getName());
        ASSERT_EQ(3u, colorProp->getChildCount());
        ASSERT_TRUE(colorProp->getChild("red"));
        ASSERT_TRUE(colorProp->getChild("blue"));
        ASSERT_TRUE(colorProp->getChild("green"));
        EXPECT_EQ(EPropertyType::Int32, colorProp->getChild("red")->getType());
        EXPECT_EQ(EPropertyType::Int32, colorProp->getChild("blue")->getType());
        EXPECT_EQ(EPropertyType::Int32, colorProp->getChild("green")->getType());
    }

    TEST_F(ALuaInterfaceWithModule, UsesTypeFromModule_MakesArrayFromIt)
    {
        constexpr std::string_view intfSrc = R"(
            modules("mymod")
            function interface(IN)
                IN.colors = Type:Array(2, mymod.colorType())
            end
        )";

        LuaModule* mod = m_logicEngine.createLuaModule(m_moduleSrc1);
        LuaConfig config;
        config.addDependency("mymod", *mod);

        const auto intf = m_logicEngine.createLuaInterface(intfSrc, "intf", config);
        ASSERT_NE(intf, nullptr);

        ASSERT_EQ(1u, intf->getInputs()->getChildCount());
        const auto colorsProp = intf->getInputs()->getChild(0);
        ASSERT_TRUE(colorsProp);
        EXPECT_EQ("colors", colorsProp->getName());
        ASSERT_EQ(2u, colorsProp->getChildCount());
        for (size_t i = 0u; i < 2u; ++i)
        {
            const auto elementProp = colorsProp->getChild(i);
            ASSERT_TRUE(elementProp->getChild("red"));
            ASSERT_TRUE(elementProp->getChild("blue"));
            ASSERT_TRUE(elementProp->getChild("green"));
            EXPECT_EQ(EPropertyType::Int32, elementProp->getChild("red")->getType());
            EXPECT_EQ(EPropertyType::Int32, elementProp->getChild("blue")->getType());
            EXPECT_EQ(EPropertyType::Int32, elementProp->getChild("green")->getType());
        }
    }

    TEST_F(ALuaInterfaceWithModule, UsesTypeFromModule_ArrayOfStructs)
    {
        constexpr std::string_view moduleSrc = R"(
            local mymodule = {}
            mymodule.colorsType = Type:Array(2, {
                    red = Type:Int32(),
                    blue = Type:Int32(),
                    green = Type:Int32()
            })
            return mymodule
        )";

        constexpr std::string_view intfSrc = R"(
            modules("mymod")
            function interface(IN)
                IN.colors = mymod.colorsType
            end
        )";

        LuaModule* mod = m_logicEngine.createLuaModule(moduleSrc);
        LuaConfig config;
        config.addDependency("mymod", *mod);

        const auto intf = m_logicEngine.createLuaInterface(intfSrc, "intf", config);
        ASSERT_NE(intf, nullptr);

        ASSERT_EQ(1u, intf->getInputs()->getChildCount());
        const auto colorsProp = intf->getInputs()->getChild(0);
        ASSERT_TRUE(colorsProp);
        EXPECT_EQ("colors", colorsProp->getName());
        ASSERT_EQ(2u, colorsProp->getChildCount());
        for (size_t i = 0u; i < 2u; ++i)
        {
            const auto elementProp = colorsProp->getChild(i);
            ASSERT_TRUE(elementProp->getChild("red"));
            ASSERT_TRUE(elementProp->getChild("blue"));
            ASSERT_TRUE(elementProp->getChild("green"));
            EXPECT_EQ(EPropertyType::Int32, elementProp->getChild("red")->getType());
            EXPECT_EQ(EPropertyType::Int32, elementProp->getChild("blue")->getType());
            EXPECT_EQ(EPropertyType::Int32, elementProp->getChild("green")->getType());
        }
    }

    TEST_F(ALuaInterfaceWithModule, UsesTypeFromModule_StructWithArray)
    {
        constexpr std::string_view moduleSrc = R"(
            local mymodule = {}
            mymodule.type = {
                    arr = Type:Array(2, Type:Float()),
                    val = Type:Int32()
            }
            return mymodule
        )";

        constexpr std::string_view intfSrc = R"(
            modules("mymod")
            function interface(IN)
                IN.structWithArray = mymod.type
            end
        )";

        LuaModule* mod = m_logicEngine.createLuaModule(moduleSrc);
        LuaConfig config;
        config.addDependency("mymod", *mod);

        const auto intf = m_logicEngine.createLuaInterface(intfSrc, "intf", config);
        ASSERT_NE(intf, nullptr);

        ASSERT_EQ(1u, intf->getInputs()->getChildCount());
        const auto structWithArrayProp = intf->getInputs()->getChild(0);
        ASSERT_TRUE(structWithArrayProp);
        EXPECT_EQ("structWithArray", structWithArrayProp->getName());
        ASSERT_EQ(2u, structWithArrayProp->getChildCount());
        const auto arrProp = structWithArrayProp->getChild("arr");
        ASSERT_TRUE(arrProp);
        ASSERT_EQ(2u, arrProp->getChildCount());
        for (size_t i = 0u; i < 2u; ++i)
        {
            EXPECT_EQ(EPropertyType::Float, arrProp->getChild(i)->getType());
        }
        const auto valProp = structWithArrayProp->getChild("val");
        ASSERT_TRUE(valProp);
        EXPECT_EQ(EPropertyType::Int32, valProp->getType());
    }

    TEST_F(ALuaInterfaceWithModule, CanGetTableSizeWithCustomMethod)
    {
        constexpr std::string_view moduleSrc = R"(
            local mod = {}
            mod.table1 = { a=1, b=2 }
            mod.table2 = { 4, 5, 6, 7 }
            mod.table3 = { a=1, b=2, 42 } -- expected size 1, according to Lua semantics
            return mod
        )";

        LuaModule* mod = m_logicEngine.createLuaModule(moduleSrc);
        LuaConfig config;
        config.addDependency("mymod", *mod);
        config.addStandardModuleDependency(EStandardModule::Base);

        const auto intf = m_logicEngine.createLuaInterface(R"(
            modules("mymod")
            function interface(IN)
                IN.dummy = Type:Int32()
                assert(rl_len(mymod.table1) == 0)
                assert(rl_len(mymod.table2) == 4)
                assert(rl_len(mymod.table3) == 1)
            end
        )", "intf", config);
        ASSERT_TRUE(intf);
    }

    TEST_F(ALuaInterfaceWithModule, UsesModuleWhichDependsOnAnotherModule)
    {
        constexpr std::string_view wrapModuleSrc = R"(
            modules("originalMod")
            local mod = {}
            mod.wrappedColorType = originalMod.colorType()
            return mod
        )";

        constexpr std::string_view intfSrc = R"(
            modules("mymod")
            function interface(IN)
                IN.color = mymod.wrappedColorType
            end
        )";

        const LuaModule* originalMod = m_logicEngine.createLuaModule(m_moduleSrc1);
        LuaConfig modConfig;
        modConfig.addDependency("originalMod", *originalMod);
        const LuaModule* wrapMod = m_logicEngine.createLuaModule(wrapModuleSrc, modConfig);

        LuaConfig config;
        config.addDependency("mymod", *wrapMod);
        const auto intf = m_logicEngine.createLuaInterface(intfSrc, "intf", config);
        ASSERT_NE(intf, nullptr);

        const auto colorProp = intf->getInputs()->getChild(0);
        ASSERT_TRUE(colorProp);
        EXPECT_EQ("color", colorProp->getName());
        ASSERT_EQ(3u, colorProp->getChildCount());
        ASSERT_TRUE(colorProp->getChild("red"));
        ASSERT_TRUE(colorProp->getChild("blue"));
        ASSERT_TRUE(colorProp->getChild("green"));
        EXPECT_EQ(EPropertyType::Int32, colorProp->getChild("red")->getType());
        EXPECT_EQ(EPropertyType::Int32, colorProp->getChild("blue")->getType());
        EXPECT_EQ(EPropertyType::Int32, colorProp->getChild("green")->getType());
    }

    TEST_F(ALuaInterfaceWithModule, CanUseStandardModule)
    {
        constexpr std::string_view intfSrc = R"(
            function interface(IN)
                IN.vals = Type:Array(math.floor(2.3), Type:Int32())
            end
        )";

        LuaConfig config;
        config.addStandardModuleDependency(EStandardModule::Math);

        const auto intf = m_logicEngine.createLuaInterface(intfSrc, "intf", config);
        ASSERT_NE(intf, nullptr);

        ASSERT_EQ(1u, intf->getInputs()->getChildCount());
        const auto valsProp = intf->getInputs()->getChild(0);
        ASSERT_TRUE(valsProp);
        EXPECT_EQ("vals", valsProp->getName());
        ASSERT_EQ(2u, valsProp->getChildCount());
        for (size_t i = 0u; i < 2u; ++i)
        {
            EXPECT_EQ(EPropertyType::Int32, valsProp->getChild(i)->getType());
        }
    }

    TEST_F(ALuaInterfaceWithModule, FailsToBeCreatedIfDeclaredDependencyDoesNotMatchProvidedDependency_NotProvidedButDeclared)
    {
        constexpr std::string_view src = R"(
            modules("dep1", "dep2")
            function interface(IN)
            end
        )";

        LuaModule* mod = m_logicEngine.createLuaModule(m_moduleSrc1);
        LuaConfig config;
        config.addDependency("dep2", *mod);

        EXPECT_EQ(nullptr, m_logicEngine.createLuaInterface(src, "intf", config));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("Module dependencies declared in source code: dep1, dep2\n  Module dependencies provided on create API: dep2"));
    }

    TEST_F(ALuaInterfaceWithModule, FailsToBeCreatedIfDeclaredDependencyDoesNotMatchProvidedDependency_ProvidedButNotDeclared)
    {
        constexpr std::string_view src = R"(
            modules("dep1", "dep2")
            function interface(IN)
            end
        )";

        LuaModule* mod = m_logicEngine.createLuaModule(m_moduleSrc1);
        LuaConfig config;
        config.addDependency("dep1", *mod);
        config.addDependency("dep2", *mod);
        config.addDependency("dep3", *mod);

        EXPECT_EQ(nullptr, m_logicEngine.createLuaInterface(src, "intf", config));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("Module dependencies declared in source code: dep1, dep2\n  Module dependencies provided on create API: dep1, dep2, dep3"));
    }

    TEST_F(ALuaInterfaceWithModule, FailsToBeCreatedIfDeclaredDependencyDoesNotMatchProvidedDependency_ExractionError)
    {
        constexpr std::string_view src = R"(
            modules("dep1", "dep1") -- duplicate dependency
            function interface(IN)
            end
        )";

        LuaModule* mod = m_logicEngine.createLuaModule(m_moduleSrc1);
        LuaConfig config;
        config.addDependency("dep1", *mod);

        EXPECT_EQ(nullptr, m_logicEngine.createLuaInterface(src, "intf", config));
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("Error while extracting module dependencies: 'dep1' appears more than once in dependency list"));
    }

    TEST_F(ALuaInterfaceWithModule, FailsToBeCreatedWhenOverwritingModuleData)
    {
        constexpr std::string_view moduleSrc = R"(
            local mymodule = {}
            mymodule.val = 100
            return mymodule
        )";

        constexpr std::string_view intfSrc = R"(
            modules("mymod")
            function interface(IN)
                IN.dummy = Type:Int32()
                mymod.val = 42
            end
        )";

        LuaModule* mod = m_logicEngine.createLuaModule(moduleSrc);
        LuaConfig config;
        config.addDependency("mymod", *mod);

        const auto intf = m_logicEngine.createLuaInterface(intfSrc, "intf", config);
        EXPECT_EQ(nullptr, intf);
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("Modifying module data is not allowed!"));
    }

    TEST_F(ALuaInterfaceWithModule, FailsToBeCreatedWhenUsingModuleOverwritingTypeSymbols)
    {
        constexpr std::string_view modSrc = R"(
            local m = {}
            function m.killTypes()
                Type = {}
            end
            return m
        )";
        const auto luaModule = m_logicEngine.createLuaModule(modSrc);
        LuaConfig config;
        config.addDependency("mymod", *luaModule);

        const auto intf = m_logicEngine.createLuaInterface(R"(
            modules("mymod")
            function interface(IN)
                IN.dummy = Type:Int32()
                mymod.killTypes()
            end
        )", "intf", config);
        EXPECT_EQ(nullptr, intf);
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("Special global 'Type' symbol should not be overwritten in modules!"));
    }

    TEST_F(ALuaInterfaceWithModule, FailsToBeCreatedWhenUsingModuleFromAnotherLogicInstance)
    {
        LogicEngine otherLogicEngine{ m_logicEngine.getFeatureLevel() };
        const auto luaModule = otherLogicEngine.createLuaModule(m_moduleSrc1);
        LuaConfig config;
        config.addDependency("mymod", *luaModule);

        const auto intf = m_logicEngine.createLuaInterface(R"(
            modules("mymod")
            function interface(IN)
                IN.dummy = Type:Int32()
            end
        )", "intf", config);
        EXPECT_EQ(nullptr, intf);
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_EQ(m_logicEngine.getErrors().front().message, "Failed to map Lua module 'mymod'! It was created on a different instance of LogicEngine.");
    }

    TEST_F(ALuaInterfaceWithModule, CanBeSerializedAndLoadedInBaseFeatureLevel_confidenceTest) // just to verify that module dependency does not affect serialized interface
    {
        WithTempDirectory tempDir;
        {
            LogicEngine logicEngine{ m_logicEngine.getFeatureLevel() };
            const auto luaModule = logicEngine.createLuaModule(m_moduleSrc1);
            LuaConfig config;
            config.addDependency("mymod", *luaModule);

            logicEngine.createLuaInterface(R"(
                modules("mymod")
                function interface(IN)
                    IN.color = mymod.colorType()
                end
            )", "intf", config);

            SaveFileConfig configNoValidation;
            configNoValidation.setValidationEnabled(false);
            EXPECT_TRUE(logicEngine.saveToFile("temp.rlogic", configNoValidation));
        }

        EXPECT_TRUE(m_logicEngine.loadFromFile("temp.rlogic"));
        const auto intf = m_logicEngine.findByName<LuaInterface>("intf");
        ASSERT_NE(intf, nullptr);

        const auto colorProp = intf->getInputs()->getChild(0);
        ASSERT_TRUE(colorProp);
        ASSERT_EQ(3u, colorProp->getChildCount());
        ASSERT_TRUE(colorProp->getChild("red"));
        ASSERT_TRUE(colorProp->getChild("blue"));
        ASSERT_TRUE(colorProp->getChild("green"));
        EXPECT_EQ(EPropertyType::Int32, colorProp->getChild("red")->getType());
        EXPECT_EQ(EPropertyType::Int32, colorProp->getChild("blue")->getType());
        EXPECT_EQ(EPropertyType::Int32, colorProp->getChild("green")->getType());
    }

    // tests that behavior of deprecated API is unchanged, some old assets have modules keyword in interface with invalid arguments
    TEST_F(ALuaInterfaceWithModule, IgnoresModulesDeclaredInScriptIfUsingOldMethodToCreateInterface)
    {
        constexpr std::string_view intfSrc = R"(
            modules("nonExistentMod")
            function interface(IN)
                IN.dummy = Type:Int32()
            end
        )";

        const auto intf = m_logicEngine.createLuaInterface(intfSrc, "intf");
        ASSERT_NE(intf, nullptr);
        EXPECT_TRUE(m_logicEngine.getErrors().empty());

        ASSERT_EQ(1u, intf->getInputs()->getChildCount());
        const auto prop = intf->getInputs()->getChild(0);
        EXPECT_EQ("dummy", prop->getName());
        EXPECT_EQ(EPropertyType::Int32, prop->getType());

        // if created with new API this will be error
        const auto intfInvalid = m_logicEngine.createLuaInterface(intfSrc, "intf", {});
        EXPECT_EQ(intfInvalid, nullptr);
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors().front().message, ::testing::HasSubstr("Module dependencies declared in source code do not match those provided by LuaConfig"));
    }

    class ALuaInterface_Serialization : public ALuaInterface
    {
    protected:
        enum class ESerializationIssue
        {
            AllValid,
            NameIdMissing,
            EmptyName,
            RootMissing,
            RootNotStruct
        };

        std::unique_ptr<LuaInterfaceImpl> deserializeSerializedDataWithIssue(ESerializationIssue issue)
        {
            {
                HierarchicalTypeData inputs = (issue == ESerializationIssue::RootNotStruct ? MakeType("", EPropertyType::Bool) : MakeStruct("", {}));
                auto inputsImpl = std::make_unique<PropertyImpl>(std::move(inputs), EPropertySemantics::Interface);

                const std::string_view name = (issue == ESerializationIssue::EmptyName ? "" : "intf");
                auto intf = rlogic_serialization::CreateLuaInterface(m_flatBufferBuilder,
                    issue == ESerializationIssue::NameIdMissing ? 0 : rlogic_serialization::CreateLogicObject(m_flatBufferBuilder, m_flatBufferBuilder.CreateString(name), 1u, 0u, 0u),
                    issue == ESerializationIssue::RootMissing ? 0 : PropertyImpl::Serialize(*inputsImpl, m_flatBufferBuilder, m_serializationMap)
                );
                m_flatBufferBuilder.Finish(intf);
            }

            const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::LuaInterface>(m_flatBufferBuilder.GetBufferPointer());
            return LuaInterfaceImpl::Deserialize(serialized, m_errorReporting, m_deserializationMap);
        }

        flatbuffers::FlatBufferBuilder m_flatBufferBuilder;
        SerializationTestUtils m_testUtils{ m_flatBufferBuilder };
        SerializationMap m_serializationMap;
        DeserializationMap m_deserializationMap;
        ErrorReporting m_errorReporting;
    };

    TEST_F(ALuaInterface_Serialization, CanSerializeAndDeserializeLuaInterface)
    {
        WithTempDirectory tempDirectory;

        // Serialize
        {
            LogicEngine otherEngine{ m_logicEngine.getFeatureLevel() };
            const LuaScript* inputsScript = otherEngine.createLuaScript(R"LUA_SCRIPT(
                function interface(IN,OUT)
                    IN.param1 = Type:Int32()
                    IN.param2 = { x = Type:Float(), y = Type:Array(2, Type:String()) }
                end

                function run(IN,OUT)
                end
                )LUA_SCRIPT", {});

            const LuaInterface* intf = otherEngine.createLuaInterface(R"(
                function interface(inout)
                    inout.param1 = Type:Int32()
                    inout.param2 = { x = Type:Float(), y = Type:Array(2, Type:String()) }
                end
                )", "intf");

            otherEngine.link(*intf->getOutputs()->getChild("param1"), *inputsScript->getInputs()->getChild("param1"));
            otherEngine.link(*intf->getOutputs()->getChild("param2")->getChild("x"), *inputsScript->getInputs()->getChild("param2")->getChild("x"));
            otherEngine.link(*intf->getOutputs()->getChild("param2")->getChild("y")->getChild(0u), *inputsScript->getInputs()->getChild("param2")->getChild("y")->getChild(0u));
            otherEngine.link(*intf->getOutputs()->getChild("param2")->getChild("y")->getChild(1u), *inputsScript->getInputs()->getChild("param2")->getChild("y")->getChild(1u));

            SaveFileConfig configNoValidation;
            configNoValidation.setValidationEnabled(false);
            ASSERT_TRUE(otherEngine.saveToFile("interface.rlogic", configNoValidation));
        }

        EXPECT_TRUE(m_logicEngine.loadFromFile("interface.rlogic"));
        const auto loadedIntf = m_logicEngine.findByName<LuaInterface>("intf");
        ASSERT_NE(nullptr, loadedIntf);

        EXPECT_EQ(2u, loadedIntf->getId());
        EXPECT_EQ(loadedIntf->getInputs(), loadedIntf->getOutputs());
        ASSERT_EQ(2u, loadedIntf->getInputs()->getChildCount());
        const auto param1 = loadedIntf->getInputs()->getChild("param1");
        ASSERT_NE(nullptr, param1);
        EXPECT_EQ(EPropertyType::Int32, param1->getType());

        const auto param2 = loadedIntf->getInputs()->getChild("param2");
        ASSERT_NE(nullptr, param2);
        EXPECT_EQ(EPropertyType::Struct, param2->getType());
        ASSERT_EQ(2u, param2->getChildCount());

        const auto param2x = param2->getChild("x");
        ASSERT_NE(nullptr, param2x);
        EXPECT_EQ(EPropertyType::Float, param2x->getType());

        const auto param2y = param2->getChild("y");
        ASSERT_NE(nullptr, param2y);
        EXPECT_EQ(EPropertyType::Array, param2y->getType());
        ASSERT_EQ(2u, param2y->getChildCount());
        EXPECT_EQ(EPropertyType::String, param2y->getChild(0u)->getType());
        EXPECT_EQ(EPropertyType::String, param2y->getChild(1u)->getType());
    }

    TEST_F(ALuaInterface_Serialization, FailsToSaveToFileIfInterfaceOutputsNotLinked)
    {
        createTestInterface(m_minimalInterface, "intf name");
        EXPECT_FALSE(m_logicEngine.saveToFile("interface.rlogic"));
    }

    TEST_F(ALuaInterface_Serialization, CanSerializeWithNoIssue)
    {
        EXPECT_TRUE(deserializeSerializedDataWithIssue(ALuaInterface_Serialization::ESerializationIssue::AllValid));
        EXPECT_TRUE(m_errorReporting.getErrors().empty());
    }

    TEST_F(ALuaInterface_Serialization, FailsDeserializationIfEssentialDataMissing)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ALuaInterface_Serialization::ESerializationIssue::NameIdMissing));
        ASSERT_FALSE(m_errorReporting.getErrors().empty());
        EXPECT_EQ("Fatal error during loading of LuaInterface from serialized data: missing name and/or ID!", m_errorReporting.getErrors().back().message);
    }

    TEST_F(ALuaInterface_Serialization, FailsDeserializationIfNameEmpty)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ALuaInterface_Serialization::ESerializationIssue::EmptyName));
        ASSERT_FALSE(m_errorReporting.getErrors().empty());
        EXPECT_EQ("Fatal error during loading of LuaInterface from serialized data: empty name!", m_errorReporting.getErrors().back().message);
    }

    TEST_F(ALuaInterface_Serialization, FailsDeserializationIfRootPropertyMissing)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ALuaInterface_Serialization::ESerializationIssue::RootMissing));
        ASSERT_FALSE(m_errorReporting.getErrors().empty());
        EXPECT_EQ("Fatal error during loading of LuaInterface from serialized data: missing root property!", m_errorReporting.getErrors().back().message);
    }

    TEST_F(ALuaInterface_Serialization, FailsDeserializationIfRootNotStructType)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ALuaInterface_Serialization::ESerializationIssue::RootNotStruct));
        ASSERT_FALSE(m_errorReporting.getErrors().empty());
        EXPECT_EQ("Fatal error during loading of LuaScript from serialized data: root property has unexpected type!", m_errorReporting.getErrors().back().message);
    }
}
