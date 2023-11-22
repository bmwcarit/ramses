//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineTest_Base.h"

#include "ScopedLogContextLevel.h"
#include "ramses/client/logic/Property.h"
#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/client/logic/AnimationNodeConfig.h"

#include "internal/Core/Utils/LogMacros.h"

#include "LogTestUtils.h"
#include "WithTempDirectory.h"
#include "RamsesTestUtils.h"

namespace ramses::internal
{
    const size_t SCENE_ISSUES = 9u;

    class ALogicEngine_Validation : public ALogicEngine
    {
    public:
        ValidationReport report;
    };

    TEST_F(ALogicEngine_Validation, LogsNoWarningsWhenSavingFile_WhenContentValid)
    {
        int warningCount = 0;

        ScopedLogContextLevel logCollector{ CONTEXT_CLIENT, ELogLevel::Trace, [&](ELogLevel type, std::string_view /*message*/) {
            if (type == ELogLevel::Warn)
                ++warningCount;
        }};

        m_logicEngine->validate(report);
        EXPECT_FALSE(report.hasIssue());

        WithTempDirectory tmpDir;
        // Empty logic engine has ho warnings
        saveToFile("noWarnings.ramses");
        EXPECT_LE(warningCount, SCENE_ISSUES); // logic is valid, but ramses scene is not
    }

    TEST_F(ALogicEngine_Validation, ProducesWarningIfBindingValuesHaveDirtyValue)
    {
        // Create a binding in a "dirty" state - has a non-default value, but update() wasn't called and didn't pass the value to ramses
        NodeBinding* nodeBinding = m_logicEngine->createNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "binding");

        auto* intf = m_logicEngine->createLuaInterface(m_interfaceSourceCode, "intf");
        m_logicEngine->link(*intf->getOutputs()->getChild("param_vec3f"), *nodeBinding->getInputs()->getChild("scaling"));

        nodeBinding->getInputs()->getChild("visibility")->set<bool>(false);

        // Expects warning
        m_logicEngine->validate(report);
        const auto& warnings = report.getIssues();

