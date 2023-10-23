//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gmock/gmock.h>

#include "LogicEngineTest_Base.h"

#include "RamsesTestUtils.h"

#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/CameraBinding.h"

#include "ramses/client/logic/Property.h"

#include "ramses/client/EffectDescription.h"
#include "ramses/client/Effect.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/client/Node.h"

#include "impl/logic/LogicNodeImpl.h"
#include "impl/logic/LogicEngineImpl.h"
#include "impl/logic/LuaScriptImpl.h"

#include "fmt/format.h"

namespace ramses::internal
{
    class ALogicEngine_Update : public ALogicEngine
    {
    };

    TEST_F(ALogicEngine_Update, UpdatesNodeBindingValuesOnUpdate)
    {
        auto luaScript = m_logicEngine->createLuaScript(R"(
            function interface(IN,OUT)
                IN.param = Type:Bool()
                OUT.param = Type:Bool()
            end
            function run(IN,OUT)
                OUT.param = IN.param
            end
        )");

        auto nodeBinding = m_logicEngine->createNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");

        auto scriptInput  = luaScript->getInputs()->getChild("param");
        auto scriptOutput = luaScript->getOutputs()->getChild("param");
        auto nodeInput    = nodeBinding->getInputs()->getChild("visibility");
        scriptInput->set(true);
        nodeInput->set(false);

        m_logicEngine->link(*scriptOutput, *nodeInput);

        EXPECT_FALSE(*nodeInput->get<bool>());
        EXPECT_TRUE(m_logicEngine->update());
        EXPECT_TRUE(*nodeInput->get<bool>());
    }

    TEST_F(ALogicEngine_Update, UpdatesCameraBindingValuesOnUpdate)
    {
        auto luaScript = m_logicEngine->createLuaScript(R"(
            function interface(IN,OUT)
                IN.param = Type:Int32()
                OUT.param = Type:Int32()
            end
            function run(IN,OUT)
                OUT.param = IN.param
            end
        )");

        auto cameraBinding = m_logicEngine->createCameraBinding(*m_scene->createPerspectiveCamera(), "CameraBinding");

        auto scriptInput = luaScript->getInputs()->getChild("param");
        auto scriptOutput = luaScript->getOutputs()->getChild("param");
        auto cameraInput = cameraBinding->getInputs()->getChild("viewport")->getChild("offsetX");
        scriptInput->set(34);
        cameraInput->set(21);

        m_logicEngine->link(*scriptOutput, *cameraInput);

        EXPECT_EQ(21, *cameraInput->get<int32_t>());
        EXPECT_TRUE(m_logicEngine->update());
        EXPECT_EQ(34, *cameraInput->get<int32_t>());
    }

    TEST_F(ALogicEngine_Update, UpdatesAAppearanceBinding)
    {
        ramses::EffectDescription effectDesc;
        effectDesc.setFragmentShader(R"(
        #version 100

        void main(void)
        {
            gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        })");

        effectDesc.setVertexShader(R"(
        #version 100

        uniform highp float floatUniform;
        attribute vec3 a_position;

        void main()
        {
            gl_Position = floatUniform * vec4(a_position, 1.0);
        })");

        const ramses::Effect* effect = m_scene->createEffect(effectDesc);
        ramses::Appearance* appearance = m_scene->createAppearance(*effect);

        auto appearanceBinding = m_logicEngine->createAppearanceBinding(*appearance, "appearancebinding");

        auto floatUniform = appearanceBinding->getInputs()->getChild("floatUniform");
        floatUniform->set(47.11f);

        m_logicEngine->update();

        const auto floatInput = effect->findUniformInput("floatUniform");
        ASSERT_TRUE(floatInput.has_value());
        float result = 0.0f;
        appearance->getInputValue(*floatInput, result);
        EXPECT_FLOAT_EQ(47.11f, result);
    }

    TEST_F(ALogicEngine_Update, ProducesErrorIfLinkedScriptHasRuntimeError)
    {
        auto scriptSource = R"(
            function interface(IN,OUT)
                IN.param = Type:Bool()
                OUT.param = Type:Bool()
            end
            function run(IN,OUT)
                error("This will die")
            end
        )";

        auto sourceScript = m_logicEngine->createLuaScript(scriptSource, WithStdModules({EStandardModule::Base}));
        auto targetScript = m_logicEngine->createLuaScript(scriptSource, WithStdModules({EStandardModule::Base}));

        auto output = sourceScript->getOutputs()->getChild("param");
        auto input  = targetScript->getInputs()->getChild("param");
        input->set(true);

        m_logicEngine->link(*output, *input);

