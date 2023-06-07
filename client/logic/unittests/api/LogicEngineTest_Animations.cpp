//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/DataArray.h"
#include "ramses-logic/AnimationNode.h"
#include "ramses-logic/AnimationNodeConfig.h"
#include "ramses-logic/TimerNode.h"
#include "ramses-logic/Property.h"
#include "ramses-logic/RamsesNodeBinding.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Node.h"
#include <thread>

namespace ramses::internal
{
    class ALogicEngine_Animations : public ::testing::Test
    {
    public:
        void SetUp() override
        {
            const auto dataArray = m_logicEngine.createDataArray(std::vector<float>{ 0.f, 1.f }, "dataarray");
            const AnimationChannel channel{ "channel", dataArray, dataArray, ramses::EInterpolationType::Linear };
            AnimationNodeConfig config;
            config.addChannel(channel);

            m_animation1 = m_logicEngine.createAnimationNode(config, "animNode1");
            m_animation2 = m_logicEngine.createAnimationNode(config, "animNode2");
            m_timer = m_logicEngine.createTimerNode();
        }

    protected:
        void setTickerAndUpdate(int64_t tick)
        {
            m_timer->getInputs()->getChild("ticker_us")->set(tick);
            m_logicEngine.update();
        }

        void expectNodeValues(float expectedTranslate, float expectedRotate)
        {
            // we lose more than just single float epsilon precision in timer+animation logic
            // but this error is still negligible considering input data <0, 1>
            static constexpr float maxError = 1e-5f;

            vec3f vals;
            m_node->getTranslation(vals);
            EXPECT_NEAR(expectedTranslate, vals[0], maxError);
            EXPECT_NEAR(expectedTranslate, vals[1], maxError);
            EXPECT_NEAR(expectedTranslate, vals[2], maxError);
            m_node->getRotation(vals);
            EXPECT_NEAR(expectedRotate, vals[0], maxError);
            EXPECT_NEAR(expectedRotate, vals[1], maxError);
            EXPECT_NEAR(expectedRotate, vals[2], maxError);
            EXPECT_EQ(ramses::ERotationType::Euler_XYZ, m_node->getRotationType());
        }

        ramses::RamsesFramework m_ramsesFramework{ ramses::RamsesFrameworkConfig{ramses::EFeatureLevel_Latest} };
        ramses::RamsesClient* m_ramsesClient{ m_ramsesFramework.createClient("client") };
        ramses::Scene* m_scene{ m_ramsesClient->createScene(ramses::sceneId_t{123u}) };
        ramses::Node* m_node{ m_scene->createNode() };

        LogicEngine m_logicEngine{ ramses::EFeatureLevel_Latest };
        AnimationNode* m_animation1 = nullptr;
        AnimationNode* m_animation2 = nullptr;
        TimerNode* m_timer = nullptr;

