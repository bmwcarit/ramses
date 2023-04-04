//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gmock/gmock.h>

#include "ramses-logic/LuaScript.h"
#include "ramses-logic/LuaInterface.h"
#include "ramses-logic/Property.h"

#include "LogicEngineTest_Base.h"
#include "impl/LogicEngineImpl.h"
#include "impl/LogicNodeImpl.h"
#include "internals/ApiObjects.h"

#include "FeatureLevelTestValues.h"
#include "RamsesTestUtils.h"

namespace rlogic::internal
{
    class ALogicEngine_DirtinessBase : public ALogicEngineBase
    {
    public:
        explicit ALogicEngine_DirtinessBase(EFeatureLevel featureLevel = EFeatureLevel_01) : ALogicEngineBase{ featureLevel }
        {
        }

    protected:
        ApiObjects& m_apiObjects = { m_logicEngine.m_impl->getApiObjects() };

        const std::string_view m_minimal_script = R"(
            function interface(IN,OUT)
                IN.data = Type:Int32()
                OUT.data = Type:Int32()
            end
            function run(IN,OUT)
                OUT.data = IN.data
            end
        )";

        const std::string_view m_nested_properties_script = R"(
            function interface(IN,OUT)
                IN.data = {
                    nested = Type:Int32()
                }
                OUT.data = {
                    nested = Type:Int32()
                }
            end
            function run(IN,OUT)
                OUT.data.nested = IN.data.nested
            end
        )";

        const std::string_view m_valid_empty_interface = R"(
            function interface(IN,OUT)
            end
        )";

        const std::string_view m_minimal_interface = R"(
            function interface(IN)
                IN.data = Type:Int32()
            end
        )";
    };

    class ALogicEngine_Dirtiness : public ALogicEngine_DirtinessBase, public ::testing::TestWithParam<EFeatureLevel>
    {
    public:
        ALogicEngine_Dirtiness() : ALogicEngine_DirtinessBase{ GetParam() }
        {
        }
    };

    INSTANTIATE_TEST_SUITE_P(
        ALogicEngine_DirtinessTests,
        ALogicEngine_Dirtiness,
        GetFeatureLevelTestValues());

    TEST_P(ALogicEngine_Dirtiness, CreatedObjectsAreDirtyAfterCreating)
    {
        const auto obj1 = m_logicEngine.createLuaScript(m_valid_empty_script);
        EXPECT_TRUE(obj1->m_impl.isDirty());
        const auto obj2 = m_logicEngine.createLuaInterface(m_valid_empty_interface, "iface name");
        EXPECT_TRUE(obj2->m_impl.isDirty());
    }

    TEST_P(ALogicEngine_Dirtiness, CreatedBindingsAreNotDirtyAfterCreating)
    {
        const auto obj3 = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
        EXPECT_FALSE(obj3->m_impl.isDirty());
        const auto obj4 = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "");
        EXPECT_FALSE(obj4->m_impl.isDirty());

        if (GetParam() >= EFeatureLevel_02)
        {
            const auto obj5 = m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "");
            EXPECT_FALSE(obj5->m_impl.isDirty());
        }
        if (GetParam() >= EFeatureLevel_03)
        {
            const auto obj6 = createRenderGroupBinding();
            EXPECT_FALSE(obj6->m_impl.isDirty());
        }
        if (GetParam() >= EFeatureLevel_05)
        {
            const auto obj = m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode);
            EXPECT_FALSE(obj->m_impl.isDirty());
        }
    }

    TEST_P(ALogicEngine_Dirtiness, DirtyAfterCreatingNodeBinding_AndChangingInput)
    {
        RamsesNodeBinding* nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
        nodeBinding->getInputs()->getChild("scaling")->set(vec3f{1.5f, 1.f, 1.f});
        EXPECT_TRUE(nodeBinding->m_impl.isDirty());
    }

    TEST_P(ALogicEngine_Dirtiness, DirtyAfterCreatingAppearanceBinding_AndChangingInput)
    {
        RamsesAppearanceBinding* appBinding = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "");
        appBinding->getInputs()->getChild("floatUniform")->set(15.f);
        EXPECT_TRUE(appBinding->m_impl.isDirty());
    }

    TEST_P(ALogicEngine_Dirtiness, DirtyAfterCreatingCameraBinding_AndChangingInput)
    {
        RamsesCameraBinding* camBinding = m_logicEngine.createRamsesCameraBinding(*m_camera, "");
        camBinding->getInputs()->getChild("viewport")->getChild("width")->set<int32_t>(15);
        EXPECT_TRUE(camBinding->m_impl.isDirty());
    }

    TEST_P(ALogicEngine_Dirtiness, DirtyAfterCreatingRenderPassBinding_AndChangingInput)
    {
        if (GetParam() < EFeatureLevel_02)
            GTEST_SKIP();

        RamsesRenderPassBinding* binding = m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "");
        binding->getInputs()->getChild("enabled")->set(false);
        EXPECT_TRUE(binding->m_impl.isDirty());
    }

    TEST_P(ALogicEngine_Dirtiness, DirtyAfterCreatingRenderGroupBinding_AndChangingInput)
    {
        if (GetParam() < EFeatureLevel_03)
            GTEST_SKIP();

        auto binding = createRenderGroupBinding();
        binding->getInputs()->getChild("renderOrders")->getChild("mesh")->set(42);
        EXPECT_TRUE(binding->m_impl.isDirty());
    }

    TEST_P(ALogicEngine_Dirtiness, DirtyAfterCreatingMeshNodeBinding_AndChangingInput)
    {
        if (GetParam() < EFeatureLevel_05)
            GTEST_SKIP();

        auto binding = m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode);
        binding->getInputs()->getChild("vertexOffset")->set(42);
        EXPECT_TRUE(binding->m_impl.isDirty());
    }

    TEST_P(ALogicEngine_Dirtiness, NotDirty_AfterCreatingObjectsAndCallingUpdate)
    {
        const auto obj1 = m_logicEngine.createLuaScript(m_valid_empty_script);
        const auto obj2 = m_logicEngine.createLuaInterface(m_valid_empty_interface, "iface name");
        const auto obj3 = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
        const auto obj4 = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "");
        const auto obj5 = m_logicEngine.createRamsesCameraBinding(*m_camera, "");
        const RamsesRenderPassBinding* obj6 = nullptr;
        const RamsesRenderGroupBinding* obj7 = nullptr;
        const RamsesMeshNodeBinding* obj8 = nullptr;
        if (GetParam() >= EFeatureLevel_02)
            obj6 = m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "");
        if (GetParam() >= EFeatureLevel_03)
            obj7 = createRenderGroupBinding();
        if (GetParam() >= EFeatureLevel_05)
            obj8 = m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode);

        m_logicEngine.update();
        EXPECT_FALSE(obj1->m_impl.isDirty());
        EXPECT_FALSE(obj2->m_impl.isDirty());
        EXPECT_FALSE(obj3->m_impl.isDirty());
        EXPECT_FALSE(obj4->m_impl.isDirty());
        EXPECT_FALSE(obj5->m_impl.isDirty());
        if (GetParam() >= EFeatureLevel_02)
        {
            EXPECT_FALSE(obj6->m_impl.isDirty());
        }
        if (GetParam() >= EFeatureLevel_03)
        {
            EXPECT_FALSE(obj7->m_impl.isDirty());
        }
        if (GetParam() >= EFeatureLevel_05)
        {
            EXPECT_FALSE(obj8->m_impl.isDirty());
        }
    }

    TEST_P(ALogicEngine_Dirtiness, Dirty_AfterSettingScriptInput)
    {
        LuaScript* script = m_logicEngine.createLuaScript(m_minimal_script);
        m_logicEngine.update();

        script->getInputs()->getChild("data")->set<int32_t>(5);

        EXPECT_TRUE(script->m_impl.isDirty());
        m_logicEngine.update();
        EXPECT_FALSE(script->m_impl.isDirty());
    }

    TEST_P(ALogicEngine_Dirtiness, Dirty_AfterSettingNestedScriptInput)
    {
        LuaScript* script = m_logicEngine.createLuaScript(m_nested_properties_script);
        m_logicEngine.update();

        script->getInputs()->getChild("data")->getChild("nested")->set<int32_t>(5);

        EXPECT_TRUE(script->m_impl.isDirty());
        m_logicEngine.update();
        EXPECT_FALSE(script->m_impl.isDirty());
    }

    TEST_P(ALogicEngine_Dirtiness, Dirty_AfterSettingInterfaceInput)
    {
        LuaInterface* intf = m_logicEngine.createLuaInterface(m_minimal_interface, "iface name");
        m_logicEngine.update();

        intf->getInputs()->getChild("data")->set<int32_t>(5);

        EXPECT_TRUE(intf->m_impl.isDirty());
        m_logicEngine.update();
        EXPECT_FALSE(intf->m_impl.isDirty());
    }

    TEST_P(ALogicEngine_Dirtiness, Dirty_WhenSettingBindingInputToDefaultValue)
    {
        RamsesNodeBinding* binding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
        m_logicEngine.update();

        // zeroes is the default value
        binding->getInputs()->getChild("translation")->set<vec3f>({0, 0, 0});
        EXPECT_TRUE(binding->m_impl.isDirty());
        m_logicEngine.update();

        // Set different value, and then set again
        binding->getInputs()->getChild("translation")->set<vec3f>({1, 2, 3});
        m_logicEngine.update();
        binding->getInputs()->getChild("translation")->set<vec3f>({1, 2, 3});
        EXPECT_TRUE(binding->m_impl.isDirty());
    }

    TEST_P(ALogicEngine_Dirtiness, Dirty_WhenSettingBindingInputToDifferentValue)
    {
        RamsesNodeBinding* binding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
        m_logicEngine.update();

        // Set non-default value, and then set again to different value
        binding->getInputs()->getChild("translation")->set<vec3f>({ 1, 2, 3 });
        m_logicEngine.update();
        EXPECT_FALSE(binding->m_impl.isDirty());
        binding->getInputs()->getChild("translation")->set<vec3f>({ 11, 12, 13 });
        EXPECT_TRUE(binding->m_impl.isDirty());
    }

    class ALogicEngine_DirtinessViaLink : public ALogicEngine_DirtinessBase, public ::testing::TestWithParam<bool>
    {
    protected:
        void link(const Property& src, const Property& dst)
        {
            if (GetParam())
            {
                EXPECT_TRUE(m_logicEngine.linkWeak(src, dst));
            }
            else
            {
                EXPECT_TRUE(m_logicEngine.link(src, dst));
            }
        }
    };

    INSTANTIATE_TEST_SUITE_P(
        ALogicEngine_DirtinessViaLink_TestInstances,
        ALogicEngine_DirtinessViaLink,
        ::testing::Values(false, true));

    TEST_P(ALogicEngine_DirtinessViaLink, Dirty_WhenAddingLink)
    {
        LuaScript* script1 = m_logicEngine.createLuaScript(m_minimal_script);
        LuaScript* script2 = m_logicEngine.createLuaScript(m_minimal_script);
        m_logicEngine.update();

        link(*script1->getOutputs()->getChild("data"), *script2->getInputs()->getChild("data"));
        EXPECT_TRUE(script1->m_impl.isDirty());
        EXPECT_TRUE(script2->m_impl.isDirty());
        m_logicEngine.update();
        EXPECT_FALSE(script1->m_impl.isDirty());
        EXPECT_FALSE(script2->m_impl.isDirty());
    }

    TEST_P(ALogicEngine_DirtinessViaLink, NotDirty_WhenRemovingLink)
    {
        LuaScript* script1 = m_logicEngine.createLuaScript(m_minimal_script);
        LuaScript* script2 = m_logicEngine.createLuaScript(m_minimal_script);
        link(*script1->getOutputs()->getChild("data"), *script2->getInputs()->getChild("data"));
        m_logicEngine.update();

        EXPECT_FALSE(script1->m_impl.isDirty());
        EXPECT_FALSE(script2->m_impl.isDirty());
        m_logicEngine.unlink(*script1->getOutputs()->getChild("data"), *script2->getInputs()->getChild("data"));

        EXPECT_FALSE(script1->m_impl.isDirty());
        EXPECT_FALSE(script2->m_impl.isDirty());
        m_logicEngine.update();
        EXPECT_FALSE(script1->m_impl.isDirty());
        EXPECT_FALSE(script2->m_impl.isDirty());
    }

    TEST_P(ALogicEngine_DirtinessViaLink, NotDirty_WhenRemovingNestedLink)
    {
        LuaScript* script1 = m_logicEngine.createLuaScript(m_nested_properties_script);
        LuaScript* script2 = m_logicEngine.createLuaScript(m_nested_properties_script);
        link(*script1->getOutputs()->getChild("data")->getChild("nested"), *script2->getInputs()->getChild("data")->getChild("nested"));
        m_logicEngine.update();

        EXPECT_FALSE(script1->m_impl.isDirty());
        EXPECT_FALSE(script2->m_impl.isDirty());
        m_logicEngine.unlink(*script1->getOutputs()->getChild("data")->getChild("nested"), *script2->getInputs()->getChild("data")->getChild("nested"));

        EXPECT_FALSE(script1->m_impl.isDirty());
        EXPECT_FALSE(script2->m_impl.isDirty());
        m_logicEngine.update();
        EXPECT_FALSE(script1->m_impl.isDirty());
        EXPECT_FALSE(script2->m_impl.isDirty());
    }

    // Removing link does not mark things dirty, but setting value does
    TEST_P(ALogicEngine_DirtinessViaLink, Dirty_WhenRemovingLink_AndSettingValueByCallingSetAfterwards)
    {
        LuaScript* script1 = m_logicEngine.createLuaScript(m_nested_properties_script);
        LuaScript* script2 = m_logicEngine.createLuaScript(m_nested_properties_script);
        m_logicEngine.update();

        link(*script1->getOutputs()->getChild("data")->getChild("nested"), *script2->getInputs()->getChild("data")->getChild("nested"));
        m_logicEngine.update();
        EXPECT_FALSE(script1->m_impl.isDirty());
        EXPECT_FALSE(script2->m_impl.isDirty());

        m_logicEngine.unlink(*script1->getOutputs()->getChild("data")->getChild("nested"), *script2->getInputs()->getChild("data")->getChild("nested"));
        script2->getInputs()->getChild("data")->getChild("nested")->set<int32_t>(5);
        EXPECT_FALSE(script1->m_impl.isDirty());
        EXPECT_TRUE(script2->m_impl.isDirty());
    }

    TEST_P(ALogicEngine_DirtinessViaLink, Dirty_WhenAddingLinkToInterfaceInput)
    {
        LuaScript* script = m_logicEngine.createLuaScript(m_minimal_script);
        LuaInterface* intf = m_logicEngine.createLuaInterface(m_minimal_interface, "iface name");
        m_logicEngine.update();

        link(*script->getOutputs()->getChild("data"), *intf->getInputs()->getChild("data"));
        EXPECT_TRUE(script->m_impl.isDirty());
        EXPECT_TRUE(intf->m_impl.isDirty());
        m_logicEngine.update();
        EXPECT_FALSE(script->m_impl.isDirty());
        EXPECT_FALSE(intf->m_impl.isDirty());
    }

    TEST_P(ALogicEngine_DirtinessViaLink, Dirty_WhenAddingLinkToInterfaceOutput)
    {
        LuaScript* script = m_logicEngine.createLuaScript(m_minimal_script);
        LuaInterface* intf = m_logicEngine.createLuaInterface(m_minimal_interface, "iface name");
        m_logicEngine.update();

        link(*intf->getOutputs()->getChild("data"), *script->getInputs()->getChild("data"));
        EXPECT_TRUE(script->m_impl.isDirty());
        EXPECT_TRUE(intf->m_impl.isDirty());
        m_logicEngine.update();
        EXPECT_FALSE(script->m_impl.isDirty());
        EXPECT_FALSE(intf->m_impl.isDirty());
    }

    TEST_P(ALogicEngine_DirtinessViaLink, NotDirty_WhenRemovingLinkToInterface)
    {
        LuaScript* script = m_logicEngine.createLuaScript(m_minimal_script);
        LuaInterface* intf = m_logicEngine.createLuaInterface(m_minimal_interface, "iface name");
        link(*script->getOutputs()->getChild("data"), *intf->getInputs()->getChild("data"));
        m_logicEngine.update();

        EXPECT_FALSE(script->m_impl.isDirty());
        EXPECT_FALSE(intf->m_impl.isDirty());
        m_logicEngine.unlink(*script->getOutputs()->getChild("data"), *intf->getInputs()->getChild("data"));

        EXPECT_FALSE(script->m_impl.isDirty());
        EXPECT_FALSE(intf->m_impl.isDirty());
        m_logicEngine.update();
        EXPECT_FALSE(script->m_impl.isDirty());
        EXPECT_FALSE(intf->m_impl.isDirty());
    }

    TEST_P(ALogicEngine_Dirtiness, Dirty_WhenScriptHadRuntimeError)
    {
        const std::string_view scriptWithError = R"(
            function interface(IN,OUT)
            end
            function run(IN,OUT)
                error("Snag!")
            end
        )";

        const auto script = m_logicEngine.createLuaScript(scriptWithError);
        EXPECT_FALSE(m_logicEngine.update());

        EXPECT_TRUE(script->m_impl.isDirty());
    }

    // This is a bit of a special case, but an important one. If script A provides a value for script B and script A has an error
    // AFTER it set a new value to B, then script B will be dirty (and not executed!) until the error in script A was fixed
    TEST_P(ALogicEngine_Dirtiness, KeepsDirtynessStateOfDependentScript_UntilErrorInSourceScriptIsFixed)
    {
        const std::string_view scriptWithFixableError = R"(
            function interface(IN,OUT)
                IN.triggerError = Type:Bool()
                IN.data = Type:Int32()
                OUT.data = Type:Int32()
            end
            function run(IN,OUT)
                OUT.data = IN.data
                if IN.triggerError then
                    error("Snag!")
                end
            end
        )";

        LuaScript* script1 = m_logicEngine.createLuaScript(scriptWithFixableError);
        LuaScript* script2 = m_logicEngine.createLuaScript(m_minimal_script);

        // No error -> have normal run -> nothing is dirty
        script1->getInputs()->getChild("triggerError")->set<bool>(false);
        m_logicEngine.link(*script1->getOutputs()->getChild("data"), *script2->getInputs()->getChild("data"));
        m_logicEngine.update();
        EXPECT_FALSE(script1->m_impl.isDirty());
        EXPECT_FALSE(script2->m_impl.isDirty());

        // Trigger error -> keep in dirty state
        script1->getInputs()->getChild("triggerError")->set<bool>(true);
        EXPECT_FALSE(m_logicEngine.update());
        EXPECT_FALSE(m_logicEngine.update());
        EXPECT_TRUE(script1->m_impl.isDirty());
        EXPECT_FALSE(script2->m_impl.isDirty());
        // "fix" the error and set a value -> expect nothing is dirty and value was propagated
        script1->getInputs()->getChild("triggerError")->set<bool>(false);
        script1->getInputs()->getChild("data")->set<int32_t>(15);

        m_logicEngine.update();
        EXPECT_FALSE(script1->m_impl.isDirty());
        EXPECT_FALSE(script2->m_impl.isDirty());
        EXPECT_EQ(15, *script2->getOutputs()->getChild("data")->get<int32_t>());
    }

    class ALogicEngine_BindingDirtiness : public ALogicEngine_Dirtiness
    {
    protected:
        const std::string_view m_bindningDataScript = R"(
            function interface(IN,OUT)
                OUT.vec3f = Type:Vec3f()
            end
            function run(IN,OUT)
                OUT.vec3f = {1, 2, 3}
            end
        )";
    };

    INSTANTIATE_TEST_SUITE_P(
        ALogicEngine_BindingDirtinessTests,
        ALogicEngine_BindingDirtiness,
        GetFeatureLevelTestValues());

    TEST_P(ALogicEngine_BindingDirtiness, NotDirtyAfterConstruction)
    {
        EXPECT_FALSE(m_apiObjects.bindingsDirty());
    }

    TEST_P(ALogicEngine_BindingDirtiness, NotDirtyAfterCreatingScript)
    {
        m_logicEngine.createLuaScript(m_valid_empty_script);
        EXPECT_FALSE(m_apiObjects.bindingsDirty());
    }

    TEST_P(ALogicEngine_BindingDirtiness, NotDirtyAfterCreatingNodeBinding)
    {
        m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
        EXPECT_FALSE(m_apiObjects.bindingsDirty());
    }

    TEST_P(ALogicEngine_BindingDirtiness, NotDirtyAfterCreatingAppearanceBinding)
    {
        m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "");
        EXPECT_FALSE(m_apiObjects.bindingsDirty());
    }

    TEST_P(ALogicEngine_BindingDirtiness, Dirty_WhenSettingBindingInputToDefaultValue)
    {
        RamsesNodeBinding* binding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
        m_logicEngine.update();

        // zeroes is the default value
        binding->getInputs()->getChild("translation")->set<vec3f>({ 0, 0, 0 });
        EXPECT_TRUE(m_apiObjects.bindingsDirty());
        m_logicEngine.update();

        // Set different value, and then set again
        binding->getInputs()->getChild("translation")->set<vec3f>({ 1, 2, 3 });
        m_logicEngine.update();
        binding->getInputs()->getChild("translation")->set<vec3f>({ 1, 2, 3 });
        EXPECT_TRUE(m_apiObjects.bindingsDirty());
    }

    TEST_P(ALogicEngine_BindingDirtiness, Dirty_WhenSettingBindingInputToDifferentValue)
    {
        RamsesNodeBinding* binding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
        m_logicEngine.update();

        // Set non-default value, and then set again to different value
        binding->getInputs()->getChild("translation")->set<vec3f>({ 1, 2, 3 });
        m_logicEngine.update();
        EXPECT_FALSE(m_apiObjects.bindingsDirty());
        binding->getInputs()->getChild("translation")->set<vec3f>({ 11, 12, 13 });
        EXPECT_TRUE(m_apiObjects.bindingsDirty());
    }

    TEST_P(ALogicEngine_BindingDirtiness, Dirty_WhenAddingLink)
    {
        LuaScript* script = m_logicEngine.createLuaScript(m_bindningDataScript);
        RamsesNodeBinding* binding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
        m_logicEngine.update();

        m_logicEngine.link(*script->getOutputs()->getChild("vec3f"), *binding->getInputs()->getChild("rotation"));
        EXPECT_TRUE(m_apiObjects.bindingsDirty());

        // After update - not dirty
        m_logicEngine.update();
        EXPECT_FALSE(m_apiObjects.bindingsDirty());
    }

    TEST_P(ALogicEngine_BindingDirtiness, NotDirty_WhenRemovingLink)
    {
        LuaScript* script = m_logicEngine.createLuaScript(m_bindningDataScript);
        RamsesNodeBinding* binding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
        m_logicEngine.link(*script->getOutputs()->getChild("vec3f"), *binding->getInputs()->getChild("rotation"));
        m_logicEngine.update();

        EXPECT_FALSE(script->m_impl.isDirty());
        EXPECT_FALSE(binding->m_impl.isDirty());
        m_logicEngine.unlink(*script->getOutputs()->getChild("vec3f"), *binding->getInputs()->getChild("rotation"));

        EXPECT_FALSE(script->m_impl.isDirty());
        EXPECT_FALSE(binding->m_impl.isDirty());
        m_logicEngine.update();
        EXPECT_FALSE(script->m_impl.isDirty());
        EXPECT_FALSE(binding->m_impl.isDirty());
    }

    // Special case, but worth testing as we want that bindings are always
    // executed when adding link, even if the link was just "re-added"
    TEST_P(ALogicEngine_BindingDirtiness, Dirty_WhenReAddingLink)
    {
        LuaScript* script = m_logicEngine.createLuaScript(m_bindningDataScript);
        RamsesNodeBinding* binding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "");
        ASSERT_TRUE(m_logicEngine.link(*script->getOutputs()->getChild("vec3f"), *binding->getInputs()->getChild("rotation")));
        m_logicEngine.update();

        EXPECT_FALSE(script->m_impl.isDirty());
        EXPECT_FALSE(binding->m_impl.isDirty());
        m_logicEngine.unlink(*script->getOutputs()->getChild("vec3f"), *binding->getInputs()->getChild("rotation"));
        m_logicEngine.link(*script->getOutputs()->getChild("vec3f"), *binding->getInputs()->getChild("rotation"));

        EXPECT_TRUE(script->m_impl.isDirty());
        EXPECT_TRUE(binding->m_impl.isDirty());
        m_logicEngine.update();
        EXPECT_FALSE(script->m_impl.isDirty());
        EXPECT_FALSE(binding->m_impl.isDirty());
    }

    TEST_P(ALogicEngine_BindingDirtiness, Dirty_WhenSettingDataToNestedAppearanceBindingInputs)
    {
        // Vertex shader with array -> results in nested binding inputs
        const std::string_view vertShader_array = R"(
            #version 300 es

            uniform highp vec4  vec4Array[2];

            void main()
            {
                gl_Position = vec4Array[1];
            })";

        const std::string_view fragShader_trivial = R"(
            #version 300 es

            out lowp vec4 color;
            void main(void)
            {
                color = vec4(1.0, 0.0, 0.0, 1.0);
            })";

        RamsesAppearanceBinding* binding = m_logicEngine.createRamsesAppearanceBinding(RamsesTestSetup::CreateTestAppearance(*m_scene, vertShader_array, fragShader_trivial), "");

        m_logicEngine.update();
        EXPECT_FALSE(m_apiObjects.bindingsDirty());

        EXPECT_TRUE(binding->getInputs()->getChild("vec4Array")->getChild(0)->set<vec4f>({ .1f, .2f, .3f, .4f }));
        EXPECT_TRUE(m_apiObjects.bindingsDirty());

        m_logicEngine.update();
        EXPECT_FALSE(m_apiObjects.bindingsDirty());
    }
}

