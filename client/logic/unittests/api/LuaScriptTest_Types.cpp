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

namespace ramses
{
    class ALuaScript_Types : public ALuaScript
    {
    };

    TEST_F(ALuaScript_Types, FailsToSetValueOfTopLevelInput_WhenTemplateDoesNotMatchDeclaredInputType)
    {
        auto* script = m_logicEngine.createLuaScript(m_minimalScriptWithInputs);
        auto  inputs = script->getInputs();

        auto speedInt32  = inputs->getChild("speed");
        auto tempFloat   = inputs->getChild("temp");
        auto nameString  = inputs->getChild("name");
        auto enabledBool = inputs->getChild("enabled");
        auto vec_2f      = inputs->getChild("vec2f");
        auto vec_3f      = inputs->getChild("vec3f");
        auto vec_4f      = inputs->getChild("vec4f");
        auto vec_2i      = inputs->getChild("vec2i");
        auto vec_3i      = inputs->getChild("vec3i");
        auto vec_4i      = inputs->getChild("vec4i");

        EXPECT_FALSE(speedInt32->set<float>(4711.f));
        EXPECT_FALSE(tempFloat->set<int32_t>(4711));
        EXPECT_FALSE(nameString->set<bool>(true));
        EXPECT_FALSE(enabledBool->set<std::string>("some string"));
        EXPECT_FALSE(vec_2f->set<float>(4711.f));
        EXPECT_FALSE(vec_3f->set<float>(4711.f));
        EXPECT_FALSE(vec_4f->set<float>(4711.f));
        EXPECT_FALSE(vec_2i->set<int32_t>(4711));
        EXPECT_FALSE(vec_3i->set<int32_t>(4711));
        EXPECT_FALSE(vec_4i->set<int32_t>(4711));
    }

    TEST_F(ALuaScript_Types, FailsToSetArrayDirectly)
    {
        auto* script = m_logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.array_int = Type:Array(2, Type:Int32())
            end

            function run(IN,OUT)
            end
        )");
        auto  array_int = script->getInputs()->getChild("array_int");

