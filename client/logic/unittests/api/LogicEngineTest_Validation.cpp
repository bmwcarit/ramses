//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"

#include "ramses-logic/Logger.h"
#include "ramses-logic/Property.h"
#include "ramses-logic/ELogMessageType.h"
#include "ramses-logic/AnimationNodeConfig.h"

#include "impl/LoggerImpl.h"

#include "LogTestUtils.h"
#include "WithTempDirectory.h"
#include "FeatureLevelTestValues.h"
#include "RamsesTestUtils.h"

namespace rlogic
{
    class ALogicEngine_Validation : public ALogicEngine
    {
    protected:
        explicit ALogicEngine_Validation(EFeatureLevel featureLevel = EFeatureLevel_01)
            : ALogicEngine{ featureLevel }
        {
        }
    };

    TEST_F(ALogicEngine_Validation, LogsNoWarningsWhenSavingFile_WhenContentValid)
    {
        ScopedLogContextLevel logCollector{ ELogMessageType::Trace, [&](ELogMessageType type, std::string_view /*message*/)
            {
                if (type == ELogMessageType::Warn)
                {
                    FAIL() << "Should have no warnings!";
                }
            }
        };

        WithTempDirectory tmpDir;
        // Empty logic engine has ho warnings
        m_logicEngine.saveToFile("noWarnings.rlogic");
    }

    TEST_F(ALogicEngine_Validation, LogsWarningsWhenSavingFile_WhenContentHasValidationIssues)
    {
        std::vector<std::string> warnings;
        ScopedLogContextLevel logCollector{ ELogMessageType::Trace, [&](ELogMessageType type, std::string_view message)
            {
                if (type == ELogMessageType::Warn)
                {
                    warnings.emplace_back(message);
                }
        }
        };

        WithTempDirectory tmpDir;
        // Cause some validation issues on purpose
        RamsesNodeBinding& nodeBinding = *m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "NodeBinding");
        nodeBinding.getInputs()->getChild("scaling")->set<vec3f>({1.5f, 1.f, 1.f});

        m_logicEngine.saveToFile("noWarnings.rlogic");

        ASSERT_EQ(warnings.size(), 2u);
        EXPECT_EQ(warnings[0], "Saving logic engine content with manually updated binding values without calling update() will result in those values being lost!");
        EXPECT_EQ(warnings[1], "[NodeBinding [Id=1]] Node [NodeBinding] has no ingoing links! Node should be deleted or properly linked!");

        // Fixing the problems -> removes the warning
        warnings.clear();
        const auto* intf = m_logicEngine.createLuaInterface(m_interfaceSourceCode, "intf");
        m_logicEngine.link(*intf->getOutputs()->getChild("param_vec3f"), *nodeBinding.getInputs()->getChild("scaling"));
        m_logicEngine.update();

        m_logicEngine.saveToFile("noWarnings.rlogic");

