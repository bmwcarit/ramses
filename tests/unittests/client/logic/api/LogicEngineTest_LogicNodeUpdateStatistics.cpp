//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <chrono>
#include <random>

#include "gtest/gtest.h"
#include "LogicEngineTest_Base.h"
#include "ramses/client/logic/Property.h"
#include "internal/logic/UpdateReport.h"
#include "internal/logic/LogicNodeUpdateStatistics.h"

#include "LogTestUtils.h"

namespace ramses::internal
{
    class ALogicEngine_LogicObjectStatistics : public ALogicEngine
    {
    protected:
        std::vector<ELogLevel> m_logTypes;
        std::vector<std::string>     m_logMessages;
        ScopedLogContextLevel m_logCollector{CONTEXT_PERIODIC, ELogLevel::Info, [this](ELogLevel type, std::string_view message)
            {
                m_logTypes.emplace_back(type);
                m_logMessages.emplace_back(message);
            }};
    };

    TEST_F(ALogicEngine_LogicObjectStatistics, verifyLogsWithNoNode)
    {
        LogicNodeUpdateStatistics statistics;
        UpdateReport report;

        statistics.setLoggingRate(1u);
        statistics.collect(report, 0);
        statistics.calculateAndLog();

        ASSERT_EQ(5u, m_logMessages.size());
        EXPECT_TRUE(m_logMessages[0].find("First Statistics Log") != std::string::npos);
        EXPECT_TRUE(m_logMessages[1].find("Update Execution time (min/max/avg): 0/0/0 [u]sec") != std::string::npos);
        EXPECT_TRUE(m_logMessages[2].find("Time between Update calls cannot be measured with loggingRate = 1") != std::string::npos);
        EXPECT_TRUE(m_logMessages[3].find("Nodes Executed (min/max/avg): 0%/0%/0% (0/0/0) of 0 nodes total") != std::string::npos);
        EXPECT_TRUE(m_logMessages[4].find("Activated links (min/max/avg): 0/0/0") != std::string::npos);
    }

    TEST_F(ALogicEngine_LogicObjectStatistics, verifyLogsForOneNode)
    {
        LogicNodeUpdateStatistics statistics;
        UpdateReport report;
        statistics.setLoggingRate(1u);

        auto* node1 = m_logicEngine->createTimerNode("test node1");
        auto& dummyNodes = const_cast<UpdateReport::LogicNodesTimed&>(report.getNodesExecuted());
        dummyNodes.emplace_back(&static_cast<LogicNode*>(node1)->impl(), std::chrono::microseconds(3));

        statistics.collect(report, dummyNodes.size());
        statistics.calculateAndLog();

        ASSERT_EQ(6u, m_logMessages.size());
        EXPECT_TRUE(m_logMessages[0].find("First Statistics Log") != std::string::npos);
        EXPECT_TRUE(m_logMessages[1].find("Update Execution time (min/max/avg): 0/0/0 [u]sec") != std::string::npos);
        EXPECT_TRUE(m_logMessages[2].find("Time between Update calls cannot be measured with loggingRate = 1") != std::string::npos);
        EXPECT_TRUE(m_logMessages[3].find("Nodes Executed (min/max/avg): 0%/0%/0% (0/0/0) of 1 nodes total") != std::string::npos);
        EXPECT_TRUE(m_logMessages[4].find("Activated links (min/max/avg): 0/0/0") != std::string::npos);
        EXPECT_TRUE(m_logMessages[5].find("Slowest nodes [name:time_us]: [test node1:3]") != std::string::npos);
    }

    TEST_F(ALogicEngine_LogicObjectStatistics, verifyLogsForOneNodeWithZeroExecutionTime)
    {
        LogicNodeUpdateStatistics statistics;
        UpdateReport report;
        statistics.setLoggingRate(1u);

        auto* node1 = m_logicEngine->createTimerNode("test node1");
        auto& dummyNodes = const_cast<UpdateReport::LogicNodesTimed&>(report.getNodesExecuted());
        dummyNodes.emplace_back(&static_cast<LogicNode*>(node1)->impl(), std::chrono::microseconds(0));

        statistics.collect(report, dummyNodes.size());
        statistics.calculateAndLog();

        ASSERT_EQ(6u, m_logMessages.size());
        EXPECT_TRUE(m_logMessages[0].find("First Statistics Log") != std::string::npos);
        EXPECT_TRUE(m_logMessages[1].find("Update Execution time (min/max/avg): 0/0/0 [u]sec") != std::string::npos);
        EXPECT_TRUE(m_logMessages[2].find("Time between Update calls cannot be measured with loggingRate = 1") != std::string::npos);
        EXPECT_TRUE(m_logMessages[3].find("Nodes Executed (min/max/avg): 0%/0%/0% (0/0/0) of 1 nodes total") != std::string::npos);
        EXPECT_TRUE(m_logMessages[4].find("Activated links (min/max/avg): 0/0/0") != std::string::npos);
        EXPECT_TRUE(m_logMessages[5].find("Slowest nodes [name:time_us]: [test node1:0]") != std::string::npos);
    }

