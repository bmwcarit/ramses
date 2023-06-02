//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicViewerTestBase.h"
#include "ramses-logic/AnimationNodeConfig.h"
#include "ramses-logic/RamsesRenderGroupBindingElements.h"

const char* const logicFile = "test.rlogic";

namespace ramses::internal
{
    /**
     * Tests for the lua configuration file handling
     */
    class ALogicViewerLua : public ALogicViewerBase
    {
    public:
        ALogicViewerLua()
        {
            createLogicFile();
            EXPECT_TRUE(viewer.loadRamsesLogic(logicFile, m_scene));
        }

        void createLogicFile()
        {
            LogicEngine engine{ ramses::EFeatureLevel_Latest };
            auto *interface = engine.createLuaInterface(R"(
                function interface(IN,OUT)
                    IN.paramInt32 = Type:Int32()
                end
            )", "foo");

            auto* script = engine.createLuaScript(R"(
                function interface(IN,OUT)
                    IN.paramBool = Type:Bool()
                    IN.paramInt32 = Type:Int32()
                    IN.paramInt32_2 = Type:Int32()
                    IN.paramInt64 = Type:Int64()
                    IN.paramInt64_2 = Type:Int64()
                    IN.paramFloat = Type:Float()
                    IN.paramFloat_2 = Type:Float()
                    IN.paramString = Type:String()
                    IN.paramVec2f = Type:Vec2f()
                    IN.paramVec3f = Type:Vec3f()
                    IN.paramVec4f = Type:Vec4f()
                    IN.paramVec2i = Type:Vec2i()
                    IN.paramVec3i = Type:Vec3i()
                    IN.paramVec4i = Type:Vec4i()
                    IN.array = Type:Array(5, Type:Float())
                    IN.struct = {
                        nested = {
                            data1 = Type:String(),
                            data2 = Type:Int32()
                        }
                    }
                    IN.anchorData1 = Type:Vec2f()
                    IN.anchorData2 = Type:Float()

                    OUT.paramBool = Type:Bool()
                    OUT.paramInt32 = Type:Int32()
                    OUT.paramInt64 = Type:Int64()
                    OUT.paramFloat = Type:Float()
                    OUT.paramString = Type:String()
                    OUT.paramVec2f = Type:Vec2f()
                    OUT.paramVec3f = Type:Vec3f()
                    OUT.paramVec4f = Type:Vec4f()
                    OUT.paramVec2i = Type:Vec2i()
                    OUT.paramVec3i = Type:Vec3i()
                    OUT.paramVec4i = Type:Vec4i()
                    OUT.array = Type:Array(5, Type:Float())
                    OUT.struct = {
                        nested = {
                            data1 = Type:String(),
                            data2 = Type:Int32()
                        }
                    }
                    OUT.anchorData1 = Type:Vec2f()
                    OUT.anchorData2 = Type:Float()
                end
                function run(IN,OUT)
                    OUT.paramBool = not IN.paramBool
                    OUT.paramInt32 = 2 * IN.paramInt32
                    OUT.paramInt64 = 1 + IN.paramInt64
                    OUT.paramFloat = 3 * IN.paramFloat
                    OUT.paramString = IN.paramString.."foo"
                    OUT.paramVec2f = {0, IN.paramFloat}
                    OUT.paramVec3f = {0, 0, IN.paramFloat}
                    OUT.paramVec4f = {0, 0, 0, IN.paramFloat}
                    OUT.paramVec2i = {0, IN.paramInt32}
                    OUT.paramVec3i = {0, 0, IN.paramInt32}
                    OUT.paramVec4i = {0, 0, 0, IN.paramInt32}
                    OUT.array = {10,20,30,44,50}
                    OUT.struct.nested.data1 = IN.paramString
                    OUT.struct.nested.data2 = IN.paramInt32
                    OUT.anchorData1 = IN.anchorData1
                    OUT.anchorData2 = IN.anchorData2
                end
            )", {}, "foo");

            ASSERT_TRUE(script != nullptr);

            auto* nodeBinding = engine.createRamsesNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "foo");

            // make camera valid
            m_camera->setFrustum(-1.f, 1.f, -1.f, 1.f, 0.1f, 10.f);

            auto* appearanceBind = engine.createRamsesAppearanceBinding(*m_appearance, "foo");
            auto* cameraBinding = engine.createRamsesCameraBinding(*m_camera, "foo");
            auto* passBinding = engine.createRamsesRenderPassBinding(*m_renderPass, "foo");
            auto* timer = engine.createTimerNode("foo");
            auto* anchor = engine.createAnchorPoint(*nodeBinding, *cameraBinding, "foo");
            m_renderGroup->addRenderGroup(*m_nestedRenderGroup);
            ramses::RamsesRenderGroupBindingElements elements;
            elements.addElement(*m_nestedRenderGroup, "nestedRG");
            auto rgBinding = engine.createRamsesRenderGroupBinding(*m_renderGroup, elements, "rg");
            auto meshBinding = engine.createRamsesMeshNodeBinding(*m_meshNode, "mn");

            ramses::DataArray* animTimestamps = engine.createDataArray(std::vector<float>{ 0.f, 0.5f, 1.f, 1.5f }); // will be interpreted as seconds
            ramses::DataArray* animKeyframes = engine.createDataArray(std::vector<ramses::vec3f>{ {0.f, 0.f, 0.f}, {0.f, 0.f, 180.f}, {0.f, 0.f, 100.f}, {0.f, 0.f, 360.f} });
            const ramses::AnimationChannel stepAnimChannel { "rotationZstep", animTimestamps, animKeyframes, ramses::EInterpolationType::Step };
            ramses::AnimationNodeConfig config;
            config.addChannel(stepAnimChannel);
            auto* animation = engine.createAnimationNode(config, "foo");

            // link some of the created objects' inputs and outputs, so that none of the scripts
            // generate a warning for having unlinked inputs or outputs on saving to file
            engine.link(
                *interface->getOutputs()->getChild("paramInt32"),
                *script->getInputs()->getChild("paramInt32"));

            engine.link(
                *script->getOutputs()->getChild("paramVec3f"),
                *nodeBinding->getInputs()->getChild("rotation"));

