//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "LogicEngineTest_Base.h"
#include "ramses-logic/Property.h"

#include "LogTestUtils.h"

namespace rlogic
{
    class ALogicEngine_LogicObjectStatistics : public ALogicEngine
    {
    protected:
        std::vector<ELogMessageType> m_logTypes;
        std::vector<std::string>     m_logMessages;
        ScopedLogContextLevel        m_logCollector{ELogMessageType::Debug, [this](ELogMessageType type, std::string_view message)
            {
                m_logTypes.emplace_back(type);
                m_logMessages.emplace_back(message);
            }};
    };

    TEST_F(ALogicEngine_LogicObjectStatistics, LogsAllMessages)
    {
        m_logicEngine.setStatisticsLoggingRate(2u);

        constexpr auto scriptSource = R"(
            function interface(IN,OUT)
                IN.param = Type:Int32()
                OUT.param = Type:Int32()
            end
            function run(IN,OUT)
                OUT.param = IN.param
            end
        )";

        auto node1 = m_logicEngine.createLuaScript(scriptSource);
        auto node2 = m_logicEngine.createLuaScript(scriptSource);
        auto node3 = m_logicEngine.createLuaScript(scriptSource);
        auto node4 = m_logicEngine.createLuaScript(scriptSource);
        auto node5 = m_logicEngine.createLuaScript(scriptSource);

        m_logicEngine.link(*node1->getOutputs()->getChild(0u), *node2->getInputs()->getChild(0u));
        m_logicEngine.link(*node2->getOutputs()->getChild(0u), *node3->getInputs()->getChild(0u));
        m_logicEngine.link(*node3->getOutputs()->getChild(0u), *node4->getInputs()->getChild(0u));
        m_logicEngine.link(*node4->getOutputs()->getChild(0u), *node5->getInputs()->getChild(0u));

        for (int32_t i = 0; i < 4; ++i)
        {
            node1->getInputs()->getChild(0u)->set(i);
            EXPECT_TRUE(m_logicEngine.update());
        }

        //5 log lines per frame and 2 frames
        EXPECT_EQ(10u, m_logMessages.size());

        EXPECT_TRUE(m_logMessages[0].find("First Statistics Log") != std::string::npos);
        EXPECT_TRUE(m_logMessages[1].find("Update Execution time (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[2].find("Time between Update calls (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[3].find("Nodes Executed (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[4].find("Activated links (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[5].find("Time since last log:") != std::string::npos);
        EXPECT_TRUE(m_logMessages[6].find("Update Execution time (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[7].find("Time between Update calls (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[8].find("Nodes Executed (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[9].find("Activated links (min/max/avg):") != std::string::npos);
    }

    TEST_F(ALogicEngine_LogicObjectStatistics, NoLogsWhenLoggingRateZero)
    {
        m_logicEngine.setStatisticsLoggingRate(0u);

        for (int32_t i = 0; i < 4; ++i)
        {
            EXPECT_TRUE(m_logicEngine.update());
        }

        EXPECT_EQ(0u, m_logMessages.size());
    }

    TEST_F(ALogicEngine_LogicObjectStatistics, OnlyLogWhenLogLevelSmallerOrEqualToLogVerbosityLimit)
    {
        m_logicEngine.setStatisticsLoggingRate(2u);
        m_logicEngine.setStatisticsLogLevel(ELogMessageType::Trace);

        for (int32_t i = 0; i < 4; ++i)
        {
            EXPECT_TRUE(m_logicEngine.update());
        }
        EXPECT_TRUE(m_logMessages.empty());

        m_logicEngine.setStatisticsLogLevel(ELogMessageType::Info);
        for (int32_t i = 0; i < 4; ++i)
        {
            EXPECT_TRUE(m_logicEngine.update());
        }
        EXPECT_FALSE(m_logMessages.empty());

        m_logMessages.clear();
        EXPECT_TRUE(m_logMessages.empty());

        m_logicEngine.setStatisticsLogLevel(ELogMessageType::Warn);
        for (int32_t i = 0; i < 4; ++i)
        {
            EXPECT_TRUE(m_logicEngine.update());
        }
        EXPECT_FALSE(m_logMessages.empty());
    }

    TEST_F(ALogicEngine_LogicObjectStatistics, LogsAccordingToLoggingRate)
    {
        m_logicEngine.setStatisticsLoggingRate(2u);

        m_logicEngine.update();
        EXPECT_TRUE(m_logMessages.empty());

        m_logicEngine.update();
        EXPECT_EQ(5u ,m_logMessages.size());

        m_logicEngine.update();
        EXPECT_EQ(5u, m_logMessages.size());

        m_logicEngine.update();
        EXPECT_EQ(10u, m_logMessages.size());
    }

    TEST_F(ALogicEngine_LogicObjectStatistics, NoTimeBetweenUpdateCallsCalculationWithLoggingRateOne)
    {
        m_logicEngine.setStatisticsLoggingRate(1u);

        m_logicEngine.update();
        EXPECT_TRUE(m_logMessages[2].find("Time between Update calls cannot be measured with loggingRate = 1") != std::string::npos);

        m_logicEngine.update();
        EXPECT_TRUE(m_logMessages[7].find("Time between Update calls cannot be measured with loggingRate = 1") != std::string::npos);
    }
}