        ASSERT_EQ(1u, warnings.size());
        EXPECT_EQ(warnings[0].message, "Saving logic engine content with manually updated binding values without calling update() will result in those values being lost!");
        EXPECT_EQ(warnings[0].type, EIssueType::Warning);
    }

    TEST_F(ALogicEngine_Validation, ProducesWarningIfInterfaceHasUnboundOutputs)
    {
        LuaInterface* intf = m_logicEngine->createLuaInterface(R"(
            function interface(IN,OUT)

                IN.param1 = Type:Int32()
                IN.param2 = {a=Type:Float(), b=Type:Int32()}

            end
        )", "intf name");
        ASSERT_NE(nullptr, intf);

        // Expects warning
        m_logicEngine->validate(report);
        const auto& warnings = report.getIssues();

        ASSERT_EQ(3u, warnings.size());
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&Issue::message, ::testing::HasSubstr("Interface [intf name] has unlinked output"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&Issue::type, ::testing::Eq(EIssueType::Warning))));
    }

    class ALogicEngine_ValidatingDanglingNodes : public ALogicEngine
    {
    protected:
        ALogicEngine_ValidatingDanglingNodes()
        {
            createValidTestSetup();
        }

        void createValidTestSetup()
        {
            //create two identical scripts each containing 1 input and 1 output
            m_testScript = m_logicEngine->createLuaScript(m_scriptSrc, {}, "script1");
            auto* script2 = m_logicEngine->createLuaScript(m_scriptSrc, {}, "script2");

            //cross link inputs and outputs of each script so that none of the scripts is "dangling"
            m_logicEngine->link(*m_testScript->getOutputs()->getChild("paramInt32"), *script2->getInputs()->getChild("paramTestReserved"));
            m_logicEngine->linkWeak(*script2->getOutputs()->getChild("paramInt32"), *m_testScript->getInputs()->getChild("paramTestReserved"));
        }

        void linkNodeInput(LogicNode& node, std::string_view inputName, std::string_view testScriptOutputName = "paramInt32")
        {
            linkNodeInput(*node.getInputs()->getChild(inputName), testScriptOutputName);
        }

        void linkNodeInput(Property& nodeInput, std::string_view testScriptOutputName = "paramInt32")
        {
            m_logicEngine->link(*m_testScript->getOutputs()->getChild(testScriptOutputName), nodeInput);
        }

        void linkNodeOutput(LogicNode& node, std::string_view outputName, std::string_view testScriptInputName = "paramInt32")
        {
            m_logicEngine->link(*node.getOutputs()->getChild(outputName), *m_testScript->getInputs()->getChild(testScriptInputName));
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
        ValidationReport report;
    };

    // -- scripts -- //

    TEST_F(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfScriptHasNoLinks)
    {
        auto* script = m_logicEngine->createLuaScript(m_scriptSrc, {}, "scr");
        ASSERT_NE(nullptr, script);

        m_logicEngine->validate(report);
        const auto& warnings = report.getIssues();
        ASSERT_EQ(2u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&Issue::message, ::testing::StrEq("Node [scr] has no outgoing links! Node should be deleted or properly linked!")),
                                                    ::testing::Field(&Issue::message, ::testing::StrEq("Node [scr] has no incoming links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&Issue::type, ::testing::Eq(EIssueType::Warning))));
    }

    TEST_F(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfScriptHasNoIncomingLinks)
    {
        auto* script = m_logicEngine->createLuaScript(m_scriptSrc, {}, "scr");
        ASSERT_NE(nullptr, script);

        linkNodeOutput(*script, "paramInt32");

        m_logicEngine->validate(report);
        const auto& warnings = report.getIssues();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&Issue::message, ::testing::StrEq("Node [scr] has no incoming links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&Issue::type, ::testing::Eq(EIssueType::Warning))));
    }

    TEST_F(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfScriptHasNoOutgoingLinks)
    {
        auto* script = m_logicEngine->createLuaScript(m_scriptSrc, {}, "scr");
        ASSERT_NE(nullptr, script);

        linkNodeInput(*script, "paramInt32");

        m_logicEngine->validate(report);
        const auto& warnings = report.getIssues();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&Issue::message, ::testing::StrEq("Node [scr] has no outgoing links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&Issue::type, ::testing::Eq(EIssueType::Warning))));
    }

    TEST_F(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfScriptHasIncomingAndOutgoingLinks)
    {
        auto* script = m_logicEngine->createLuaScript(m_scriptSrc, {}, "scr");
        ASSERT_NE(nullptr, script);

        linkNodeOutput(*script, "paramInt32");
        linkNodeInput(*script, "paramInt32");

        m_logicEngine->validate(report);
        ASSERT_FALSE(report.hasIssue());
    }

    // -- interfaces -- //

    TEST_F(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfInterfaceHasNoIncomingLinks)
    {
        LuaInterface* intf = m_logicEngine->createLuaInterface(R"(
            function interface(INOUT)
                INOUT.param = Type:Int32()
            end
        )", "intf name");
        ASSERT_NE(nullptr, intf);

        linkNodeOutput(*intf, "param");

        m_logicEngine->validate(report);
        ASSERT_FALSE(report.hasIssue());
    }

    // -- node bindings -- //

    TEST_F(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfNodeBindingHasNoIncomingLinks)
    {
        auto* nodeBind = m_logicEngine->createNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "binding");
        ASSERT_NE(nullptr, nodeBind);

        m_logicEngine->validate(report);
        const auto& warnings = report.getIssues();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&Issue::message, ::testing::StrEq("Node [binding] has no incoming links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&Issue::type, ::testing::Eq(EIssueType::Warning))));
    }

    TEST_F(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfNodeBindingHasIncomingLinks)
    {
        auto* nodeBind = m_logicEngine->createNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "binding");
        ASSERT_NE(nullptr, nodeBind);

        linkNodeInput(*nodeBind, "scaling", "paramVec3f");
        m_logicEngine->update();

        m_logicEngine->validate(report);
        ASSERT_FALSE(report.hasIssue());
    }

    // -- appearance bindings -- //

    TEST_F(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfAppearanceBindingHasNoIncomingLinks)
    {
        auto* appearanceBind = m_logicEngine->createAppearanceBinding(*m_appearance, "binding");
        ASSERT_NE(nullptr, appearanceBind);

        m_logicEngine->validate(report);
        const auto& warnings = report.getIssues();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&Issue::message, ::testing::StrEq("Node [binding] has no incoming links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&Issue::type, ::testing::Eq(EIssueType::Warning))));
    }

    TEST_F(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfAppearanceBindingHasIncomingLinks)
    {
        auto* appearanceBind = m_logicEngine->createAppearanceBinding(*m_appearance, "binding");
        ASSERT_NE(nullptr, appearanceBind);

        linkNodeInput(*appearanceBind, "floatUniform", "paramFloat");
        m_logicEngine->update();

        m_logicEngine->validate(report);
        ASSERT_FALSE(report.hasIssue());
    }

    // -- camera bindings -- //

    TEST_F(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfCameraBindingHasNoIncomingLinks)
    {
        auto* cameraBind = m_logicEngine->createCameraBinding(*m_camera, "binding");
        ASSERT_NE(nullptr, cameraBind);

        m_logicEngine->validate(report);
        const auto& warnings = report.getIssues();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&Issue::message, ::testing::StrEq("Node [binding] has no incoming links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&Issue::type, ::testing::Eq(EIssueType::Warning))));
    }

    TEST_F(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfCameraBindingHasIncomingLinks)
    {
        auto* cameraBind = m_logicEngine->createCameraBinding(*m_camera, "binding");
        ASSERT_NE(nullptr, cameraBind);

        m_logicEngine->link(*m_testScript->getOutputs()->getChild("paramInt32"), *cameraBind->getInputs()->getChild("viewport")->getChild("offsetX"));
        m_logicEngine->update();

        m_logicEngine->validate(report);
        ASSERT_FALSE(report.hasIssue());
    }

    // -- render pass bindings -- //

    TEST_F(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfRenderPassBindingHasNoIncomingLinks)
    {
        auto* passBind = m_logicEngine->createRenderPassBinding(*m_renderPass, "binding");
        ASSERT_NE(nullptr, passBind);

        m_logicEngine->validate(report);
        const auto& warnings = report.getIssues();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&Issue::message, ::testing::StrEq("Node [binding] has no incoming links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&Issue::type, ::testing::Eq(EIssueType::Warning))));
    }

    TEST_F(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfRenderPassBindingHasIncomingLinks)
    {
        auto* passBind = m_logicEngine->createRenderPassBinding(*m_renderPass, "binding");
        ASSERT_NE(nullptr, passBind);

        linkNodeInput(*passBind, "renderOrder", "paramInt32");
        m_logicEngine->update();

        m_logicEngine->validate(report);
        ASSERT_FALSE(report.hasIssue());
    }

    // -- render group bindings -- //

    TEST_F(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfRenderGroupBindingHasNoIncomingLinks)
    {
        RenderGroupBindingElements elements;
        elements.addElement(*m_meshNode, "mesh");
        auto* binding = m_logicEngine->createRenderGroupBinding(*m_renderGroup, elements, "binding");
        ASSERT_NE(nullptr, binding);

        m_logicEngine->validate(report);
        const auto& warnings = report.getIssues();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&Issue::message, ::testing::StrEq("Node [binding] has no incoming links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&Issue::type, ::testing::Eq(EIssueType::Warning))));
    }

    TEST_F(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfRenderGroupBindingHasIncomingLinks)
    {
        RenderGroupBindingElements elements;
        elements.addElement(*m_meshNode, "mesh");
        auto* binding = m_logicEngine->createRenderGroupBinding(*m_renderGroup, elements, "binding");
        ASSERT_NE(nullptr, binding);

        linkNodeInput(*binding->getInputs()->getChild("renderOrders")->getChild("mesh"), "paramInt32");
        m_logicEngine->update();

        m_logicEngine->validate(report);
        ASSERT_FALSE(report.hasIssue());
    }

    // -- mesh node bindings -- //

    TEST_F(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfMeshNodeBindingHasNoIncomingLinks)
    {
        m_logicEngine->createMeshNodeBinding(*m_meshNode, "binding");

        m_logicEngine->validate(report);
        const auto& warnings = report.getIssues();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&Issue::message, ::testing::StrEq("Node [binding] has no incoming links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&Issue::type, ::testing::Eq(EIssueType::Warning))));
    }

    TEST_F(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfMeshNodeBindingHasIncomingLinks)
    {
        const auto binding = m_logicEngine->createMeshNodeBinding(*m_meshNode, "binding");
        ASSERT_NE(nullptr, binding);

        linkNodeInput(*binding->getInputs()->getChild("indexOffset"), "paramInt32");
        EXPECT_TRUE(m_logicEngine->update());

        m_logicEngine->validate(report);
        ASSERT_FALSE(report.hasIssue());
    }

    // -- animations -- //

    TEST_F(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfAnimationHasNoLinks)
    {
        const auto* m_dataFloat = m_logicEngine->createDataArray(std::vector<float>{ 1.f, 2.f, 3.f });
        const auto* m_dataVec2 = m_logicEngine->createDataArray(std::vector<vec2f>{ { 1.f, 2.f }, { 3.f, 4.f }, { 5.f, 6.f } });
        const AnimationChannel channel{ "channel", m_dataFloat, m_dataVec2 };
        AnimationNodeConfig config;
        config.addChannel(channel);
        auto* anim = m_logicEngine->createAnimationNode(config, "anim");
        ASSERT_NE(nullptr, anim);

        m_logicEngine->validate(report);
        const auto& warnings = report.getIssues();
        ASSERT_EQ(2u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&Issue::message, ::testing::StrEq("Node [anim] has no outgoing links! Node should be deleted or properly linked!")),
            ::testing::Field(&Issue::message, ::testing::StrEq("Node [anim] has no incoming links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&Issue::type, ::testing::Eq(EIssueType::Warning))));
    }

    TEST_F(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfAnimationHasNoOutgoingLinks)
    {
        const auto* m_dataFloat = m_logicEngine->createDataArray(std::vector<float>{ 1.f, 2.f, 3.f });
        const auto* m_dataVec2 = m_logicEngine->createDataArray(std::vector<vec2f>{ { 1.f, 2.f }, { 3.f, 4.f }, { 5.f, 6.f } });
        const AnimationChannel channel{ "channel", m_dataFloat, m_dataVec2 };
        AnimationNodeConfig config;
        config.addChannel(channel);
        auto* anim = m_logicEngine->createAnimationNode(config, "anim");
        ASSERT_NE(nullptr, anim);

        linkNodeInput(*anim, "progress", "paramFloat");

        m_logicEngine->validate(report);
        const auto& warnings = report.getIssues();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&Issue::message, ::testing::StrEq("Node [anim] has no outgoing links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&Issue::type, ::testing::Eq(EIssueType::Warning))));
    }

    TEST_F(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfAnimationHasNoIncomingLinks)
    {
        const auto* m_dataFloat = m_logicEngine->createDataArray(std::vector<float>{ 1.f, 2.f, 3.f });
        const auto* m_dataVec2 = m_logicEngine->createDataArray(std::vector<vec2f>{ { 1.f, 2.f }, { 3.f, 4.f }, { 5.f, 6.f } });
        const AnimationChannel channel{ "channel", m_dataFloat, m_dataVec2 };
        AnimationNodeConfig config;
        config.addChannel(channel);
        auto* anim = m_logicEngine->createAnimationNode(config, "anim");
        ASSERT_NE(nullptr, anim);

        linkNodeOutput(*anim, "duration", "paramFloat");

        m_logicEngine->validate(report);
        const auto& warnings = report.getIssues();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&Issue::message, ::testing::StrEq("Node [anim] has no incoming links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&Issue::type, ::testing::Eq(EIssueType::Warning))));
    }

    TEST_F(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfAnimationHasIncomingAndOutgoingLinks)
    {
        const auto* m_dataFloat = m_logicEngine->createDataArray(std::vector<float>{ 1.f, 2.f, 3.f });
        const auto* m_dataVec2 = m_logicEngine->createDataArray(std::vector<vec2f>{ { 1.f, 2.f }, { 3.f, 4.f }, { 5.f, 6.f } });
        const AnimationChannel channel{ "channel", m_dataFloat, m_dataVec2 };
        AnimationNodeConfig config;
        config.addChannel(channel);
        auto* anim = m_logicEngine->createAnimationNode(config, "anim");
        ASSERT_NE(nullptr, anim);

        linkNodeInput(*anim, "progress", "paramFloat");
        linkNodeOutput(*anim, "duration", "paramFloat");

        m_logicEngine->validate(report);
        ASSERT_FALSE(report.hasIssue());
    }

    // -- timers -- //

    TEST_F(ALogicEngine_ValidatingDanglingNodes, ProducesWarningIfTimerHasNoOutgoingLinks)
    {
        auto* timer = m_logicEngine->createTimerNode("timer");
        ASSERT_NE(nullptr, timer);

        m_logicEngine->validate(report);
        const auto& warnings = report.getIssues();
        ASSERT_EQ(1u, warnings.size());
        EXPECT_THAT(warnings, ::testing::ElementsAre(::testing::Field(&Issue::message, ::testing::StrEq("Node [timer] has no outgoing links! Node should be deleted or properly linked!"))));
        EXPECT_THAT(warnings, ::testing::Each(::testing::Field(&Issue::type, ::testing::Eq(EIssueType::Warning))));
    }

    TEST_F(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfTimerHasOutgoingLinks)
    {
        auto* timer = m_logicEngine->createTimerNode("timer");
        ASSERT_NE(nullptr, timer);

        linkNodeOutput(*timer, "ticker_us", "paramInt64");

        m_logicEngine->validate(report);
        ASSERT_FALSE(report.hasIssue());
    }

    // -- anchor points -- //

    TEST_F(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfAnchorPointHasNoOutgoingLinksAndItsBindingsNoIncomingLinks)
    {
        auto* nodeBind = m_logicEngine->createNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "nodeBinding");
        auto* cameraBind = m_logicEngine->createCameraBinding(*m_camera, "cameraBinding");
        auto* anchor = m_logicEngine->createAnchorPoint(*nodeBind, *cameraBind, "anchorBinding");
        ASSERT_NE(nullptr, anchor);

        m_logicEngine->update();
        m_logicEngine->validate(report);
        ASSERT_FALSE(report.hasIssue());
    }

    TEST_F(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningIfAnchorPointHasOutgoingLink)
    {
        auto* nodeBind = m_logicEngine->createNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "nodeBinding");
        auto* cameraBind = m_logicEngine->createCameraBinding(*m_camera, "cameraBinding");
        auto* anchor = m_logicEngine->createAnchorPoint(*nodeBind, *cameraBind, "anchorBinding");
        ASSERT_NE(nullptr, anchor);

        linkNodeOutput(*anchor, "depth", "paramFloat");

        m_logicEngine->update();
        m_logicEngine->validate(report);
        ASSERT_FALSE(report.hasIssue());
    }

    // -- skin bindings -- //

    TEST_F(ALogicEngine_ValidatingDanglingNodes, DoesNotProduceWarningWhenCheckingSkinBindingAndItsBindingsHaveNoIncomingLinks)
    {
        const auto skin = createSkinBinding(*m_logicEngine);
        ASSERT_NE(nullptr, skin);

        m_logicEngine->update();
        m_logicEngine->validate(report);
        ASSERT_FALSE(report.hasIssue());
    }
}