            engine.link(
                *animation->getOutputs()->getChild("duration"),
                *script->getInputs()->getChild("paramFloat_2"));

            // use weak link because of circular dependency. The link has no meaning, it is just
            // needed to make the setup valid (fee of dangling content)
            engine.linkWeak(
                *script->getOutputs()->getChild("paramFloat"),
                *animation->getInputs()->getChild("progress"));

            engine.link(
                *script->getOutputs()->getChild("paramBool"),
                *passBinding->getInputs()->getChild("enabled"));

            engine.link(
                *script->getOutputs()->getChild("paramInt32"),
                *rgBinding->getInputs()->getChild("renderOrders")->getChild("nestedRG"));

            engine.link(
                *script->getOutputs()->getChild("paramInt32"),
                *meshBinding->getInputs()->getChild("vertexOffset"));

            engine.link(
                *timer->getOutputs()->getChild("ticker_us"),
                *script->getInputs()->getChild("paramInt64_2"));

            engine.link(
                *script->getOutputs()->getChild("paramFloat"),
                *appearanceBind->getInputs()->getChild("floatUniform"));

            engine.link(
                *script->getOutputs()->getChild("paramFloat"),
                *cameraBinding->getInputs()->getChild("frustum")->getChild("leftPlane"));

            // use weak link because of circular dependency
            engine.linkWeak(
                *anchor->getOutputs()->getChild("viewportCoords"),
                *script->getInputs()->getChild("anchorData1"));
            engine.linkWeak(
                *anchor->getOutputs()->getChild("depth"),
                *script->getInputs()->getChild("anchorData2"));

            engine.update();

            engine.saveToFile(logicFile);
        }

