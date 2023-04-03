//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LuaScriptTest_Base.h"

#include "ramses-logic/Property.h"
#include "impl/PropertyImpl.h"
#include "LogTestUtils.h"
#include "fmt/format.h"
#include <deque>

namespace rlogic
{
    class ALuaScript_Interface : public ALuaScript
    {
    protected:
        // Silence logs, unless explicitly enabled, to reduce spam and speed up tests
        ScopedLogContextLevel m_silenceLogs{ ELogMessageType::Off };
    };

    TEST_F(ALuaScript_Interface, DoesNotGenerateErrorWhenOverwritingInputsInInterfaceFunction)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN = {}
            end

            function run(IN,OUT)
            end
        )");

        ASSERT_NE(nullptr, script);

        EXPECT_EQ(m_logicEngine.getErrors().size(), 0u);
    }

    TEST_F(ALuaScript_Interface, DoesNotGenerateErrorWhenOverwritingOutputsInInterfaceFunction)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                OUT = {}
            end

            function run(IN,OUT)
            end
        )");

        ASSERT_NE(nullptr, script);

        EXPECT_EQ(m_logicEngine.getErrors().size(), 0u);
    }

    TEST_F(ALuaScript_Interface, ProducesErrorsIfARuntimeErrorOccursInInterface)
    {
        auto script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                error("emits error")
            end

            function run(IN,OUT)
            end
        )", WithStdModules({EStandardModule::Base}), "errorInInterface");

        ASSERT_EQ(nullptr, script);
        EXPECT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::AllOf(
            ::testing::HasSubstr("[errorInInterface] Error while loading script. Lua stack trace:"),
            ::testing::HasSubstr("[C]: in function 'error'")));
    }

    TEST_F(ALuaScript_Interface, DeclaresStruct_WithExplicitTypeSyntax)
    {
        std::string_view explicitStructSyntax = R"(
            function interface(IN,OUT)
                IN.struct = Type:Struct({})
            end

            function run(IN,OUT)
            end
        )";

        LuaScript* script = m_logicEngine.createLuaScript(explicitStructSyntax);

        auto inputs = script->getInputs();

        ASSERT_EQ(1u, inputs->getChildCount());
        EXPECT_EQ("struct", inputs->getChild(0)->getName());
        EXPECT_EQ(EPropertyType::Struct, inputs->getChild(0)->getType());
        ASSERT_EQ(0u, inputs->getChild(0)->getChildCount());
    }

    TEST_F(ALuaScript_Interface, DeclaresNestedStruct_WithExplicitTypeSyntax)
    {
        std::string_view explicitStructSyntax = R"(
            function interface(IN,OUT)
                IN.struct = Type:Struct({
                    nested = Type:Struct({subnested = Type:Float()})
                })
            end

            function run(IN,OUT)
            end
        )";

        LuaScript* script = m_logicEngine.createLuaScript(explicitStructSyntax);

        auto inputs = script->getInputs();
        ASSERT_EQ(1u, inputs->getChildCount());

        auto str = inputs->getChild(0);
        EXPECT_EQ(EPropertyType::Struct, str->getType());
        EXPECT_EQ("struct", str->getName());
        ASSERT_EQ(1u, str->getChildCount());

        auto nested = str->getChild(0);
        EXPECT_EQ(EPropertyType::Struct, nested->getType());
        EXPECT_EQ("nested", nested->getName());
        ASSERT_EQ(1u, nested->getChildCount());

        auto subnested = nested->getChild(0);
        EXPECT_EQ(EPropertyType::Float, subnested->getType());
        EXPECT_EQ("subnested", subnested->getName());
        ASSERT_EQ(0u, subnested->getChildCount());
    }

    TEST_F(ALuaScript_Interface, ReportsError_WhenDeclaredStructWithExplicitTypeWithoutArgument)
    {
        std::string_view explicitStructSyntax = R"(
            function interface(IN,OUT)
                IN.struct = Type:Struct()
            end

            function run(IN,OUT)
            end
        )";

        LuaScript* script = m_logicEngine.createLuaScript(explicitStructSyntax);
        ASSERT_EQ(nullptr, script);
        EXPECT_THAT(m_logicEngine.getErrors().back().message, ::testing::HasSubstr("Type:Struct(T) invoked with invalid type parameter T!"));
    }

    TEST_F(ALuaScript_Interface, ReturnsItsTopLevelOutputsByIndex_OrderedLexicographically)
    {
        auto* script = m_logicEngine.createLuaScript(m_minimalScriptWithOutputs);

        auto outputs = script->getOutputs();

        ASSERT_EQ(11u, outputs->getChildCount());
        EXPECT_EQ("enabled", outputs->getChild(0)->getName());
        EXPECT_EQ(EPropertyType::Bool, outputs->getChild(0)->getType());
        EXPECT_EQ("name", outputs->getChild(1)->getName());
        EXPECT_EQ(EPropertyType::String, outputs->getChild(1)->getType());
        EXPECT_EQ("speed", outputs->getChild(2)->getName());
        EXPECT_EQ(EPropertyType::Int32, outputs->getChild(2)->getType());
        EXPECT_EQ("speed2", outputs->getChild(3)->getName());
        EXPECT_EQ(EPropertyType::Int64, outputs->getChild(3)->getType());
        EXPECT_EQ("temp", outputs->getChild(4)->getName());
        EXPECT_EQ(EPropertyType::Float, outputs->getChild(4)->getType());

        // Vec2/3/4 f/i
        EXPECT_EQ("vec2f", outputs->getChild(5)->getName());
        EXPECT_EQ(EPropertyType::Vec2f, outputs->getChild(5)->getType());
        EXPECT_EQ("vec2i", outputs->getChild(6)->getName());
        EXPECT_EQ(EPropertyType::Vec2i, outputs->getChild(6)->getType());
        EXPECT_EQ("vec3f", outputs->getChild(7)->getName());
        EXPECT_EQ(EPropertyType::Vec3f, outputs->getChild(7)->getType());
        EXPECT_EQ("vec3i", outputs->getChild(8)->getName());
        EXPECT_EQ(EPropertyType::Vec3i, outputs->getChild(8)->getType());
        EXPECT_EQ("vec4f", outputs->getChild(9)->getName());
        EXPECT_EQ(EPropertyType::Vec4f, outputs->getChild(9)->getType());
        EXPECT_EQ("vec4i", outputs->getChild(10)->getName());
        EXPECT_EQ(EPropertyType::Vec4i, outputs->getChild(10)->getType());
    }

    TEST_F(ALuaScript_Interface, ReturnsNestedOutputsByIndex_OrderedLexicographically_whenDeclaredOneByOne)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                OUT.struct = {}
                OUT.struct.field3 = {}
                OUT.struct.field2 = Type:Float()
                OUT.struct.field1 = Type:Int32()
                OUT.struct.field3.subfield2 = Type:Float()
                OUT.struct.field3.subfield1 = Type:Int32()
            end

            function run(IN,OUT)
            end
        )");

        ASSERT_NE(nullptr, script);

        auto outputs = script->getOutputs();
        ASSERT_EQ(1u, outputs->getChildCount());
        auto structField = outputs->getChild(0);
        EXPECT_EQ("struct", structField->getName());
        EXPECT_EQ(EPropertyType::Struct, structField->getType());

        ASSERT_EQ(3u, structField->getChildCount());
        auto field1 = structField->getChild(0);
        auto field2 = structField->getChild(1);
        auto field3 = structField->getChild(2);

        EXPECT_EQ("field1", field1->getName());
        EXPECT_EQ(EPropertyType::Int32, field1->getType());
        EXPECT_EQ("field2", field2->getName());
        EXPECT_EQ(EPropertyType::Float, field2->getType());
        EXPECT_EQ("field3", field3->getName());
        EXPECT_EQ(EPropertyType::Struct, field3->getType());

        ASSERT_EQ(2u, field3->getChildCount());
        auto subfield1 = field3->getChild(0);
        auto subfield2 = field3->getChild(1);

        EXPECT_EQ("subfield1", subfield1->getName());
        EXPECT_EQ(EPropertyType::Int32, subfield1->getType());
        EXPECT_EQ("subfield2", subfield2->getName());
        EXPECT_EQ(EPropertyType::Float, subfield2->getType());
    }

    TEST_F(ALuaScript_Interface, CanDeclarePropertiesProgramatically)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                OUT.root = {}
                local lastStruct = OUT.root
                for i=1,2 do
                    lastStruct["sub" .. tostring(i)] = {}
                    lastStruct = lastStruct["sub" .. tostring(i)]
                end
            end

            function run(IN,OUT)
            end
        )", WithStdModules({ EStandardModule::Base }));

        ASSERT_NE(nullptr, script);

        auto outputs = script->getOutputs();

        ASSERT_EQ(1u, outputs->getChildCount());
        auto root = outputs->getChild(0);
        EXPECT_EQ("root", root->getName());
        EXPECT_EQ(EPropertyType::Struct, root->getType());

        ASSERT_EQ(1u, root->getChildCount());
        auto sub1 = root->getChild(0);

        EXPECT_EQ("sub1", sub1->getName());
        EXPECT_EQ(EPropertyType::Struct, sub1->getType());

        ASSERT_EQ(1u, sub1->getChildCount());
        auto sub2 = sub1->getChild(0);
        EXPECT_EQ("sub2", sub2->getName());
        EXPECT_EQ(EPropertyType::Struct, sub2->getType());

        EXPECT_EQ(0u, sub2->getChildCount());
    }

    TEST_F(ALuaScript_Interface, MarksInputsAsInput)
    {
        auto* script = m_logicEngine.createLuaScript(m_minimalScriptWithInputs);
        auto  inputs = script->getInputs();
        const auto inputCount = inputs->getChildCount();
        for (size_t i = 0; i < inputCount; ++i)
        {
            EXPECT_EQ(internal::EPropertySemantics::ScriptInput, inputs->getChild(i)->m_impl->getPropertySemantics());
        }
    }

    TEST_F(ALuaScript_Interface, MarksOutputsAsOutput)
    {
        auto*      script     = m_logicEngine.createLuaScript(m_minimalScriptWithOutputs);
        auto       outputs     = script->getOutputs();
        const auto outputCount = outputs->getChildCount();
        for (size_t i = 0; i < outputCount; ++i)
        {
            EXPECT_EQ(internal::EPropertySemantics::ScriptOutput, outputs->getChild(i)->m_impl->getPropertySemantics());
        }
    }

    TEST_F(ALuaScript_Interface, AllowsAccessToWrappedUserdata)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.array_int = Type:Array(2, Type:Int32())
                OUT.struct = {a=Type:Int32(), b={c = Type:Int32(), d=Type:Float()}}
                OUT.array_struct = Type:Array(3, {a=Type:Int32(), b=Type:Float()})

                local expectedUserdata = {
                    IN,
                    IN.array_int,
                    OUT,
                    OUT.struct,
                    OUT.struct.a,
                    OUT.struct.b,
                    OUT.struct.b.c,
                    OUT.struct.b.d,
                    OUT.array_struct,
                    OUT.array_struct[2],
                    OUT.array_struct[2].a,
                    OUT.array_struct[2].b
                }

                for k, v in pairs(expectedUserdata) do
                    if type(v) ~= 'userdata' then
                        error("Expected userdata!")
                    end
                end
            end

            function run(IN,OUT)
            end
        )", WithStdModules({EStandardModule::Base}));
        ASSERT_NE(nullptr, script);
    }

    TEST_F(ALuaScript_Interface, ReportsErrorWhenAccessingStructByIndex)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                OUT.struct = {a=Type:Int32(), b={c = Type:Int32(), d=Type:Float()}}
                local causesError = OUT.struct[1]
            end

            function run(IN,OUT)
            end
        )");
        ASSERT_EQ(nullptr, script);
        EXPECT_THAT(m_logicEngine.getErrors().back().message, ::testing::HasSubstr("lua: error: Bad index access to struct 'struct': Expected a string but got object of type number instead!"));
    }

    TEST_F(ALuaScript_Interface, ReportsErrorWhenAccessingArrayByString)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.array_int = Type:Array(2, Type:Int32())
                local causesError = IN.array_int["causesError"]
            end

            function run(IN,OUT)
            end
        )");
        ASSERT_EQ(nullptr, script);
        EXPECT_THAT(m_logicEngine.getErrors().back().message, ::testing::HasSubstr("lua: error: Invalid index access in array 'array_int'"));
    }

    TEST_F(ALuaScript_Interface, ReportsErrorWhenAccessingArrayWithIndexOverflow)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.array_int = Type:Array(2, Type:Int32())
                local causesError = IN.array_int[3]
            end

            function run(IN,OUT)
            end
        )");
        ASSERT_EQ(nullptr, script);
        EXPECT_THAT(m_logicEngine.getErrors().back().message, ::testing::HasSubstr("Invalid index access in array 'array_int'. Expected index in the range [0, 2] but got 3 instead!"));
    }

    TEST_F(ALuaScript_Interface, AssignsDefaultValuesToItsInputs)
    {
        auto* script = m_logicEngine.createLuaScript(m_minimalScriptWithInputs);
        auto  inputs = script->getInputs();

        auto speedInt32 = inputs->getChild("speed");
        auto tempFloat = inputs->getChild("temp");
        auto nameString = inputs->getChild("name");
        auto enabledBool = inputs->getChild("enabled");
        auto vec_2f = inputs->getChild("vec2f");
        auto vec_3f = inputs->getChild("vec3f");
        auto vec_4f = inputs->getChild("vec4f");
        auto vec_2i = inputs->getChild("vec2i");
        auto vec_3i = inputs->getChild("vec3i");
        auto vec_4i = inputs->getChild("vec4i");
        EXPECT_EQ(0, *speedInt32->get<int32_t>());
        EXPECT_FLOAT_EQ(0.0f, *tempFloat->get<float>());
        EXPECT_EQ("", *nameString->get<std::string>());
        EXPECT_TRUE(enabledBool->get<bool>());
        EXPECT_FALSE(*enabledBool->get<bool>());
        EXPECT_THAT(*vec_2f->get<vec2f>(), ::testing::ElementsAre(0.0f, 0.0f));
        EXPECT_THAT(*vec_3f->get<vec3f>(), ::testing::ElementsAre(0.0f, 0.0f, 0.0f));
        EXPECT_THAT(*vec_4f->get<vec4f>(), ::testing::ElementsAre(0.0f, 0.0f, 0.0f, 0.0f));
        EXPECT_THAT(*vec_2i->get<vec2i>(), ::testing::ElementsAre(0, 0));
        EXPECT_THAT(*vec_3i->get<vec3i>(), ::testing::ElementsAre(0, 0, 0));
        EXPECT_THAT(*vec_4i->get<vec4i>(), ::testing::ElementsAre(0, 0, 0, 0));
    }

    TEST_F(ALuaScript_Interface, AssignsDefaultValuesToItsOutputs)
    {
        auto* script = m_logicEngine.createLuaScript(m_minimalScriptWithOutputs);
        auto  outputs = script->getOutputs();

        auto speedInt32 = outputs->getChild("speed");
        auto tempFloat = outputs->getChild("temp");
        auto nameString = outputs->getChild("name");
        auto enabledBool = outputs->getChild("enabled");
        auto vec_2f = outputs->getChild("vec2f");
        auto vec_3f = outputs->getChild("vec3f");
        auto vec_4f = outputs->getChild("vec4f");
        auto vec_2i = outputs->getChild("vec2i");
        auto vec_3i = outputs->getChild("vec3i");
        auto vec_4i = outputs->getChild("vec4i");

        EXPECT_TRUE(speedInt32->get<int32_t>());
        EXPECT_EQ(0, speedInt32->get<int32_t>().value());
        EXPECT_TRUE(tempFloat->get<float>());
        EXPECT_EQ(0.0f, tempFloat->get<float>().value());
        EXPECT_TRUE(nameString->get<std::string>());
        EXPECT_EQ("", nameString->get<std::string>().value());
        EXPECT_TRUE(enabledBool->get<bool>());
        EXPECT_EQ(false, *enabledBool->get<bool>());

        EXPECT_TRUE(vec_2f->get<vec2f>());
        EXPECT_TRUE(vec_3f->get<vec3f>());
        EXPECT_TRUE(vec_4f->get<vec4f>());
        EXPECT_TRUE(vec_2i->get<vec2i>());
        EXPECT_TRUE(vec_3i->get<vec3i>());
        EXPECT_TRUE(vec_4i->get<vec4i>());
        vec2f zeroVec2f{ 0.0f, 0.0f };
        vec3f zeroVec3f{ 0.0f, 0.0f, 0.0f };
        vec4f zeroVec4f{ 0.0f, 0.0f, 0.0f, 0.0f };
        vec2i zeroVec2i{ 0, 0 };
        vec3i zeroVec3i{ 0, 0, 0 };
        vec4i zeroVec4i{ 0, 0, 0, 0 };
        EXPECT_EQ(zeroVec2f, *vec_2f->get<vec2f>());
        EXPECT_EQ(zeroVec3f, *vec_3f->get<vec3f>());
        EXPECT_EQ(zeroVec4f, *vec_4f->get<vec4f>());
        EXPECT_EQ(zeroVec2i, *vec_2i->get<vec2i>());
        EXPECT_EQ(zeroVec3i, *vec_3i->get<vec3i>());
        EXPECT_EQ(zeroVec4i, *vec_4i->get<vec4i>());
    }

    TEST_F(ALuaScript_Interface, AssignsDefaultValuesToArrays)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.array_int = Type:Array(3, Type:Int32())
                IN.array_float = Type:Array(3, Type:Float())
                IN.array_vec2f = Type:Array(3, Type:Vec2f())
                OUT.array_int = Type:Array(3, Type:Int32())
                OUT.array_float = Type:Array(3, Type:Float())
                OUT.array_vec2f = Type:Array(3, Type:Vec2f())
            end

            function run(IN,OUT)
            end
        )");

        std::initializer_list<const Property*> rootProperties = { script->getInputs(), script->getOutputs() };

        for (auto rootProp : rootProperties)
        {
            auto array_int = rootProp->getChild("array_int");
            auto array_float = rootProp->getChild("array_float");
            auto array_vec2f = rootProp->getChild("array_vec2f");

            for (size_t i = 0; i < 3; ++i)
            {
                EXPECT_TRUE(array_int->getChild(i)->get<int32_t>());
                EXPECT_EQ(0, *array_int->getChild(i)->get<int32_t>());
                EXPECT_TRUE(array_float->getChild(i)->get<float>());
                EXPECT_FLOAT_EQ(0.0f, *array_float->getChild(i)->get<float>());
                EXPECT_TRUE(array_vec2f->getChild(i)->get<vec2f>());
                EXPECT_THAT(*array_vec2f->getChild(i)->get<vec2f>(), testing::ElementsAre(0.0f, 0.0f));
            }
        }
    }

    TEST_F(ALuaScript_Interface, ComputesSizeOfCustomPropertiesUsingCustomLengthFunction)
    {
        std::string_view scriptSrc = R"(
            function init()
                -- use global variables to store size info from interface(IN,OUT) - otherwise no way to transfer data
                -- outside of interface()!
                GLOBAL.sizes = {}
            end
            function interface(IN,OUT)
                IN.unused = Type:Int32()
                -- store size of IN with one property
                GLOBAL.sizes.inputsSizeSingle = rl_len(IN)
                IN.unused2 = Type:Int32()
                -- store size of IN with two properties
                GLOBAL.sizes.inputsSizeTwo = rl_len(IN)

                -- Create nested output to store (and provide) the sizes of all containers
                OUT.sizes = {
                    inputsSizeSingle = Type:Int32(),
                    inputsSizeTwo = Type:Int32(),
                    outputSizeSingleStruct = Type:Int32(),
                    outputSizeNested = Type:Int32(),
                    outputSizeArray = Type:Int32(),
                    outputSizeArrayElem = Type:Int32()
                }
                -- Store size of OUT
                GLOBAL.sizes.outputSizeSingleStruct = rl_len(OUT)
                -- Store size of OUT.sizes (nested container)
                GLOBAL.sizes.outputSizeNested = rl_len(OUT.sizes)

                OUT.array = Type:Array(5, {a=Type:Int32(), b=Type:Float()})
                GLOBAL.sizes.outputSizeArray = rl_len(OUT.array)
                GLOBAL.sizes.outputSizeArrayElem = rl_len(OUT.array[1])
            end

            function run(IN,OUT)
                OUT.sizes = GLOBAL.sizes
            end
        )";
        auto* script = m_logicEngine.createLuaScript(scriptSrc);
        ASSERT_NE(nullptr, script);
        EXPECT_TRUE(m_logicEngine.update());

        EXPECT_EQ(1, *script->getOutputs()->getChild("sizes")->getChild("inputsSizeSingle")->get<int32_t>());
        EXPECT_EQ(2, *script->getOutputs()->getChild("sizes")->getChild("inputsSizeTwo")->get<int32_t>());
        EXPECT_EQ(1, *script->getOutputs()->getChild("sizes")->getChild("outputSizeSingleStruct")->get<int32_t>());
        EXPECT_EQ(6, *script->getOutputs()->getChild("sizes")->getChild("outputSizeNested")->get<int32_t>());
        EXPECT_EQ(5, *script->getOutputs()->getChild("sizes")->getChild("outputSizeArray")->get<int32_t>());
        EXPECT_EQ(2, *script->getOutputs()->getChild("sizes")->getChild("outputSizeArrayElem")->get<int32_t>());
    }

    class ALuaScript_Interface_Sandboxing : public ALuaScript_Interface
    {
    };

    TEST_F(ALuaScript_Interface_Sandboxing, DeclaringGlobalSymbolsCausesCompilationErrors)
    {
        auto script = m_logicEngine.createLuaScript(R"(
            globalVar = "this will cause error"
        )", WithStdModules({ EStandardModule::Base }));

        ASSERT_EQ(nullptr, script);
        EXPECT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message,
            ::testing::HasSubstr("Declaring global variables is forbidden (exceptions: the functions 'init', 'interface' and 'run')! (found value of type 'string')"));
    }

    TEST_F(ALuaScript_Interface_Sandboxing, ReportsErrorWhenTryingToReadUnknownGlobals)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                local t = someGlobalVariable
            end

            function run(IN,OUT)
            end)");
        ASSERT_EQ(nullptr, script);

        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr(
                "Unexpected global access to key 'someGlobalVariable' in interface()! Only 'GLOBAL' and 'Type' are allowed as a key"));
    }

    TEST_F(ALuaScript_Interface_Sandboxing, ReportsErrorWhenSettingGlobals)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                thisCausesError = 'bad'
            end

            function run(IN,OUT)
            end)");
        ASSERT_EQ(nullptr, script);

        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr(
                "Unexpected global variable definition 'thisCausesError' in interface()! "
                "Use the GLOBAL table inside the init() function to declare global data and functions, or use modules!"));
    }

    TEST_F(ALuaScript_Interface_Sandboxing, ReportsErrorWhenTryingToOverrideGlobals)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function init()
            end

            function interface(IN,OUT)
                GLOBAL = {}
            end

            function run(IN,OUT)
            end)");
        ASSERT_EQ(nullptr, script);

        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr("Trying to override the GLOBAL table in interface()! You can only read data, but not overwrite the GLOBAL table!"));
    }

    TEST_F(ALuaScript_Interface_Sandboxing, ReportsErrorWhenTryingToOverrideTypesTable)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function init()
            end

            function interface(IN,OUT)
                Type = {}
            end

            function run(IN,OUT)
            end)");
        ASSERT_EQ(nullptr, script);

        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr("Can't override the 'Type' symbol in interface()!"));
    }

    TEST_F(ALuaScript_Interface_Sandboxing, ReportsErrorWhenTryingToDeclareInterfaceFunctionTwice)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            end

            function interface(IN,OUT)
            end

            function run(IN,OUT)
            end)");
        ASSERT_EQ(nullptr, script);

        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr("Function 'interface' can only be declared once!"));
    }

    TEST_F(ALuaScript_Interface_Sandboxing, ForbidsCallingSpecialFunctionsFromInsideInterface)
    {
        for (const auto& specialFunction  : std::vector<std::string>{ "init", "run", "interface" })
        {
            LuaScript* script = m_logicEngine.createLuaScript(fmt::format(R"(
                function init()
                end
                function run(IN,OUT)
                end

                function interface(IN,OUT)
                    {}()
                end
            )", specialFunction));

            ASSERT_EQ(nullptr, script);

            EXPECT_THAT(m_logicEngine.getErrors()[0].message,
                ::testing::HasSubstr(fmt::format("Unexpected global access to key '{}' in interface()! Only 'GLOBAL' and 'Type' are allowed as a key", specialFunction)));
        }
    }

    // Don't modify this test - it captures specific behavior related to a bug with Ramses Logic 1.0 after reworking the type system
    // Need to keep as confidence test to safeguard against regressions
    TEST_F(ALuaScript_Interface, Bugfix_Confidence_HandlesComplexTypeDeclarationWithNestedStructs)
    {
        rlogic::LogicEngine logicEngine;

        std::string scriptText = R"(
            function interface(IN,OUT)
                local craneGimbal = {
                    cam_Translation = Type:Vec3f(),
                    POS_ORIGIN_Translation = Type:Vec3f(),
                    PITCH_Rotation = Type:Vec3f(),
                    YAW_Rotation = Type:Vec3f()
                }

                local viewport = {
                    offsetX = Type:Int32(),
                    offsetY = Type:Int32(),
                    width = Type:Int32(),
                    height = Type:Int32()
                }

                local frustum_persp = {
                    nearPlane = Type:Float(),
                    farPlane =  Type:Float(),
                    fieldOfView = Type:Float(),
                    aspectRatio = Type:Float()
                }

                local frustum_ortho = {
                    nearPlane = Type:Float(),
                    farPlane =  Type:Float(),
                    leftPlane = Type:Float(),
                    rightPlane = Type:Float(),
                    bottomPlane = Type:Float(),
                    topPlane = Type:Float()
                }

                OUT.CameraSettings = {
                    CraneGimbal = craneGimbal,
                    CraneGimbal_R = craneGimbal,
                    scene_camera = {
                        Viewport = viewport,
                        Frustum = frustum_persp,
                    },
                    ui_camera = {
                        Frustum = frustum_ortho,
                        Viewport = viewport,
                        translation = Type:Vec3f(),
                    }
                }
            end
            function run(IN,OUT)
            end
        )";

        auto* script = logicEngine.createLuaScript(scriptText);
        ASSERT_TRUE(script != nullptr);
    }

    TEST_F(ALuaScript_Interface, OwnsAllProperties_confidenceTest)
    {
        const auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.array_int = Type:Array(2, Type:Int32())
                OUT.struct = {a=Type:Int32(), b={c = Type:Int32(), d=Type:Float()}}
                OUT.array_struct = Type:Array(3, {a=Type:Int32(), b=Type:Float()})
            end

            function run(IN,OUT)
            end
        )");
        ASSERT_NE(nullptr, script);

        int propsCount = 0;
        std::deque<const Property*> props{ script->getInputs(), script->getOutputs() };
        while (!props.empty())
        {
            const auto prop = props.back();
            props.pop_back();
            propsCount++;

            EXPECT_EQ(script, &prop->getOwningLogicNode());

            for (size_t i = 0u; i < prop->getChildCount(); ++i)
                props.push_back(prop->getChild(i));
        }

        EXPECT_EQ(20, propsCount); // just check that the unrolled recursion works
    }
}