    TEST_F(ALogicEngine_LogicObjectStatistics, verifyLogsForTwoNodes)
    {
        LogicNodeUpdateStatistics statistics;
        UpdateReport report;
        statistics.setLoggingRate(1u);

        auto* node1 = m_logicEngine->createTimerNode("test node1");
        auto* node2 = m_logicEngine->createTimerNode("test node2");
        auto& dummyNodes = const_cast<UpdateReport::LogicNodesTimed&>(report.getNodesExecuted());
        dummyNodes.emplace_back(&static_cast<LogicNode*>(node1)->impl(), std::chrono::microseconds(3));
        dummyNodes.emplace_back(&static_cast<LogicNode*>(node2)->impl(), std::chrono::microseconds(10));

        statistics.collect(report, dummyNodes.size());
        statistics.calculateAndLog();

        ASSERT_EQ(6u, m_logMessages.size());
        EXPECT_TRUE(m_logMessages[0].find("First Statistics Log") != std::string::npos);
        EXPECT_TRUE(m_logMessages[1].find("Update Execution time (min/max/avg): 0/0/0 [u]sec") != std::string::npos);
        EXPECT_TRUE(m_logMessages[2].find("Time between Update calls cannot be measured with loggingRate = 1") != std::string::npos);
        EXPECT_TRUE(m_logMessages[3].find("Nodes Executed (min/max/avg): 0%/0%/0% (0/0/0) of 2 nodes total") != std::string::npos);
        EXPECT_TRUE(m_logMessages[4].find("Activated links (min/max/avg): 0/0/0") != std::string::npos);
        EXPECT_TRUE(m_logMessages[5].find("Slowest nodes [name:time_us]: [test node2:10] [test node1:3]") != std::string::npos);
    }

    TEST_F(ALogicEngine_LogicObjectStatistics, verifyLogsForFiveNodesWithSameTime)
    {
        LogicNodeUpdateStatistics statistics;
        UpdateReport report;
        statistics.setLoggingRate(1u);

        auto* node1 = m_logicEngine->createTimerNode("test node1");
        auto* node2 = m_logicEngine->createTimerNode("test node2");
        auto* node3 = m_logicEngine->createTimerNode("test node3");
        auto* node4 = m_logicEngine->createTimerNode("test node4");
        auto* node5 = m_logicEngine->createTimerNode("test node5");
        auto& dummyNodes = const_cast<UpdateReport::LogicNodesTimed&>(report.getNodesExecuted());
        dummyNodes.emplace_back(&static_cast<LogicNode*>(node1)->impl(), std::chrono::microseconds(10));
        dummyNodes.emplace_back(&static_cast<LogicNode*>(node2)->impl(), std::chrono::microseconds(10));
        dummyNodes.emplace_back(&static_cast<LogicNode*>(node3)->impl(), std::chrono::microseconds(10));
        dummyNodes.emplace_back(&static_cast<LogicNode*>(node4)->impl(), std::chrono::microseconds(10));
        dummyNodes.emplace_back(&static_cast<LogicNode*>(node5)->impl(), std::chrono::microseconds(10));

        statistics.collect(report, dummyNodes.size());
        statistics.calculateAndLog();

        ASSERT_EQ(6u, m_logMessages.size());
        EXPECT_TRUE(m_logMessages[0].find("First Statistics Log") != std::string::npos);
        EXPECT_TRUE(m_logMessages[1].find("Update Execution time (min/max/avg): 0/0/0 [u]sec") != std::string::npos);
        EXPECT_TRUE(m_logMessages[2].find("Time between Update calls cannot be measured with loggingRate = 1") != std::string::npos);
        EXPECT_TRUE(m_logMessages[3].find("Nodes Executed (min/max/avg): 0%/0%/0% (0/0/0) of 5 nodes total") != std::string::npos);
        EXPECT_TRUE(m_logMessages[4].find("Activated links (min/max/avg): 0/0/0") != std::string::npos);
        EXPECT_TRUE(m_logMessages[5].find("Slowest nodes [name:time_us]: [test node1:10] [test node2:10] [test node3:10] [test node4:10] [test node5:10]") != std::string::npos);
    }