        void unlinkInput(const ramses::Property& inputProperty)
        {
            assert(inputProperty.hasIncomingLink());
            const ramses::Property& sourceProperty = *inputProperty.getIncomingLink()->source;
            EXPECT_TRUE(viewer.getEngine().unlink(sourceProperty, inputProperty));
        }
    };

    TEST_F(ALogicViewerLua, loadLuaFileEmpty)
    {
        EXPECT_EQ(Result(), loadLua(""));
    }

    TEST_F(ALogicViewerLua, setInputBool)
    {
        const Result ok;
        EXPECT_EQ(ok, loadLua(R"(rlogic.scripts.foo.IN.paramBool.value = true)"));
        auto* property = getInput<ramses::LuaScript>("foo", "paramBool");
        EXPECT_TRUE(property->get<bool>().value());
        EXPECT_EQ(ok, loadLua(R"(rlogic.scripts.foo.IN.paramBool.value = false)"));
        EXPECT_FALSE(property->get<bool>().value());
    }

    TEST_F(ALogicViewerLua, setInputInt32)
    {
        const Result ok;
        EXPECT_EQ(ok, loadLua(R"(rlogic.interfaces.foo.IN.paramInt32.value = 42)"));
        auto* property = getInput<ramses::LuaScript>("foo", "paramInt32");
        EXPECT_EQ(42, property->get<int32_t>());
    }

    TEST_F(ALogicViewerLua, setInputInt64)
    {
        const Result ok;
        auto* property = getInput<ramses::LuaScript>("foo", "paramInt64");
        EXPECT_EQ(ok, loadLua(R"(rlogic.scripts.foo.IN.paramInt64.value = 42)"));
        EXPECT_EQ(42, property->get<int64_t>());
    }

    TEST_F(ALogicViewerLua, setInputFloat)
    {
        const Result ok;
        EXPECT_EQ(ok, loadLua(R"(rlogic.scripts.foo.IN.paramFloat.value = 42.5)"));
        auto* property = getInput<ramses::LuaScript>("foo", "paramFloat");
        EXPECT_FLOAT_EQ(42.5f, property->get<float>().value());
    }

    TEST_F(ALogicViewerLua, setInputString)
    {
        const Result ok;
        auto* property = getInput<ramses::LuaScript>("foo", "paramString");
        EXPECT_EQ(ok, loadLua(R"(rlogic.scripts.foo.IN.paramString.value = "Hello World")"));
        EXPECT_EQ("Hello World", property->get<std::string>());
    }

    TEST_F(ALogicViewerLua, setInputVec2f)
    {
        const Result ok;
        auto* property = getInput<ramses::LuaScript>("foo", "paramVec2f");
        EXPECT_EQ(ok, loadLua(R"(rlogic.scripts.foo.IN.paramVec2f.value = {42.5, 1.3})"));
        EXPECT_FLOAT_EQ(42.5f, property->get<ramses::vec2f>().value()[0]);
        EXPECT_FLOAT_EQ(1.3f, property->get<ramses::vec2f>().value()[1]);
    }

    TEST_F(ALogicViewerLua, setInputVec3f)
    {
        const Result ok;
        auto* property = getInput<ramses::LuaScript>("foo", "paramVec3f");
        EXPECT_EQ(ok, loadLua(R"(rlogic.scripts.foo.IN.paramVec3f.value = {42.5, 1.3, 100000})"));
        EXPECT_FLOAT_EQ(42.5f, property->get<ramses::vec3f>().value()[0]);
        EXPECT_FLOAT_EQ(1.3f, property->get<ramses::vec3f>().value()[1]);
        EXPECT_FLOAT_EQ(100000.f, property->get<ramses::vec3f>().value()[2]);
    }

    TEST_F(ALogicViewerLua, setInputVec4f)
    {
        const Result ok;
        auto* property = getInput<ramses::LuaScript>("foo", "paramVec4f");
        EXPECT_EQ(ok, loadLua(R"(rlogic.scripts.foo.IN.paramVec4f.value = {42.5, 1.3, -8.2, 0.0001})"));
        EXPECT_FLOAT_EQ(42.5f, property->get<ramses::vec4f>().value()[0]);
        EXPECT_FLOAT_EQ(1.3f, property->get<ramses::vec4f>().value()[1]);
        EXPECT_FLOAT_EQ(-8.2f, property->get<ramses::vec4f>().value()[2]);
        EXPECT_FLOAT_EQ(0.0001f, property->get<ramses::vec4f>().value()[3]);
    }

    TEST_F(ALogicViewerLua, setInputVec2i)
    {
        const Result ok;
        auto* property = getInput<ramses::LuaScript>("foo", "paramVec2i");
        EXPECT_EQ(ok, loadLua(R"(rlogic.scripts.foo.IN.paramVec2i.value = {5, -18})"));
        EXPECT_EQ(ramses::vec2i({5, -18}), property->get<ramses::vec2i>());
    }

    TEST_F(ALogicViewerLua, setInputVec3i)
    {
        const Result ok;
        auto* property = getInput<ramses::LuaScript>("foo", "paramVec3i");
        EXPECT_EQ(ok, loadLua(R"(rlogic.scripts.foo.IN.paramVec3i.value = {5, 0xffad, -10000})"));
        EXPECT_EQ(ramses::vec3i({5, 0xffad, -10000}), property->get<ramses::vec3i>());
    }

    TEST_F(ALogicViewerLua, setInputVec4i)
    {
        const Result ok;
        auto* property = getInput<ramses::LuaScript>("foo", "paramVec4i");
        EXPECT_EQ(ok, loadLua(R"(rlogic.scripts.foo.IN.paramVec4i.value = {2147483647, -2147483647, 5, -18})"));
        EXPECT_EQ(ramses::vec4i({2147483647, -2147483647, 5, -18}), property->get<ramses::vec4i>());
    }

    TEST_F(ALogicViewerLua, setInputArrayFloat)
    {
        const Result ok;
        auto* property = getInput<ramses::LuaScript>("foo", "array");
        EXPECT_EQ(ok, loadLua(R"(rlogic.scripts.foo.IN.array.value = {99, 118.119, 1.3, -8.2, 0.0001})"));
        EXPECT_EQ(5u, property->getChildCount());
        EXPECT_FLOAT_EQ(99.f, property->getChild(0)->get<float>().value());
        EXPECT_FLOAT_EQ(118.119f, property->getChild(1)->get<float>().value());
        EXPECT_FLOAT_EQ(1.3f, property->getChild(2)->get<float>().value());
        EXPECT_FLOAT_EQ(-8.2f, property->getChild(3)->get<float>().value());
        EXPECT_FLOAT_EQ(0.0001f, property->getChild(4)->get<float>().value());
    }

    TEST_F(ALogicViewerLua, setInputArrayByIndex)
    {
        auto* property = getInput<ramses::LuaScript>("foo", "array");
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.scripts.foo.IN.array.value = {99, 42, 11, 13, 0.1}
            rlogic.scripts.foo.IN.array[1].value = rlogic.scripts.foo.IN.array[5].value
        )"));
        EXPECT_FLOAT_EQ(0.1f, property->getChild(0)->get<float>().value());
    }

    TEST_F(ALogicViewerLua, setInputArrayFloatOutOfBounds)
    {
        EXPECT_THAT(loadLua(R"(rlogic.scripts.foo.IN.array.value = {99, 118.119, 1.3, -8.2, 0.0001, 12})").getMessage(),
            testing::HasSubstr("index 6 out of bounds for array array[5]"));
    }

    TEST_F(ALogicViewerLua, setInputArrayByIndexOutOfBounds)
    {
        EXPECT_THAT(loadLua(R"(rlogic.scripts.foo.IN.array[6].value = 14)").getMessage(),
            testing::HasSubstr("attempt to index field '?' (a nil value)"));
    }

    TEST_F(ALogicViewerLua, setInputArrayByIndexBadSyntax)
    {
        EXPECT_THAT(loadLua(R"(rlogic.scripts.foo.IN.array.value[6] = 14)").getMessage(),
            testing::HasSubstr("attempt to index field 'value' (a nil value)"));
    }

    TEST_F(ALogicViewerLua, setInputStruct)
    {
        const Result ok;
        auto* property = getInput<ramses::LuaScript>("foo", "struct");
        EXPECT_EQ(ok, loadLua(R"(rlogic.scripts.foo.IN.struct.value = { nested = {data1 = "Baz", data2 = 400}})"));
        EXPECT_EQ("Baz", property->getChild("nested")->getChild("data1")->get<std::string>());
        EXPECT_EQ(400, property->getChild("nested")->getChild("data2")->get<int32_t>());
        EXPECT_EQ(ok, loadLua(R"(rlogic.scripts.foo.IN.struct.nested.data1.value = "Foo")"));
        EXPECT_EQ("Foo", property->getChild("nested")->getChild("data1")->get<std::string>());
        EXPECT_EQ(400, property->getChild("nested")->getChild("data2")->get<int32_t>());
        EXPECT_EQ(ok, loadLua(R"(rlogic.scripts.foo.IN.struct.nested.data2.value = -12)"));
        EXPECT_EQ("Foo", property->getChild("nested")->getChild("data1")->get<std::string>());
        EXPECT_EQ(-12, property->getChild("nested")->getChild("data2")->get<int32_t>());
    }

    TEST_F(ALogicViewerLua, setInputStructNotAKey)
    {
        EXPECT_THAT(loadLua(R"(rlogic.scripts.foo.IN.struct.value = { notAKey = {data1 = "Baz", data2 = 400}})").getMessage(),
            testing::HasSubstr("Property not found in struct: notAKey"));
    }

    TEST_F(ALogicViewerLua, getOutputBool)
    {
        const auto lua = R"(
            if rlogic.scripts.foo.OUT.paramBool.value then
                rlogic.scripts.foo.IN.paramString.value = "true"
            else
                rlogic.scripts.foo.IN.paramString.value = "false"
            end
        )";
        EXPECT_TRUE(getInput<ramses::LuaScript>("foo", "paramBool")->set(false));
        EXPECT_EQ(Result(), viewer.update());
        EXPECT_EQ(Result(), loadLua(lua));
        EXPECT_EQ("true", getInput<ramses::LuaScript>("foo", "paramString")->get<std::string>().value());
        EXPECT_TRUE(getInput<ramses::LuaScript>("foo", "paramBool")->set(true));
        EXPECT_EQ(Result(), viewer.update());
        EXPECT_EQ(Result(), loadLua(lua));
        EXPECT_EQ("false", getInput<ramses::LuaScript>("foo", "paramString")->get<std::string>().value());
    }

    TEST_F(ALogicViewerLua, getOutputInt32)
    {
        EXPECT_TRUE(getInput<ramses::LuaInterface>("foo", "paramInt32")->set(static_cast<int32_t>(43)));
        EXPECT_EQ(Result(), viewer.update());
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.scripts.foo.IN.paramString.value = rlogic.scripts.foo.OUT.paramInt32.value
        )"));
        EXPECT_EQ("86", getInput<ramses::LuaScript>("foo", "paramString")->get<std::string>().value());
    }

    TEST_F(ALogicViewerLua, getOutputInt64)
    {
        EXPECT_TRUE(getInput<ramses::LuaScript>("foo", "paramInt64")->set(static_cast<int64_t>(12)));
        EXPECT_EQ(Result(), viewer.update());
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.scripts.foo.IN.paramString.value = rlogic.scripts.foo.OUT.paramInt64.value
        )"));
        EXPECT_EQ("13", getInput<ramses::LuaScript>("foo", "paramString")->get<std::string>().value());
    }

    TEST_F(ALogicViewerLua, getOutputFloat)
    {
        EXPECT_TRUE(getInput<ramses::LuaScript>("foo", "paramFloat")->set(-3.2f));
        EXPECT_EQ(Result(), viewer.update());
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.scripts.foo.IN.paramString.value = rlogic.scripts.foo.OUT.paramFloat.value
        )"));
        EXPECT_THAT(getInput<ramses::LuaScript>("foo", "paramString")->get<std::string>().value(), testing::StartsWith("-9.6"));
    }

    TEST_F(ALogicViewerLua, getOutputString)
    {
        EXPECT_TRUE(getInput<ramses::LuaScript>("foo", "paramString")->set(std::string("Hello")));
        EXPECT_EQ(Result(), viewer.update());
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.scripts.foo.IN.paramString.value = rlogic.scripts.foo.OUT.paramString.value
        )"));
        EXPECT_EQ("Hellofoo", getInput<ramses::LuaScript>("foo", "paramString")->get<std::string>().value());
    }

    TEST_F(ALogicViewerLua, getOutputVec2f)
    {
        EXPECT_TRUE(getInput<ramses::LuaScript>("foo", "paramFloat")->set(1.0f));
        EXPECT_EQ(Result(), viewer.update());
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.scripts.foo.IN.paramString.value = rlogic.scripts.foo.OUT.paramVec2f.value[2]
        )"));
        EXPECT_EQ("1", getInput<ramses::LuaScript>("foo", "paramString")->get<std::string>().value());
    }

    TEST_F(ALogicViewerLua, getOutputVec3f)
    {
        EXPECT_TRUE(getInput<ramses::LuaScript>("foo", "paramFloat")->set(2.0f));
        EXPECT_EQ(Result(), viewer.update());
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.scripts.foo.IN.paramString.value = rlogic.scripts.foo.OUT.paramVec3f.value[3]
        )"));
        EXPECT_EQ("2", getInput<ramses::LuaScript>("foo", "paramString")->get<std::string>().value());
    }

    TEST_F(ALogicViewerLua, getOutputVec4f)
    {
        EXPECT_TRUE(getInput<ramses::LuaScript>("foo", "paramFloat")->set(-1.0f));
        EXPECT_EQ(Result(), viewer.update());
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.scripts.foo.IN.paramString.value = rlogic.scripts.foo.OUT.paramVec4f.value[4]
        )"));
        EXPECT_EQ("-1", getInput<ramses::LuaScript>("foo", "paramString")->get<std::string>().value());
    }

    TEST_F(ALogicViewerLua, getOutputVec2i)
    {
        EXPECT_TRUE(getInput<ramses::LuaInterface>("foo", "paramInt32")->set(17));
        EXPECT_EQ(Result(), viewer.update());
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.scripts.foo.IN.paramString.value = rlogic.scripts.foo.OUT.paramVec2i.value[2]
        )"));
        EXPECT_EQ("17", getInput<ramses::LuaScript>("foo", "paramString")->get<std::string>().value());
    }

    TEST_F(ALogicViewerLua, getOutputVec3i)
    {
        EXPECT_TRUE(getInput<ramses::LuaInterface>("foo", "paramInt32")->set(18));
        EXPECT_EQ(Result(), viewer.update());
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.scripts.foo.IN.paramString.value = rlogic.scripts.foo.OUT.paramVec3i.value[3]
        )"));
        EXPECT_EQ("18", getInput<ramses::LuaScript>("foo", "paramString")->get<std::string>().value());
    }

    TEST_F(ALogicViewerLua, getOutputVec4i)
    {
        EXPECT_TRUE(getInput<ramses::LuaInterface>("foo", "paramInt32")->set(19));
        EXPECT_EQ(Result(), viewer.update());
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.scripts.foo.IN.paramString.value = rlogic.scripts.foo.OUT.paramVec4i.value[4]
        )"));
        EXPECT_EQ("19", getInput<ramses::LuaScript>("foo", "paramString")->get<std::string>().value());
    }

    TEST_F(ALogicViewerLua, getOutputArray)
    {
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.scripts.foo.IN.paramString.value = rlogic.scripts.foo.OUT.array[5].value
        )"));
        EXPECT_EQ("50", getInput<ramses::LuaScript>("foo", "paramString")->get<std::string>().value());
    }

    TEST_F(ALogicViewerLua, getOutputArrayOutOfBounds)
    {
        EXPECT_THAT(loadLua(R"(
            rlogic.scripts.foo.IN.paramString.value = rlogic.scripts.foo.OUT.array[6].value
        )").getMessage(), testing::HasSubstr("attempt to index field '?' (a nil value)"));
    }

    TEST_F(ALogicViewerLua, getOutputStruct)
    {
        EXPECT_TRUE(getInput<ramses::LuaInterface>("foo", "paramInt32")->set(15));
        EXPECT_TRUE(getInput<ramses::LuaScript>("foo", "paramString")->set(std::string("Hello")));
        EXPECT_EQ(Result(), viewer.update());
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.scripts.foo.IN.paramString.value = rlogic.scripts.foo.OUT.struct.nested.data1.value..rlogic.scripts.foo.OUT.struct.nested.data2.value
        )"));
        EXPECT_EQ("Hello15", getInput<ramses::LuaScript>("foo", "paramString")->get<std::string>().value());
    }

    TEST_F(ALogicViewerLua, outputPropertyToString)
    {
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.scripts.foo.IN.paramString.value = tostring(rlogic.scripts.foo.OUT.paramInt32)
        )"));
        EXPECT_EQ("ConstProperty: paramInt32", getInput<ramses::LuaScript>("foo", "paramString")->get<std::string>().value());
    }

    TEST_F(ALogicViewerLua, inputPropertyToString)
    {
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.scripts.foo.IN.paramString.value = tostring(rlogic.scripts.foo.IN.paramInt32)
        )"));
        EXPECT_EQ("Property: paramInt32", getInput<ramses::LuaScript>("foo", "paramString")->get<std::string>().value());
    }

    TEST_F(ALogicViewerLua, nodeToString)
    {
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.scripts.foo.IN.paramString.value = tostring(rlogic.scripts.foo)
        )"));
        EXPECT_EQ("LogicNode: foo", getInput<ramses::LuaScript>("foo", "paramString")->get<std::string>().value());
    }

    TEST_F(ALogicViewerLua, invalidView)
    {
        EXPECT_EQ(0u, viewer.getViewCount());
        auto view = viewer.getView(0u);
        EXPECT_FALSE(view.isValid());
        EXPECT_EQ("", view.name());
        EXPECT_EQ("", view.description());
        EXPECT_EQ(nullptr, view.getInput(0));
        EXPECT_EQ(0u, view.getInputCount());
    }

    TEST_F(ALogicViewerLua, simpleView)
    {
        const Result ok;
        auto* property = getInput<ramses::LuaScript>("foo", "paramInt32");
        EXPECT_EQ(0u, viewer.getViewCount());
        EXPECT_FALSE(viewer.getView(0u).isValid());
        EXPECT_FALSE(viewer.getView(1u).isValid());
        EXPECT_EQ(0, property->get<int32_t>().value());

        EXPECT_EQ(ok, loadLua(R"(
            defaultView = {
                name = "Default View",
                description = "Description for the default view",
                update = function(time_ms)
                    rlogic.interfaces.foo.IN.paramInt32.value = 1773
                end
            }

            rlogic.views = {defaultView}
        )"));

        EXPECT_EQ(1u, viewer.getViewCount());
        EXPECT_EQ(1u, viewer.getCurrentView());
        auto view = viewer.getView(1u);
        EXPECT_TRUE(view.isValid());
        EXPECT_EQ("Default View", view.name());
        EXPECT_EQ("Description for the default view", view.description());
        EXPECT_EQ(0u, view.getInputCount());
        EXPECT_EQ(nullptr, view.getInput(0u));

        EXPECT_EQ(ok, viewer.update());
        EXPECT_EQ(1773, property->get<int32_t>().value());
    }

    TEST_F(ALogicViewerLua, viewMissingUpdate)
    {
        EXPECT_THAT(loadLua(R"(
            defaultView = {
                name = "View with missing update() function",
            }

            rlogic.views = {defaultView}
        )").getMessage(), testing::HasSubstr("update() function is missing for current view"));
    }

    TEST_F(ALogicViewerLua, viewReportsErrorOnUpdate)
    {
        EXPECT_THAT(loadLua(R"(
            defaultView = {
                name = "View with error in update",
                update = function(time_ms)
                    error("view error during update")
                end
            }

            rlogic.views = {defaultView}
        )").getMessage(), testing::HasSubstr("view error during update"));
    }

    TEST_F(ALogicViewerLua, viewWithInput)
    {
        const Result ok;
        auto* property = getInput<ramses::LuaInterface>("foo", "paramInt32");
        ASSERT_TRUE(property != nullptr);

        EXPECT_EQ(ok, loadLua(R"(
            defaultView = {
                name = "View1",
                description = "View with 1 user input",
                update = function(time_ms)
                end,
                inputs = {rlogic.interfaces.foo.IN.paramInt32}
            }

            rlogic.views = {defaultView}
        )"));

        EXPECT_EQ(1u, viewer.getViewCount());
        EXPECT_EQ(1u, viewer.getCurrentView());
        auto view = viewer.getView(1u);
        EXPECT_TRUE(view.isValid());
        EXPECT_EQ("View1", view.name());
        EXPECT_EQ("View with 1 user input", view.description());
        EXPECT_EQ(1u, view.getInputCount());
        auto* input1 = view.getInput(0u);
        EXPECT_EQ(property, input1);
    }

    TEST_F(ALogicViewerLua, changeView)
    {
        const Result ok;
        auto* property1 = getInput<ramses::LuaInterface>("foo", "paramInt32");
        auto* property2 = getInput<ramses::LuaScript>("foo", "paramString");
        ASSERT_TRUE(property1 != nullptr);
        ASSERT_TRUE(property2 != nullptr);

        EXPECT_EQ(ok, loadLua(R"(
            view1 = {
                name = "View1",
                description = "View1 with 2 user inputs",
                update = function(time_ms)
                end,
                inputs = {
                    rlogic.interfaces.foo.IN.paramInt32,
                    rlogic.scripts.foo.IN.paramString,
                }
            }

            view2 = {
                name = "View2",
                description = "View2 with no inputs",
                update = function(time_ms)
                    rlogic.interfaces.foo.IN.paramInt32.value = 1773
                end,
            }

            rlogic.views = {view1, view2}
        )"));

        EXPECT_EQ(2u, viewer.getViewCount());
        EXPECT_EQ(1u, viewer.getCurrentView());
        auto view1 = viewer.getView(1u);
        EXPECT_TRUE(view1.isValid());
        EXPECT_EQ("View1", view1.name());
        EXPECT_EQ("View1 with 2 user inputs", view1.description());
        EXPECT_EQ(2u, view1.getInputCount());
        EXPECT_EQ(property1, view1.getInput(0u));
        EXPECT_EQ(property2, view1.getInput(1u));

        EXPECT_EQ(ok, viewer.update());
        EXPECT_EQ(0, property1->get<int32_t>().value());

        viewer.setCurrentView(2u);

        EXPECT_EQ(2u, viewer.getCurrentView());
        auto view2 = viewer.getView(2u);
        EXPECT_EQ("View2", view2.name());
        EXPECT_EQ("View2 with no inputs", view2.description());
        EXPECT_EQ(0u, view2.getInputCount());

        EXPECT_EQ(ok, viewer.update());
        EXPECT_EQ(1773, property1->get<int32_t>().value());
    }

    TEST_F(ALogicViewerLua, callByName)
    {
        const Result ok;
        auto* property1 = getInput<ramses::LuaInterface>("foo", "paramInt32");

        EXPECT_EQ(ok, loadLua(R"(
            function f1()
                rlogic.interfaces.foo.IN.paramInt32.value = -91
            end

            function f2()
                rlogic.interfaces.foo.IN.paramInt32.value = 908
            end
        )"));

        EXPECT_EQ(0, property1->get<int32_t>().value());
        EXPECT_EQ(ok, viewer.call("f1"));
        EXPECT_EQ(-91, property1->get<int32_t>().value());
        EXPECT_EQ(ok, viewer.call("f2"));
        EXPECT_EQ(908, property1->get<int32_t>().value());
        EXPECT_THAT(viewer.call("fNotExisting").getMessage(), testing::StartsWith("attempt to call a nil value"));
    }

    TEST_F(ALogicViewerLua, screenshot)
    {
        const Result ok;
        MockScreenshot mock;
        EXPECT_CALL(mock, screenshot("foo.png"));
        EXPECT_EQ(ok, loadLua(R"(
            rlogic.screenshot("foo.png")
        )"));
    }

    TEST_F(ALogicViewerLua, update)
    {
        EXPECT_TRUE(getInput<ramses::LuaInterface>("foo", "paramInt32")->set(static_cast<int32_t>(43)));
        EXPECT_EQ(0, getOutput<ramses::LuaScript>("foo", "paramInt32")->get<int32_t>());
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.update()
        )"));
        EXPECT_EQ(86, getOutput<ramses::LuaScript>("foo", "paramInt32")->get<int32_t>());
    }

    TEST_F(ALogicViewerLua, link_unlink)
    {
        auto* in = getInput<ramses::TimerNode>("foo", "ticker_us");
        auto* out = getOutput<ramses::LuaScript>("foo", "paramInt64");
        EXPECT_FALSE(in->isLinked());
        EXPECT_FALSE(in->hasIncomingLink());
        EXPECT_FALSE(in->hasOutgoingLink());
        EXPECT_FALSE(out->isLinked());
        EXPECT_FALSE(out->hasIncomingLink());
        EXPECT_FALSE(out->hasOutgoingLink());

        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.link(rlogic.scripts.foo.OUT.paramInt64, rlogic.timerNodes.foo.IN.ticker_us)
        )"));

        EXPECT_TRUE(in->isLinked());
        EXPECT_TRUE(in->hasIncomingLink());
        EXPECT_FALSE(in->hasOutgoingLink());
        EXPECT_TRUE(out->isLinked());
        EXPECT_FALSE(out->hasIncomingLink());
        EXPECT_TRUE(out->hasOutgoingLink());

        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.unlink(rlogic.scripts.foo.OUT.paramInt64, rlogic.timerNodes.foo.IN.ticker_us)
        )"));
        EXPECT_FALSE(in->isLinked());
        EXPECT_FALSE(in->hasIncomingLink());
        EXPECT_FALSE(in->hasOutgoingLink());
        EXPECT_FALSE(out->isLinked());
        EXPECT_FALSE(out->hasIncomingLink());
        EXPECT_FALSE(out->hasOutgoingLink());
    }

    TEST_F(ALogicViewerLua, nodeBindingByName)
    {
        auto* translation = getInput<ramses::RamsesNodeBinding>("foo", "translation");
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.nodeBindings.foo.IN.translation.value = {1,2,3}
        )"));
        EXPECT_FLOAT_EQ(1.f, translation->get<ramses::vec3f>().value()[0]);
        EXPECT_FLOAT_EQ(2.f, translation->get<ramses::vec3f>().value()[1]);
        EXPECT_FLOAT_EQ(3.f, translation->get<ramses::vec3f>().value()[2]);
    }

    TEST_F(ALogicViewerLua, appearanceBindingByName)
    {
        auto* floatUniform = getInput<ramses::RamsesAppearanceBinding>("foo", "floatUniform");
        //unlink input to avoid generating error for setting value for a linked input
        unlinkInput(*floatUniform);

        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.appearanceBindings.foo.IN.floatUniform.value = 9.1
        )"));
        EXPECT_FLOAT_EQ(9.1f, floatUniform->get<float>().value());
    }

    TEST_F(ALogicViewerLua, cameraBindingByName)
    {
        auto* frustum = getInput<ramses::RamsesCameraBinding>("foo", "frustum");
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.cameraBindings.foo.IN.frustum.nearPlane.value = 0.93
        )"));
        EXPECT_FLOAT_EQ(0.93f, frustum->getChild("nearPlane")->get<float>().value());
    }

    TEST_F(ALogicViewerLua, renderPassBindingByName)
    {
        auto* renderOrder = getInput<ramses::RamsesRenderPassBinding>("foo", "renderOrder");
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.renderPassBindings.foo.IN.renderOrder.value = 42
        )"));
        EXPECT_EQ(42, renderOrder->get<int32_t>().value());
    }

    TEST_F(ALogicViewerLua, renderGroupBindingByName)
    {
        auto* renderOrder = getInput<ramses::RamsesRenderGroupBinding>("rg", "renderOrders")->getChild("nestedRG");
        //unlink input to avoid generating error for setting value for a linked input
        unlinkInput(*renderOrder);

        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.renderGroupBindings.rg.IN.renderOrders.nestedRG.value = 42
        )"));
        EXPECT_EQ(42, renderOrder->get<int32_t>().value());
    }

    TEST_F(ALogicViewerLua, meshNodeBindingByName)
    {
        auto* vertexOffset = getInput<ramses::RamsesMeshNodeBinding>("mn", "vertexOffset");
        auto* indexOffset = getInput<ramses::RamsesMeshNodeBinding>("mn", "indexOffset");
        auto* indexCount = getInput<ramses::RamsesMeshNodeBinding>("mn", "indexCount");
        auto* instanceCount = getInput<ramses::RamsesMeshNodeBinding>("mn", "instanceCount");
        //unlink input to avoid generating error for setting value for a linked input
        unlinkInput(*vertexOffset);

        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.meshNodeBindings.mn.IN.vertexOffset.value = 42
            rlogic.meshNodeBindings.mn.IN.indexOffset.value = 43
            rlogic.meshNodeBindings.mn.IN.indexCount.value = 44
            rlogic.meshNodeBindings.mn.IN.instanceCount.value = 45
        )"));
        EXPECT_EQ(42, vertexOffset->get<int32_t>().value());
        EXPECT_EQ(43, indexOffset->get<int32_t>().value());
        EXPECT_EQ(44, indexCount->get<int32_t>().value());
        EXPECT_EQ(45, instanceCount->get<int32_t>().value());
    }

    TEST_F(ALogicViewerLua, timerNodeByName)
    {
        auto* ticker = getInput<ramses::TimerNode>("foo", "ticker_us");
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.timerNodes.foo.IN.ticker_us.value = 19083
        )"));
        EXPECT_EQ(19083, ticker->get<int64_t>().value());
    }

    TEST_F(ALogicViewerLua, animationNodeByName)
    {
        auto* progress = getInput<ramses::AnimationNode>("foo", "progress");
        //unlink input to avoid generating error for setting value for a linked input
        unlinkInput(*progress);

        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.animationNodes.foo.IN.progress.value = 198
        )"));
        EXPECT_FLOAT_EQ(198.f, progress->get<float>().value());
    }

    TEST_F(ALogicViewerLua, anchorPointByName)
    {
        // unlink anchor from script inputs so that we can overwrite values
        unlinkInput(*getInput<ramses::LuaScript>("foo", "anchorData1"));
        unlinkInput(*getInput<ramses::LuaScript>("foo", "anchorData2"));

        // first overwrite with some dummy values to 'reset' them
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.scripts.foo.IN.anchorData1.value = { 1, 2 }
            rlogic.scripts.foo.IN.anchorData2.value = 3
        )"));
        auto* outScript = getNode<ramses::LuaScript>("foo");
        EXPECT_FLOAT_EQ(1.f, GetOutput(outScript, "anchorData1")->get<vec2f>().value()[0]);
        EXPECT_FLOAT_EQ(2.f, GetOutput(outScript, "anchorData1")->get<vec2f>().value()[1]);
        EXPECT_FLOAT_EQ(3.f, GetOutput(outScript, "anchorData2")->get<float>().value());

        // now access values from anchor point and assign them to script inputs, overwriting the dummy values from before
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.scripts.foo.IN.anchorData1.value = { rlogic.anchorPoints.foo.OUT.viewportCoords.value[1], rlogic.anchorPoints.foo.OUT.viewportCoords.value[2] }
            rlogic.scripts.foo.IN.anchorData2.value = rlogic.anchorPoints.foo.OUT.depth.value
        )"));

        EXPECT_FLOAT_EQ(0.f, GetOutput(outScript, "anchorData1")->get<vec2f>().value()[0]);
        EXPECT_FLOAT_EQ(8.f, GetOutput(outScript, "anchorData1")->get<vec2f>().value()[1]);
        EXPECT_FLOAT_EQ(-0.01010102f, GetOutput(outScript, "anchorData2")->get<float>().value());
    }

    TEST_F(ALogicViewerLua, interfaceById)
    {
        auto* node = getNode<ramses::LuaInterface>("foo");
        ASSERT_EQ(1u, node->getId());
        EXPECT_EQ(Result(), loadLua(R"(rlogic.interfaces[1].IN.paramInt32.value = 42)"));
        EXPECT_EQ(42, GetInput(node, "paramInt32")->get<int32_t>().value());
    }

    TEST_F(ALogicViewerLua, scriptById)
    {
        auto* node = getNode<ramses::LuaScript>("foo");
        ASSERT_EQ(2u, node->getId());
        EXPECT_EQ(Result(), loadLua(R"(rlogic.scripts[2].IN.paramInt64.value = 99)"));
        EXPECT_EQ(99, GetInput(node, "paramInt64")->get<int64_t>().value());
    }

    TEST_F(ALogicViewerLua, nodeBindingById)
    {
        auto* node = getNode<ramses::RamsesNodeBinding>("foo");
        ASSERT_EQ(3u, node->getId());
        auto* translation = GetInput(node, "translation");
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.nodeBindings[3].IN.translation.value = {1,2,3}
        )"));
        EXPECT_FLOAT_EQ(1.f, translation->get<ramses::vec3f>().value()[0]);
        EXPECT_FLOAT_EQ(2.f, translation->get<ramses::vec3f>().value()[1]);
        EXPECT_FLOAT_EQ(3.f, translation->get<ramses::vec3f>().value()[2]);
    }

    TEST_F(ALogicViewerLua, appearanceBindingById)
    {
        auto* node = getNode<ramses::RamsesAppearanceBinding>("foo");
        ASSERT_EQ(4u, node->getId());
        auto* floatUniform = GetInput(node, "floatUniform");
        //unlink input to avoid generating error for setting value for a linked input
        unlinkInput(*floatUniform);

        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.appearanceBindings[4].IN.floatUniform.value = 9.1
        )"));
        EXPECT_FLOAT_EQ(9.1f, floatUniform->get<float>().value());
    }

    TEST_F(ALogicViewerLua, cameraBindingById)
    {
        auto* node = getNode<ramses::RamsesCameraBinding>("foo");
        ASSERT_EQ(5u, node->getId());
        auto* frustum = GetInput(node, "frustum");
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.cameraBindings[5].IN.frustum.nearPlane.value = 0.93
        )"));
        EXPECT_FLOAT_EQ(0.93f, frustum->getChild("nearPlane")->get<float>().value());
    }

    TEST_F(ALogicViewerLua, renderPassBindingById)
    {
        auto* rp = getNode<ramses::RamsesRenderPassBinding>("foo");
        ASSERT_EQ(6u, rp->getId());
        auto* renderOrder = GetInput(rp, "renderOrder");
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.renderPassBindings[6].IN.renderOrder.value = 42
        )"));
        EXPECT_EQ(42, renderOrder->get<int32_t>().value());
    }

    TEST_F(ALogicViewerLua, renderGroupBindingById)
    {
        auto* rg = getNode<ramses::RamsesRenderGroupBinding>("rg");
        ASSERT_EQ(9u, rg->getId());
        auto* renderOrder = GetInput(rg, "renderOrders")->getChild("nestedRG");
        //unlink input to avoid generating error for setting value for a linked input
        unlinkInput(*renderOrder);

        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.renderGroupBindings[9].IN.renderOrders.nestedRG.value = 42
        )"));
        EXPECT_EQ(42, renderOrder->get<int32_t>().value());
    }

    TEST_F(ALogicViewerLua, meshNodeBindingById)
    {
        auto* mn = getNode<ramses::RamsesMeshNodeBinding>("mn");
        ASSERT_EQ(10u, mn->getId());
        auto* vertexOffset = GetInput(mn, "vertexOffset");
        auto* indexOffset = GetInput(mn, "indexOffset");
        auto* indexCount = GetInput(mn, "indexCount");
        auto* instanceCount = GetInput(mn, "instanceCount");

        //unlink input to avoid generating error for setting value for a linked input
        unlinkInput(*vertexOffset);

        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.meshNodeBindings[10].IN.vertexOffset.value = 42
            rlogic.meshNodeBindings[10].IN.indexOffset.value = 43
            rlogic.meshNodeBindings[10].IN.indexCount.value = 44
            rlogic.meshNodeBindings[10].IN.instanceCount.value = 45
        )"));
        EXPECT_EQ(42, vertexOffset->get<int32_t>().value());
        EXPECT_EQ(43, indexOffset->get<int32_t>().value());
        EXPECT_EQ(44, indexCount->get<int32_t>().value());
        EXPECT_EQ(45, instanceCount->get<int32_t>().value());
    }

    TEST_F(ALogicViewerLua, timerNodeById)
    {
        auto* node = getNode<ramses::TimerNode>("foo");
        ASSERT_EQ(7u, node->getId());
        auto* ticker = GetInput(node, "ticker_us");
        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.timerNodes[7].IN.ticker_us.value = 19083
        )"));
        EXPECT_EQ(19083, ticker->get<int64_t>().value());
    }

    TEST_F(ALogicViewerLua, anchorPointById)
    {
        // unlink anchor from script inputs so that we can overwrite values
        unlinkInput(*getInput<ramses::LuaScript>("foo", "anchorData1"));
        unlinkInput(*getInput<ramses::LuaScript>("foo", "anchorData2"));

        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.scripts.foo.IN.anchorData1.value = { rlogic.anchorPoints[8].OUT.viewportCoords.value[1], rlogic.anchorPoints[8].OUT.viewportCoords.value[2] }
            rlogic.scripts.foo.IN.anchorData2.value = rlogic.anchorPoints[8].OUT.depth.value
        )"));

        auto* outScript = getNode<ramses::LuaScript>("foo");
        EXPECT_FLOAT_EQ(0.f, GetOutput(outScript, "anchorData1")->get<vec2f>().value()[0]);
        EXPECT_FLOAT_EQ(8.f, GetOutput(outScript, "anchorData1")->get<vec2f>().value()[1]);
        EXPECT_FLOAT_EQ(-0.01010102f, GetOutput(outScript, "anchorData2")->get<float>().value());
    }

    TEST_F(ALogicViewerLua, animationNodeById)
    {
        auto* node = getNode<ramses::AnimationNode>("foo");
        ASSERT_EQ(13u, node->getId());
        auto* progress = GetInput(node, "progress");
        //unlink input to avoid generating error for setting value for a linked input
        unlinkInput(*progress);

        EXPECT_EQ(Result(), loadLua(R"(
            rlogic.animationNodes[13].IN.progress.value = 198
        )"));
        EXPECT_FLOAT_EQ(198.f, progress->get<float>().value());
    }

    TEST_F(ALogicViewerLua, animationNodeWrongId)
    {
        auto* node = getNode<ramses::AnimationNode>("foo");
        ASSERT_EQ(13u, node->getId());
        EXPECT_THAT(loadLua(R"(
            rlogic.animationNodes[89032].IN.progress.value = 198
        )").getMessage(), testing::HasSubstr("attempt to index field '?' (a nil value)"));
    }

    TEST_F(ALogicViewerLua, iterateAnimationNodes)
    {
        auto* progress = getInput<ramses::AnimationNode>("foo", "progress");
        //unlink input to avoid generating error for setting value for a linked input
        unlinkInput(*progress);

        EXPECT_EQ(Result(), loadLua(R"(
            for node in rlogic.animationNodes() do
                node.IN.progress.value = 78.6
            end
        )"));
        EXPECT_FLOAT_EQ(78.6f, progress->get<float>().value());
    }

    TEST_F(ALogicViewerLua, reportsUserError)
    {
        EXPECT_THAT(loadLua(R"(
            error("custom lua exception")
        )").getMessage(), testing::HasSubstr("custom lua exception"));
    }

    TEST_F(ALogicViewerLua, reportsParseError)
    {
        EXPECT_THAT(loadLua(R"(
            foobar)
        )").getMessage(), testing::HasSubstr("'=' expected near"));
    }

    TEST_F(ALogicViewerLua, enableUpdateReport)
    {
        // set update interval to 1 to avoid random test failures
        // (only the longest update is reported for an interval)
        const size_t updateInterval = 1u; // in frames
        EXPECT_TRUE(viewer.loadRamsesLogic(logicFile, m_scene));
        EXPECT_FALSE(viewer.isUpdateReportEnabled());
        auto& summary = viewer.getUpdateReport();

        EXPECT_EQ(0u, summary.getTotalTime().maxValue.count());
        EXPECT_EQ(0u, summary.getSortTime().maxValue.count());
        EXPECT_EQ(0u, summary.getLinkActivations().maxValue);
        EXPECT_EQ(0u, summary.getNodesExecuted().size());
        EXPECT_EQ(0u, summary.getNodesSkippedExecution().size());
        viewer.enableUpdateReport(true, updateInterval);
        EXPECT_TRUE(viewer.isUpdateReportEnabled());

        EXPECT_EQ(Result(), viewer.update());

        EXPECT_EQ(2u, summary.getLinkActivations().maxValue);
        EXPECT_EQ(11u, summary.getNodesExecuted().size());
        EXPECT_EQ(0u, summary.getNodesSkippedExecution().size());
    }
}
