//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LuaScriptTest_Base.h"

#include "ramses-logic/LuaScript.h"
#include "ramses-logic/Property.h"

#include "fmt/format.h"

namespace rlogic
{
    class ALuaScript_Syntax : public ALuaScript
    {
    protected:
    };

    TEST_F(ALuaScript_Syntax, ProducesErrorIfNoInterfaceIsPresent)
    {
        LuaScript* scriptNoInterface = m_logicEngine.createLuaScript(R"(
            function run(IN,OUT)
            end
        )", {}, "scriptNoInterface");

        ASSERT_EQ(nullptr, scriptNoInterface);
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("[scriptNoInterface] No 'interface' function defined!"));
    }

    TEST_F(ALuaScript_Syntax, ProducesErrorIfNoRunIsPresent)
    {
        LuaScript* scriptNoRun = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            end
        )", {}, "scriptNoRun");

        ASSERT_EQ(nullptr, scriptNoRun);
        EXPECT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("[scriptNoRun] No 'run' function defined!"));
    }

    TEST_F(ALuaScript_Syntax, CannotBeCreatedFromSyntacticallyIncorrectScript)
    {
        LuaScript* script = m_logicEngine.createLuaScript("this.is.not.valid.lua.code", {}, "badSyntaxScript");
        ASSERT_EQ(nullptr, script);
        ASSERT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("'<name>' expected near 'not'"));
    }

    TEST_F(ALuaScript_Syntax, ProducesErrorIfScriptReturnsValue)
    {
        LuaScript* scriptNoRun = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            end
            function run(IN,OUT)
            end
            return "a string"
        )");

        ASSERT_EQ(nullptr, scriptNoRun);
        EXPECT_EQ(1u, m_logicEngine.getErrors().size());
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("Expected no return value in script source, but a value of type 'string' was returned!"));
    }

    TEST_F(ALuaScript_Syntax, PropagatesErrorsEmittedInLua_FromGlobalScope)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            error("Expect this error!")

            function interface(IN,OUT)
            end

            function run(IN,OUT)
            end
        )", WithStdModules({EStandardModule::Base}), "scriptWithErrorInGlobalCode");
        EXPECT_EQ(nullptr, script);
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr(
            "Expect this error!\nstack traceback:\n"
            "\t[C]: in function 'error'"));
    }

    TEST_F(ALuaScript_Syntax, PropagatesErrorsEmittedInLua_DuringInterfaceDeclaration)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                error("Expect this error!")
            end

            function run(IN,OUT)
            end
        )", WithStdModules({EStandardModule::Base}), "scriptWithErrorInInterface");
        EXPECT_EQ(nullptr, script);
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr(
            "Expect this error!\nstack traceback:\n"
            "\t[C]: in function 'error'"));
    }

    TEST_F(ALuaScript_Syntax, PropagatesErrorsEmittedInLua_DuringRun)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            end

            function run(IN,OUT)
                error("Expect this error!")
            end
        )", WithStdModules({EStandardModule::Base}), "scriptWithErrorInRun");

        ASSERT_NE(nullptr, script);

        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr(
            "Expect this error!\n"
            "stack traceback:\n"
            "\t[C]: in function 'error'"));
        EXPECT_EQ(script, m_logicEngine.getErrors()[0].object);
        EXPECT_EQ(m_logicEngine.getErrors()[0].object->getName(), "scriptWithErrorInRun");
    }

    TEST_F(ALuaScript_Syntax, ProducesErrorWhenIndexingVectorPropertiesOutOfRange)
    {
        LuaScript* script  = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.vec2f = Type:Vec2f()
                IN.vec3f = Type:Vec3f()
                IN.vec4f = Type:Vec4f()
                IN.vec2i = Type:Vec2i()
                IN.vec3i = Type:Vec3i()
                IN.vec4i = Type:Vec4i()

                -- Parametrize test in lua, this simplifies test readibility
                IN.propertyName = Type:String()
                IN.index = Type:Int32()
            end

            function run(IN,OUT)
                local message = "Value of " .. IN.propertyName .. "[" .. tostring(IN.index) .. "]" .. " is " .. IN[IN.propertyName][IN.index]
            end
        )", WithStdModules({EStandardModule::Base}), "scriptOOR");
        Property*  inputs  = script->getInputs();

        inputs->getChild("vec2f")->set<vec2f>({1.1f, 1.2f});
        inputs->getChild("vec3f")->set<vec3f>({2.1f, 2.2f, 2.3f});
        inputs->getChild("vec4f")->set<vec4f>({3.1f, 3.2f, 3.3f, 3.4f});
        inputs->getChild("vec2i")->set<vec2i>({1, 2});
        inputs->getChild("vec3i")->set<vec3i>({3, 4, 5});
        inputs->getChild("vec4i")->set<vec4i>({6, 7, 8, 9});

        Property* index = inputs->getChild("index");
        Property* name = inputs->getChild("propertyName");

        std::map<std::string, int32_t> sizeOfEachType =
        {
            {"vec2f", 2},
            {"vec3f", 3},
            {"vec4f", 4},
            {"vec2i", 2},
            {"vec3i", 3},
            {"vec4i", 4},
        };


        for (const auto& typeSizePair : sizeOfEachType)
        {
            const std::string& typeName = typeSizePair.first;
            int32_t componentCount = typeSizePair.second;
            name->set<std::string>(typeName);

            // Include invalid values -1 and N + 1
            for (int32_t i = -1; i <= componentCount + 1; ++i)
            {
                index->set<int32_t>(i);

                if (i < 1 || i > componentCount)
                {
                    EXPECT_FALSE(m_logicEngine.update());
                    ASSERT_EQ(1u, m_logicEngine.getErrors().size());

                    if (i < 0)
                    {
                        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("Only non-negative integers supported as array index type! Error while extracting integer: expected non-negative number, received '-1'"));
                    }
                    else
                    {
                        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr(fmt::format("Bad index '{}', expected 1 <= i <= {}", i, componentCount)));
                    }

                    EXPECT_EQ(m_logicEngine.getErrors()[0].object->getName(), "scriptOOR");
                    EXPECT_EQ(m_logicEngine.getErrors()[0].object, script);
                }
                else
                {
                    EXPECT_TRUE(m_logicEngine.update());
                    EXPECT_TRUE(m_logicEngine.getErrors().empty());
                }
            }
        }
    }

    TEST_F(ALuaScript_Syntax, ForbidsCallingInterfaceFunctionDirectly)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            end

            function run(IN,OUT)
            end

            interface(IN,OUT)
        )");

        ASSERT_EQ(nullptr, script);

        EXPECT_THAT(m_logicEngine.getErrors()[0].message,
            ::testing::HasSubstr("Trying to read global variable 'interface' outside the scope of init(), interface() and run() functions! "
                "This can cause undefined behavior and is forbidden!"));
    }

    TEST_F(ALuaScript_Syntax, ForbidsCallingRunFunctionDirectly)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            end

            function run(IN,OUT)
            end

            run(IN,OUT)
        )");

        ASSERT_EQ(nullptr, script);

        EXPECT_THAT(m_logicEngine.getErrors()[0].message,
            ::testing::HasSubstr("Trying to read global variable 'run' outside the scope of init(), interface() and run() functions! "
                "This can cause undefined behavior and is forbidden!"));
    }

    TEST_F(ALuaScript_Syntax, ForbidsCallingInitFunctionDirectly)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function init()
            end
            function interface(IN,OUT)
            end
            function run(IN,OUT)
            end

            init()
        )");

        ASSERT_EQ(nullptr, script);

        EXPECT_THAT(m_logicEngine.getErrors()[0].message,
            ::testing::HasSubstr("Trying to read global variable 'init' outside the scope of init(), interface() and run() functions! "
                "This can cause undefined behavior and is forbidden!"));
    }

    TEST_F(ALuaScript_Syntax, CanUseLuaSyntaxForComputingArraySize)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.array = Type:Array(3, Type:Int32())
                OUT.array_size = Type:Int32()
            end

            function run(IN,OUT)
                OUT.array_size = #IN.array
            end
        )");

        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_EQ(3, *script->getOutputs()->getChild("array_size")->get<int32_t>());
    }

    TEST_F(ALuaScript_Syntax, CanUseLuaSyntaxForComputingComplexArraySize)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.array = Type:Array(3,
                    {
                        vec3 = Type:Vec3f(),
                        vec4i = Type:Vec4i()
                    }
                )
                OUT.array_size = Type:Int32()
            end

            function run(IN,OUT)
                OUT.array_size = #IN.array
            end
        )");

        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_EQ(3, *script->getOutputs()->getChild("array_size")->get<int32_t>());
    }

    TEST_F(ALuaScript_Syntax, CanUseLuaSyntaxForComputingStructSize)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.struct = {
                    data1 = Type:Vec3f(),
                    data2 = Type:Vec4i(),
                    data3 = Type:Int32()
                }
                OUT.struct_size = Type:Int32()
            end

            function run(IN,OUT)
                OUT.struct_size = #IN.struct
            end
        )");

        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_EQ(3, *script->getOutputs()->getChild("struct_size")->get<int32_t>());
    }

    TEST_F(ALuaScript_Syntax, CanUseLuaSyntaxForComputingVec234Size)
    {
        m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.vec2f = Type:Vec2f()
                IN.vec3f = Type:Vec3f()
                IN.vec4f = Type:Vec4f()
                IN.vec2i = Type:Vec2i()
                IN.vec3i = Type:Vec3i()
                IN.vec4i = Type:Vec4i()
            end

            function run(IN,OUT)
                if #IN.vec2i ~= 2 then error("Expected vec2i has size 2!") end
                if #IN.vec2f ~= 2 then error("Expected vec2f has size 2!") end
                if #IN.vec3i ~= 3 then error("Expected vec3i has size 3!") end
                if #IN.vec3f ~= 3 then error("Expected vec3f has size 3!") end
                if #IN.vec4i ~= 4 then error("Expected vec4i has size 4!") end
                if #IN.vec4f ~= 4 then error("Expected vec4f has size 4!") end
            end
        )");

        EXPECT_TRUE(m_logicEngine.update());
    }

    TEST_F(ALuaScript_Syntax, CanUseLuaSyntaxForComputingSizeOfStrings)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.string = Type:String()
                OUT.string_size = Type:Int32()
            end

            function run(IN,OUT)
                OUT.string_size = #IN.string
            end
        )");

        script->getInputs()->getChild("string")->set<std::string>("abcde");
        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_EQ(5, *script->getOutputs()->getChild("string_size")->get<int32_t>());
    }

    TEST_F(ALuaScript_Syntax, RaisesErrorWhenTryingToGetSizeOfNonArrayTypes)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.notArray = Type:Int32()
            end

            function run(IN,OUT)
                local size = #IN.notArray
            end
        )", {}, "invalidArraySizeAccess");

        ASSERT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("attempt to get length of field 'notArray' (a number value)"));
        EXPECT_EQ(m_logicEngine.getErrors()[0].object->getName(), "invalidArraySizeAccess");
        EXPECT_EQ(m_logicEngine.getErrors()[0].object, script);
    }

    TEST_F(ALuaScript_Syntax, ProdocesErrorWhenIndexingVectorWithNonIntegerIndices)
    {
        LuaScript* script  = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.vec = Type:Vec4i()

                IN.errorType = Type:String()
            end

            function run(IN,OUT)
                if IN.errorType == "indexWithNil" then
                    local thisWillFail = IN.vec[nil]
                elseif IN.errorType == "indexIsATable" then
                    local thisWillFail = IN.vec[{1}]
                elseif IN.errorType == "indexIsAString" then
                    local thisWillFail = IN.vec["nope..."]
                elseif IN.errorType == "indexIsAFloat" then
                    local thisWillFail = IN.vec[1.5]
                elseif IN.errorType == "indexIsAUserdata" then
                    local thisWillFail = IN.vec[IN.vec]
                else
                    error("Test problem - check error cases below")
                end
            end
        )", {}, "invalidIndexingScript");
        Property*  inputs = script->getInputs();

        Property* errorType = inputs->getChild("errorType");

        std::vector<std::string> errorTypes =
        {
            "indexWithNil",
            "indexIsATable",
            "indexIsAString",
            "indexIsAFloat",
            "indexIsAUserdata",
        };

        for (const auto& error : errorTypes)
        {
            errorType->set<std::string>(error);
            EXPECT_FALSE(m_logicEngine.update());
            ASSERT_EQ(1u, m_logicEngine.getErrors().size());

            EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("Only non-negative integers supported as array index type!"));
            EXPECT_EQ(m_logicEngine.getErrors()[0].object->getName(), "invalidIndexingScript");
            EXPECT_EQ(m_logicEngine.getErrors()[0].object, script);
        }
    }

    TEST_F(ALuaScript_Syntax, ReportsErrorWhenTryingToAssignVectorTypes_WithMismatchedComponentCount)
    {
        const std::vector<LuaTestError> allCases =
        {
            {
                "OUT.vec2f = {}                 -- none at all",
                "Error while assigning output Vec2 property 'vec2f'. Error while extracting array: expected 2 array components in table but got 0 instead!"
            },
            {
                "OUT.vec3f = {1, 2, 3, 4}       -- more than expected",
                "Error while assigning output Vec3 property 'vec3f'. Error while extracting array: expected 3 array components in table but got 4 instead!"
            },
            {
                "OUT.vec4f = {1, 2, 3}          -- fewer than required",
                "Error while assigning output Vec4 property 'vec4f'. Error while extracting array: expected 4 array components in table but got 3 instead!"
            },
            {
                "OUT.vec2i = {1, 2, 'wrong'}    -- extra component of wrong type",
                "Error while assigning output Vec2 property 'vec2i'. Error while extracting array: expected 2 array components in table but got 3 instead!"
            },
            {
                "OUT.vec3i = {1, 2, {}}         -- extra nested table",
                "Error while assigning output Vec3 property 'vec3i'. Error while extracting array: unexpected value (type: 'table') at array element # 3! "
                "Reason: Error while extracting integer: expected a number, received 'table'"
            },
            {
                "OUT.vec4i = {1, 2, nil, 4}     -- wrong size, nil in-between",
                "Error while assigning output Vec4 property 'vec4i'. Error while extracting array: unexpected value (type: 'nil') at array element # 3! "
                "Reason: Error while extracting integer: expected a number, received 'nil'"
            },
            {
                "OUT.vec4i = {1, 2, nil, 3, 4}     -- correct size, nil in-between",
                "Error while assigning output Vec4 property 'vec4i'. Error while extracting array: expected 4 array components in table but got 5 instead!"
            },
        };

        for(const auto& errorCase : allCases)
        {
            std::string scriptSource =(R"(
            function interface(IN,OUT)
                OUT.vec2f = Type:Vec2f()
                OUT.vec3f = Type:Vec3f()
                OUT.vec4f = Type:Vec4f()
                OUT.vec2i = Type:Vec2i()
                OUT.vec3i = Type:Vec3i()
                OUT.vec4i = Type:Vec4i()
                OUT.nested = {
                    vec = Type:Vec3i(),
                    float = Type:Float()
                }
            end

            function run(IN,OUT)
            )");
            scriptSource += errorCase.errorCode;
            scriptSource += "\nend\n";

            LuaScript* script = m_logicEngine.createLuaScript(scriptSource, {}, "mismatchedVecSizes");

            ASSERT_NE(nullptr, script);
            EXPECT_FALSE(m_logicEngine.update());

            ASSERT_EQ(1u, m_logicEngine.getErrors().size());
            EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr(errorCase.expectedErrorMessage));
            EXPECT_EQ(m_logicEngine.getErrors()[0].object->getName(), "mismatchedVecSizes");
            EXPECT_EQ(m_logicEngine.getErrors()[0].object, script);

            EXPECT_TRUE(m_logicEngine.destroy(*script));
        }
    }

    TEST_F(ALuaScript_Syntax, ProducesErrorIfRunFunctionDoesNotEndCorrectly)
    {
        LuaScript* scriptWithWrongEndInRun = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            end
            function run(IN,OUT)
            ENDE
        )", {}, "missingEndInScript");

        ASSERT_EQ(nullptr, scriptWithWrongEndInRun);
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("'=' expected near '<eof>'"));
    }

    TEST_F(ALuaScript_Syntax, ProducesErrorIfInterfaceFunctionDoesNotEndCorrectly)
    {
        LuaScript* scriptWithWrongEndInInterface = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            ENDE
            function run(IN,OUT)
            end
        )", {}, "missingEndInScript");

        ASSERT_EQ(nullptr, scriptWithWrongEndInInterface);
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("'=' expected near 'function'"));
    }

    TEST_F(ALuaScript_Syntax, ProducesErrorIfInterfaceFunctionDoesNotEndAtAll)
    {
        LuaScript* scriptWithNoEndInInterface = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            function run(IN,OUT)
            end
        )", {}, "endlessInterface");

        ASSERT_EQ(nullptr, scriptWithNoEndInInterface);
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("'end' expected (to close 'function' at line 2) near '<eof>'"));
    }

    TEST_F(ALuaScript_Syntax, ProducesErrorIfRunFunctionDoesNotEndAtAll)
    {
        LuaScript* scriptWithNoEndInRun = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            end
            function run(IN,OUT)
        )", {}, "endlessRun");

        ASSERT_EQ(nullptr, scriptWithNoEndInRun);
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("'end' expected (to close 'function' at line 4) near '<eof>'"));
    }

    TEST_F(ALuaScript_Syntax, ProducesErrorMessageCorrectlyNotConflictingWithFmtFormatSyntax)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            end
            function run(IN,OUT)
                local coalaModule = {}
                coalaModule.coalaStruct = {
                    oink1
                    oink2
                }
            end
        )", {}, "missingComma");

        ASSERT_EQ(nullptr, script);
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("'}' expected (to close '{'"));
    }
}
