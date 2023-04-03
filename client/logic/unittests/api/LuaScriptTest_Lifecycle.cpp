//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LuaScriptTest_Base.h"
#include "WithTempDirectory.h"

#include "impl/LuaScriptImpl.h"
#include "impl/LogicEngineImpl.h"
#include "impl/PropertyImpl.h"

#include "fmt/format.h"
#include <fstream>

namespace rlogic::internal
{
    class ALuaScript_Lifecycle : public ALuaScript
    {
    protected:
        WithTempDirectory tempFolder;
    };

    TEST_F(ALuaScript_Lifecycle, ProducesNoErrorsWhenCreatedFromMinimalScript)
    {
        auto* script = m_logicEngine.createLuaScript(m_minimalScript);
        ASSERT_NE(nullptr, script);
        EXPECT_TRUE(m_logicEngine.getErrors().empty());
    }

    TEST_F(ALuaScript_Lifecycle, ProvidesNameAsPassedDuringCreation)
    {
        auto* script = m_logicEngine.createLuaScript(m_minimalScript, {}, "script name");
        EXPECT_EQ("script name", script->getName());
    }

    //This is actually bad (because it can cause undefined behavior), but we still have a test to show/test how it works
    TEST_F(ALuaScript_Lifecycle, KeepsLocalScopeSymbolsDuringRunMethod)
    {
        LuaScript* script = m_logicEngine.createLuaScript(R"(
            -- 'Local' symbols in the global space are inherited by functions
            local localSymbol = "localSymbol"

            function interface(IN,OUT)
                OUT.result = Type:String()
            end

            function run(IN,OUT)
                -- local symbols from global scope are available here
                OUT.result = localSymbol
            end
        )");

        ASSERT_NE(nullptr, script);
        EXPECT_TRUE(m_logicEngine.update());
        EXPECT_EQ(script->getOutputs()->getChild("result")->get<std::string>(), "localSymbol");
    }

    class ALuaScript_LifecycleWithFiles : public ALuaScript_Lifecycle
    {
    };

    TEST_F(ALuaScript_LifecycleWithFiles, NoOutputs)
    {
        {
            LogicEngine tempLogicEngine;
            auto script = tempLogicEngine.createLuaScript(
                R"(
                    function interface(IN,OUT)
                        IN.param = Type:Int32()
                    end
                    function run(IN,OUT)
                    end
                )");

            ASSERT_NE(nullptr, script);
            EXPECT_TRUE(SaveToFileWithoutValidation(tempLogicEngine, "script.bin"));
        }
        {
            EXPECT_TRUE(m_logicEngine.loadFromFile("script.bin"));
            const LuaScript* loadedScript = *m_logicEngine.getCollection<LuaScript>().begin();

            ASSERT_NE(nullptr, loadedScript);

            auto inputs = loadedScript->getInputs();
            auto outputs = loadedScript->getOutputs();

            ASSERT_NE(nullptr, inputs);
            ASSERT_NE(nullptr, outputs);

            ASSERT_EQ(inputs->getChildCount(), 1u);
            ASSERT_EQ(outputs->getChildCount(), 0u);

            EXPECT_EQ("param", inputs->getChild(0u)->getName());
            EXPECT_EQ(EPropertyType::Int32, inputs->getChild(0u)->getType());

            EXPECT_TRUE(m_logicEngine.update());
        }
    }

