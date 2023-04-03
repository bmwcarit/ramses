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
#include "ramses-logic/RamsesAppearanceBinding.h"
#include "ramses-logic/RamsesNodeBinding.h"
#include "ramses-logic/RamsesCameraBinding.h"

#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/PerspectiveCamera.h"

#include <fstream>

namespace rlogic
{
    class ALuaScript_Runtime : public ALuaScript
    {
    protected:
    };

    TEST_F(ALuaScript_Runtime, ReportsErrorWhenAssigningVectorComponentsIndividually)
    {
        m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                OUT.vec3f = Type:Vec3f()
            end

            function run(IN,OUT)
                OUT.vec3f[1] = 1.0
            end
        )");

        m_logicEngine.update();

        EXPECT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("Error while writing to 'vec3f'. Can't assign individual components of vector types, must assign the whole vector"));
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorIfUndefinedInputIsUsedInRun)
    {
        auto script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            end
            function run(IN,OUT)
                local undefined = IN.undefined
            end
        )");

        ASSERT_NE(nullptr, script);
        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("Tried to access undefined struct property 'undefined'"));
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorIfUndefinedOutputIsUsedInRun)
    {
        auto script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            end
            function run(IN,OUT)
                OUT.undefined = 5
            end
        )");

        ASSERT_NE(nullptr, script);
        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("Tried to access undefined struct property 'undefined'"));
    }

    TEST_F(ALuaScript_Runtime, ReportsSourceNodeOnRuntimeError)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            end
            function run(IN,OUT)
                error("this causes an error")
            end
        )", WithStdModules({ EStandardModule::Base }));

        ASSERT_NE(nullptr, script);
        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("this causes an error"));
        EXPECT_EQ(script, m_logicEngine.getErrors()[0].object);
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorWhenTryingToWriteInputValues)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.value = Type:Float()
            end

            function run(IN,OUT)
                IN.value = 5
            end
        )");

        EXPECT_FALSE(m_logicEngine.update());

        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("lua: error: Error while writing to 'value'. Writing input values is not allowed, only outputs!"));
        EXPECT_EQ(script, m_logicEngine.getErrors()[0].object);
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorWhenTryingToAccessPropertiesWithNonStringIndexAtRunTime)
    {
        const std::vector<std::string> wrongIndexTypes = {"[1]", "[true]", "[{x=5}]", "[nil]"};

        std::vector<LuaTestError> allErrorCases;
        for (const auto& errorType : wrongIndexTypes)
        {
            allErrorCases.emplace_back(LuaTestError{"inputs" + errorType + " = 5", "lua: error: Bad access to property ''! Expected a string but got object of type"});
            allErrorCases.emplace_back(LuaTestError{"outputs" + errorType + " = 5", "lua: error: Bad access to property ''! Expected a string but got object of type"});
        }

        for (const auto& singleCase : allErrorCases)
        {
            auto script = m_logicEngine.createLuaScript(
                "function interface(inputs,outputs)\n"
                "end\n"
                "function run(inputs,outputs)\n" +
                singleCase.errorCode +
                "\n"
                "end\n");

            ASSERT_NE(nullptr, script);
            m_logicEngine.update();

            ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
            EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr(singleCase.expectedErrorMessage));
            m_logicEngine.destroy(*script);
        }
    }

    TEST_F(ALuaScript_Runtime, SetsValueOfTopLevelInputSuccessfully_WhenTemplateMatchesDeclaredInputType)
    {
        auto* script = m_logicEngine.createLuaScript(m_minimalScriptWithInputs);
        auto inputs = script->getInputs();

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

        EXPECT_TRUE(speedInt32->set<int32_t>(4711));
        EXPECT_EQ(4711, *speedInt32->get<int32_t>());
        EXPECT_TRUE(tempFloat->set<float>(5.5f));
        EXPECT_FLOAT_EQ(5.5f, *tempFloat->get<float>());
        EXPECT_TRUE(nameString->set<std::string>("name"));
        EXPECT_EQ("name", *nameString->get<std::string>());
        EXPECT_TRUE(enabledBool->set<bool>(true));
        EXPECT_EQ(true, *enabledBool->get<bool>());

        vec2f testvalVec2f{ 1.1f, 1.2f };
        vec3f testvalVec3f{ 2.1f, 2.2f, 2.3f };
        vec4f testvalVec4f{ 3.1f, 3.2f, 3.3f, 3.4f };
        vec2i testvalVec2i{ 1, 2 };
        vec3i testvalVec3i{ 3, 4, 5 };
        vec4i testvalVec4i{ 6, 7, 8, 9 };
        EXPECT_TRUE(vec_2f->set<vec2f>(testvalVec2f));
        EXPECT_TRUE(vec_3f->set<vec3f>(testvalVec3f));
        EXPECT_TRUE(vec_4f->set<vec4f>(testvalVec4f));
        EXPECT_TRUE(vec_2i->set<vec2i>(testvalVec2i));
        EXPECT_TRUE(vec_3i->set<vec3i>(testvalVec3i));
        EXPECT_TRUE(vec_4i->set<vec4i>(testvalVec4i));
        EXPECT_EQ(testvalVec2f, *vec_2f->get<vec2f>());
        EXPECT_EQ(testvalVec3f, *vec_3f->get<vec3f>());
        EXPECT_EQ(testvalVec4f, *vec_4f->get<vec4f>());
        EXPECT_EQ(testvalVec2i, *vec_2i->get<vec2i>());
        EXPECT_EQ(testvalVec3i, *vec_3i->get<vec3i>());
        EXPECT_EQ(testvalVec4i, *vec_4i->get<vec4i>());
    }

    TEST_F(ALuaScript_Runtime, ProvidesCalculatedValueAfterExecution)
    {
        auto* script = m_logicEngine.createLuaScript(R"(

            function interface(IN,OUT)
                IN.a = Type:Int32()
                IN.b = Type:Int32()
                OUT.result = Type:Int32()
            end

            function run(IN,OUT)
                OUT.result = IN.a + IN.b
            end
        )");

        auto inputs = script->getInputs();
        auto inputA = inputs->getChild("a");
        auto inputB = inputs->getChild("b");

        auto outputs = script->getOutputs();
        auto result = outputs->getChild("result");

        inputA->set(3);
        inputB->set(4);

        m_logicEngine.update();

        EXPECT_EQ(7, *result->get<int32_t>());
    }

    TEST_F(ALuaScript_Runtime, ReadsDataFromVec234Inputs)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.vec2f = Type:Vec2f()
                IN.vec3f = Type:Vec3f()
                IN.vec4f = Type:Vec4f()
                IN.vec2i = Type:Vec2i()
                IN.vec3i = Type:Vec3i()
                IN.vec4i = Type:Vec4i()
                OUT.sumOfAllFloats = Type:Float()
                OUT.sumOfAllInts = Type:Int32()
            end

            function run(IN,OUT)
                OUT.sumOfAllFloats =
                    IN.vec2f[1] + IN.vec2f[2] +
                    IN.vec3f[1] + IN.vec3f[2] + IN.vec3f[3] +
                    IN.vec4f[1] + IN.vec4f[2] + IN.vec4f[3] + IN.vec4f[4]
                OUT.sumOfAllInts =
                    IN.vec2i[1] + IN.vec2i[2] +
                    IN.vec3i[1] + IN.vec3i[2] + IN.vec3i[3] +
                    IN.vec4i[1] + IN.vec4i[2] + IN.vec4i[3] + IN.vec4i[4]
            end
        )");
        auto  inputs = script->getInputs();
        auto  outputs = script->getOutputs();

        inputs->getChild("vec2f")->set<vec2f>({ 1.1f, 1.2f });
        inputs->getChild("vec3f")->set<vec3f>({ 2.1f, 2.2f, 2.3f });
        inputs->getChild("vec4f")->set<vec4f>({ 3.1f, 3.2f, 3.3f, 3.4f });
        inputs->getChild("vec2i")->set<vec2i>({ 1, 2 });
        inputs->getChild("vec3i")->set<vec3i>({ 3, 4, 5 });
        inputs->getChild("vec4i")->set<vec4i>({ 6, 7, 8, 9 });

        ASSERT_TRUE(m_logicEngine.update());

        EXPECT_FLOAT_EQ(21.9f, *outputs->getChild("sumOfAllFloats")->get<float>());
        EXPECT_EQ(45, *outputs->getChild("sumOfAllInts")->get<int32_t>());
    }

    TEST_F(ALuaScript_Runtime, WritesValuesToVectorTypeOutputs)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
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
                OUT.vec2f = {0.1, 0.2}
                OUT.vec3f = {1.1, 1.2, 1.3}
                OUT.vec4f = {2.1, 2.2, 2.3, 2.4}
                OUT.vec2i = {1, 2}
                OUT.vec3i = {3, 4, 5}
                OUT.vec4i = {6, 7, 8, 9}

                OUT.nested =
                {
                    vec = {11, 12, 13},
                    float = 15.5
                }
            end
        )");

        EXPECT_TRUE(m_logicEngine.update());

        auto  outputs = script->getOutputs();

        EXPECT_THAT(*outputs->getChild("vec2f")->get<vec2f>(), ::testing::ElementsAre(0.1f, 0.2f));
        EXPECT_THAT(*outputs->getChild("vec3f")->get<vec3f>(), ::testing::ElementsAre(1.1f, 1.2f, 1.3f));
        EXPECT_THAT(*outputs->getChild("vec4f")->get<vec4f>(), ::testing::ElementsAre(2.1f, 2.2f, 2.3f, 2.4f));

        EXPECT_THAT(*outputs->getChild("vec2i")->get<vec2i>(), ::testing::ElementsAre(1, 2));
        EXPECT_THAT(*outputs->getChild("vec3i")->get<vec3i>(), ::testing::ElementsAre(3, 4, 5));
        EXPECT_THAT(*outputs->getChild("vec4i")->get<vec4i>(), ::testing::ElementsAre(6, 7, 8, 9));

        EXPECT_THAT(*outputs->getChild("nested")->getChild("vec")->get<vec3i>(), ::testing::ElementsAre(11, 12, 13));
        EXPECT_FLOAT_EQ(*outputs->getChild("nested")->getChild("float")->get<float>(), 15.5f);
    }

    TEST_F(ALuaScript_Runtime, PermitsAssigningOfVector_FromTable_WithNilsAtTheEnd)
    {
        // Lua+sol seem to not iterate over nil entries when creating a table
        // Still, we test the behavior explicitly
        const std::vector<std::string> allCases =
        {
            "OUT.vec2f = {1, 2, nil} -- single nil",
            "OUT.vec3f = {1, 2, 3, nil}",
            "OUT.vec4f = {1, 2, 3, 4, nil}",
            "OUT.vec2i = {1, 2, nil}",
            "OUT.vec3i = {1, 2, 3, nil}",
            "OUT.vec4i = {1, 2, 3, 4, nil}",
            "OUT.vec2f = {1, 2, nil, nil} -- two nils",
        };

        for (const auto& aCase : allCases)
        {
            auto scriptSource = std::string(R"(
            function interface(IN,OUT)
                OUT.vec2f = Type:Vec2f()
                OUT.vec3f = Type:Vec3f()
                OUT.vec4f = Type:Vec4f()
                OUT.vec2i = Type:Vec2i()
                OUT.vec3i = Type:Vec3i()
                OUT.vec4i = Type:Vec4i()
            end

            function run(IN,OUT)
            )");
            scriptSource += aCase;
            scriptSource += "\nend\n";

            auto* script = m_logicEngine.createLuaScript(scriptSource);

            ASSERT_NE(nullptr, script);
            EXPECT_TRUE(m_logicEngine.update());

            EXPECT_TRUE(m_logicEngine.getErrors().empty());
            EXPECT_TRUE(m_logicEngine.destroy(*script));
        }
    }

    TEST_F(ALuaScript_Runtime, PermitsAssigningOfVector_FromTable_WithKeyValuePairs)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                OUT.vec2f = Type:Vec2f()
                OUT.vec3i = Type:Vec3i()
            end

            function run(IN,OUT)
                OUT.vec2f = {[1] = 0.1, [2] = 0.2}
                OUT.vec3i = {[3] = 13, [2] = 12, [1] = 11} -- shuffled
            end
        )");

        ASSERT_NE(nullptr, script);
        EXPECT_TRUE(m_logicEngine.update());

        auto outputs = script->getOutputs();

        EXPECT_THAT(*outputs->getChild("vec2f")->get<vec2f>(), ::testing::ElementsAre(0.1f, 0.2f));

        EXPECT_THAT(*outputs->getChild("vec3i")->get<vec3i>(), ::testing::ElementsAre(11, 12, 13));
    }

    TEST_F(ALuaScript_Runtime, UsesNestedInputsToProduceResult)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.data = {
                    a = Type:Int32(),
                    b = Type:Int32()
                }
                OUT.result = Type:Int32()
            end
            function run(IN,OUT)
                OUT.result = IN.data.a + IN.data.b
            end
        )");

        auto inputs = script->getInputs();
        auto inputA = inputs->getChild("data")->getChild("a");
        auto inputB = inputs->getChild("data")->getChild("b");

        auto outputs = script->getOutputs();
        auto result = outputs->getChild("result");

        inputA->set(3);
        inputB->set(4);

        m_logicEngine.update();
        m_logicEngine.update();

        EXPECT_EQ(7, *result->get<int32_t>());
    }


    TEST_F(ALuaScript_Runtime, StoresDataToNestedOutputs_AsWholeStruct)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.data = Type:Int32()
                OUT.struct = {
                    field1 = Type:Int32(),
                    field2 = Type:Int32()
                }
            end
            function run(IN,OUT)
                OUT.struct = {
                    field1 = IN.data + IN.data,
                    field2 = IN.data * IN.data
                }
            end
        )");

        auto inputs = script->getInputs();
        auto input = inputs->getChild("data");

        auto outputs = script->getOutputs();
        auto field1 = outputs->getChild("struct")->getChild("field1");
        auto field2 = outputs->getChild("struct")->getChild("field2");

        input->set(5);

        EXPECT_TRUE(m_logicEngine.update());

        EXPECT_EQ(10, *field1->get<int32_t>());
        EXPECT_EQ(25, *field2->get<int32_t>());
    }

    TEST_F(ALuaScript_Runtime, StoresDataToNestedOutputs_Individually)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.data = Type:Int32()
                OUT.data = {
                    field1 = Type:Int32(),
                    field2 = Type:Int32()
                }
            end
            function run(IN,OUT)
                OUT.data.field1 = IN.data + IN.data
                OUT.data.field2 = IN.data * IN.data
            end
        )");

        auto inputs = script->getInputs();
        auto input = inputs->getChild("data");

        auto outputs = script->getOutputs();
        auto field1 = outputs->getChild("data")->getChild("field1");
        auto field2 = outputs->getChild("data")->getChild("field2");

        input->set(5);

        EXPECT_TRUE(m_logicEngine.update());

        EXPECT_EQ(10, *field1->get<int32_t>());
        EXPECT_EQ(25, *field2->get<int32_t>());
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorWhenAssigningNestedProperties_Underspecified)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                OUT.data = {
                    field1 = Type:Int32(),
                    field2 = Type:Int32()
                }
            end
            function run(IN,OUT)
                OUT.data = {
                    field1 = 5
                }
            end
        )");

        auto outputs = script->getOutputs();
        auto field1 = outputs->getChild("data")->getChild("field1");
        auto field2 = outputs->getChild("data")->getChild("field2");

        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("lua: error: Error while assigning struct 'data', expected a value for property 'field2' but found none!"));

        EXPECT_EQ(5, *field1->get<int32_t>());
        EXPECT_EQ(0, *field2->get<int32_t>());
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorWhenAssigningNestedProperties_Overspecified)
    {
        m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                OUT.data = {
                    field1 = Type:Int32(),
                    field2 = Type:Int32()
                }
            end
            function run(IN,OUT)
                OUT.data = {
                    field1 = 5,
                    field2 = 5,
                    not_specified = 5
                }
            end
        )");

        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("Unexpected property 'not_specified' while assigning values to struct 'data'!"));
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorWhenAssigningNestedProperties_WhenFieldHasWrongType)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                OUT.data = {
                    field1 = Type:Int32(),
                    field2 = Type:Int32()
                }
                OUT.field2 = Type:Int32()
            end
            function run(IN,OUT)
                OUT.field2 = "this is no integer"
                OUT.data = {
                    field1 = 5,
                    field2 = "this is no integer"
                }
            end
        )");

        auto outputs = script->getOutputs();
        auto field1 = outputs->getChild("data")->getChild("field1");
        auto field2 = outputs->getChild("data")->getChild("field2");

        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("Assigning string to 'Int32' output 'field2'!"));

        EXPECT_EQ(0, *field1->get<int32_t>());
        EXPECT_EQ(0, *field2->get<int32_t>());
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorWhenAssigningNestedProperties_WhenNestedSubStructDoesNotMatch_AndInterruptsAssignment)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                OUT.data = {
                    a_fieldBefore = Type:Int32(),
                    b_nested = {
                        field = Type:Int32()
                    },
                    c_fieldAfter = Type:Int32(),
                }
            end
            function run(IN,OUT)
                OUT.data = {
                    a_fieldBefore = 4,
                    b_nested = {
                        wrong_field = 5
                    },
                    c_fieldAfter = 6,
                }
            end
        )");

        auto outputs = script->getOutputs();
        auto fieldBefore = outputs->getChild("data")->getChild("a_fieldBefore");
        auto nestedfield = outputs->getChild("data")->getChild("b_nested")->getChild("field");
        auto fieldAfter = outputs->getChild("data")->getChild("c_fieldAfter");

        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("Unexpected property 'wrong_field' while assigning values to struct 'b_nested'"));

        // Assigned, because it was ordered alphabetically before the erroneous field
        EXPECT_EQ(4, *fieldBefore->get<int32_t>());
        // Had error -> keeps old value
        EXPECT_EQ(0, *nestedfield->get<int32_t>());
        // Ordered alphabetically after error field -> also keeps old value
        EXPECT_EQ(0, *fieldAfter->get<int32_t>());
    }

    TEST_F(ALuaScript_Runtime, AssignsValuesToArrays)
    {
        std::string_view scriptWithArrays = R"(
            function interface(IN,OUT)
                IN.array_int = Type:Array(2, Type:Int32())
                IN.array_int64 = Type:Array(2, Type:Int64())
                IN.array_float = Type:Array(3, Type:Float())
                OUT.array_int = Type:Array(2, Type:Int32())
                OUT.array_int64 = Type:Array(2, Type:Int64())
                OUT.array_float = Type:Array(3, Type:Float())
            end

            function run(IN,OUT)
                OUT.array_int = IN.array_int
                OUT.array_int[2] = 5
                OUT.array_int64 = IN.array_int64
                OUT.array_int64[2] = 5
                OUT.array_float = IN.array_float
                OUT.array_float[1] = 1.5
            end
        )";

        auto* script = m_logicEngine.createLuaScript(scriptWithArrays);

        auto inputs = script->getInputs();
        auto in_array_int = inputs->getChild("array_int");
        auto in_array_int64 = inputs->getChild("array_int64");
        auto in_array_float = inputs->getChild("array_float");
        in_array_int->getChild(0)->set<int32_t>(1);
        in_array_int->getChild(1)->set<int32_t>(2);
        in_array_int64->getChild(0)->set<int64_t>(3);
        in_array_int64->getChild(1)->set<int64_t>(4);
        in_array_float->getChild(0)->set<float>(0.1f);
        in_array_float->getChild(1)->set<float>(0.2f);
        in_array_float->getChild(2)->set<float>(0.3f);

        EXPECT_TRUE(m_logicEngine.update());

        auto outputs = script->getOutputs();
        auto out_array_int = outputs->getChild("array_int");
        auto out_array_int64 = outputs->getChild("array_int64");
        auto out_array_float = outputs->getChild("array_float");

        EXPECT_EQ(1, *out_array_int->getChild(0)->get<int32_t>());
        EXPECT_EQ(5, *out_array_int->getChild(1)->get<int32_t>());

        EXPECT_EQ(3, *out_array_int64->getChild(0)->get<int64_t>());
        EXPECT_EQ(5, *out_array_int64->getChild(1)->get<int64_t>());

        EXPECT_FLOAT_EQ(1.5f, *out_array_float->getChild(0)->get<float>());
        EXPECT_FLOAT_EQ(0.2f, *out_array_float->getChild(1)->get<float>());
        EXPECT_FLOAT_EQ(0.3f, *out_array_float->getChild(2)->get<float>());
    }

    TEST_F(ALuaScript_Runtime, AssignsNestedTableToMultidimensionalArrays)
    {
        std::string_view scriptWithMultiDimArrays = R"(
            function interface(IN,OUT)
                OUT.array2d = Type:Array(2, Type:Array(2, Type:Int32()))
            end

            function run(IN,OUT)
                OUT.array2d = {
                    {1,2}, {3, 4},
                }
            end
        )";

        auto* script = m_logicEngine.createLuaScript(scriptWithMultiDimArrays);

        EXPECT_TRUE(m_logicEngine.update());

        auto array2DOut = script->getOutputs()->getChild("array2d");

        int32_t value = 1;
        for (int32_t i = 0; i < 2; ++i)
        {
            for (int32_t j = 0; j < 2; ++j)
            {
                EXPECT_EQ(value, *array2DOut->getChild(i)->getChild(j)->get<int32_t>());
                ++value;
            }
        }
    }

    TEST_F(ALuaScript_Runtime, AssignsInputValuesToMultidimensionalArrays)
    {
        std::string_view scriptWithMultiDimArrays = R"(
            function interface(IN,OUT)
                local arrayType = Type:Array(2, Type:Array(2, Type:Array(2, Type:Int32())))
                IN.array3d = arrayType
                OUT.array3d = arrayType
            end

            function run(IN,OUT)
                OUT.array3d = IN.array3d
            end
        )";

        auto* script = m_logicEngine.createLuaScript(scriptWithMultiDimArrays);

        auto array3DIn = script->getInputs()->getChild("array3d");

        for (int32_t i = 0; i < 2; ++i)
        {
            for (int32_t j = 0; j < 2; ++j)
            {
                for (int32_t k = 0; k < 2; ++k)
                {
                    array3DIn->getChild(i)->getChild(j)->getChild(k)->set<int32_t>(i * j * k);
                }
            }
        }

        EXPECT_TRUE(m_logicEngine.update());

        auto array3DOut = script->getOutputs()->getChild("array3d");

        for (int32_t i = 0; i < 2; ++i)
        {
            for (int32_t j = 0; j < 2; ++j)
            {
                for (int32_t k = 0; k < 2; ++k)
                {
                    EXPECT_EQ(i * j * k, *array3DOut->getChild(i)->getChild(j)->getChild(k)->get<int32_t>());
                }
            }
        }
    }

    TEST_F(ALuaScript_Runtime, AssignsDataToMultidimensionalArraysWithStructs_FromExampleInDocs)
    {
        std::string_view scriptSrc = R"(
            function interface(IN, OUT)
                local coalaStruct = Type:Struct({name = Type:String()})
                -- 100 x 100 array (2 dimensions), element type is a struct
                OUT.coala_army = Type:Array(100, Type:Array(100, coalaStruct))
            end

            function run(IN, OUT)
                for i,row in rl_ipairs(OUT.coala_army) do
                    for j,coala in rl_ipairs(row) do
                        coala.name = "soldier " .. tostring(i) .. "-" .. tostring(j)
                    end
                end
            end
        )";

        auto* script = m_logicEngine.createLuaScript(scriptSrc, WithStdModules({EStandardModule::Base}));

        ASSERT_NE(nullptr, script);
        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_EQ("soldier 8-7", *script->getOutputs()->getChild("coala_army")->getChild(7)->getChild(6)->getChild("name")->get<std::string>());
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorWhenAccessingArrayWithNonIntegerIndex)
    {
        const std::string_view scriptTemplate = (R"(
            function interface(IN,OUT)
                IN.array = Type:Array(2, Type:Int32())
                OUT.array = Type:Array(2, Type:Int32())
            end
            function run(IN,OUT)
                {}
            end
        )");

        const std::vector<std::string> invalidStatements
        {
            "IN.array.name = 5",
            "OUT.array.name = 5",
            "IN.array[true] = 5",
            "OUT.array[true] = 5",
            "IN.array[{x=5}] = 5",
            "OUT.array[{x=5}] = 5",
            "IN.array[nil] = 5",
            "OUT.array[nil] = 5",
            "IN.array[IN] = 5",
            "OUT.array[IN] = 5",
        };

        for (const auto& invalidStatement : invalidStatements)
        {
            auto script = m_logicEngine.createLuaScript(fmt::format(scriptTemplate, invalidStatement));

            ASSERT_NE(nullptr, script);
            EXPECT_FALSE(m_logicEngine.update());

            ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
            EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("Bad access to property 'array'! Error while extracting integer: expected a number, received"));
            m_logicEngine.destroy(*script);
        }
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorWhenAccessingArrayOutOfRange)
    {
        const std::string_view scriptTemplate = (R"(
            function interface(IN,OUT)
                IN.array = Type:Array(2, Type:Int32())
                OUT.array = Type:Array(2, Type:Int32())
            end
            function run(IN,OUT)
                {}
            end
        )");

        std::vector<LuaTestError> allErrorCases;
        for (auto idx : std::initializer_list<int>{ -1, 0, 3})
        {
            std::string errorTemplate =
                (idx < 0) ?
                "Bad access to property 'array'! Error while extracting integer: expected non-negative number, received '-1'" :
                "Index out of range! Expected 0 < index <= 2 but received index == {}";

            for (const auto& prop : std::vector<std::string>{ "IN", "OUT" })
            {
                allErrorCases.emplace_back(LuaTestError{
                    fmt::format("{}.array[{}] = 5", prop, idx),
                    fmt::format(errorTemplate, idx)
                    });
            }
        }

        for (const auto& singleCase : allErrorCases)
        {
            auto script = m_logicEngine.createLuaScript(fmt::format(scriptTemplate, singleCase.errorCode));

            ASSERT_NE(nullptr, script);
            EXPECT_FALSE(m_logicEngine.update());

            ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
            EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr(singleCase.expectedErrorMessage));
            m_logicEngine.destroy(*script);
        }
    }

    TEST_F(ALuaScript_Runtime, AssignArrayValuesFromLuaTable)
    {
        auto script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                OUT.int_array = Type:Array(2, Type:Int32())
                OUT.int64_array = Type:Array(2, Type:Int64())
                OUT.float_array = Type:Array(2, Type:Float())
                OUT.vec2i_array = Type:Array(2, Type:Vec2i())
                OUT.vec3f_array = Type:Array(2, Type:Vec3f())
            end
            function run(IN,OUT)
                OUT.int_array = {1, 2}
                OUT.int64_array = {3, 4}
                OUT.float_array = {0.1, 0.2}
                OUT.vec2i_array = {{11, 12}, {21, 22}}
                OUT.vec3f_array = {{0.11, 0.12, 0.13}, {0.21, 0.22, 0.23}}
            end
        )");

        ASSERT_NE(nullptr, script);

        EXPECT_TRUE(m_logicEngine.update());

        const auto int_array = script->getOutputs()->getChild("int_array");
        const auto int64_array = script->getOutputs()->getChild("int64_array");
        const auto float_array = script->getOutputs()->getChild("float_array");
        const auto vec2i_array = script->getOutputs()->getChild("vec2i_array");
        const auto vec3f_array = script->getOutputs()->getChild("vec3f_array");

        EXPECT_EQ(1, *int_array->getChild(0)->get<int32_t>());
        EXPECT_EQ(2, *int_array->getChild(1)->get<int32_t>());
        EXPECT_EQ(3, *int64_array->getChild(0)->get<int64_t>());
        EXPECT_EQ(4, *int64_array->getChild(1)->get<int64_t>());
        EXPECT_FLOAT_EQ(0.1f, *float_array->getChild(0)->get<float>());
        EXPECT_FLOAT_EQ(0.2f, *float_array->getChild(1)->get<float>());
        EXPECT_THAT(*vec2i_array->getChild(0)->get<vec2i>(), ::testing::ElementsAre(11, 12));
        EXPECT_THAT(*vec2i_array->getChild(1)->get<vec2i>(), ::testing::ElementsAre(21, 22));
        EXPECT_THAT(*vec3f_array->getChild(0)->get<vec3f>(), ::testing::ElementsAre(0.11f, 0.12f, 0.13f));
        EXPECT_THAT(*vec3f_array->getChild(1)->get<vec3f>(), ::testing::ElementsAre(0.21f, 0.22f, 0.23f));
    }

    TEST_F(ALuaScript_Runtime, AssignArrayValuesFromLuaTable_WithExplicitKeys)
    {
        auto script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                OUT.int_array = Type:Array(3, Type:Int32())
            end
            function run(IN,OUT)
                OUT.int_array = {[1] = 11, [2] = 12, [3] = 13}
            end
        )");

        ASSERT_NE(nullptr, script);

        EXPECT_TRUE(m_logicEngine.update());

        auto int_array = script->getOutputs()->getChild("int_array");

        EXPECT_EQ(11, *int_array->getChild(0)->get<int32_t>());
        EXPECT_EQ(12, *int_array->getChild(1)->get<int32_t>());
        EXPECT_EQ(13, *int_array->getChild(2)->get<int32_t>());
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorWhenAssigningArrayWithFewerElementsThanRequired_UsingExplicitIndices)
    {
        auto script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                OUT.int_array = Type:Array(3, Type:Int32())
            end
            function run(IN,OUT)
                OUT.int_array = {[1] = 11, [2] = 12}
            end
        )");

        ASSERT_NE(nullptr, script);

        EXPECT_FALSE(m_logicEngine.update());

        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("Error during assignment of array property 'int_array'! Expected a value at index 3"));
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorWhenAssigningArrayFromLuaTableWithCorrectSizeButWrongIndices)
    {
        auto script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                OUT.int_array = Type:Array(3, Type:Int32())
            end
            function run(IN,OUT)
                -- 3 values, but use [1, 3, 4] instead of [1, 2, 3]
                OUT.int_array = {[1] = 11, [3] = 13, [4] = 14}
            end
        )");

        ASSERT_NE(nullptr, script);

        EXPECT_FALSE(m_logicEngine.update());

        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("Error during assignment of array property 'int_array'! Expected a value at index 2"));
    }

    TEST_F(ALuaScript_Runtime, AssignsMultidimensionalArrays)
    {
        const std::string_view scriptSrc = (R"(
            function interface(IN,OUT)
                OUT.array2d_int = Type:Array(2, Type:Array(2, Type:Int32()))
                OUT.array3d_float = Type:Array(1, Type:Array(1, Type:Array(1, Type:Float())))
            end
            function run(IN,OUT)
                OUT.array2d_int = {{1, 2}, {3, 4}}
                OUT.array3d_float = {{{1.4}}}
            end
        )");

        auto script = m_logicEngine.createLuaScript(scriptSrc);

        ASSERT_NE(nullptr, script);
        EXPECT_TRUE(m_logicEngine.update());

        auto outArray2d = script->getOutputs()->getChild("array2d_int");
        EXPECT_EQ(1, *outArray2d->getChild(0)->getChild(0)->get<int32_t>());
        EXPECT_EQ(2, *outArray2d->getChild(0)->getChild(1)->get<int32_t>());
        EXPECT_EQ(3, *outArray2d->getChild(1)->getChild(0)->get<int32_t>());
        EXPECT_EQ(4, *outArray2d->getChild(1)->getChild(1)->get<int32_t>());
        auto outArray3d = script->getOutputs()->getChild("array3d_float");
        EXPECT_FLOAT_EQ(1.4f, *outArray3d->getChild(0)->getChild(0)->getChild(0)->get<float>());
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorWhenAssigningArraysWrongValues)
    {
        const std::string_view scriptTemplate = (R"(
            function interface(IN,OUT)
                OUT.array_int = Type:Array(2, Type:Int32())
                OUT.array_int64 = Type:Array(2, Type:Int64())
                OUT.array_string = Type:Array(2, Type:String())
                OUT.array_vec2f = Type:Array(2, Type:Vec2f())
            end
            function run(IN,OUT)
                {}
            end
        )");

        // This is a subset of all possible permutations, but should cover most types and cases
        const std::vector<LuaTestError> allErrorCases = {
            {"OUT.array_int = {}", "Error during assignment of array property 'array_int'! Expected a value at index 1"},
            {"OUT.array_int = {1}", "Error during assignment of array property 'array_int'! Expected a value at index 2"},
            {"OUT.array_int = {1, 2, 3}", "Element size mismatch when assigning array property 'array_int'! Expected array size: 2"},
            {"OUT.array_int = {1, 2.2}", "Error while extracting integer: implicit rounding (fractional part '0.20000000000000018' is not negligible)"},
            {"OUT.array_int = {1, true}", "Assigning bool to 'Int32' output ''"},
            {"OUT.array_int = {nil, 1, 3}", "Error during assignment of array property 'array_int'! Expected a value at index 1"},
            {"OUT.array_int = {1, nil, 3}", "Error during assignment of array property 'array_int'! Expected a value at index 2"},
            {"OUT.array_int64 = {}", "Error during assignment of array property 'array_int64'! Expected a value at index 1"},
            {"OUT.array_int64 = {1}", "Error during assignment of array property 'array_int64'! Expected a value at index 2"},
            {"OUT.array_int64 = {1, 2, 3}", "Element size mismatch when assigning array property 'array_int64'! Expected array size: 2"},
            {"OUT.array_int64 = {1, 2.2}", "Error while extracting integer: implicit rounding (fractional part '0.20000000000000018' is not negligible)"},
            {"OUT.array_int64 = {1, true}", "Assigning bool to 'Int64' output ''"},
            {"OUT.array_int64 = {nil, 1, 3}", "Error during assignment of array property 'array_int64'! Expected a value at index 1"},
            {"OUT.array_int64 = {1, nil, 3}", "Error during assignment of array property 'array_int64'! Expected a value at index 2"},
            // TODO Violin Improve error messages to contain info which array field failed to be assigned
            // currently we report empty string which is the name of array elements) - not easy to understand by TA
            {"OUT.array_string = {'somestring', 2}", "Assigning number to 'String' output ''"},
            {"OUT.array_string = {'somestring', {}}", "Assigning table to 'String' output ''"},
            {"OUT.array_string = {'somestring', OUT.array_int}", "Can't assign property 'array_int' (type Array) to property '' (type String)"},
            {"OUT.array_vec2f = {1, 2}", "Error while assigning output Vec2 property ''. Expected a Lua table with 2 entries but got object of type number instead!"},
            {"OUT.array_vec2f = {{1, 2}, {5}}", "Error while assigning output Vec2 property ''. Error while extracting array: expected 2 array components in table but got 1 instead!"},
            {"OUT.array_vec2f = {{1, 2}, {}}", "Error while assigning output Vec2 property ''. Error while extracting array: expected 2 array components in table but got 0 instead!"},
            {"OUT.array_int = OUT", "Can't assign property '' (type Struct) to property 'array_int' (type Array)"},
            {"OUT.array_int = IN", "Can't assign property '' (type Struct) to property 'array_int' (type Array)"},
        };

        for (const auto& singleCase : allErrorCases)
        {
            auto script = m_logicEngine.createLuaScript(fmt::format(scriptTemplate, singleCase.errorCode));

            ASSERT_NE(nullptr, script);
            EXPECT_FALSE(m_logicEngine.update());

            ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
            EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr(singleCase.expectedErrorMessage));
            m_logicEngine.destroy(*script);
        }
    }

    TEST_F(ALuaScript_Runtime, AssignsValuesArraysInVariousLuaSyntaxStyles)
    {
        auto script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.array = Type:Array(3, Type:Vec2i())
                OUT.array = Type:Array(3, Type:Vec2i())
            end
            function run(IN,OUT)
                -- assign from "everything" towards "just one value" to cover as many cases as possible
                OUT.array = IN.array
                OUT.array[2] = IN.array[2]
                OUT.array[3] = {5, 6}
            end
        )");

        ASSERT_NE(nullptr, script);
        EXPECT_TRUE(script->getInputs()->getChild("array")->getChild(0)->set<vec2i>({ 1, 2 }));
        EXPECT_TRUE(script->getInputs()->getChild("array")->getChild(1)->set<vec2i>({ 3, 4 }));
        EXPECT_TRUE(script->getInputs()->getChild("array")->getChild(2)->set<vec2i>({ 5, 6 }));
        ASSERT_TRUE(m_logicEngine.update());
        EXPECT_THAT(*script->getOutputs()->getChild("array")->getChild(0)->get<vec2i>(), ::testing::ElementsAre(1, 2));
        EXPECT_THAT(*script->getOutputs()->getChild("array")->getChild(1)->get<vec2i>(), ::testing::ElementsAre(3, 4));
        EXPECT_THAT(*script->getOutputs()->getChild("array")->getChild(2)->get<vec2i>(), ::testing::ElementsAre(5, 6));
    }

    TEST_F(ALuaScript_Runtime, AssignsValuesArraysInVariousLuaSyntaxStyles_InNestedStruct)
    {
        auto script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.struct = {
                    array1  = Type:Array(1, Type:Vec2f()),
                    array2  = Type:Array(2, Type:Vec3f())
                }
                OUT.struct = {
                    array1  = Type:Array(1, Type:Vec2f()),
                    array2  = Type:Array(2, Type:Vec3f())
                }
            end
            function run(IN,OUT)
                -- assign from "everything" towards "just one value" to cover as many cases as possible
                OUT.struct = IN.struct
                OUT.struct.array1    = IN.struct.array1
                OUT.struct.array2[1]    = {1.1, 1.2, 1.3}
                OUT.struct.array2[2]    = IN.struct.array2[2]
            end
        )");

        ASSERT_NE(nullptr, script);
        EXPECT_TRUE(script->getInputs()->getChild("struct")->getChild("array1")->getChild(0)->set<vec2f>({ 0.1f, 0.2f }));
        EXPECT_TRUE(script->getInputs()->getChild("struct")->getChild("array2")->getChild(0)->set<vec3f>({ 1.1f, 1.2f, 1.3f }));
        EXPECT_TRUE(script->getInputs()->getChild("struct")->getChild("array2")->getChild(1)->set<vec3f>({ 2.1f, 2.2f, 2.3f }));
        ASSERT_TRUE(m_logicEngine.update());
        EXPECT_THAT(*script->getOutputs()->getChild("struct")->getChild("array1")->getChild(0)->get<vec2f>(), ::testing::ElementsAre(0.1f, 0.2f));
        EXPECT_THAT(*script->getOutputs()->getChild("struct")->getChild("array2")->getChild(0)->get<vec3f>(), ::testing::ElementsAre(1.1f, 1.2f, 1.3f));
        EXPECT_THAT(*script->getOutputs()->getChild("struct")->getChild("array2")->getChild(1)->get<vec3f>(), ::testing::ElementsAre(2.1f, 2.2f, 2.3f));
    }

    TEST_F(ALuaScript_Runtime, AllowsAssigningArraysFromTableWithNilAtTheEnd)
    {
        const std::string_view scriptTemplate = (R"(
            function interface(IN,OUT)
                OUT.array_2ints = Type:Array(2, Type:Int32())
                OUT.array_3ints = Type:Array(3, Type:Int32())
                OUT.array_4ints = Type:Array(4, Type:Int32())
                OUT.array_vec2i = Type:Array(1, Type:Vec2i())
            end

            function run(IN,OUT)
                {}
            end
        )");

        // Lua+sol seem to not iterate over nil entries when creating a table
        // Still, we test the behavior explicitly
        const std::vector<std::string> allCases =
        {
            "OUT.array_2ints = {1, 2, nil} -- single nil",
            "OUT.array_2ints = {1, 2, nil, nil} -- two nils",
            "OUT.array_3ints = {1, 2, 3, nil}",
            "OUT.array_4ints = {1, 2, 3, 4, nil}",
            "OUT.array_vec2i = {{1, 2}, nil}",
        };

        for (const auto& aCase : allCases)
        {
            auto* script = m_logicEngine.createLuaScript(fmt::format(scriptTemplate, aCase));

            ASSERT_NE(nullptr, script);
            EXPECT_TRUE(m_logicEngine.update());

            EXPECT_TRUE(m_logicEngine.getErrors().empty());
            EXPECT_TRUE(m_logicEngine.destroy(*script));
        }
    }

    TEST_F(ALuaScript_Runtime, ReportsErrorWhenAssigningArraysWithMismatchedSizes)
    {
        const std::string_view scriptTemplate = (R"(
            function interface(IN,OUT)
                IN.array_float2 = Type:Array(2, Type:Float())
                IN.array_float4 = Type:Array(4, Type:Float())
                IN.array_vec3f = Type:Array(1, Type:Vec3f())
                OUT.array_float3 = Type:Array(3, Type:Float())
            end

            function run(IN,OUT)
                {}
            end
        )");

        const std::vector<LuaTestError> allCases =
        {
            {"OUT.array_float3 = IN.array_float2", "Can't assign property 'array_float2' (#fields=2) to property 'array_float3' (#fields=3)"},
            {"OUT.array_float3 = IN.array_float4", "Can't assign property 'array_float4' (#fields=4) to property 'array_float3' (#fields=3)!"},
            {"OUT.array_float3 = IN.array_vec3f", "Can't assign property 'array_vec3f' (#fields=1) to property 'array_float3' (#fields=3)"},
            {"OUT.array_float3 = {0.1, 0.2}", "Error during assignment of array property 'array_float3'! Expected a value at index 3"},
            {"OUT.array_float3 = {0.1, 0.2, 0.3, 0.4}", "Element size mismatch when assigning array property 'array_float3'! Expected array size: 3"},
            {"OUT.array_float3 = {}", "Error during assignment of array property 'array_float3'! Expected a value at index 1"},
        };

        for (const auto& aCase : allCases)
        {
            auto* script = m_logicEngine.createLuaScript(fmt::format(scriptTemplate, aCase.errorCode));

            ASSERT_NE(nullptr, script);
            EXPECT_FALSE(m_logicEngine.update());

            ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
            EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr(aCase.expectedErrorMessage));
            EXPECT_TRUE(m_logicEngine.destroy(*script));
        }
    }

    TEST_F(ALuaScript_Runtime, ReportsErrorWhenAssigningUserdataArraysWithMismatchedTypes)
    {
        const std::string_view scriptTemplate = (R"(
            function interface(IN,OUT)
                IN.array_float = Type:Array(2, Type:Float())
                IN.array_vec2f = Type:Array(2, Type:Vec2f())
                IN.array_vec2i = Type:Array(2, Type:Vec2i())
                OUT.array_int = Type:Array(2, Type:Int32())
                OUT.array_int64 = Type:Array(2, Type:Int64())
            end

            function run(IN,OUT)
                {}
            end
        )");

        const std::vector<LuaTestError> allCases =
        {
            {"OUT.array_int = IN.array_float", "Can't assign property '' (type Float) to property '' (type Int32)!"},
            {"OUT.array_int = IN.array_vec2f", "Can't assign property '' (type Vec2f) to property '' (type Int32)!"},
            {"OUT.array_int = IN.array_vec2i", "Can't assign property '' (type Vec2i) to property '' (type Int32)!"},
            {"OUT.array_int64 = IN.array_float", "Can't assign property '' (type Float) to property '' (type Int64)!"},
            {"OUT.array_int64 = IN.array_vec2f", "Can't assign property '' (type Vec2f) to property '' (type Int64)!"},
            {"OUT.array_int64 = IN.array_vec2i", "Can't assign property '' (type Vec2i) to property '' (type Int64)!"},
        };

        for (const auto& aCase : allCases)
        {
            auto* script = m_logicEngine.createLuaScript(fmt::format(scriptTemplate, aCase.errorCode));

            ASSERT_NE(nullptr, script);
            EXPECT_FALSE(m_logicEngine.update());

            ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
            EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr(aCase.expectedErrorMessage));
            EXPECT_TRUE(m_logicEngine.destroy(*script));
        }
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorWhenImplicitlyRoundingNumbers)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.float1 = Type:Float()
                IN.float2 = Type:Float()
                OUT.int = Type:Int32()
                OUT.int64 = Type:Int64()
            end
            function run(IN,OUT)
                OUT.int = IN.float1
                OUT.int64 = IN.float2
            end
        )");

        auto float1Input = script->getInputs()->getChild("float1");
        auto float2Input = script->getInputs()->getChild("float2");
        auto intOutput = script->getOutputs()->getChild("int");
        auto int64Output = script->getOutputs()->getChild("int64");

        float1Input->set<float>(1.0f);
        float2Input->set<float>(1.0f);

        EXPECT_TRUE(m_logicEngine.update());
        ASSERT_TRUE(m_logicEngine.getErrors().empty());
        EXPECT_EQ(1, *intOutput->get<int32_t>());
        EXPECT_EQ(1, *int64Output->get<int64_t>());

        float1Input->set<float>(2.5f);
        float2Input->set<float>(1.f);

        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message,
            ::testing::HasSubstr("Error during assignment of property 'int'! Error while extracting integer: implicit rounding (fractional part '0.5' is not negligible)"));
        EXPECT_EQ(1, *intOutput->get<int32_t>());
        EXPECT_EQ(1, *int64Output->get<int64_t>());

        float1Input->set<float>(1.f);
        float2Input->set<float>(2.5f);

        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message,
            ::testing::HasSubstr("Error during assignment of property 'int64'! Error while extracting integer: implicit rounding (fractional part '0.5' is not negligible)"));
        EXPECT_EQ(1, *intOutput->get<int32_t>());
        EXPECT_EQ(1, *int64Output->get<int64_t>());
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorWhenAssigningNilToIntOutputs)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                OUT.int = Type:Int32()
            end
            function run(IN,OUT)
                OUT.int = nil
            end
        )");

        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("Assigning nil to 'Int32' output 'int'!"));
        EXPECT_EQ(0, *script->getOutputs()->getChild("int")->get<int32_t>());
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorWhenAssigningBoolToIntOutputs)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                OUT.int = Type:Int32()
            end
            function run(IN,OUT)
                OUT.int = true
            end
        )");

        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("Assigning bool to 'Int32' output 'int'!"));
        EXPECT_EQ(0, *script->getOutputs()->getChild("int")->get<int32_t>());
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorWhenAssigningBoolToStringOutputs)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                OUT.str = Type:String()
            end
            function run(IN,OUT)
                OUT.str = "this is quite ok"
                OUT.str = true   -- this is not ok
            end
        )");

        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("Assigning bool to 'String' output 'str'!"));
        EXPECT_EQ("this is quite ok", *script->getOutputs()->getChild("str")->get<std::string>());
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorWhenAssigningNumberToStringOutputs)
    {
        m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                OUT.str = Type:String()
            end
            function run(IN,OUT)
                OUT.str = 42   -- this is not ok
            end
        )");

        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr("Assigning number to 'String' output 'str'!"));
    }

    TEST_F(ALuaScript_Runtime, SupportsMultipleLevelsOfNestedInputs_confidenceTest)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.rabbit = {
                    color = {
                        r = Type:Float(),
                        g = Type:Float(),
                        b = Type:Float()
                    },
                    speed = Type:Int32()
                }
                OUT.result = Type:Float()

            end
            function run(IN,OUT)
                OUT.result = (IN.rabbit.color.r + IN.rabbit.color.b + IN.rabbit.color.g) * IN.rabbit.speed
            end
        )");

        auto inputs = script->getInputs();
        auto rabbit = inputs->getChild("rabbit");
        auto color = rabbit->getChild("color");
        auto speed = rabbit->getChild("speed");

        auto outputs = script->getOutputs();
        auto result = outputs->getChild("result");

        color->getChild("r")->set(0.5f);
        color->getChild("g")->set(1.0f);
        color->getChild("b")->set(0.75f);
        speed->set(20);

        m_logicEngine.update();

        EXPECT_EQ(45, result->get<float>());
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorWhenTryingToAccessFieldsWithNonStringIndexAtRuntime)
    {
        const std::vector<LuaTestError> allCases = {
            {"local var = IN[0]",
            "Bad access to property ''! Expected a string but got object of type number instead!"},
            {"var = IN[true]",
            "Bad access to property ''! Expected a string but got object of type bool instead!"},
            {"var = IN[{x = 5}]",
            "Bad access to property ''! Expected a string but got object of type table instead!"},
            {"OUT[0] = 5",
            "Bad access to property ''! Expected a string but got object of type number instead!"},
            {"OUT[true] = 5",
            "Bad access to property ''! Expected a string but got object of type bool instead!"},
            {"OUT[{x = 5}] = 5",
            "Bad access to property ''! Expected a string but got object of type table instead!"},
        };

        for (const auto& singleCase : allCases)
        {
            auto script = m_logicEngine.createLuaScript(
                "function interface(IN,OUT)\n"
                "end\n"
                "function run(IN,OUT)\n" +
                singleCase.errorCode + "\n"
                "end\n"
            );

            ASSERT_NE(nullptr, script);
            EXPECT_FALSE(m_logicEngine.update());
            ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
            EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr(singleCase.expectedErrorMessage));
            m_logicEngine.destroy(*script);
        }
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorWhenTryingToCreatePropertiesAtRuntime)
    {
        const std::vector<LuaTestError> allCases =
        {
            {"IN.cannot_create_inputs_here = 5",
            "Tried to access undefined struct property 'cannot_create_inputs_here'"},
            {"OUT.cannot_create_outputs_here = 5",
            "Tried to access undefined struct property 'cannot_create_outputs_here'"},
        };

        for (const auto& singleCase : allCases)
        {
            auto script = m_logicEngine.createLuaScript(
                "function interface(IN,OUT)\n"
                "end\n"
                "function run(IN,OUT)\n" +
                singleCase.errorCode + "\n"
                "end\n"
            );

            ASSERT_NE(nullptr, script);
            EXPECT_FALSE(m_logicEngine.update());
            ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
            EXPECT_THAT(m_logicEngine.getErrors()[0].message, ::testing::HasSubstr(singleCase.expectedErrorMessage));
            m_logicEngine.destroy(*script);
        }
    }

    TEST_F(ALuaScript_Runtime, AssignsValuesToArraysWithStructs)
    {
        std::string_view scriptWithArrays = R"(
            function interface(IN,OUT)
                IN.array_structs = Type:Array(2, {name = Type:String(), age = Type:Int32()})
                OUT.array_structs = Type:Array(2, {name = Type:String(), age = Type:Int32()})
            end

            function run(IN,OUT)
                OUT.array_structs = IN.array_structs
                OUT.array_structs[2] = {name = "joe", age = 99}
                OUT.array_structs[2].age = 78
            end
        )";

        auto* script = m_logicEngine.createLuaScript(scriptWithArrays);

        auto inputs = script->getInputs();
        auto IN_array = inputs->getChild("array_structs");
        IN_array->getChild(0)->getChild("name")->set<std::string>("donald");

        EXPECT_TRUE(m_logicEngine.update());

        auto outputs = script->getOutputs();
        auto OUT_array = outputs->getChild("array_structs");

        EXPECT_EQ("donald", *OUT_array->getChild(0)->getChild("name")->get<std::string>());
        EXPECT_EQ("joe", *OUT_array->getChild(1)->getChild("name")->get<std::string>());
        EXPECT_EQ(78, *OUT_array->getChild(1)->getChild("age")->get<int32_t>());
    }

    // The below test is a truly evil attempt to violate the sandbox/environment protection
    // The test makes sure we catch it and report an error accordingly
    TEST_F(ALuaScript_Runtime, ForbidsCallingInterfaceFunctionInsideTheRunFunction)
    {
        m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            end

            function run(IN,OUT)
                interface(IN,OUT)
            end
        )");

        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors().begin()->message, ::testing::HasSubstr("Unexpected global access to key 'interface' in run()!"));
    }


    TEST_F(ALuaScript_Runtime, AbortsAfterFirstRuntimeError)
    {
        auto script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.float = Type:Float()
                OUT.float = Type:Float()
            end
            function run(IN,OUT)
                error("next line will not be executed")
                OUT.float = IN.float
            end
        )");

        ASSERT_NE(nullptr, script);
        script->getInputs()->getChild("float")->set<float>(0.1f);
        EXPECT_FALSE(m_logicEngine.update());
        EXPECT_FLOAT_EQ(0.0f, *script->getOutputs()->getChild("float")->get<float>());
    }

    TEST_F(ALuaScript_Runtime, AssignOutputsFromInputsInDifferentWays_ConfidenceTest)
    {
        auto script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.assignmentType = Type:String()

                IN.float = Type:Float()
                IN.int   = Type:Int32()
                IN.struct = {
                    float = Type:Float(),
                    int   = Type:Int32(),
                    struct = {
                        float   = Type:Float(),
                        int     = Type:Int32(),
                        bool    = Type:Bool(),
                        string  = Type:String(),
                        vec2f  = Type:Vec2f(),
                        vec3f  = Type:Vec3f(),
                        vec4f  = Type:Vec4f(),
                        vec2i  = Type:Vec2i(),
                        vec3i  = Type:Vec3i(),
                        vec4i  = Type:Vec4i(),
                        array  = Type:Array(2, Type:Vec2i())
                    }
                }

                OUT.float = Type:Float()
                OUT.int   = Type:Int32()
                OUT.struct = {
                    float = Type:Float(),
                    int   = Type:Int32(),
                    struct = {
                        float   = Type:Float(),
                        int     = Type:Int32(),
                        bool    = Type:Bool(),
                        string  = Type:String(),
                        vec2f  = Type:Vec2f(),
                        vec3f  = Type:Vec3f(),
                        vec4f  = Type:Vec4f(),
                        vec2i  = Type:Vec2i(),
                        vec3i  = Type:Vec3i(),
                        vec4i  = Type:Vec4i(),
                        array  = Type:Array(2, Type:Vec2i())
                    }
                }
            end
            function run(IN,OUT)
                if IN.assignmentType == "nullify" then
                    OUT.float = 0
                    OUT.int   = 0
                    OUT.struct.float = 0
                    OUT.struct.int   = 0
                    OUT.struct.struct.float     = 0
                    OUT.struct.struct.int       = 0
                    OUT.struct.struct.bool      = false
                    OUT.struct.struct.string    = ""
                    OUT.struct.struct.vec2f    = {0, 0}
                    OUT.struct.struct.vec3f    = {0, 0, 0}
                    OUT.struct.struct.vec4f    = {0, 0, 0, 0}
                    OUT.struct.struct.vec2i    = {0, 0}
                    OUT.struct.struct.vec3i    = {0, 0, 0}
                    OUT.struct.struct.vec4i    = {0, 0, 0, 0}
                    OUT.struct.struct.array    = {{0, 0}, {0, 0}}
                elseif IN.assignmentType == "mirror_individually" then
                    OUT.float = IN.float
                    OUT.int   = IN.int
                    OUT.struct.float = IN.struct.float
                    OUT.struct.int   = IN.struct.int
                    OUT.struct.struct.float     = IN.struct.struct.float
                    OUT.struct.struct.int       = IN.struct.struct.int
                    OUT.struct.struct.bool      = IN.struct.struct.bool
                    OUT.struct.struct.string    = IN.struct.struct.string
                    OUT.struct.struct.vec2f     = IN.struct.struct.vec2f
                    OUT.struct.struct.vec3f     = IN.struct.struct.vec3f
                    OUT.struct.struct.vec4f     = IN.struct.struct.vec4f
                    OUT.struct.struct.vec2i     = IN.struct.struct.vec2i
                    OUT.struct.struct.vec3i     = IN.struct.struct.vec3i
                    OUT.struct.struct.vec4i     = IN.struct.struct.vec4i
                    OUT.struct.struct.array[1]  = IN.struct.struct.array[1]
                    OUT.struct.struct.array[2]  = IN.struct.struct.array[2]
                elseif IN.assignmentType == "assign_constants" then
                    OUT.float = 0.1
                    OUT.int   = 1
                    OUT.struct.float = 0.2
                    OUT.struct.int   = 2
                    OUT.struct.struct.float     = 0.3
                    OUT.struct.struct.int       = 3
                    OUT.struct.struct.bool      = true
                    OUT.struct.struct.string    = "somestring"
                    OUT.struct.struct.vec2f     = { 0.1, 0.2 }
                    OUT.struct.struct.vec3f     = { 1.1, 1.2, 1.3 }
                    OUT.struct.struct.vec4f     = { 2.1, 2.2, 2.3, 2.4 }
                    OUT.struct.struct.vec2i     = { 1, 2 }
                    OUT.struct.struct.vec3i     = { 3, 4, 5 }
                    OUT.struct.struct.vec4i     = { 6, 7, 8, 9 }
                    OUT.struct.struct.array     = { {11, 12}, {13, 14} }
                elseif IN.assignmentType == "assign_struct" then
                    OUT.float = IN.float
                    OUT.int   = IN.int
                    OUT.struct = IN.struct
                else
                    error("unsupported assignment type!")
                end
            end
        )");

        ASSERT_NE(nullptr, script);

        script->getInputs()->getChild("float")->set<float>(0.1f);
        script->getInputs()->getChild("int")->set<int32_t>(1);
        script->getInputs()->getChild("struct")->getChild("float")->set<float>(0.2f);
        script->getInputs()->getChild("struct")->getChild("int")->set<int32_t>(2);
        script->getInputs()->getChild("struct")->getChild("struct")->getChild("float")->set<float>(0.3f);
        script->getInputs()->getChild("struct")->getChild("struct")->getChild("int")->set<int32_t>(3);
        script->getInputs()->getChild("struct")->getChild("struct")->getChild("bool")->set<bool>(true);
        script->getInputs()->getChild("struct")->getChild("struct")->getChild("string")->set<std::string>("somestring");
        script->getInputs()->getChild("struct")->getChild("struct")->getChild("vec2f")->set<vec2f>({ 0.1f, 0.2f });
        script->getInputs()->getChild("struct")->getChild("struct")->getChild("vec3f")->set<vec3f>({ 1.1f, 1.2f, 1.3f });
        script->getInputs()->getChild("struct")->getChild("struct")->getChild("vec4f")->set<vec4f>({ 2.1f, 2.2f, 2.3f, 2.4f });
        script->getInputs()->getChild("struct")->getChild("struct")->getChild("vec2i")->set<vec2i>({ 1, 2 });
        script->getInputs()->getChild("struct")->getChild("struct")->getChild("vec3i")->set<vec3i>({ 3, 4, 5 });
        script->getInputs()->getChild("struct")->getChild("struct")->getChild("vec4i")->set<vec4i>({ 6, 7, 8, 9 });
        script->getInputs()->getChild("struct")->getChild("struct")->getChild("array")->getChild(0)->set<vec2i>({ 11, 12 });
        script->getInputs()->getChild("struct")->getChild("struct")->getChild("array")->getChild(1)->set<vec2i>({ 13, 14 });

        std::array<std::string, 3> assignmentTypes =
        {
            "mirror_individually",
            "assign_constants",
            "assign_struct",
        };

        auto outputs = script->getOutputs();
        for (const auto& assignmentType : assignmentTypes)
        {
            EXPECT_TRUE(script->getInputs()->getChild("assignmentType")->set<std::string>("nullify"));
            EXPECT_TRUE(m_logicEngine.update());

            EXPECT_TRUE(script->getInputs()->getChild("assignmentType")->set<std::string>(assignmentType));
            EXPECT_TRUE(m_logicEngine.update());
            EXPECT_TRUE(m_logicEngine.getErrors().empty());


            EXPECT_FLOAT_EQ(0.1f, *outputs->getChild("float")->get<float>());
            EXPECT_EQ(1, *outputs->getChild("int")->get<int32_t>());

            auto struct_lvl1 = outputs->getChild("struct");
            EXPECT_FLOAT_EQ(0.2f, *struct_lvl1->getChild("float")->get<float>());
            EXPECT_EQ(2, *struct_lvl1->getChild("int")->get<int32_t>());

            auto struct_lvl2 = struct_lvl1->getChild("struct");
            EXPECT_FLOAT_EQ(0.3f, *struct_lvl2->getChild("float")->get<float>());
            EXPECT_EQ(3, *struct_lvl2->getChild("int")->get<int32_t>());
            EXPECT_EQ(true, *struct_lvl2->getChild("bool")->get<bool>());
            EXPECT_EQ("somestring", *struct_lvl2->getChild("string")->get<std::string>());

            EXPECT_THAT(*struct_lvl2->getChild("vec2f")->get<vec2f>(), ::testing::ElementsAre(0.1f, 0.2f));
            EXPECT_THAT(*struct_lvl2->getChild("vec3f")->get<vec3f>(), ::testing::ElementsAre(1.1f, 1.2f, 1.3f));
            EXPECT_THAT(*struct_lvl2->getChild("vec4f")->get<vec4f>(), ::testing::ElementsAre(2.1f, 2.2f, 2.3f, 2.4f));
            EXPECT_THAT(*struct_lvl2->getChild("vec2i")->get<vec2i>(), ::testing::ElementsAre(1, 2));
            EXPECT_THAT(*struct_lvl2->getChild("vec3i")->get<vec3i>(), ::testing::ElementsAre(3, 4, 5));
            EXPECT_THAT(*struct_lvl2->getChild("vec4i")->get<vec4i>(), ::testing::ElementsAre(6, 7, 8, 9));
            EXPECT_THAT(*struct_lvl2->getChild("array")->getChild(0)->get<vec2i>(), ::testing::ElementsAre(11, 12));
            EXPECT_THAT(*struct_lvl2->getChild("array")->getChild(1)->get<vec2i>(), ::testing::ElementsAre(13, 14));
        }
    }

    TEST_F(ALuaScript_Runtime, ForbidsOverwritingRunFunctionInsideTheRunFunction)
    {
        m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            end
            function run(IN,OUT)
                run = function()
                    OUT.str = "... go left! A Kansas city shuffle, lol!"
                end
            end
        )");

        EXPECT_FALSE(m_logicEngine.update());
        ASSERT_EQ(m_logicEngine.getErrors().size(), 1u);
        EXPECT_THAT(m_logicEngine.getErrors().back().message,
            ::testing::HasSubstr("Unexpected global variable definition 'run' in run()! "
                "Use the init() function to declare global data and functions, or use modules!"));
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorIfInvalidOutPropertyIsAccessed)
    {
        auto scriptWithInvalidOutParam = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            end
            function run(IN,OUT)
                OUT.param = 47.11
            end
        )");

        ASSERT_NE(nullptr, scriptWithInvalidOutParam);
        m_logicEngine.update();
        EXPECT_FALSE(m_logicEngine.getErrors().empty());
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorIfInvalidNestedOutPropertyIsAccessed)
    {
        auto scriptWithInvalidStructAccess = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            end
            function run(IN,OUT)
                OUT.struct.param = 47.11
            end
        )");

        ASSERT_NE(nullptr, scriptWithInvalidStructAccess);
        m_logicEngine.update();
        EXPECT_FALSE(m_logicEngine.getErrors().empty());
    }

    TEST_F(ALuaScript_Runtime, ProducesErrorIfValidestedButInvalidOutPropertyIsAccessed)
    {
        auto scriptWithValidStructButInvalidField = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                OUT.struct = {
                    param = Type:Int32()
                }
            end
            function run(IN,OUT)
                OUT.struct.invalid = 47.11
            end
        )");

        ASSERT_NE(nullptr, scriptWithValidStructButInvalidField);
        m_logicEngine.update();
        EXPECT_FALSE(m_logicEngine.getErrors().empty());
    }

    TEST_F(ALuaScript_Runtime, CanAssignInputDirectlyToOutput)
    {
        auto script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.param_struct = {
                    param1 = Type:Float(),
                    param2_struct = {
                        a = Type:Int32(),
                        b = Type:Int32()
                    }
                }
                OUT.param_struct = {
                    param1 = Type:Float(),
                    param2_struct = {
                        a = Type:Int32(),
                        b = Type:Int32()
                    }
                }
            end
            function run(IN,OUT)
                OUT.param_struct = IN.param_struct
            end
        )");

        ASSERT_NE(nullptr, script);

        {
            auto inputs = script->getInputs();
            auto param_struct = inputs->getChild("param_struct");
            param_struct->getChild("param1")->set(1.0f);
            auto param2_struct = param_struct->getChild("param2_struct");
            param2_struct->getChild("a")->set(2);
            param2_struct->getChild("b")->set(3);
        }

        m_logicEngine.update();

        {
            auto outputs = script->getOutputs();
            auto param_struct = outputs->getChild("param_struct");
            EXPECT_FLOAT_EQ(1.0f, *param_struct->getChild("param1")->get<float>());
            auto param2_struct = param_struct->getChild("param2_struct");
            EXPECT_EQ(2, param2_struct->getChild("a")->get<int32_t>());
            EXPECT_EQ(3, param2_struct->getChild("b")->get<int32_t>());
        }
    }

    TEST_F(ALuaScript_Runtime, ProducesNoErrorIfOutputIsSetInFunction)
    {
        auto script = m_logicEngine.createLuaScript(R"(
            function init()
                GLOBAL.setPrimitive = function (output)
                    output.param = 42
                end
                GLOBAL.setSubStruct = function (output)
                    output.struct = {
                        param = 43
                    }
                end
            end

            function interface(IN,OUT)
                OUT.param = Type:Int32()
                OUT.struct = {
                    param = Type:Int32()
                }
            end
            function run(IN,OUT)
                GLOBAL.setPrimitive(OUT)
                GLOBAL.setSubStruct(OUT)
            end
        )");

        ASSERT_NE(nullptr, script);
        EXPECT_TRUE(m_logicEngine.update());
        const auto outputs = script->getOutputs();
        ASSERT_NE(nullptr, outputs);

        ASSERT_EQ(2u, outputs->getChildCount());
        const auto param = outputs->getChild(0);
        const auto struct1 = outputs->getChild(1);

        EXPECT_EQ(42, param->get<int32_t>());

        ASSERT_EQ(1u, struct1->getChildCount());
        EXPECT_EQ(43, struct1->getChild(0)->get<int32_t>());
    }

    TEST_F(ALuaScript_Runtime, HasNoInfluenceOnBindingsIfTheyAreNotLinked)
    {
        auto        scriptSource = R"(
            function interface(IN,OUT)
                IN.inFloat = Type:Float()
                IN.inVec3  = Type:Vec3f()
                OUT.outFloat = Type:Float()
                OUT.outVec3  = Type:Vec3f()
            end
            function run(IN,OUT)
                OUT.outFloat = IN.inFloat
                OUT.outVec3 = IN.inVec3
            end
        )";

        const std::string_view vertexShaderSource = R"(
            #version 300 es

            uniform highp float floatUniform;

            void main()
            {
                gl_Position = floatUniform * vec4(1.0);
            })";

        const std::string_view fragmentShaderSource = R"(
            #version 300 es

            out lowp vec4 color;
            void main(void)
            {
                color = vec4(1.0, 0.0, 0.0, 1.0);
            })";

        auto script1 = m_logicEngine.createLuaScript(scriptSource);
        auto script2 = m_logicEngine.createLuaScript(scriptSource);
        auto script3 = m_logicEngine.createLuaScript(scriptSource);

        auto script1FloatInput  = script1->getInputs()->getChild("inFloat");
        auto script1FloatOutput = script1->getOutputs()->getChild("outFloat");
        auto script1Vec3Input   = script1->getInputs()->getChild("inVec3");
        auto script1Vec3Output  = script1->getOutputs()->getChild("outVec3");
        auto script2FloatInput  = script2->getInputs()->getChild("inFloat");
        auto script2FloatOutput = script2->getOutputs()->getChild("outFloat");
        auto script2Vec3Input   = script2->getInputs()->getChild("inVec3");
        auto script2Vec3Output  = script2->getOutputs()->getChild("outVec3");
        auto script3FloatInput  = script3->getInputs()->getChild("inFloat");
        auto script3FloatOutput = script3->getOutputs()->getChild("outFloat");
        auto script3Vec3Input   = script3->getInputs()->getChild("inVec3");
        auto script3Vec3Output  = script3->getOutputs()->getChild("outVec3");

        ramses::RamsesFramework ramsesFramework;
        auto                    ramsesClient = ramsesFramework.createClient("client");
        auto                    ramsesScene  = ramsesClient->createScene(ramses::sceneId_t(1));

        ramses::EffectDescription ramsesEffectDesc;
        ramsesEffectDesc.setVertexShader(vertexShaderSource.data());
        ramsesEffectDesc.setFragmentShader(fragmentShaderSource.data());
        auto ramsesEffect     = ramsesScene->createEffect(ramsesEffectDesc);
        auto ramsesAppearance = ramsesScene->createAppearance(*ramsesEffect);
        ramses::PerspectiveCamera* camera = ramsesScene->createPerspectiveCamera();

        auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "NodeBinding");
        auto appearanceBinding = m_logicEngine.createRamsesAppearanceBinding(*ramsesAppearance, "AppearanceBinding");
        auto cameraBinding = m_logicEngine.createRamsesCameraBinding(*camera, "CameraBinding");

        m_logicEngine.update();

        EXPECT_TRUE(*nodeBinding->getInputs()->getChild("visibility")->get<bool>());
        EXPECT_THAT(*nodeBinding->getInputs()->getChild("translation")->get<vec3f>(), ::testing::ElementsAre(0.f, 0.f, 0.f));
        EXPECT_THAT(*nodeBinding->getInputs()->getChild("rotation")->get<vec3f>(), ::testing::ElementsAre(0.f, 0.f, 0.f));
        EXPECT_THAT(*nodeBinding->getInputs()->getChild("scaling")->get<vec3f>(), ::testing::ElementsAre(1.f, 1.f, 1.f));
        EXPECT_EQ(0.0f, appearanceBinding->getInputs()->getChild("floatUniform")->get<float>());
        EXPECT_EQ(camera->getViewportX(), 0);
        EXPECT_EQ(camera->getViewportY(), 0);
        EXPECT_EQ(camera->getViewportWidth(), 16u);
        EXPECT_EQ(camera->getViewportHeight(), 16u);
        EXPECT_NEAR(camera->getVerticalFieldOfView(), 168.579f, 0.001f);
        EXPECT_EQ(camera->getAspectRatio(), 1.f);
        EXPECT_EQ(camera->getNearPlane(), 0.1f);
        EXPECT_EQ(camera->getFarPlane(), 1.f);

        m_logicEngine.link(*script1FloatOutput, *script2FloatInput);
        m_logicEngine.link(*script2FloatOutput, *script3FloatInput);
        m_logicEngine.link(*script1Vec3Output,  *script2Vec3Input);
        m_logicEngine.link(*script2Vec3Output,  *script3Vec3Input);

        m_logicEngine.update();

        EXPECT_TRUE(*nodeBinding->getInputs()->getChild("visibility")->get<bool>());
        EXPECT_THAT(*nodeBinding->getInputs()->getChild("translation")->get<vec3f>(), ::testing::ElementsAre(0.f, 0.f, 0.f));
        EXPECT_THAT(*nodeBinding->getInputs()->getChild("rotation")->get<vec3f>(), ::testing::ElementsAre(0.f, 0.f, 0.f));
        EXPECT_THAT(*nodeBinding->getInputs()->getChild("scaling")->get<vec3f>(), ::testing::ElementsAre(1.f, 1.f, 1.f));
        EXPECT_EQ(0.0f, appearanceBinding->getInputs()->getChild("floatUniform")->get<float>());
        EXPECT_EQ(camera->getViewportX(), 0);
        EXPECT_EQ(camera->getViewportY(), 0);
        EXPECT_EQ(camera->getViewportWidth(), 16u);
        EXPECT_EQ(camera->getViewportHeight(), 16u);
        EXPECT_NEAR(camera->getVerticalFieldOfView(), 168.579f, 0.001f);
        EXPECT_EQ(camera->getAspectRatio(), 1.f);
        EXPECT_EQ(camera->getNearPlane(), 0.1f);
        EXPECT_EQ(camera->getFarPlane(), 1.f);

        m_logicEngine.link(*script3Vec3Output, *nodeBinding->getInputs()->getChild("translation"));

        script1Vec3Input->set(vec3f{1.f, 2.f, 3.f});

        m_logicEngine.update();

        EXPECT_THAT(*nodeBinding->getInputs()->getChild("translation")->get<vec3f>(), ::testing::ElementsAre(1.f, 2.f, 3.f));

        m_logicEngine.link(*script3FloatOutput, *appearanceBinding->getInputs()->getChild("floatUniform"));
        m_logicEngine.link(*script3FloatOutput, *cameraBinding->getInputs()->getChild("frustum")->getChild("farPlane"));

        script1FloatInput->set(42.f);

        m_logicEngine.update();

        EXPECT_FLOAT_EQ(42.f, *appearanceBinding->getInputs()->getChild("floatUniform")->get<float>());
        EXPECT_EQ(42.f, *cameraBinding->getInputs()->getChild("frustum")->getChild("farPlane")->get<float>());

        m_logicEngine.unlink(*script3Vec3Output, *nodeBinding->getInputs()->getChild("translation"));

        script1FloatInput->set(23.f);
        script1Vec3Input->set(vec3f{3.f, 2.f, 1.f});

        m_logicEngine.update();

        EXPECT_THAT(*nodeBinding->getInputs()->getChild("translation")->get<vec3f>(), ::testing::ElementsAre(1.f, 2.f, 3.f));
        EXPECT_FLOAT_EQ(23.f, *appearanceBinding->getInputs()->getChild("floatUniform")->get<float>());
        EXPECT_EQ(23.f, *cameraBinding->getInputs()->getChild("frustum")->getChild("farPlane")->get<float>());
    }


    TEST_F(ALuaScript_Runtime, IncludesStandardLibraries_WhenConfiguredWithThem)
    {
        const std::string_view scriptSrc = R"(
            function init()
                GLOBAL.debug_func = function (arg)
                    print(arg)
                end
            end

            function interface(IN,OUT)
                OUT.floored_float = Type:Int32()
                OUT.string_gsub = Type:String()
                OUT.table_maxn = Type:Int32()
                OUT.language_of_debug_func = Type:String()
            end
            function run(IN,OUT)
                -- test math lib
                OUT.floored_float = math.floor(42.7)
                -- test string lib
                OUT.string_gsub = string.gsub("This is the text", "the text", "the modified text")
                -- test table lib
                OUT.table_maxn = table.maxn ({11, 12, 13})
                -- test debug lib
                local debuginfo = debug.getinfo (GLOBAL.debug_func)
                OUT.language_of_debug_func = debuginfo.what
            end
        )";
        auto script = m_logicEngine.createLuaScript(scriptSrc, WithStdModules({EStandardModule::Base, EStandardModule::String, EStandardModule::Table, EStandardModule::Debug, EStandardModule::Math}));
        ASSERT_NE(nullptr, script);

        m_logicEngine.update();

        EXPECT_EQ(42, *script->getOutputs()->getChild("floored_float")->get<int32_t>());
        EXPECT_EQ("This is the modified text", *script->getOutputs()->getChild("string_gsub")->get<std::string>());
        EXPECT_EQ(3, *script->getOutputs()->getChild("table_maxn")->get<int32_t>());
        EXPECT_EQ("Lua", *script->getOutputs()->getChild("language_of_debug_func")->get<std::string>());
    }

    class ALuaScript_RuntimeIterators : public ALuaScript_Runtime
    {
    protected:
    };

    TEST_F(ALuaScript_RuntimeIterators, ComputesSizeOfCustomPropertiesUsingCustomLengthFunction)
    {
        std::string_view scriptSrc = R"(
            function interface(IN,OUT)
                IN.array_int = Type:Array(2, Type:Int32())
                OUT.struct = {a=Type:Int32(), b={c = Type:Int32()}}
                OUT.array_struct = Type:Array(3, {a=Type:Int32(), b=Type:Float()})
            end

            function run(IN,OUT)
                if rl_len(IN) ~= 1 then
                    error("Wrong IN size!")
                end

                if rl_len(IN.array_int) ~= 2 then
                    error("Wrong array size!")
                end

                if rl_len(OUT) ~= 2 then
                    error("Wrong OUT size!")
                end

                if rl_len(OUT.struct) ~= 2 then
                    error("Wrong struct size!")
                end

                if rl_len(OUT.struct.b) ~= 1 then
                    error("Wrong nested struct size!")
                end

                if rl_len(OUT.array_struct) ~= 3 then
                    error("Wrong array struct size!")
                end

                if rl_len(OUT.array_struct[1]) ~= 2 then
                    error("Wrong array struct element size!")
                end
            end
        )";
        auto* script = m_logicEngine.createLuaScript(scriptSrc, WithStdModules({EStandardModule::Base}));
        ASSERT_NE(nullptr, script);
        EXPECT_TRUE(m_logicEngine.update());
    }

    TEST_F(ALuaScript_RuntimeIterators, CallingCustomLengthFunctionOnNormalLuaTables_YieldsSameResultAsBuiltInSizeOperator)
    {
        std::string_view scriptSrc = R"(
            function interface(IN,OUT)
            end

            function run(IN,OUT)
                local emptyTable = {}
                assert(rl_len(emptyTable) == #emptyTable)
                local numericTable = {1, 2, 3}
                assert(rl_len(numericTable) == #numericTable)
                local nonNumericTable = {a=5, b=6}
                assert(rl_len(nonNumericTable) == #nonNumericTable)
                local nonNumericTable = {a=5, b=6}
                assert(rl_len(nonNumericTable) == #nonNumericTable)
            end
        )";
        auto* script = m_logicEngine.createLuaScript(scriptSrc, WithStdModules({ EStandardModule::Base }));
        ASSERT_NE(nullptr, script);
        EXPECT_TRUE(m_logicEngine.update());
    }

    TEST_F(ALuaScript_RuntimeIterators, CustomRlNextFunctionWorksLikeItsBuiltInCounterpart_Structs)
    {
        std::string_view scriptSrc = R"(
            function interface(IN,OUT)
                IN.struct = {a = Type:Int32(), b = Type:Int32()}
                IN.nested = {
                    struct = {a = Type:Int32(), b = Type:Int32()}
                }
                OUT.struct = {a = Type:Int32(), b = Type:Int32()}
                OUT.nested = {
                    struct = {a = Type:Int32(), b = Type:Int32()}
                }
            end

            function run(IN,OUT)
                -- propagate data to OUT so that we can test both further down
                OUT.struct = IN.struct
                OUT.nested = IN.nested

                local objectsToCheck = {IN.struct, IN.nested.struct, OUT.struct, OUT.nested.struct}

                for unused, container in pairs(objectsToCheck) do
                    ---- no index specified is the same as providing nil (see below)
                    local k, v = rl_next(container)
                    assert(k == 'a')
                    assert(v == 11)
                    -- index=nil -> yields first element of container and its index
                    local k, v = rl_next(container, nil)
                    assert(k == 'a')
                    assert(v == 11)
                    -- index==N -> yields element N+1 and its index
                    local k, v = rl_next(container, 'a')
                    assert(k == 'b')
                    assert(v == 12)
                    local k, v = rl_next(container, 'b')
                    assert(k == nil)
                    assert(v == nil)
                end
            end
        )";
        auto* script = m_logicEngine.createLuaScript(scriptSrc, WithStdModules({ EStandardModule::Base, EStandardModule::String }));
        ASSERT_NE(nullptr, script);
        script->getInputs()->getChild("struct")->getChild("a")->set<int32_t>(11);
        script->getInputs()->getChild("struct")->getChild("b")->set<int32_t>(12);
        script->getInputs()->getChild("nested")->getChild("struct")->getChild("a")->set<int32_t>(11);
        script->getInputs()->getChild("nested")->getChild("struct")->getChild("b")->set<int32_t>(12);
        EXPECT_TRUE(m_logicEngine.update());
    }

    TEST_F(ALuaScript_RuntimeIterators, CustomRlNextFunctionWorksLikeItsBuiltInCounterpart_Arrays)
    {
        std::string_view scriptSrc = R"(
            function interface(IN,OUT)
                IN.array_int = Type:Array(2, Type:Int32())
                IN.nested = {
                    array_int = Type:Array(2, Type:Int32())
                }
                OUT.array_int = Type:Array(2, Type:Int32())
                OUT.nested = {
                    array_int = Type:Array(2, Type:Int32())
                }
            end

            function run(IN,OUT)
                -- propagate data to OUT so that we can test both further down
                OUT.array_int = IN.array_int
                OUT.nested = IN.nested

                local objectsToCheck = {IN.array_int, IN.nested.array_int, OUT.array_int, OUT.nested.array_int}

                for k, container in pairs(objectsToCheck) do
                    -- no index specified is the same as providing nil (see below)
                    local a, b = rl_next(container)
                    assert(a == 1)
                    assert(b == 11)
                    -- index=nil -> yields first element of container and its index
                    local a, b = rl_next(container, nil)
                    assert(a == 1)
                    assert(b == 11)
                    -- index==N -> yields element N+1 and its index
                    local a, b = rl_next(container, 1)
                    assert(a == 2)
                    assert(b == 12)
                    local a, b = rl_next(container, 2)
                    assert(a == nil)
                    assert(b == nil)
                end
            end
        )";
        auto* script = m_logicEngine.createLuaScript(scriptSrc, WithStdModules({ EStandardModule::Base, EStandardModule::String }));
        ASSERT_NE(nullptr, script);
        script->getInputs()->getChild("array_int")->getChild(0)->set<int32_t>(11);
        script->getInputs()->getChild("array_int")->getChild(1)->set<int32_t>(12);
        script->getInputs()->getChild("nested")->getChild("array_int")->getChild(0)->set<int32_t>(11);
        script->getInputs()->getChild("nested")->getChild("array_int")->getChild(1)->set<int32_t>(12);
        EXPECT_TRUE(m_logicEngine.update());
    }

    TEST_F(ALuaScript_RuntimeIterators, Custom_IPairs_BehavesTheSameAsStandard_IPairs_Function_ForArrays)
    {
        std::string_view scriptSrc = R"(
            function interface(IN,OUT)
                IN.array_int = Type:Array(2, Type:Int32())
                IN.nested = {
                    array_int = Type:Array(2, Type:Int32())
                }
                OUT.array_int = Type:Array(2, Type:Int32())
                OUT.nested = {
                    array_int = Type:Array(2, Type:Int32())
                }
            end

            function run(IN,OUT)
                -- propagate data to OUT so that we can test both further down
                OUT.array_int = IN.array_int
                OUT.nested = IN.nested

                -- compare iteration results to a static reference table
                local refTable = {[1] = 11, [2] = 12}

                -- test multiple containers (which all have the same contents)
                local objectsToCheck = {IN.array_int, IN.nested.array_int, OUT.array_int, OUT.nested.array_int}
                for k, container in pairs(objectsToCheck) do
                    -- iterate manually over reference table...
                    local refKey = 1
                    local refValue = nil
                    for key, value in rl_ipairs(container) do
                        if type(key) ~= 'number' then
                            error('Key should be of type number!')
                        end

                        if key ~= refKey then
                            error("Expected key==refKey, but found " .. tostring(key) .. " != " .. tostring(refKey))
                        end

                        refValue = refTable[refKey]
                        if value ~= refValue then
                            error("Expected value==refValue, but found " .. tostring(value) .. " != " .. tostring(refValue))
                        end
                        -- progress refTable manually
                        refKey = refKey + 1
                    end

                    -- make sure there were exactly as many elements in refTable by checking no element is left to iterate
                    assert(refKey == 3)
                    assert(refValue == 12)
                end

            end
        )";
        auto* script = m_logicEngine.createLuaScript(scriptSrc, WithStdModules({ EStandardModule::Base, EStandardModule::String }));
        ASSERT_NE(nullptr, script);
        script->getInputs()->getChild("array_int")->getChild(0)->set<int32_t>(11);
        script->getInputs()->getChild("array_int")->getChild(1)->set<int32_t>(12);
        script->getInputs()->getChild("nested")->getChild("array_int")->getChild(0)->set<int32_t>(11);
        script->getInputs()->getChild("nested")->getChild("array_int")->getChild(1)->set<int32_t>(12);
        EXPECT_TRUE(m_logicEngine.update());
    }

    TEST_F(ALuaScript_RuntimeIterators, Custom_Pairs_BehavesTheSameAsStandard_Pairs_Function_ForArrays)
    {
        std::string_view scriptSrc = R"(
            function interface(IN,OUT)
                IN.array_int = Type:Array(2, Type:Int32())
                IN.nested = {
                    array_int = Type:Array(2, Type:Int32())
                }
                OUT.array_int = Type:Array(2, Type:Int32())
                OUT.nested = {
                    array_int = Type:Array(2, Type:Int32())
                }
            end

            function run(IN,OUT)
                -- propagate data to OUT so that we can test both further down
                OUT.array_int = IN.array_int
                OUT.nested = IN.nested

                -- compare iteration results to a static reference table
                local refTable = {[1] = 11, [2] = 12}

                -- test multiple containers (which all have the same contents)
                local objectsToCheck = {IN.array_int, IN.nested.array_int, OUT.array_int, OUT.nested.array_int}
                for k, container in pairs(objectsToCheck) do
                    -- iterate manually over reference table...
                    local refKey,refValue = next(refTable)
                    -- ...and compare to rl_pairs results
                    for key, value in rl_pairs(container) do
                        if type(key) ~= 'number' then
                            error('Key should be of type number!')
                        end

                        if key ~= refKey then
                            error("Expected key==refKey, but found " .. tostring(key) .. " != " .. tostring(refKey))
                        end
                        if value ~= refValue then
                            error("Expected value==refValue, but found " .. tostring(value) .. " != " .. tostring(refValue))
                        end
                        -- progress refTable manually
                        refKey,refValue = next(refTable, refKey)
                    end

                    -- make sure there were exactly as many elements in refTable by checking no element is left to iterate
                    assert(refKey == nil)
                    assert(refValue == nil)
                end

            end
        )";
        auto* script = m_logicEngine.createLuaScript(scriptSrc, WithStdModules({ EStandardModule::Base, EStandardModule::String }));
        ASSERT_NE(nullptr, script);
        script->getInputs()->getChild("array_int")->getChild(0)->set<int32_t>(11);
        script->getInputs()->getChild("array_int")->getChild(1)->set<int32_t>(12);
        script->getInputs()->getChild("nested")->getChild("array_int")->getChild(0)->set<int32_t>(11);
        script->getInputs()->getChild("nested")->getChild("array_int")->getChild(1)->set<int32_t>(12);
        EXPECT_TRUE(m_logicEngine.update());
    }

    TEST_F(ALuaScript_RuntimeIterators, Custom_Pairs_BehavesTheSameAsStandard_Pairs_Function_ForStructs)
    {
        std::string_view scriptSrc = R"(
            function interface(IN,OUT)
                IN.int = Type:Int32()
                IN.bool = Type:Bool()
                IN.nested = {
                    int = Type:Int32(),
                    bool = Type:Bool(),
                    nested = {
                        notUsed = Type:Float()
                    }
                }
                OUT.int = Type:Int32()
                OUT.bool = Type:Bool()
                OUT.nested = {
                    int = Type:Int32(),
                    bool = Type:Bool(),
                    nested = {
                        notUsed = Type:Float()
                    }
                }
            end

            function run(IN,OUT)
                -- propagate data to OUT so that we can test both further down
                OUT.int = IN.int
                OUT.bool = IN.bool
                OUT.nested = IN.nested

                -- compare iteration results to a static reference table
                local refTable = {int = 42, bool = false, nested = {int = 42, bool = false, nested = {}}}

                -- test multiple containers (which all have the same contents)
                local objectsToCheck = {IN, IN.nested, OUT, OUT.nested}
                for k, container in pairs(objectsToCheck) do
                    -- iterate manually over reference table...
                    local refKey,refValue = next(refTable)
                    -- ...and compare to rl_pairs results
                    for key, value in rl_pairs(container) do
                        if type(key) ~= 'string' then
                            error('Key should be of type string!')
                        end

                        if key ~= refKey then
                            error("Expected key==refKey, but found " .. tostring(key) .. " != " .. tostring(refKey))
                        end
                        -- compare all values except 'nested', because no value comparison semantics for tables/userdata
                        if key ~= "nested" and value ~= refValue then
                            error("Expected value==refValue, but found " .. tostring(value) .. " != " .. tostring(refValue))
                        end
                        -- progress refTable manually
                        refKey,refValue = next(refTable, refKey)
                    end

                    -- make sure there are no leftover elements in refTable
                    assert(refKey == nil)
                    assert(refValue == nil)
                end

            end
        )";
        auto* script = m_logicEngine.createLuaScript(scriptSrc, WithStdModules({ EStandardModule::Base, EStandardModule::String }));
        ASSERT_NE(nullptr, script);
        script->getInputs()->getChild("int")->set<int32_t>(42);
        script->getInputs()->getChild("bool")->set<bool>(false);
        script->getInputs()->getChild("nested")->getChild("int")->set<int32_t>(42);
        script->getInputs()->getChild("nested")->getChild("bool")->set<bool>(false);
        EXPECT_TRUE(m_logicEngine.update());
    }

    class ALuaScript_Runtime_Sandboxing : public ALuaScript_Runtime
    {
    };

    TEST_F(ALuaScript_Runtime_Sandboxing, ReportsErrorWhenTryingToReadUnknownGlobals)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            end

            function run(IN,OUT)
                local t = someGlobalVariable
            end)");
        ASSERT_NE(nullptr, script);

        EXPECT_FALSE(m_logicEngine.update());

        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr(
                "Unexpected global access to key 'someGlobalVariable' in run()! Only 'GLOBAL' is allowed as a key"));
    }

    TEST_F(ALuaScript_Runtime_Sandboxing, ReportsErrorWhenSettingGlobals)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            end

            function run(IN,OUT)
                thisCausesError = 'bad'
            end)");
        ASSERT_NE(nullptr, script);

        EXPECT_FALSE(m_logicEngine.update());
        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr("Unexpected global variable definition 'thisCausesError' in run()! Use the init() function to declare global data and functions, or use modules!"));
    }

    TEST_F(ALuaScript_Runtime_Sandboxing, ReportsErrorWhenTryingToOverrideGlobals)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function init()
            end

            function interface(IN,OUT)
            end

            function run(IN,OUT)
                GLOBAL = {}
            end)");
        ASSERT_NE(nullptr, script);

        EXPECT_FALSE(m_logicEngine.update());
        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr(" Trying to override the GLOBAL table in run()! You can only read data, but not overwrite the table!"));
    }

    TEST_F(ALuaScript_Runtime_Sandboxing, ReportsErrorWhenTryingToDeclareRunFunctionTwice)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
            end

            function run(IN,OUT)
            end

            function run(IN,OUT)
            end)");
        ASSERT_EQ(nullptr, script);

        EXPECT_THAT(m_logicEngine.getErrors().begin()->message,
            ::testing::HasSubstr("Function 'run' can only be declared once!"));
    }

    TEST_F(ALuaScript_Runtime_Sandboxing, ForbidsCallingSpecialFunctions)
    {
        for (const auto& specialFunction  : std::vector<std::string>{ "init", "run", "interface" })
        {
            LuaScript* script = m_logicEngine.createLuaScript(fmt::format(R"(
                function init()
                end
                function interface(IN,OUT)
                end

                function run(IN,OUT)
                    {}()
                end
            )", specialFunction));

            ASSERT_NE(nullptr, script);

            EXPECT_FALSE(m_logicEngine.update());
            EXPECT_THAT(m_logicEngine.getErrors()[0].message,
                ::testing::HasSubstr(fmt::format("Unexpected global access to key '{}' in run()! Only 'GLOBAL' is allowed as a key", specialFunction)));

            ASSERT_TRUE(m_logicEngine.destroy(*script));
        }
    }
}