    TEST_F(ALogicEngine_LogicObjectStatistics, verifyLogsForOneHundredNodesWithRandomTime)
    {
        LogicNodeUpdateStatistics statistics;
        UpdateReport report;
        statistics.setLoggingRate(1u);

        std::string nodeBaseName = "test node";
        auto& dummyNodes = const_cast<UpdateReport::LogicNodesTimed&>(report.getNodesExecuted());

        // Generate random numbers from 1 to 100
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> distribution(1, 100);
        for (int32_t i = 0; i < 100; ++i)
        {
            auto* timerNode = m_logicEngine->createTimerNode(nodeBaseName + std::to_string(i));
            int randomTime = distribution(rng);
            switch (i)
            {
            case 10:
                dummyNodes.emplace_back(&static_cast<LogicNode*>(timerNode)->impl(), std::chrono::microseconds(609));
                break;
            case 35:
                dummyNodes.emplace_back(&static_cast<LogicNode*>(timerNode)->impl(), std::chrono::microseconds(721));
                break;
            case 58:
                dummyNodes.emplace_back(&static_cast<LogicNode*>(timerNode)->impl(), std::chrono::microseconds(831));
                break;
            case 72:
                dummyNodes.emplace_back(&static_cast<LogicNode*>(timerNode)->impl(), std::chrono::microseconds(101));
                break;
            case 93:
                dummyNodes.emplace_back(&static_cast<LogicNode*>(timerNode)->impl(), std::chrono::microseconds(616));
                break;
            default:
                dummyNodes.emplace_back(&static_cast<LogicNode*>(timerNode)->impl(), std::chrono::microseconds(randomTime));
                break;
            }
        }

        statistics.collect(report, dummyNodes.size());
        statistics.calculateAndLog();

        ASSERT_EQ(6u, m_logMessages.size());
        EXPECT_TRUE(m_logMessages[0].find("First Statistics Log") != std::string::npos);
        EXPECT_TRUE(m_logMessages[1].find("Update Execution time (min/max/avg): 0/0/0 [u]sec") != std::string::npos);
        EXPECT_TRUE(m_logMessages[2].find("Time between Update calls cannot be measured with loggingRate = 1") != std::string::npos);
        EXPECT_TRUE(m_logMessages[3].find("Nodes Executed (min/max/avg): 0%/0%/0% (0/0/0) of 100 nodes total") != std::string::npos);
        EXPECT_TRUE(m_logMessages[4].find("Activated links (min/max/avg): 0/0/0") != std::string::npos);
        EXPECT_TRUE(m_logMessages[5].find("Slowest nodes [name:time_us]: [test node58:831] [test node35:721] [test node93:616] [test node10:609] [test node72:101]") != std::string::npos);
    }