    TEST_F(ALuaScript_LifecycleWithFiles, Arrays)
    {
        {
            LogicEngine tempLogicEngine;
            auto script = tempLogicEngine.createLuaScript(
                R"(
                function interface(IN,OUT)
                    IN.array = Type:Array(2, Type:Float())
                end
                function run(IN,OUT)
                end
            )", {}, "MyScript");

            ASSERT_NE(nullptr, script);


            script->getInputs()->getChild("array")->getChild(0)->set<float>(0.1f);
            script->getInputs()->getChild("array")->getChild(1)->set<float>(0.2f);
            EXPECT_TRUE(SaveToFileWithoutValidation(tempLogicEngine, "script.bin"));
        }
        {
            m_logicEngine.loadFromFile("script.bin");
            const LuaScript* loadedScript = m_logicEngine.findByName<LuaScript>("MyScript");

            const auto inputs = loadedScript->getInputs();

            ASSERT_NE(nullptr, inputs);

            ASSERT_EQ(inputs->getChildCount(), 1u);

            // Full type inspection of array type, children and values
            EXPECT_EQ("array", inputs->getChild(0u)->getName());
            EXPECT_EQ(EPropertyType::Array, inputs->getChild(0u)->getType());
            EXPECT_EQ(EPropertyType::Float, inputs->getChild(0u)->getChild(0u)->getType());
            EXPECT_EQ(EPropertyType::Float, inputs->getChild(0u)->getChild(1u)->getType());
            EXPECT_EQ("", inputs->getChild(0u)->getChild(0u)->getName());
            EXPECT_EQ("", inputs->getChild(0u)->getChild(1u)->getName());
            EXPECT_EQ(0u, inputs->getChild(0u)->getChild(0u)->getChildCount());
            EXPECT_EQ(0u, inputs->getChild(0u)->getChild(1u)->getChildCount());
            EXPECT_FLOAT_EQ(0.1f, *inputs->getChild(0u)->getChild(0u)->get<float>());
            EXPECT_FLOAT_EQ(0.2f, *inputs->getChild(0u)->getChild(1u)->get<float>());

        }
    }

    TEST_F(ALuaScript_LifecycleWithFiles, NestedArray)
    {
        {
            LogicEngine tempLogicEngine;
            auto script = tempLogicEngine.createLuaScript(
                R"(
                function interface(IN,OUT)
                    IN.nested =
                    {
                        array = Type:Array(1, Type:Vec3f())
                    }
                end
                function run(IN,OUT)
                end
            )", {}, "MyScript");

            script->getInputs()->getChild("nested")->getChild("array")->getChild(0)->set<vec3f>({1.1f, 1.2f, 1.3f});
            EXPECT_TRUE(SaveToFileWithoutValidation(tempLogicEngine, "arrays.bin"));
        }
        {
            m_logicEngine.loadFromFile("arrays.bin");
            const LuaScript* loadedScript = m_logicEngine.findByName<LuaScript>("MyScript");

            const auto inputs = loadedScript->getInputs();

            ASSERT_EQ(inputs->getChildCount(), 1u);

            // Type inspection on nested array
            const auto nested = inputs->getChild(0u);
            EXPECT_EQ("nested", nested->getName());
            auto nested_array = nested->getChild(0u);
            EXPECT_EQ("array", nested_array->getName());

            // Check children of nested array, also values
            EXPECT_EQ(1u, nested_array->getChildCount());
            EXPECT_EQ("", nested_array->getChild(0u)->getName());
            EXPECT_EQ(EPropertyType::Vec3f, nested_array->getChild(0u)->getType());
            EXPECT_EQ(0u, nested_array->getChild(0u)->getChildCount());
            EXPECT_THAT(*nested_array->getChild(0u)->get<vec3f>(), ::testing::ElementsAre(1.1f, 1.2f, 1.3f));

        }
    }