        const std::string_view ScriptScalarToVecSrc = R"(
            function interface(IN,OUT)
                IN.scalar = Type:Float()
                OUT.vec = Type:Vec3f()
            end
            function run(IN,OUT)
                OUT.vec = { IN.scalar, IN.scalar, IN.scalar }
            end
            )";
    };

    TEST_F(ALogicEngine_Animations, ScriptsControllingAnimationsLinkedToScene_UsingTimerNodeWithUserProvidedTicker)
    {
        const auto scriptSrc = R"(
        function init()
            GLOBAL.startTick = 0
        end

        function interface(IN,OUT)
            IN.ticker = Type:Int64()
            IN.anim1Duration = Type:Float()
            IN.anim2Duration = Type:Float()

            OUT.anim1Progress = Type:Float()
            OUT.anim2Progress = Type:Float()
        end

        function run(IN,OUT)
            if GLOBAL.startTick == 0 then
                GLOBAL.startTick = IN.ticker
            end

            local elapsedTime = IN.ticker - GLOBAL.startTick
            -- ticker from TimerNode is in microseconds, our animation timestamps are in seconds, conversion is needed
            elapsedTime = elapsedTime / 1000000

            -- play anim1 right away
            OUT.anim1Progress = elapsedTime / IN.anim1Duration
            -- play anim2 after anim1
            OUT.anim2Progress = (elapsedTime - IN.anim1Duration) / IN.anim2Duration
        end
        )";

        const auto script = m_logicEngine.createLuaScript(scriptSrc);
        const auto scriptScalarToVec1 = m_logicEngine.createLuaScript(ScriptScalarToVecSrc);
        const auto scriptScalarToVec2 = m_logicEngine.createLuaScript(ScriptScalarToVecSrc);

        const auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node);

        script->getInputs()->getChild("anim1Duration")->set(*m_animation1->getOutputs()->getChild("duration")->get<float>());
        script->getInputs()->getChild("anim2Duration")->set(*m_animation2->getOutputs()->getChild("duration")->get<float>());

        // controlScript -> anim1/channel -> scalarToVec -> nodeRotation
        //               -> anim2/channel -> scalarToVec -> nodeScaling
        m_logicEngine.link(*script->getOutputs()->getChild("anim1Progress"), *m_animation1->getInputs()->getChild("progress"));
        m_logicEngine.link(*script->getOutputs()->getChild("anim2Progress"), *m_animation2->getInputs()->getChild("progress"));
        // link anim outputs to node bindings via helper to convert scalar to vec3
        m_logicEngine.link(*m_animation1->getOutputs()->getChild("channel"), *scriptScalarToVec1->getInputs()->getChild("scalar"));
        m_logicEngine.link(*m_animation2->getOutputs()->getChild("channel"), *scriptScalarToVec2->getInputs()->getChild("scalar"));
        m_logicEngine.link(*scriptScalarToVec1->getOutputs()->getChild("vec"), *nodeBinding->getInputs()->getChild("translation"));
        m_logicEngine.link(*scriptScalarToVec2->getOutputs()->getChild("vec"), *nodeBinding->getInputs()->getChild("rotation"));

        // link timer
        m_logicEngine.link(*m_timer->getOutputs()->getChild("ticker_us"), *script->getInputs()->getChild("ticker"));

        setTickerAndUpdate(1); // don't use 0 because it triggers timer auto generated time
        expectNodeValues(0.f, 0.f);

        setTickerAndUpdate(100000);
        expectNodeValues(0.1f, 0.f);

        setTickerAndUpdate(500000);
        expectNodeValues(0.5f, 0.f);

        setTickerAndUpdate(1000000);
        expectNodeValues(1.f, 0.f);

        // anim2 plays when anim1 finished
        setTickerAndUpdate(1100000);
        expectNodeValues(1.f, 0.1f);

        setTickerAndUpdate(1500000);
        expectNodeValues(1.f, 0.5f);

        setTickerAndUpdate(2000000);
        expectNodeValues(1.f, 1.f);

        setTickerAndUpdate(9999999);
        expectNodeValues(1.f, 1.f);
    }

    TEST_F(ALogicEngine_Animations, AnimationProgressesWhenUsingTimerWithAutogeneratedTicker)
    {
        const auto scriptSrc = R"(
        function init()
            GLOBAL.startTick = 0
        end

        function interface(IN,OUT)
            IN.ticker = Type:Int64()
            IN.anim1Duration = Type:Float()
            IN.anim2Duration = Type:Float()

            OUT.anim1Progress = Type:Float()
            OUT.anim2Progress = Type:Float()
        end

        function run(IN,OUT)
            if GLOBAL.startTick == 0 then
                GLOBAL.startTick = IN.ticker
            end

            local elapsedTime = IN.ticker - GLOBAL.startTick
            -- ticker from TimerNode is in microseconds, our animation timestamps are in seconds, conversion is needed
            elapsedTime = elapsedTime / 1000000

            -- play both animations right away
            OUT.anim1Progress = elapsedTime / IN.anim1Duration
            OUT.anim2Progress = elapsedTime / IN.anim2Duration
        end
        )";

        const auto script = m_logicEngine.createLuaScript(scriptSrc);
        const auto scriptScalarToVec1 = m_logicEngine.createLuaScript(ScriptScalarToVecSrc);
        const auto scriptScalarToVec2 = m_logicEngine.createLuaScript(ScriptScalarToVecSrc);

        const auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node);

        script->getInputs()->getChild("anim1Duration")->set(*m_animation1->getOutputs()->getChild("duration")->get<float>());
        script->getInputs()->getChild("anim2Duration")->set(*m_animation2->getOutputs()->getChild("duration")->get<float>());

        // controlScript -> anim1/channel -> scalarToVec -> nodeRotation
        //               -> anim2/channel -> scalarToVec -> nodeScaling
        m_logicEngine.link(*script->getOutputs()->getChild("anim1Progress"), *m_animation1->getInputs()->getChild("progress"));
        m_logicEngine.link(*script->getOutputs()->getChild("anim2Progress"), *m_animation2->getInputs()->getChild("progress"));
        // link anim outputs to node bindings via helper to convert scalar to vec3
        m_logicEngine.link(*m_animation1->getOutputs()->getChild("channel"), *scriptScalarToVec1->getInputs()->getChild("scalar"));
        m_logicEngine.link(*m_animation2->getOutputs()->getChild("channel"), *scriptScalarToVec2->getInputs()->getChild("scalar"));
        m_logicEngine.link(*scriptScalarToVec1->getOutputs()->getChild("vec"), *nodeBinding->getInputs()->getChild("translation"));
        m_logicEngine.link(*scriptScalarToVec2->getOutputs()->getChild("vec"), *nodeBinding->getInputs()->getChild("rotation"));

        // link timer
        m_logicEngine.link(*m_timer->getOutputs()->getChild("ticker_us"), *script->getInputs()->getChild("ticker"));

        setTickerAndUpdate(0);
        expectNodeValues(0.f, 0.f);

        // loop for at most 2 secs
        // break when one of animated outputs reaches 0.1 sec progress
        for (int i = 0; i < 100; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds{ 20 });
            m_logicEngine.update();

            vec3f vals;
            m_node->getTranslation(vals);
            if (vals[0] >= 0.1f)
                break;
        }

        vec3f vals;
        m_node->getTranslation(vals);
        EXPECT_GE(vals[0], 0.1f);
        EXPECT_GE(vals[1], 0.1f);
        EXPECT_GE(vals[2], 0.1f);
        m_node->getRotation(vals);
        EXPECT_GE(vals[0], 0.1f);
        EXPECT_GE(vals[1], 0.1f);
        EXPECT_GE(vals[2], 0.1f);
    }

    TEST_F(ALogicEngine_Animations, ScriptsControllingAnimationsLinkedToScene_WithWeakLinkedDuration)
    {
        const auto scriptSrc = R"(
        function init()
            GLOBAL.startTick = 0
        end

        function interface(IN,OUT)
            IN.ticker = Type:Int64()
            IN.animDuration = Type:Float()

            OUT.animProgress = Type:Float()
        end

        function run(IN,OUT)
            if GLOBAL.startTick == 0 then
                GLOBAL.startTick = IN.ticker
            end

            local elapsedTime = IN.ticker - GLOBAL.startTick
            -- ticker from TimerNode is in microseconds, our animation timestamps are in seconds, conversion is needed
            elapsedTime = elapsedTime / 1000000

            OUT.animProgress = elapsedTime / IN.animDuration
        end
        )";

        const auto script = m_logicEngine.createLuaScript(scriptSrc);
        const auto scriptScalarToVec = m_logicEngine.createLuaScript(ScriptScalarToVecSrc);

        const auto nodeBinding = m_logicEngine.createRamsesNodeBinding(*m_node);

        // weak link duration - weak because it goes against the data flow direction
        m_logicEngine.linkWeak(*m_animation1->getOutputs()->getChild("duration"), *script->getInputs()->getChild("animDuration"));

        // link animation to control script and to destination node property
        m_logicEngine.link(*script->getOutputs()->getChild("animProgress"), *m_animation1->getInputs()->getChild("progress"));
        m_logicEngine.link(*m_animation1->getOutputs()->getChild("channel"), *scriptScalarToVec->getInputs()->getChild("scalar"));
        m_logicEngine.link(*scriptScalarToVec->getOutputs()->getChild("vec"), *nodeBinding->getInputs()->getChild("translation"));

        // link timer
        m_logicEngine.link(*m_timer->getOutputs()->getChild("ticker_us"), *script->getInputs()->getChild("ticker"));

        // weak link limitation is undefined value during 1st update, we need to add extra update before expecting valid output
        setTickerAndUpdate(1); // don't use 0 because it triggers timer auto generated time

        // check begin-middle-end
        setTickerAndUpdate(1); // don't use 0 because it triggers timer auto generated time
        expectNodeValues(0.f, 0.f);
        setTickerAndUpdate(500000);
        expectNodeValues(0.5f, 0.f);
        setTickerAndUpdate(1000000);
        expectNodeValues(1.f, 0.f);
    }
}