        EXPECT_FALSE(array_int->set<float>(4711.f));
        EXPECT_FALSE(array_int->set<int32_t>(4711));
        EXPECT_FALSE(array_int->set<bool>(true));
        EXPECT_FALSE(array_int->set<std::string>("some string"));
    }

    TEST_F(ALuaScript_Types, ProvidesIndexBasedAndNameBasedAccessToInputProperties)
    {
        auto* script = m_logicEngine.createLuaScript(m_minimalScriptWithInputs);
        ASSERT_NE(nullptr, script);
        auto enabled_byIndex = script->getInputs()->getChild(0);
        EXPECT_NE(nullptr, enabled_byIndex);
        EXPECT_EQ("enabled", enabled_byIndex->getName());
        auto speed_byName = script->getInputs()->getChild("speed");
        EXPECT_NE(nullptr, speed_byName);
        EXPECT_EQ("speed", speed_byName->getName());
    }

    TEST_F(ALuaScript_Types, ProvidesIndexBasedAndNameBasedAccessToOutputProperties)
    {
        auto* script = m_logicEngine.createLuaScript(m_minimalScriptWithOutputs);
        ASSERT_NE(nullptr, script);
        auto enabled_byIndex = script->getOutputs()->getChild(0);
        EXPECT_NE(nullptr, enabled_byIndex);
        EXPECT_EQ("enabled", enabled_byIndex->getName());
        auto speed_byName = script->getOutputs()->getChild("speed");
        EXPECT_NE(nullptr, speed_byName);
        EXPECT_EQ("speed", speed_byName->getName());
    }

    TEST_F(ALuaScript_Types, AssignsTypeToGlobalInputsStruct)
    {
        auto* script = m_logicEngine.createLuaScript(m_minimalScript);

        auto inputs = script->getInputs();

        ASSERT_EQ(0u, inputs->getChildCount());
        EXPECT_EQ("", inputs->getName());
        EXPECT_EQ(EPropertyType::Struct, inputs->getType());
    }

    TEST_F(ALuaScript_Types, AssignsTypeToGlobalOutputsStruct)
    {
        auto* script  = m_logicEngine.createLuaScript(m_minimalScript);
        auto  outputs = script->getOutputs();

        ASSERT_EQ(0u, outputs->getChildCount());
        EXPECT_EQ("", outputs->getName());
        EXPECT_EQ(EPropertyType::Struct, outputs->getType());
    }

    TEST_F(ALuaScript_Types, ReturnsItsTopLevelInputsByIndex_IndexEqualsLexicographicOrder)
    {
        auto* script = m_logicEngine.createLuaScript(m_minimalScriptWithInputs);

        auto inputs = script->getInputs();

        ASSERT_EQ(11u, inputs->getChildCount());
        EXPECT_EQ("enabled", inputs->getChild(0)->getName());
        EXPECT_EQ(EPropertyType::Bool, inputs->getChild(0)->getType());
        EXPECT_EQ("name", inputs->getChild(1)->getName());
        EXPECT_EQ(EPropertyType::String, inputs->getChild(1)->getType());
        EXPECT_EQ("speed", inputs->getChild(2)->getName());
        EXPECT_EQ(EPropertyType::Int32, inputs->getChild(2)->getType());
        EXPECT_EQ("speed2", inputs->getChild(3)->getName());
        EXPECT_EQ(EPropertyType::Int64, inputs->getChild(3)->getType());
        EXPECT_EQ("temp", inputs->getChild(4)->getName());
        EXPECT_EQ(EPropertyType::Float, inputs->getChild(4)->getType());

        // Vec2/3/4 f/i
        EXPECT_EQ("vec2f", inputs->getChild(5)->getName());
        EXPECT_EQ(EPropertyType::Vec2f, inputs->getChild(5)->getType());
        EXPECT_EQ("vec2i", inputs->getChild(6)->getName());
        EXPECT_EQ(EPropertyType::Vec2i, inputs->getChild(6)->getType());
        EXPECT_EQ("vec3f", inputs->getChild(7)->getName());
        EXPECT_EQ(EPropertyType::Vec3f, inputs->getChild(7)->getType());
        EXPECT_EQ("vec3i", inputs->getChild(8)->getName());
        EXPECT_EQ(EPropertyType::Vec3i, inputs->getChild(8)->getType());
        EXPECT_EQ("vec4f", inputs->getChild(9)->getName());
        EXPECT_EQ(EPropertyType::Vec4f, inputs->getChild(9)->getType());
        EXPECT_EQ("vec4i", inputs->getChild(10)->getName());
        EXPECT_EQ(EPropertyType::Vec4i, inputs->getChild(10)->getType());
    }

    TEST_F(ALuaScript_Types, ProvidesFullTypeInformationOnArrayProperties)
    {
        std::string_view scriptWithArrays = R"(
            function interface(IN,OUT)
                IN.array_int = Type:Array(2, Type:Int32())
                IN.array_float = Type:Array(3, Type:Float())
                OUT.array_int = Type:Array(2, Type:Int32())
                OUT.array_float = Type:Array(3, Type:Float())
            end

            function run(IN,OUT)
            end
        )";

        auto* script = m_logicEngine.createLuaScript(scriptWithArrays);

        std::initializer_list<const Property*> rootProperties = {script->getInputs(), script->getOutputs()};

        for (auto rootProp : rootProperties)
        {
            ASSERT_EQ(2u, rootProp->getChildCount());
            auto array_int = rootProp->getChild("array_int");

            EXPECT_EQ("array_int", array_int->getName());
            EXPECT_EQ(EPropertyType::Array, array_int->getType());
            EXPECT_EQ(2u, array_int->getChildCount());
            EXPECT_EQ(EPropertyType::Int32, array_int->getChild(0)->getType());
            EXPECT_EQ(EPropertyType::Int32, array_int->getChild(1)->getType());
            EXPECT_EQ("", array_int->getChild(0)->getName());
            EXPECT_EQ("", array_int->getChild(1)->getName());
            EXPECT_EQ(0u, array_int->getChild(0)->getChildCount());
            EXPECT_EQ(0u, array_int->getChild(1)->getChildCount());

            auto array_float = rootProp->getChild("array_float");
            EXPECT_EQ("array_float", array_float->getName());
            EXPECT_EQ(EPropertyType::Array, array_float->getType());
            EXPECT_EQ(3u, array_float->getChildCount());
            auto array_float_0 = array_float->getChild(0);
            auto array_float_1 = array_float->getChild(1);
            auto array_float_2 = array_float->getChild(2);
            EXPECT_EQ(EPropertyType::Float, array_float_0->getType());
            EXPECT_EQ(EPropertyType::Float, array_float_1->getType());
            EXPECT_EQ(EPropertyType::Float, array_float_2->getType());
            EXPECT_EQ("", array_float_0->getName());
            EXPECT_EQ("", array_float_1->getName());
            EXPECT_EQ("", array_float_2->getName());
            EXPECT_EQ(0u, array_float_0->getChildCount());
            EXPECT_EQ(0u, array_float_1->getChildCount());
            EXPECT_EQ(0u, array_float_2->getChildCount());
        }
    }

    TEST_F(ALuaScript_Types, ProvidesFullTypeInformationOnArrayOfStructProperties)
    {
        std::string_view scriptWithArrays = R"(
            function interface(IN,OUT)
                local struct_decl = {
                    name = Type:String(),
                    address =
                    {
                        street = Type:String(),
                        number = Type:Int32()
                    }
                }
                IN.array_of_structs = Type:Array(2, struct_decl)
                OUT.array_of_structs = Type:Array(2, struct_decl)
            end

            function run(IN,OUT)
            end
        )";

        auto* script = m_logicEngine.createLuaScript(scriptWithArrays);

        std::initializer_list<const Property*> rootProperties = { script->getInputs(), script->getOutputs() };

        for (auto rootProp : rootProperties)
        {
            ASSERT_EQ(1u, rootProp->getChildCount());
            const auto arrayOfStructs = rootProp->getChild(0);

            EXPECT_EQ("array_of_structs", arrayOfStructs->getName());
            EXPECT_EQ(EPropertyType::Array, arrayOfStructs->getType());
            ASSERT_EQ(2u, arrayOfStructs->getChildCount());

            for (size_t i = 0; i < 2; ++i)
            {
                const auto structChild = arrayOfStructs->getChild(i);
                EXPECT_EQ(EPropertyType::Struct, structChild->getType());
                EXPECT_EQ("", structChild->getName());
                ASSERT_EQ(2u, structChild->getChildCount());
                const auto name = structChild->getChild("name");
                EXPECT_EQ(EPropertyType::String, name->getType());
                const auto address = structChild->getChild("address");
                ASSERT_EQ(2u, address->getChildCount());
                EXPECT_EQ(EPropertyType::Struct, address->getType());
                const auto addressStr = address->getChild("street");
                const auto addressNr = address->getChild("number");
                EXPECT_EQ(EPropertyType::String, addressStr->getType());
                EXPECT_EQ(EPropertyType::Int32, addressNr->getType());
            }
        }
    }
}