    TEST_F(ALuaScript_LifecycleWithFiles, NestedProperties)
    {
        {
            LogicEngine tempLogicEngine;
            auto script = tempLogicEngine.createLuaScript(
                R"(
                function interface(IN,OUT)
                    IN.int_param = Type:Int32()
                    IN.nested_param = {
                        int_param = Type:Int32()
                    }
                    OUT.float_param = Type:Float()
                    OUT.nested_param = {
                        float_param = Type:Float()
                    }
                end
                function run(IN,OUT)
                    OUT.float_param = 47.11
                end
            )");

            ASSERT_NE(nullptr, script);
            EXPECT_TRUE(SaveToFileWithoutValidation(tempLogicEngine, "nested_array.bin"));
        }
        {
            EXPECT_TRUE(m_logicEngine.loadFromFile("nested_array.bin"));
            const LuaScript* loadedScript = *m_logicEngine.getCollection<LuaScript>().begin();

            ASSERT_NE(nullptr, loadedScript);

            auto inputs = loadedScript->getInputs();
            auto outputs = loadedScript->getOutputs();

            ASSERT_NE(nullptr, inputs);
            ASSERT_NE(nullptr, outputs);

            ASSERT_EQ(inputs->getChildCount(), 2u);
            ASSERT_EQ(outputs->getChildCount(), 2u);

            EXPECT_EQ("int_param", inputs->getChild(0u)->getName());
            EXPECT_EQ(EPropertyType::Int32, inputs->getChild(0u)->getType());
            EXPECT_EQ("float_param", outputs->getChild(0u)->getName());
            EXPECT_EQ(EPropertyType::Float, outputs->getChild(0u)->getType());

            auto in_child = inputs->getChild(1u);
            auto out_child = outputs->getChild(1u);

            EXPECT_EQ("nested_param", in_child->getName());
            EXPECT_EQ(EPropertyType::Struct, in_child->getType());
            EXPECT_EQ("nested_param", out_child->getName());
            EXPECT_EQ(EPropertyType::Struct, out_child->getType());

            ASSERT_EQ(in_child->getChildCount(), 1u);
            ASSERT_EQ(out_child->getChildCount(), 1u);

            auto in_nested_child = in_child->getChild(0u);
            auto out_nested_child = out_child->getChild(0u);

            EXPECT_EQ("int_param", in_nested_child->getName());
            EXPECT_EQ(EPropertyType::Int32, in_nested_child->getType());
            EXPECT_EQ("float_param", out_nested_child->getName());
            EXPECT_EQ(EPropertyType::Float, out_nested_child->getType());

            EXPECT_TRUE(m_logicEngine.update());
            EXPECT_FLOAT_EQ(47.11f, *outputs->getChild(0u)->get<float>());
        }
    }

    TEST_F(ALuaScript_LifecycleWithFiles, ArrayOfStructs)
    {
        {
            LogicEngine tempLogicEngine;
            auto script = tempLogicEngine.createLuaScript(
                R"(
                function interface(IN,OUT)
                    local structDecl = {
                        str = Type:String(),
                        array = Type:Array(2, Type:Int32()),
                        nested_struct = {
                            int = Type:Int32(),
                            nested_array = Type:Array(1, Type:Float()),
                        }
                    }
                    IN.arrayOfStructs = Type:Array(2, structDecl)
                    OUT.arrayOfStructs = Type:Array(2, structDecl)
                end
                function run(IN,OUT)
                    OUT.arrayOfStructs = IN.arrayOfStructs
                end
            )");

            ASSERT_NE(nullptr, script);
            script->getInputs()->getChild("arrayOfStructs")->getChild(1)->getChild("nested_struct")->getChild("nested_array")->getChild(0)->set<float>(42.f);
            EXPECT_TRUE(SaveToFileWithoutValidation(tempLogicEngine, "array_of_structs.bin"));
        }
        {
            EXPECT_TRUE(m_logicEngine.loadFromFile("array_of_structs.bin"));
            LuaScript* loadedScript = *m_logicEngine.getCollection<LuaScript>().begin();

            ASSERT_NE(nullptr, loadedScript);

            auto inputs = loadedScript->getInputs();
            auto outputs = loadedScript->getOutputs();

            ASSERT_NE(nullptr, inputs);
            ASSERT_NE(nullptr, outputs);

            auto loadedInput = loadedScript->getInputs()->getChild("arrayOfStructs")->getChild(1)->getChild("nested_struct")->getChild("nested_array")->getChild(0);
            EXPECT_FLOAT_EQ(42.f, *loadedInput->get<float>());
            loadedInput->set<float>(100.0f);

            EXPECT_TRUE(m_logicEngine.update());
            auto loadedOutput = loadedScript->getOutputs()->getChild("arrayOfStructs")->getChild(1)->getChild("nested_struct")->getChild("nested_array")->getChild(0);
            EXPECT_FLOAT_EQ(100.0f, *loadedOutput->get<float>());
        }
    }