        EXPECT_TRUE(warnings.empty());
    }

    TEST_F(ALogicEngine_Validation, LogsNoContentWarningsWhenSavingFile_WhenContentHasValidationIssues_ButValidationIsDisabled)
    {
        std::vector<std::string> infoLogs;
        ScopedLogContextLevel logCollector{ ELogMessageType::Trace, [&](ELogMessageType type, std::string_view message)
            {
                if (type == ELogMessageType::Info)
                {
                    infoLogs.emplace_back(message);
                }
                else
                {
                    FAIL() << "Unexpected log!";
                }
        }
        };

        WithTempDirectory tmpDir;
        // Cause some validation issues on purpose
        RamsesNodeBinding& nodeBinding = *m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "NodeBinding");
        nodeBinding.getInputs()->getChild("scaling")->set<vec3f>({ 1.5f, 1.f, 1.f });

        SaveFileConfig conf;
        conf.setValidationEnabled(false);

        // Disabling the validation causes a warning
        ASSERT_EQ(infoLogs.size(), 1u);
        EXPECT_EQ(infoLogs[0], "Validation before saving was disabled during save*() calls! Possible content issues will not yield further warnings.");
        infoLogs.clear();

        // Content warning doesn't show up because disabled
        m_logicEngine.saveToFile("noWarnings.rlogic", conf);
    }

    TEST_F(ALogicEngine_Validation, ProducesWarningIfBindingValuesHaveDirtyValue)
    {
        // Create a binding in a "dirty" state - has a non-default value, but update() wasn't called and didn't pass the value to ramses
        RamsesNodeBinding* nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "binding");

        const auto* intf = m_logicEngine.createLuaInterface(m_interfaceSourceCode, "intf");
        m_logicEngine.link(*intf->getOutputs()->getChild("param_vec3f"), *nodeBinding->getInputs()->getChild("scaling"));

        nodeBinding->getInputs()->getChild("visibility")->set<bool>(false);

        // Expects warning
        auto warnings = m_logicEngine.validate();

        ASSERT_EQ(1u, warnings.size());
        EXPECT_EQ(warnings[0].message, "Saving logic engine content with manually updated binding values without calling update() will result in those values being lost!");
        EXPECT_EQ(warnings[0].type, EWarningType::UnsafeDataState);
    }

    TEST_F(ALogicEngine_Validation, ProducesWarningIfInterfaceHasUnboundOutputs)
    {
        LuaInterface* intf = m_logicEngine.createLuaInterface(R"(
            function interface(IN,OUT)

                IN.param1 = Type:Int32()
                IN.param2 = {a=Type:Float(), b=Type:Int32()}

            end
        )", "intf name");
        ASSERT_NE(nullptr, intf);

        // Expects warning
        auto warnings = m_logicEngine.validate();

        ASSERT_EQ(3u, warnings.size());
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&WarningData::message, ::testing::HasSubstr("Interface [intf name] has unlinked output"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&WarningData::type, ::testing::Eq(EWarningType::UnusedContent))));
    }

    class ALogicEngine_ValidatingDanglingNodes : public ALogicEngineBase, public ::testing::TestWithParam<EFeatureLevel>
    {
    protected:
        explicit ALogicEngine_ValidatingDanglingNodes()
            : ALogicEngineBase(GetParam())
        {
            createValidTestSetup();
        }

        void createValidTestSetup()
        {
            //create two identical scripts each containing 1 input and 1 output
            m_testScript = m_logicEngine.createLuaScript(m_scriptSrc, {}, "script1");
            auto* script2 = m_logicEngine.createLuaScript(m_scriptSrc, {}, "script2");

            //cross link inputs and outputs of each script so that none of the scripts is "dangling"
            m_logicEngine.link(*m_testScript->getOutputs()->getChild("paramInt32"), *script2->getInputs()->getChild("paramTestReserved"));
            m_logicEngine.linkWeak(*script2->getOutputs()->getChild("paramInt32"), *m_testScript->getInputs()->getChild("paramTestReserved"));
        }

        void linkNodeInput(LogicNode& node, std::string_view inputName, std::string_view testScriptOutputName = "paramInt32")
        {
            linkNodeInput(*node.getInputs()->getChild(inputName), testScriptOutputName);
        }

        void linkNodeInput(Property& nodeInput, std::string_view testScriptOutputName = "paramInt32")
        {
            m_logicEngine.link(*m_testScript->getOutputs()->getChild(testScriptOutputName), nodeInput);
        }

        void linkNodeOutput(LogicNode& node, std::string_view outputName, std::string_view testScriptInputName = "paramInt32")
        {
            m_logicEngine.link(*node.getOutputs()->getChild(outputName), *m_testScript->getInputs()->getChild(testScriptInputName));
        }

        const std::string m_scriptSrc = R"SRC(
                function interface(IN,OUT)
                    IN.paramTestReserved = Type:Int32()

                    IN.paramInt32 = Type:Int32()
                    IN.paramInt64 = Type:Int64()
                    IN.paramFloat = Type:Float()

                    OUT.paramInt32 = Type:Int32()
                    OUT.paramFloat = Type:Float()
                    OUT.paramVec3f = Type:Vec3f()
                end
                function run(IN,OUT)
                end
                )SRC";

        LuaScript* m_testScript = nullptr;
    };

    INSTANTIATE_TEST_SUITE_P(
        ALogicEngine_ValidatingDanglingNodesTests,
        ALogicEngine_ValidatingDanglingNodes,
        rlogic::internal::GetFeatureLevelTestValues());

    // -- scripts -- //

    TEST_P(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfScriptHasNoLinks)
    {
        auto* script = m_logicEngine.createLuaScript(m_scriptSrc, {}, "scr");
        ASSERT_NE(nullptr, script);

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(2u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&WarningData::message, ::testing::StrEq("Node [scr] has no outgoing links! Node should be deleted or properly linked!")),
                                                    ::testing::Field(&WarningData::message, ::testing::StrEq("Node [scr] has no ingoing links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&WarningData::type, ::testing::Eq(EWarningType::UnusedContent))));
    }

    TEST_P(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfScriptHasNoIngoingLinks)
    {
        auto* script = m_logicEngine.createLuaScript(m_scriptSrc, {}, "scr");
        ASSERT_NE(nullptr, script);

        linkNodeOutput(*script, "paramInt32");

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&WarningData::message, ::testing::StrEq("Node [scr] has no ingoing links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&WarningData::type, ::testing::Eq(EWarningType::UnusedContent))));
    }

    TEST_P(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfScriptHasNoOutgoingLinks)
    {
        auto* script = m_logicEngine.createLuaScript(m_scriptSrc, {}, "scr");
        ASSERT_NE(nullptr, script);

        linkNodeInput(*script, "paramInt32");

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&WarningData::message, ::testing::StrEq("Node [scr] has no outgoing links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&WarningData::type, ::testing::Eq(EWarningType::UnusedContent))));
    }

    TEST_P(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfScriptHasIngoingAndOutgoingLinks)
    {
        auto* script = m_logicEngine.createLuaScript(m_scriptSrc, {}, "scr");
        ASSERT_NE(nullptr, script);

        linkNodeOutput(*script, "paramInt32");
        linkNodeInput(*script, "paramInt32");

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(0u, warnings.size());
    }

    // -- interfaces -- //

    TEST_P(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfInterfaceHasNoIngoingLinks)
    {
        LuaInterface* intf = m_logicEngine.createLuaInterface(R"(
            function interface(INOUT)
                INOUT.param = Type:Int32()
            end
        )", "intf name");
        ASSERT_NE(nullptr, intf);

        linkNodeOutput(*intf, "param");

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(0u, warnings.size());
    }

    // -- node bindings -- //

    TEST_P(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfNodeBindingHasNoIngoingLinks)
    {
        auto* nodeBind = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "binding");
        ASSERT_NE(nullptr, nodeBind);

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&WarningData::message, ::testing::StrEq("Node [binding] has no ingoing links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&WarningData::type, ::testing::Eq(EWarningType::UnusedContent))));
    }

    TEST_P(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfNodeBindingHasIngoingLinks)
    {
        auto* nodeBind = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "binding");
        ASSERT_NE(nullptr, nodeBind);

        linkNodeInput(*nodeBind, "scaling", "paramVec3f");
        m_logicEngine.update();

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(0u, warnings.size());
    }

    // -- appearance bindings -- //

    TEST_P(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfAppearanceBindingHasNoIngoingLinks)
    {
        auto* appearanceBind = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "binding");
        ASSERT_NE(nullptr, appearanceBind);

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&WarningData::message, ::testing::StrEq("Node [binding] has no ingoing links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&WarningData::type, ::testing::Eq(EWarningType::UnusedContent))));
    }

    TEST_P(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfAppearanceBindingHasIngoingLinks)
    {
        auto* appearanceBind = m_logicEngine.createRamsesAppearanceBinding(*m_appearance, "binding");
        ASSERT_NE(nullptr, appearanceBind);

        linkNodeInput(*appearanceBind, "floatUniform", "paramFloat");
        m_logicEngine.update();

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(0u, warnings.size());
    }

    // -- camera bindings -- //

    TEST_P(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfCameraBindingHasNoIngoingLinks)
    {
        auto* cameraBind = m_logicEngine.createRamsesCameraBinding(*m_camera, "binding");
        ASSERT_NE(nullptr, cameraBind);

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&WarningData::message, ::testing::StrEq("Node [binding] has no ingoing links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&WarningData::type, ::testing::Eq(EWarningType::UnusedContent))));
    }

    TEST_P(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfCameraBindingHasIngoingLinks)
    {
        auto* cameraBind = m_logicEngine.createRamsesCameraBinding(*m_camera, "binding");
        ASSERT_NE(nullptr, cameraBind);

        m_logicEngine.link(*m_testScript->getOutputs()->getChild("paramInt32"), *cameraBind->getInputs()->getChild("viewport")->getChild("offsetX"));
        m_logicEngine.update();

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(0u, warnings.size());
    }

    // -- render pass bindings -- //

    TEST_P(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfRenderPassBindingHasNoIngoingLinks)
    {
        if (m_logicEngine.getFeatureLevel() < EFeatureLevel_02)
            GTEST_SKIP();

        auto* passBind = m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "binding");
        ASSERT_NE(nullptr, passBind);

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&WarningData::message, ::testing::StrEq("Node [binding] has no ingoing links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&WarningData::type, ::testing::Eq(EWarningType::UnusedContent))));
    }

    TEST_P(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfRenderPassBindingHasIngoingLinks)
    {
        if (m_logicEngine.getFeatureLevel() < EFeatureLevel_02)
            GTEST_SKIP();

        auto* passBind = m_logicEngine.createRamsesRenderPassBinding(*m_renderPass, "binding");
        ASSERT_NE(nullptr, passBind);

        linkNodeInput(*passBind, "renderOrder", "paramInt32");
        m_logicEngine.update();

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(0u, warnings.size());
    }

    // -- render group bindings -- //

    TEST_P(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfRenderGroupBindingHasNoIngoingLinks)
    {
        if (m_logicEngine.getFeatureLevel() < EFeatureLevel_03)
            GTEST_SKIP();

        RamsesRenderGroupBindingElements elements;
        elements.addElement(*m_meshNode, "mesh");
        auto* binding = m_logicEngine.createRamsesRenderGroupBinding(*m_renderGroup, elements, "binding");
        ASSERT_NE(nullptr, binding);

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&WarningData::message, ::testing::StrEq("Node [binding] has no ingoing links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&WarningData::type, ::testing::Eq(EWarningType::UnusedContent))));
    }

    TEST_P(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfRenderGroupBindingHasIngoingLinks)
    {
        if (m_logicEngine.getFeatureLevel() < EFeatureLevel_03)
            GTEST_SKIP();

        RamsesRenderGroupBindingElements elements;
        elements.addElement(*m_meshNode, "mesh");
        auto* binding = m_logicEngine.createRamsesRenderGroupBinding(*m_renderGroup, elements, "binding");
        ASSERT_NE(nullptr, binding);

        linkNodeInput(*binding->getInputs()->getChild("renderOrders")->getChild("mesh"), "paramInt32");
        m_logicEngine.update();

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(0u, warnings.size());
    }

    // -- mesh node bindings -- //

    TEST_P(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfMeshNodeBindingHasNoIngoingLinks)
    {
        if (m_logicEngine.getFeatureLevel() < EFeatureLevel_05)
            GTEST_SKIP();

        m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode, "binding");

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&WarningData::message, ::testing::StrEq("Node [binding] has no ingoing links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&WarningData::type, ::testing::Eq(EWarningType::UnusedContent))));
    }

    TEST_P(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfMeshNodeBindingHasIngoingLinks)
    {
        if (m_logicEngine.getFeatureLevel() < EFeatureLevel_05)
            GTEST_SKIP();

        const auto binding = m_logicEngine.createRamsesMeshNodeBinding(*m_meshNode, "binding");
        ASSERT_NE(nullptr, binding);

        linkNodeInput(*binding->getInputs()->getChild("indexOffset"), "paramInt32");
        EXPECT_TRUE(m_logicEngine.update());

        EXPECT_TRUE(m_logicEngine.validate().empty());
    }

    // -- animations -- //

    TEST_P(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfAnimationHasNoLinks)
    {
        const auto* m_dataFloat = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f });
        const auto* m_dataVec2 = m_logicEngine.createDataArray(std::vector<vec2f>{ { 1.f, 2.f }, { 3.f, 4.f }, { 5.f, 6.f } });
        const AnimationChannel channel{ "channel", m_dataFloat, m_dataVec2 };
        AnimationNodeConfig config;
        config.addChannel(channel);
        auto* anim = m_logicEngine.createAnimationNode(config, "anim");
        ASSERT_NE(nullptr, anim);

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(2u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&WarningData::message, ::testing::StrEq("Node [anim] has no outgoing links! Node should be deleted or properly linked!")),
            ::testing::Field(&WarningData::message, ::testing::StrEq("Node [anim] has no ingoing links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&WarningData::type, ::testing::Eq(EWarningType::UnusedContent))));
    }

    TEST_P(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfAnimationHasNoOutgoingLinks)
    {
        const auto* m_dataFloat = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f });
        const auto* m_dataVec2 = m_logicEngine.createDataArray(std::vector<vec2f>{ { 1.f, 2.f }, { 3.f, 4.f }, { 5.f, 6.f } });
        const AnimationChannel channel{ "channel", m_dataFloat, m_dataVec2 };
        AnimationNodeConfig config;
        config.addChannel(channel);
        auto* anim = m_logicEngine.createAnimationNode(config, "anim");
        ASSERT_NE(nullptr, anim);

        linkNodeInput(*anim, "progress", "paramFloat");

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&WarningData::message, ::testing::StrEq("Node [anim] has no outgoing links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&WarningData::type, ::testing::Eq(EWarningType::UnusedContent))));
    }

    TEST_P(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfAnimationHasNoIngoingLinks)
    {
        const auto* m_dataFloat = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f });
        const auto* m_dataVec2 = m_logicEngine.createDataArray(std::vector<vec2f>{ { 1.f, 2.f }, { 3.f, 4.f }, { 5.f, 6.f } });
        const AnimationChannel channel{ "channel", m_dataFloat, m_dataVec2 };
        AnimationNodeConfig config;
        config.addChannel(channel);
        auto* anim = m_logicEngine.createAnimationNode(config, "anim");
        ASSERT_NE(nullptr, anim);

        linkNodeOutput(*anim, "duration", "paramFloat");

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&WarningData::message, ::testing::StrEq("Node [anim] has no ingoing links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&WarningData::type, ::testing::Eq(EWarningType::UnusedContent))));
    }

    TEST_P(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfAnimationHasIngoingAndOutgoingLinks)
    {
        const auto* m_dataFloat = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f });
        const auto* m_dataVec2 = m_logicEngine.createDataArray(std::vector<vec2f>{ { 1.f, 2.f }, { 3.f, 4.f }, { 5.f, 6.f } });
        const AnimationChannel channel{ "channel", m_dataFloat, m_dataVec2 };
        AnimationNodeConfig config;
        config.addChannel(channel);
        auto* anim = m_logicEngine.createAnimationNode(config, "anim");
        ASSERT_NE(nullptr, anim);

        linkNodeInput(*anim, "progress", "paramFloat");
        linkNodeOutput(*anim, "duration", "paramFloat");

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(0u, warnings.size());
    }

    // -- timers -- //

    TEST_P(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfTimerHasNoOutgoingLinks)
    {
        auto* timer = m_logicEngine.createTimerNode("timer");
        ASSERT_NE(nullptr, timer);

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&WarningData::message, ::testing::StrEq("Node [timer] has no outgoing links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&WarningData::type, ::testing::Eq(EWarningType::UnusedContent))));
    }

    TEST_P(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfTimerHasOutgoingLinks)
    {
        auto* timer = m_logicEngine.createTimerNode("timer");
        ASSERT_NE(nullptr, timer);

        linkNodeOutput(*timer, "ticker_us", "paramInt64");

        auto warnings = m_logicEngine.validate();
        ASSERT_EQ(0u, warnings.size());
    }

    // -- anchor points -- //

    TEST_P(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfAnchorPointHasNoOutgoingLinksAndItsBindingsNoIncomingLinks)
    {
        if (m_logicEngine.getFeatureLevel() < EFeatureLevel_02)
            GTEST_SKIP();

        auto* nodeBind = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodeBinding");
        auto* cameraBind = m_logicEngine.createRamsesCameraBinding(*m_camera, "cameraBinding");
        auto* anchor = m_logicEngine.createAnchorPoint(*nodeBind, *cameraBind, "anchorBinding");
        ASSERT_NE(nullptr, anchor);

        m_logicEngine.update();
        EXPECT_TRUE(m_logicEngine.validate().empty());
    }

    TEST_P(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfAnchorPointHasOutgoingLink)
    {
        if (m_logicEngine.getFeatureLevel() < EFeatureLevel_02)
            GTEST_SKIP();

        auto* nodeBind = m_logicEngine.createRamsesNodeBinding(*m_node, ERotationType::Euler_XYZ, "nodeBinding");
        auto* cameraBind = m_logicEngine.createRamsesCameraBinding(*m_camera, "cameraBinding");
        auto* anchor = m_logicEngine.createAnchorPoint(*nodeBind, *cameraBind, "anchorBinding");
        ASSERT_NE(nullptr, anchor);

        linkNodeOutput(*anchor, "depth", "paramFloat");

        m_logicEngine.update();
        EXPECT_TRUE(m_logicEngine.validate().empty());
    }

    // -- skin bindings -- //

    TEST_P(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningWhenCheckingSkinBindingAndItsBindingsHaveNoIncomingLinks)
    {
        if (m_logicEngine.getFeatureLevel() < EFeatureLevel_04)
            GTEST_SKIP();

        const auto skin = createSkinBinding(m_logicEngine);
        ASSERT_NE(nullptr, skin);

        m_logicEngine.update();
        EXPECT_TRUE(m_logicEngine.validate().empty());
    }

    TEST(GetVerboseDescriptionFunction, ReturnsCorrectString)
    {
        EXPECT_STREQ("Performance", GetVerboseDescription(EWarningType::Performance));
        EXPECT_STREQ("Unsafe Data State", GetVerboseDescription(EWarningType::UnsafeDataState));
        EXPECT_STREQ("Uninitialized Data", GetVerboseDescription(EWarningType::UninitializedData));
        EXPECT_STREQ("Precision Loss", GetVerboseDescription(EWarningType::PrecisionLoss));
        EXPECT_STREQ("Unused Content", GetVerboseDescription(EWarningType::UnusedContent));
        EXPECT_STREQ("Duplicate Content", GetVerboseDescription(EWarningType::DuplicateContent));
        EXPECT_STREQ("Other", GetVerboseDescription(EWarningType::Other));
    }
}
