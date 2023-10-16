//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gmock/gmock.h>
#include "LogicEngineTest_Base.h"
#include "ramses/client/logic/Property.h"
#include <numeric>

namespace ramses::internal
{
    class ALogicEngine_UpdateReport : public ALogicEngine
    {
    protected:
        static void expectReportContainsExecutedNodes(const LogicEngineReport& report, std::vector<LogicNode*> expectedNodes)
        {
            const auto& executedNodes = report.getNodesExecuted();
            ASSERT_EQ(executedNodes.size(), expectedNodes.size());
            for (size_t i = 0u; i < executedNodes.size(); ++i)
            {
                EXPECT_EQ(executedNodes[i].first, expectedNodes[i]);
                // measured time cannot be consistently tested, in some test runs the time measured can be reported as 0
                // (fast release builds and/or not fine-grained enough clock)
                //EXPECT_TRUE(executedNodes[i].second.count() > 0);
            }
        }
    };

    TEST_F(ALogicEngine_UpdateReport, UpdateReportIsEmptyIfDisabledAndStatisticsAlsoDisabled)
    {
        // statistics are implicitly disabled
        m_logicEngine->setStatisticsLoggingRate(0u);

        constexpr auto scriptSource = R"(
            function interface(IN,OUT)
                IN.param = Type:Int32()
            end
            function run(IN,OUT)
            end
        )";

        auto node1 = m_logicEngine->createLuaScript(scriptSource);
        auto node2 = m_logicEngine->createLuaScript(scriptSource);
        auto node3 = m_logicEngine->createLuaScript(scriptSource);

        // first enable update reporting to fill some data
        m_logicEngine->enableUpdateReport(true);
        EXPECT_TRUE(m_logicEngine->update());
        EXPECT_FALSE(m_logicEngine->getLastUpdateReport().getNodesExecuted().empty());

        // make nodes dirty
        node1->getInputs()->getChild(0u)->set(13);
        node2->getInputs()->getChild(0u)->set(13);
        node3->getInputs()->getChild(0u)->set(13);