        EXPECT_FALSE(m_logicEngine->update());
        expectErrorSubstring("This will die", sourceScript);
    }

    TEST_F(ALogicEngine_Update, PropagatesValuesOnlyToConnectedLogicNodes)
    {
        auto        scriptSource = R"(
            function interface(IN,OUT)
                IN.inFloat = Type:Float()
                IN.inVec3  = Type:Vec3f()
                IN.inInt   = Type:Int32()
                OUT.outFloat = Type:Float()
                OUT.outVec3  = Type:Vec3f()
                OUT.outInt   = Type:Int32()
            end
            function run(IN,OUT)
                OUT.outFloat = IN.inFloat
                OUT.outVec3 = IN.inVec3
                OUT.outInt = IN.inInt
            end
        )";

        auto script            = m_logicEngine->createLuaScript(scriptSource);
        auto nodeBinding       = m_logicEngine->createNodeBinding(*m_node, ramses::ERotationType::Euler_XYZ, "NodeBinding");
        auto appearanceBinding = m_logicEngine->createAppearanceBinding(*m_appearance, "AppearanceBinding");
        auto cameraBinding = m_logicEngine->createCameraBinding(*m_camera, "CameraBinding");

        auto nodeBindingTranslation = nodeBinding->getInputs()->getChild("translation");
        nodeBindingTranslation->set(vec3f{1.f, 2.f, 3.f});
        auto appearanceBindingFloatUniform = appearanceBinding->getInputs()->getChild("floatUniform");
        appearanceBindingFloatUniform->set(42.f);
        auto cameraBindingViewportOffsetX = cameraBinding->getInputs()->getChild("viewport")->getChild("offsetX");
        cameraBindingViewportOffsetX->set(43);

        m_logicEngine->update();

        const auto floatInput = m_appearance->getEffect().findUniformInput("floatUniform");
        ASSERT_TRUE(floatInput.has_value());
        float floatUniformValue = 0.0f;
        m_appearance->getInputValue(*floatInput, floatUniformValue);

        EXPECT_FLOAT_EQ(42.f, floatUniformValue);
        EXPECT_EQ(43, m_camera->getViewportX());
        {
            vec3f values = {0.0f, 0.0f, 0.0f};
            m_node->getTranslation(values);
            EXPECT_EQ(values, vec3f(1.f, 2.f, 3.f));
        }

        auto nodeBindingScaling = nodeBinding->getInputs()->getChild("scaling");
        auto cameraBindingVpY   = cameraBinding->getInputs()->getChild("viewport")->getChild("offsetY");
        auto scriptOutputVec3   = script->getOutputs()->getChild("outVec3");
        auto scriptOutputFloat  = script->getOutputs()->getChild("outFloat");
        auto scriptOutputInt    = script->getOutputs()->getChild("outInt");
        auto scriptInputVec3    = script->getInputs()->getChild("inVec3");
        auto scriptInputFloat   = script->getInputs()->getChild("inFloat");
        auto scriptInputInt     = script->getInputs()->getChild("inInt");
        auto appearanceInput    = appearanceBinding->getInputs()->getChild("floatUniform");

        m_logicEngine->link(*scriptOutputVec3, *nodeBindingScaling);
        scriptInputVec3->set(vec3f{3.f, 2.f, 1.f});
        scriptInputFloat->set(42.f);
        scriptInputInt->set(43);

        m_logicEngine->update();
        EXPECT_FLOAT_EQ(42.f, floatUniformValue);
        EXPECT_EQ(43, m_camera->getViewportX());
        {
            vec3f values = {0.0f, 0.0f, 0.0f};
            m_node->getTranslation(values);
            EXPECT_EQ(values, vec3f(1.f, 2.f, 3.f));
        }
        {
            vec3f values = {0.0f, 0.0f, 0.0f};
            m_node->getScaling(values);
            EXPECT_EQ(values, vec3f(3.f, 2.f, 1.f));
        }
        {
            vec3f values = {0.0f, 0.0f, 0.0f};
            m_node->getRotation(values);
            EXPECT_EQ(values, vec3f(0.f, 0.f, 0.f));
        }

        const auto floatUniform = m_appearance->getEffect().findUniformInput("floatUniform");
        ASSERT_TRUE(floatUniform.has_value());
        floatUniformValue = 0.0f;
        m_appearance->getInputValue(*floatUniform, floatUniformValue);

        EXPECT_FLOAT_EQ(42.f, floatUniformValue);

        m_logicEngine->link(*scriptOutputFloat, *appearanceInput);
        m_logicEngine->link(*scriptOutputInt, *cameraBindingVpY);

        m_logicEngine->update();

        m_appearance->getInputValue(*floatUniform, floatUniformValue);
        EXPECT_FLOAT_EQ(42.f, floatUniformValue);

        EXPECT_EQ(43, m_camera->getViewportY());

        m_logicEngine->unlink(*scriptOutputVec3, *nodeBindingScaling);
    }

    TEST_F(ALogicEngine_Update, OnlyUpdatesDirtyLogicNodes)
    {
        m_logicEngine->enableUpdateReport(true);

        auto        scriptSource = R"(
            function interface(IN,OUT)
                IN.inFloat = Type:Float()
                OUT.outFloat = Type:Float()
            end
            function run(IN,OUT)
                OUT.outFloat = IN.inFloat
            end
        )";

        auto sourceScript = m_logicEngine->createLuaScript(scriptSource, {});
        auto targetScript = m_logicEngine->createLuaScript(scriptSource, {});

        auto sourceInput  = sourceScript->getInputs()->getChild("inFloat");
        auto sourceOutput = sourceScript->getOutputs()->getChild("outFloat");
        auto targetInput  = targetScript->getInputs()->getChild("inFloat");

        EXPECT_TRUE(sourceScript->impl().isDirty());
        EXPECT_TRUE(targetScript->impl().isDirty());

        m_logicEngine->link(*sourceOutput, *targetInput);

        EXPECT_TRUE(sourceScript->impl().isDirty());
        EXPECT_TRUE(targetScript->impl().isDirty());

        m_logicEngine->update();

        EXPECT_FALSE(sourceScript->impl().isDirty());
        EXPECT_FALSE(targetScript->impl().isDirty());

        // both scripts are updated, because its the first update
        auto executedNodes = m_logicEngine->getLastUpdateReport().getNodesExecuted();
        ASSERT_EQ(2u, executedNodes.size());
        EXPECT_EQ(sourceScript, executedNodes[0].first);
        EXPECT_EQ(targetScript, executedNodes[1].first);

        m_logicEngine->update();

        EXPECT_TRUE(m_logicEngine->getLastUpdateReport().getNodesExecuted().empty());

        targetInput->set(42.f);

        // targetScript is linked input and cannot be set manually so it is not dirty
        EXPECT_FALSE(sourceScript->impl().isDirty());
        EXPECT_FALSE(targetScript->impl().isDirty());

        m_logicEngine->update();

        EXPECT_FALSE(sourceScript->impl().isDirty());
        EXPECT_FALSE(targetScript->impl().isDirty());

        // Nothing is updated, because targetScript is linked input and cannot be set manually
        EXPECT_TRUE(m_logicEngine->getLastUpdateReport().getNodesExecuted().empty());

        sourceInput->set(24.f);
        m_logicEngine->update();

        // Both scripts are updated, because the input of the first script is changed and changes target through link.
        executedNodes = m_logicEngine->getLastUpdateReport().getNodesExecuted();
        ASSERT_EQ(2u, executedNodes.size());
        EXPECT_EQ(sourceScript, executedNodes[0].first);
        EXPECT_EQ(targetScript, executedNodes[1].first);
    }

    TEST_F(ALogicEngine_Update, OnlyUpdatesDirtyLogicNodesInAComplexLogicGraph)
    {
        auto        scriptSource = R"(
            function interface(IN,OUT)
                IN.in1 = Type:Int32()
                IN.in2 = Type:Int32()
                OUT.out = Type:Int32()
            end
            function run(IN,OUT)
                OUT.out = IN.in1 + IN.in2
            end
        )";

        std::array<LuaScript*, 6> s = {};
        for(auto& si : s)
        {
            si = m_logicEngine->createLuaScript(scriptSource);
        }

        auto in1S0  = s[0]->getInputs()->getChild("in1");
        auto out1S0 = s[0]->getOutputs()->getChild("out");
        auto in1S1  = s[1]->getInputs()->getChild("in1");
        auto in2S1  = s[1]->getInputs()->getChild("in2");
        auto out1S1 = s[1]->getOutputs()->getChild("out");
        auto in1S2  = s[2]->getInputs()->getChild("in1");
        auto in2S2  = s[2]->getInputs()->getChild("in2");
        auto out1S2 = s[2]->getOutputs()->getChild("out");
        auto in1S3  = s[3]->getInputs()->getChild("in1");
        auto in2S3  = s[3]->getInputs()->getChild("in2");
        auto out1S3 = s[3]->getOutputs()->getChild("out");
        auto in1S4  = s[4]->getInputs()->getChild("in1");
        auto in2S4  = s[4]->getInputs()->getChild("in2");
        auto out1S4 = s[4]->getOutputs()->getChild("out");
        auto in1S5  = s[5]->getInputs()->getChild("in1");
        auto in2S5  = s[5]->getInputs()->getChild("in2");

        /*
                 s2 -------
               /    \      \
            s0 ----- s1 -- s3 - s5
                            \  /
                             s4
         */

        ASSERT_TRUE(m_logicEngine->link(*out1S0, *in2S2));
        ASSERT_TRUE(m_logicEngine->link(*out1S0, *in2S1));
        ASSERT_TRUE(m_logicEngine->link(*out1S1, *in2S3));
        ASSERT_TRUE(m_logicEngine->link(*out1S2, *in1S1));
        ASSERT_TRUE(m_logicEngine->link(*out1S2, *in1S3));
        ASSERT_TRUE(m_logicEngine->link(*out1S3, *in1S5));
        ASSERT_TRUE(m_logicEngine->link(*out1S3, *in1S4));
        ASSERT_TRUE(m_logicEngine->link(*out1S4, *in2S5));

        m_logicEngine->enableUpdateReport(true);

        auto expectScriptsExecutedInOrder = [&s, this](std::vector<size_t> expectedOrder)
        {
            m_logicEngine->update();

            auto executedNodes = m_logicEngine->getLastUpdateReport().getNodesExecuted();
            ASSERT_EQ(expectedOrder.size(), executedNodes.size());
            for (size_t i = 0; i < expectedOrder.size(); ++i)
            {
                EXPECT_EQ(s[expectedOrder[i]], executedNodes[i].first) << "Wrong order for script: " << i << "; expected: " << expectedOrder[i];
            }
        };

        // Based on topology and first script dirty -> executes all scripts
        expectScriptsExecutedInOrder({0u, 2u, 1u, 3u, 4u, 5u});
        // Nothing dirty -> executes no scripts
        expectScriptsExecutedInOrder({});

        // Set value of script 4 -> scripts 4 and 5 are executed
        in2S4->set(1);
        expectScriptsExecutedInOrder({4u, 5u});
        expectScriptsExecutedInOrder({});

        in1S2->set(2);
        expectScriptsExecutedInOrder({ 2u, 1u, 3u, 4u, 5u });
        expectScriptsExecutedInOrder({});

        in1S0->set(42);
        expectScriptsExecutedInOrder({ 0u, 2u, 1u, 3u, 4u, 5u });
        expectScriptsExecutedInOrder({});

        in1S0->set(24);
        in1S2->set(23);
        expectScriptsExecutedInOrder({ 0u, 2u, 1u, 3u, 4u, 5u });
        expectScriptsExecutedInOrder({});
    }

    TEST_F(ALogicEngine_Update, AlwaysUpdatesNodeIfDirtyHandlingIsDisabled)
    {
        auto        scriptSource = R"(
            function interface(IN,OUT)
                IN.inFloat = Type:Float()
                OUT.outFloat = Type:Float()
            end
            function run(IN,OUT)
                OUT.outFloat = IN.inFloat
            end
        )";

        m_logicEngine->impl().disableTrackingDirtyNodes();
        m_logicEngine->enableUpdateReport(true);

        auto sourceScript = m_logicEngine->createLuaScript(scriptSource, {}, "SourceScript");
        auto targetScript = m_logicEngine->createLuaScript(scriptSource, {}, "TargetScript");

        auto sourceInput  = sourceScript->getInputs()->getChild("inFloat");
        auto sourceOutput = sourceScript->getOutputs()->getChild("outFloat");
        auto targetInput  = targetScript->getInputs()->getChild("inFloat");

        m_logicEngine->link(*sourceOutput, *targetInput);
        m_logicEngine->update();

        // both scripts are updated, because its the first update
        auto executedNodes = m_logicEngine->getLastUpdateReport().getNodesExecuted();
        ASSERT_EQ(2u, executedNodes.size());
        EXPECT_EQ(sourceScript, executedNodes[0].first);
        EXPECT_EQ(targetScript, executedNodes[1].first);

        m_logicEngine->unlink(*sourceOutput, *targetInput);
        targetInput->set(42.f);
        m_logicEngine->update();

        // Both scripts are updated, because dirty handling is disabled
        executedNodes = m_logicEngine->getLastUpdateReport().getNodesExecuted();
        ASSERT_EQ(2u, executedNodes.size());
        EXPECT_EQ(sourceScript, executedNodes[0].first);
        EXPECT_EQ(targetScript, executedNodes[1].first);

        sourceInput->set(24.f);
        m_logicEngine->update();

        // Both scripts are updated, because dirty handling is disabled
        executedNodes = m_logicEngine->getLastUpdateReport().getNodesExecuted();
        ASSERT_EQ(2u, executedNodes.size());
        EXPECT_EQ(sourceScript, executedNodes[0].first);
        EXPECT_EQ(targetScript, executedNodes[1].first);
    }
}