    // This is a confidence test which tests all property types, both as inputs and outputs, and as arrays
    // The combination of arrays with different sizes, types, and their values, yields a lot of possible error cases, hence this test
    TEST_F(ALuaScript_LifecycleWithFiles, AllPropertyTypes_confidenceTest)
    {
        const std::vector<EPropertyType> allPrimitiveTypes =
        {
            EPropertyType::Float,
            EPropertyType::Vec2f,
            EPropertyType::Vec3f,
            EPropertyType::Vec4f,
            EPropertyType::Int32,
            EPropertyType::Int64,
            EPropertyType::Vec2i,
            EPropertyType::Vec3i,
            EPropertyType::Vec4i,
            EPropertyType::String,
            EPropertyType::Bool
        };

        std::string scriptSrc = "function interface(IN,OUT)\n";
        size_t arraySize = 1;
        // For each type, create an input, output, and an array version of it with various sizes
        for (auto primType : allPrimitiveTypes)
        {
            const std::string typeName = GetLuaPrimitiveTypeName(primType);
            scriptSrc += fmt::format("IN.{} =  Type:{}()\n", typeName, typeName);
            scriptSrc += fmt::format("IN.array_{} = Type:Array({}, Type:{}())\n", typeName, arraySize, typeName);
            scriptSrc += fmt::format("OUT.{} = Type:{}()\n", typeName, typeName);
            scriptSrc += fmt::format("OUT.array_{} = Type:Array({}, Type:{}())\n", typeName, arraySize, typeName);
            ++arraySize;
        }

        scriptSrc += R"(
                end
                function run(IN,OUT)
                end
            )";

        {
            LogicEngine tempLogicEngine;
            auto script = tempLogicEngine.createLuaScript(scriptSrc, {}, "MyScript");

            ASSERT_NE(nullptr, script);
            EXPECT_TRUE(SaveToFileWithoutValidation(tempLogicEngine, "arrays.bin"));
        }
        {
            EXPECT_TRUE(m_logicEngine.loadFromFile("arrays.bin"));
            const LuaScript* loadedScript = m_logicEngine.findByName<LuaScript>("MyScript");

            auto inputs = loadedScript->getInputs();
            auto outputs = loadedScript->getOutputs();

            // Test both inputs and outputs
            for (auto rootProp : std::initializer_list<const Property*>{ inputs, outputs })
            {
                // One primitive for each type, and one array for each type
                ASSERT_EQ(rootProp->getChildCount(), allPrimitiveTypes.size() * 2);

                size_t expectedArraySize = 1;
                for(const auto primType: allPrimitiveTypes)
                {
                    const auto primitiveChild = rootProp->getChild(GetLuaPrimitiveTypeName(primType));
                    const auto arrayChild = inputs->getChild(std::string("array_") + GetLuaPrimitiveTypeName(primType));

                    const std::string typeName = GetLuaPrimitiveTypeName(primType);

                    EXPECT_EQ(primType, primitiveChild->getType());
                    EXPECT_EQ(typeName, primitiveChild->getName());
                    EXPECT_EQ(0u, primitiveChild->getChildCount());

                    EXPECT_EQ("array_" + typeName, arrayChild->getName());
                    EXPECT_EQ(EPropertyType::Array, arrayChild->getType());
                    EXPECT_EQ(expectedArraySize, arrayChild->getChildCount());

                    for (size_t a = 0; a < expectedArraySize; ++a)
                    {
                        const auto arrayElement = arrayChild->getChild(a);
                        EXPECT_EQ("", arrayElement->getName());
                        EXPECT_EQ(primType, arrayElement->getType());
                        EXPECT_EQ(0u, arrayElement->getChildCount());
                    }
                    ++expectedArraySize;
                }
            }
        }
    }

    TEST_F(ALuaScript_LifecycleWithFiles, OverwritesCurrentData_WhenLoadedASecondTimeFromTheSameFile)
    {
        {
            LogicEngine tempLogicEngine;
            auto script = tempLogicEngine.createLuaScript(
                R"(
                function interface(IN,OUT)
                    IN.data = Type:Int32()
                end
                function run(IN,OUT)
                end
            )");

            ASSERT_NE(nullptr, script);
            script->getInputs()->getChild("data")->set<int32_t>(42);
            EXPECT_TRUE(SaveToFileWithoutValidation(tempLogicEngine, "script.bin"));
        }

        EXPECT_TRUE(m_logicEngine.loadFromFile("script.bin"));
        auto loadedScript = *m_logicEngine.getCollection<LuaScript>().begin();
        loadedScript->getInputs()->getChild("data")->set<int32_t>(5);

        EXPECT_TRUE(m_logicEngine.loadFromFile("script.bin"));
        loadedScript = *m_logicEngine.getCollection<LuaScript>().begin();
        EXPECT_EQ(42, *loadedScript->getInputs()->getChild("data")->get<int32_t>());
    }
}