    TEST_F(ALogicEngine_LogicObjectStatistics, verifyLogsForOneHundredNodesWithRandomTimeInMultipleCollection)
    {
        LogicNodeUpdateStatistics statistics;
        UpdateReport report;
        statistics.setLoggingRate(1u);

        std::string nodeBaseName = "test node";
        auto& dummyNodes = const_cast<UpdateReport::LogicNodesTimed&>(report.getNodesExecuted());

        // Generate random numbers from 1 to 100
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> distribution(1, 100);
        int32_t curNodeCount = 0;
        while (curNodeCount < 100)
        {
            auto* timerNode = m_logicEngine->createTimerNode(nodeBaseName + std::to_string(curNodeCount++));
            int randomTime = distribution(rng);
            dummyNodes.emplace_back(&static_cast<LogicNode*>(timerNode)->impl(), std::chrono::microseconds(randomTime));
        }

        // Collect 3 times
        for (int j = 1; j <= 3; ++j)
        {
            // Change time for current nodes and make 1 slow
            for (int i = 0; i < 100; ++i)
            {
                int randomTime = distribution(rng);
                dummyNodes[i].second = std::chrono::microseconds(randomTime);
            }
            dummyNodes[30 * j].second = std::chrono::microseconds(600 + j);

            // add a new slow node
            auto* timerNode = m_logicEngine->createTimerNode(nodeBaseName + std::to_string(curNodeCount++));
            dummyNodes.emplace_back(&static_cast<LogicNode*>(timerNode)->impl(), std::chrono::microseconds(500 + j));

            statistics.collect(report, dummyNodes.size());
        }

        statistics.calculateAndLog();
        ASSERT_EQ(6u, m_logMessages.size());
        EXPECT_TRUE(m_logMessages[0].find("First Statistics Log") != std::string::npos);
        EXPECT_TRUE(m_logMessages[1].find("Update Execution time (min/max/avg): 0/0/0 [u]sec") != std::string::npos);
        EXPECT_TRUE(m_logMessages[2].find("Time between Update calls (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[3].find("Nodes Executed (min/max/avg): 0%/0%/0% (0/0/0) of 103 nodes total") != std::string::npos);
        EXPECT_TRUE(m_logMessages[4].find("Activated links (min/max/avg): 0/0/0") != std::string::npos);
        EXPECT_TRUE(m_logMessages[5].find("Slowest nodes [name:time_us]: [test node90:603] [test node60:602] [test node30:601] [test node102:503] [test node101:502]") != std::string::npos);
    }

    TEST_F(ALogicEngine_LogicObjectStatistics, LogsAllMessages)
    {
        m_logicEngine->setStatisticsLoggingRate(2u, EStatisticsLogMode::Detailed);

        constexpr auto scriptSource = R"(
            function interface(IN,OUT)
                IN.param = Type:Int32()
                OUT.param = Type:Int32()
            end
            function run(IN,OUT)
                OUT.param = IN.param
            end
        )";

        auto node1 = m_logicEngine->createLuaScript(scriptSource);
        auto node2 = m_logicEngine->createLuaScript(scriptSource);
        auto node3 = m_logicEngine->createLuaScript(scriptSource);
        auto node4 = m_logicEngine->createLuaScript(scriptSource);
        auto node5 = m_logicEngine->createLuaScript(scriptSource);

        m_logicEngine->link(*node1->getOutputs()->getChild(0u), *node2->getInputs()->getChild(0u));
        m_logicEngine->link(*node2->getOutputs()->getChild(0u), *node3->getInputs()->getChild(0u));
        m_logicEngine->link(*node3->getOutputs()->getChild(0u), *node4->getInputs()->getChild(0u));
        m_logicEngine->link(*node4->getOutputs()->getChild(0u), *node5->getInputs()->getChild(0u));

        for (int32_t i = 0; i < 4; ++i)
        {
            node1->getInputs()->getChild(0u)->set(i);
            EXPECT_TRUE(m_logicEngine->update());
        }

        // 6 log lines per frame and there are 2 frames
        ASSERT_EQ(12u, m_logMessages.size());

        EXPECT_TRUE(m_logMessages[0].find("First Statistics Log") != std::string::npos);
        EXPECT_TRUE(m_logMessages[1].find("Update Execution time (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[2].find("Time between Update calls (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[3].find("Nodes Executed (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[4].find("Activated links (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[5].find("Slowest nodes [name:time_us]:") != std::string::npos);
        EXPECT_TRUE(m_logMessages[6].find("Time since last log:") != std::string::npos);
        EXPECT_TRUE(m_logMessages[7].find("Update Execution time (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[8].find("Time between Update calls (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[9].find("Nodes Executed (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[10].find("Activated links (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[11].find("Slowest nodes [name:time_us]:") != std::string::npos);
    }

    TEST_F(ALogicEngine_LogicObjectStatistics, LogsMessagesWithCompactStatistics)
    {
        m_logicEngine->setStatisticsLoggingRate(1u, EStatisticsLogMode::Compact);

        constexpr auto scriptSource = R"(
            function interface(IN,OUT)
                IN.param = Type:Int32()
                OUT.param = Type:Int32()
            end
            function run(IN,OUT)
                OUT.param = IN.param
            end
        )";

        m_logicEngine->createLuaScript(scriptSource);
        EXPECT_TRUE(m_logicEngine->update());

        ASSERT_EQ(5u, m_logMessages.size());

        EXPECT_TRUE(m_logMessages[0].find("First Statistics Log") != std::string::npos);
        EXPECT_TRUE(m_logMessages[1].find("Update Execution time (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[2].find("Time between Update calls cannot be measured with loggingRate = 1") != std::string::npos);
        EXPECT_TRUE(m_logMessages[3].find("Nodes Executed (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[4].find("Activated links (min/max/avg):") != std::string::npos);
    }

    TEST_F(ALogicEngine_LogicObjectStatistics, LogsMessagesWithDetailedStatistics)
    {
        m_logicEngine->setStatisticsLoggingRate(1u, EStatisticsLogMode::Detailed);

        constexpr auto scriptSource = R"(
            function interface(IN,OUT)
                IN.param = Type:Int32()
                OUT.param = Type:Int32()
            end
            function run(IN,OUT)
                OUT.param = IN.param
            end
        )";

        m_logicEngine->createLuaScript(scriptSource);
        EXPECT_TRUE(m_logicEngine->update());

        ASSERT_EQ(6u, m_logMessages.size());

        EXPECT_TRUE(m_logMessages[0].find("First Statistics Log") != std::string::npos);
        EXPECT_TRUE(m_logMessages[1].find("Update Execution time (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[2].find("Time between Update calls cannot be measured with loggingRate = 1") != std::string::npos);
        EXPECT_TRUE(m_logMessages[3].find("Nodes Executed (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[4].find("Activated links (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[5].find("Slowest nodes [name:time_us]:") != std::string::npos);
    }

    TEST_F(ALogicEngine_LogicObjectStatistics, NoSlowestNodeWithUpdateReportDisabled)
    {
        m_logicEngine->setStatisticsLoggingRate(2u);
        m_logicEngine->createTimerNode("test timer node");

        m_logicEngine->update();
        EXPECT_TRUE(m_logMessages.empty());

        m_logicEngine->update();
        ASSERT_EQ(5u, m_logMessages.size());

        EXPECT_TRUE(m_logMessages[0].find("First Statistics Log") != std::string::npos);
        EXPECT_TRUE(m_logMessages[1].find("Update Execution time (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[2].find("Time between Update calls (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[3].find("Nodes Executed (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[4].find("Activated links (min/max/avg):") != std::string::npos);
    }

    TEST_F(ALogicEngine_LogicObjectStatistics, DoNotLogSameNode)
    {
        m_logicEngine->setStatisticsLoggingRate(2u, EStatisticsLogMode::Detailed);
        m_logicEngine->createTimerNode("test timer node");

        m_logicEngine->update();
        EXPECT_TRUE(m_logMessages.empty());

        m_logicEngine->update();
        ASSERT_EQ(6u, m_logMessages.size());

        EXPECT_TRUE(m_logMessages[0].find("First Statistics Log") != std::string::npos);
        EXPECT_TRUE(m_logMessages[1].find("Update Execution time (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[2].find("Time between Update calls (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[3].find("Nodes Executed (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[4].find("Activated links (min/max/avg):") != std::string::npos);
        EXPECT_TRUE(m_logMessages[5].find("Slowest nodes [name:time_us]:") != std::string::npos);

        size_t pos = m_logMessages[5].find("[test timer node:");
        EXPECT_TRUE(m_logMessages[5].find("[test timer node:", pos+1) == std::string::npos);
    }

    TEST_F(ALogicEngine_LogicObjectStatistics, NoLogsWhenLoggingRateZero)
    {
        m_logicEngine->setStatisticsLoggingRate(0u);

        for (int32_t i = 0; i < 4; ++i)
        {
            EXPECT_TRUE(m_logicEngine->update());
        }

        EXPECT_EQ(0u, m_logMessages.size());
    }

    TEST_F(ALogicEngine_LogicObjectStatistics, NoLogsWhenLoggingRateZeroAndDetailedStatistics)
    {
        m_logicEngine->setStatisticsLoggingRate(0u, EStatisticsLogMode::Detailed);
        m_logicEngine->createTimerNode("test timer node");

        for (int32_t i = 0; i < 4; ++i)
        {
            EXPECT_TRUE(m_logicEngine->update());
        }
        EXPECT_EQ(0u, m_logMessages.size());
    }

    TEST_F(ALogicEngine_LogicObjectStatistics, LogsAccordingToLoggingRate)
    {
        m_logicEngine->setStatisticsLoggingRate(2u);

        m_logicEngine->update();
        EXPECT_TRUE(m_logMessages.empty());

        m_logicEngine->update();
        EXPECT_EQ(5u ,m_logMessages.size());

        m_logicEngine->update();
        EXPECT_EQ(5u, m_logMessages.size());

        m_logicEngine->update();
        EXPECT_EQ(10u, m_logMessages.size());
    }

    TEST_F(ALogicEngine_LogicObjectStatistics, NoTimeBetweenUpdateCallsCalculationWithLoggingRateOne)
    {
        m_logicEngine->setStatisticsLoggingRate(1u);

        m_logicEngine->update();
        EXPECT_TRUE(m_logMessages[2].find("Time between Update calls cannot be measured with loggingRate = 1") != std::string::npos);

        m_logicEngine->update();
        EXPECT_TRUE(m_logMessages[7].find("Time between Update calls cannot be measured with loggingRate = 1") != std::string::npos);
    }
}