        // disable reporting and expect empty
        m_logicEngine->enableUpdateReport(false);
        EXPECT_TRUE(m_logicEngine->update());
        const auto report = m_logicEngine->getLastUpdateReport();
        EXPECT_TRUE(report.getNodesExecuted().empty());
        EXPECT_TRUE(report.getNodesSkippedExecution().empty());
        EXPECT_EQ(report.getTopologySortExecutionTime().count(), 0);
        EXPECT_EQ(report.getTotalUpdateExecutionTime().count(), 0);
        EXPECT_EQ(report.getTotalLinkActivations(), 0u);
    }

    TEST_F(ALogicEngine_UpdateReport, UpdateReportContainsUpdatedAndNotUpdatedNodes)
    {
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

        // link nodes so order of execution is deterministic
        m_logicEngine->link(*node1->getOutputs()->getChild(0u), *node2->getInputs()->getChild(0u));
        m_logicEngine->link(*node2->getOutputs()->getChild(0u), *node3->getInputs()->getChild(0u));

        m_logicEngine->enableUpdateReport(true);

        // all updated
        EXPECT_TRUE(m_logicEngine->update());
        {
            const auto report = m_logicEngine->getLastUpdateReport();
            expectReportContainsExecutedNodes(report, { node1, node2, node3 });
            EXPECT_TRUE(report.getNodesSkippedExecution().empty());
        }

        // none updated
        EXPECT_TRUE(m_logicEngine->update());
        {
            const auto report = m_logicEngine->getLastUpdateReport();
            EXPECT_TRUE(report.getNodesExecuted().empty());
            EXPECT_THAT(report.getNodesSkippedExecution(), ::testing::ElementsAre(node1, node2, node3));
        }

        // re-link to trigger dirty
        m_logicEngine->unlink(*node2->getOutputs()->getChild(0u), *node3->getInputs()->getChild(0u));
        m_logicEngine->link(*node2->getOutputs()->getChild(0u), *node3->getInputs()->getChild(0u));
        // node2 and node3 updated
        EXPECT_TRUE(m_logicEngine->update());
        {
            const auto report = m_logicEngine->getLastUpdateReport();
            expectReportContainsExecutedNodes(report, { node2, node3 });
            EXPECT_THAT(report.getNodesSkippedExecution(), ::testing::ElementsAre(node1));
        }
    }

    TEST_F(ALogicEngine_UpdateReport, UpdateReportCanBeStoredInContainer)
    {
        constexpr auto scriptSource = R"(
            function interface(IN,OUT)
                IN.param = Type:Int32()
            end
            function run(IN,OUT)
            end
        )";

        auto node1 = m_logicEngine->createLuaScript(scriptSource);
        auto node2 = m_logicEngine->createLuaScript(scriptSource);

        // first enable update reporting to fill some data
        m_logicEngine->enableUpdateReport(true);
        EXPECT_TRUE(m_logicEngine->update());
        auto report1 = m_logicEngine->getLastUpdateReport();

        EXPECT_TRUE(m_logicEngine->update());
        auto report2 = m_logicEngine->getLastUpdateReport();

        node1->getInputs()->getChild(0u)->set(13);
        node2->getInputs()->getChild(0u)->set(13);
        EXPECT_TRUE(m_logicEngine->update());
        auto report3 = m_logicEngine->getLastUpdateReport();

        std::vector<LogicEngineReport> reports;
        reports.resize(2u);
        reports[0] = std::move(report1);
        reports[1] = std::move(report2);
        reports.push_back(std::move(report3));

        EXPECT_EQ(2u, reports[0].getNodesExecuted().size());
        EXPECT_EQ(2u, reports[1].getNodesSkippedExecution().size());
        EXPECT_EQ(2u, reports[2].getNodesExecuted().size());

        auto reportsOther = std::move(reports);
        EXPECT_EQ(2u, reportsOther[0].getNodesExecuted().size());
        EXPECT_EQ(2u, reportsOther[1].getNodesSkippedExecution().size());
        EXPECT_EQ(2u, reportsOther[2].getNodesExecuted().size());
    }

    TEST_F(ALogicEngine_UpdateReport, UpdateReportHasExecutionTimings)
    {
        constexpr auto slowScriptSource = R"(
            function interface(IN,OUT)
            end
            function run(IN,OUT)
                local str="a"
                for i=0,1000 do
                    str = str .. "a"
                end
            end
        )";

        m_logicEngine->createLuaScript(slowScriptSource);
        m_logicEngine->createLuaScript(slowScriptSource);
        m_logicEngine->createLuaScript(slowScriptSource);

        m_logicEngine->enableUpdateReport(true);

        EXPECT_TRUE(m_logicEngine->update());
        {
            const auto report = m_logicEngine->getLastUpdateReport();
            EXPECT_EQ(3u, report.getNodesExecuted().size());
            EXPECT_TRUE(report.getNodesSkippedExecution().empty());

            // expect that total update time >= individual times summed up
            const auto nodesUpdatesTime = std::accumulate(report.getNodesExecuted().cbegin(), report.getNodesExecuted().cend(), std::chrono::microseconds{ 0u },
                [](auto val, const LogicEngineReport::LogicNodeTimed& n) { return val + n.second; });
            EXPECT_GE(report.getTotalUpdateExecutionTime(), report.getTopologySortExecutionTime() + nodesUpdatesTime);
        }
    }

    TEST_F(ALogicEngine_UpdateReport, HasLinkActivations)
    {
        constexpr auto scriptSource = R"(
            function interface(IN,OUT)
                IN.ints = {
                    int1 = Type:Int32(),
                    int2 = Type:Int32()
                }
                OUT.ints = {
                    int1 = Type:Int32(),
                    int2 = Type:Int32()
                }
            end
            function run(IN,OUT)
                OUT.ints = IN.ints
            end
        )";

        LuaScript* s1 = m_logicEngine->createLuaScript(scriptSource);
        LuaScript* s2 = m_logicEngine->createLuaScript(scriptSource);

        m_logicEngine->link(*s1->getOutputs()->getChild("ints")->getChild("int1"), *s2->getInputs()->getChild("ints")->getChild("int1"));
        m_logicEngine->link(*s1->getOutputs()->getChild("ints")->getChild("int2"), *s2->getInputs()->getChild("ints")->getChild("int2"));

        m_logicEngine->enableUpdateReport(true);

        // set non-default data
        s1->getInputs()->getChild("ints")->getChild("int1")->set<int32_t>(5);
        s1->getInputs()->getChild("ints")->getChild("int2")->set<int32_t>(5);
        EXPECT_TRUE(m_logicEngine->update());
        {
            const auto report = m_logicEngine->getLastUpdateReport();
            EXPECT_EQ(2u, report.getTotalLinkActivations());
        }
        EXPECT_TRUE(m_logicEngine->update());
        {
            const auto report = m_logicEngine->getLastUpdateReport();
            EXPECT_EQ(0u, report.getTotalLinkActivations());
        }
        s1->getInputs()->getChild("ints")->getChild("int1")->set<int32_t>(6);
        EXPECT_TRUE(m_logicEngine->update());
        {
            const auto report = m_logicEngine->getLastUpdateReport();
            EXPECT_EQ(1u, report.getTotalLinkActivations());
        }
        s1->getInputs()->getChild("ints")->getChild("int1")->set<int32_t>(7);
        s1->getInputs()->getChild("ints")->getChild("int2")->set<int32_t>(7);
        EXPECT_TRUE(m_logicEngine->update());
        {
            const auto report = m_logicEngine->getLastUpdateReport();
            EXPECT_EQ(2u, report.getTotalLinkActivations());
        }
    }

    TEST_F(ALogicEngine_UpdateReport, UpdateReportCanBeRetrievedNextSuccessUpdateAfterFailedUpdate)
    {
        constexpr auto scriptSource = R"(
            function interface(IN,OUT)
                IN.param = Type:Int32()
                OUT.param = Type:Int32()
            end
            function run(IN,OUT)
                if IN.param == 33 then error() end
                OUT.param = IN.param
            end
        )";

        LuaConfig config;
        config.addStandardModuleDependency(EStandardModule::Base);
        auto node1 = m_logicEngine->createLuaScript(scriptSource, config);
        auto node2 = m_logicEngine->createLuaScript(scriptSource, config);
        auto node3 = m_logicEngine->createLuaScript(scriptSource, config);

        // link nodes so order of execution is deterministic
        m_logicEngine->link(*node1->getOutputs()->getChild(0u), *node2->getInputs()->getChild(0u));
        m_logicEngine->link(*node2->getOutputs()->getChild(0u), *node3->getInputs()->getChild(0u));

        m_logicEngine->enableUpdateReport(true);

        // make it fail
        node1->getInputs()->getChild(0u)->set(33);
        EXPECT_FALSE(m_logicEngine->update());

        // all updated
        node1->getInputs()->getChild(0u)->set(1);
        EXPECT_TRUE(m_logicEngine->update());
        {
            const auto report = m_logicEngine->getLastUpdateReport();
            expectReportContainsExecutedNodes(report, { node1, node2, node3 });
            EXPECT_TRUE(report.getNodesSkippedExecution().empty());

            // expect that total update time >= individual times summed up
            const auto nodesUpdatesTime = std::accumulate(report.getNodesExecuted().cbegin(), report.getNodesExecuted().cend(), std::chrono::microseconds{ 0u },
                [](auto val, const LogicEngineReport::LogicNodeTimed& n) { return val + n.second; });
            EXPECT_GE(report.getTotalUpdateExecutionTime(), report.getTopologySortExecutionTime() + nodesUpdatesTime);
        }
    }
}
