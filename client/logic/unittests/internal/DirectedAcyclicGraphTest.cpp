//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"

#include "internals/DirectedAcyclicGraph.h"

#include "LogicNodeDummy.h"

namespace rlogic::internal
{

    class ADirectedAcyclicGraph : public ::testing::Test
    {
    protected:
        DirectedAcyclicGraph m_graph;

        std::array<LogicNodeDummyImpl, 6> m_testNodes
        {
            LogicNodeDummyImpl { "1", false },
            LogicNodeDummyImpl { "2", false },
            LogicNodeDummyImpl { "3", false },
            LogicNodeDummyImpl { "4", false },
            LogicNodeDummyImpl { "5", false },
            LogicNodeDummyImpl { "6", false },
        };

        // For easier access in the tests
        LogicNodeDummyImpl& N1 { m_testNodes[0] };
        LogicNodeDummyImpl& N2 { m_testNodes[1] };
        LogicNodeDummyImpl& N3 { m_testNodes[2] };
        LogicNodeDummyImpl& N4 { m_testNodes[3] };
        LogicNodeDummyImpl& N5 { m_testNodes[4] };
        LogicNodeDummyImpl& N6 { m_testNodes[5] };

        std::unordered_map<const LogicNodeImpl*, size_t> m_ordering;

        void addTestNodesToGraph(size_t howMany)
        {
            for (size_t i = 0; i < howMany; ++i)
            {
                m_graph.addNode(m_testNodes[i]);
            }
        }

        void updateOrdering()
        {
            m_ordering.clear();

            const NodeVector orderedNodes = *m_graph.getTopologicallySortedNodes();
            for (size_t i = 0; i < orderedNodes.size(); ++i)
            {
                m_ordering.insert({orderedNodes[i], i});
            }
        }

        NodeVector getSortedTestNodes()
        {
            return *m_graph.getTopologicallySortedNodes();
        }

        size_t getRank(const LogicNodeImpl& node) const
        {
            return m_ordering.at(&node);
        }
    };

    TEST_F(ADirectedAcyclicGraph, ContainsOnlyNodesWhichAreExplicitlyAdded)
    {
        addTestNodesToGraph(1);

        EXPECT_TRUE(m_graph.containsNode(N1));
        EXPECT_FALSE(m_graph.containsNode(N2));
    }

    TEST_F(ADirectedAcyclicGraph, SingleNodeWithNoEdgesHasZeroDegree)
    {
        addTestNodesToGraph(1);

        EXPECT_EQ(0u, m_graph.getInDegree(N1));
        EXPECT_EQ(0u, m_graph.getOutDegree(N1));
    }

    TEST_F(ADirectedAcyclicGraph, EachLinkIncreasesOutAndInDegreeOfRespectiveNodesByExactlyOne)
    {
        addTestNodesToGraph(2);
        m_graph.addEdge(N1, N2);

        EXPECT_EQ(1u, m_graph.getOutDegree(N1));
        EXPECT_EQ(1u, m_graph.getInDegree(N2));

        m_graph.addEdge(N1, N2);

        EXPECT_EQ(2u, m_graph.getOutDegree(N1));
        EXPECT_EQ(2u, m_graph.getInDegree(N2));

        m_graph.removeEdge(N1, N2);
        EXPECT_EQ(1u, m_graph.getOutDegree(N1));
        EXPECT_EQ(1u, m_graph.getInDegree(N2));

        m_graph.removeEdge(N1, N2);
        EXPECT_EQ(0u, m_graph.getOutDegree(N1));
        EXPECT_EQ(0u, m_graph.getInDegree(N2));
    }

    TEST_F(ADirectedAcyclicGraph, Given_TriangleGraph_When_RemovingOneEdge_Then_InOutDegreesUpdatedCorrectly)
    {
        /*
        *       N2
        *     /    \
        *    /      \
        * N1 ------- N3
        */

        addTestNodesToGraph(3);
        m_graph.addEdge(N1, N2);
        m_graph.addEdge(N1, N3);
        m_graph.addEdge(N2, N3);

        EXPECT_EQ(2u, m_graph.getOutDegree(N1));
        EXPECT_EQ(0u, m_graph.getInDegree(N1));
        EXPECT_EQ(1u, m_graph.getOutDegree(N2));
        EXPECT_EQ(1u, m_graph.getInDegree(N2));
        EXPECT_EQ(0u, m_graph.getOutDegree(N3));
        EXPECT_EQ(2u, m_graph.getInDegree(N3));

        /*
        *       N2
        *         \
        *           \
        * N1 ------- N3
        */
        m_graph.removeEdge(N1, N2);

        EXPECT_EQ(1u, m_graph.getOutDegree(N1));
        EXPECT_EQ(0u, m_graph.getInDegree(N1));
        EXPECT_EQ(1u, m_graph.getOutDegree(N2));
        EXPECT_EQ(0u, m_graph.getInDegree(N2));
        EXPECT_EQ(0u, m_graph.getOutDegree(N3));
        EXPECT_EQ(2u, m_graph.getInDegree(N3));
    }

    TEST_F(ADirectedAcyclicGraph, RemovingSourceNode_ResultsInCorrectInAndOutDegreesOfTargetNode)
    {
        addTestNodesToGraph(2);
        m_graph.addEdge(N1, N2);
        m_graph.addEdge(N1, N2);

        m_graph.removeNode(N1);

        EXPECT_EQ(0u, m_graph.getOutDegree(N2));
        EXPECT_EQ(0u, m_graph.getInDegree(N2));

        // Confidence - check that removed node is really gone
        updateOrdering();
        EXPECT_THAT(getSortedTestNodes(), ::testing::ElementsAre(&N2));
    }

    TEST_F(ADirectedAcyclicGraph, ReportsNewEdgeOnlyTheFirstTimeTwoNodesAreConnected)
    {
        addTestNodesToGraph(2);
        EXPECT_TRUE(m_graph.addEdge(N1, N2));
        EXPECT_FALSE(m_graph.addEdge(N1, N2));
        EXPECT_FALSE(m_graph.addEdge(N1, N2));

        // Remove all three edges, and add again -> new edge reported again
        m_graph.removeEdge(N1, N2);
        m_graph.removeEdge(N1, N2);
        m_graph.removeEdge(N1, N2);
        EXPECT_TRUE(m_graph.addEdge(N1, N2));
    }

    TEST_F(ADirectedAcyclicGraph, RemovingTargetNode_ResultsInCorrectInAndOutDegreesOfSourceNode)
    {
        addTestNodesToGraph(2);
        m_graph.addEdge(N1, N2);
        m_graph.addEdge(N1, N2);

        m_graph.removeNode(N2);

        EXPECT_EQ(0u, m_graph.getOutDegree(N1));
        EXPECT_EQ(0u, m_graph.getInDegree(N1));

        // Confidence - check that removed node is really gone
        updateOrdering();
        EXPECT_THAT(getSortedTestNodes(), ::testing::ElementsAre(&N1));
    }

    TEST_F(ADirectedAcyclicGraph, RemovingMiddleNode_ResultsInCorrectInAndOutDegreeOfOtherNodes)
    {
        /*
        * N1 -- x2--  N2  -- x2-- N3
        */
        addTestNodesToGraph(3);
        m_graph.addEdge(N1, N2);
        m_graph.addEdge(N1, N2);
        m_graph.addEdge(N2, N3);
        m_graph.addEdge(N2, N3);

        /*
        * N1   (removed)    N3
        */
        m_graph.removeNode(N2);

        EXPECT_EQ(0u, m_graph.getOutDegree(N1));
        EXPECT_EQ(0u, m_graph.getInDegree(N1));
        EXPECT_EQ(0u, m_graph.getOutDegree(N3));
        EXPECT_EQ(0u, m_graph.getInDegree(N3));

        // Confidence - check that removed node is really gone
        updateOrdering();
        EXPECT_THAT(getSortedTestNodes(), ::testing::UnorderedElementsAre(&N1, &N3));
    }

    TEST_F(ADirectedAcyclicGraph, RemovingOneSourceNode_DoesNotAffectEdgesOfOtherSourceNodes)
    {
        /*
        *   N1         <- remove this node
        *      \
        *        N3
        *      /          <- multiplicity = 2
        *   N2
        */
        addTestNodesToGraph(3);
        m_graph.addEdge(N1, N3);
        m_graph.addEdge(N2, N3);
        m_graph.addEdge(N2, N3);

        m_graph.removeNode(N1);

        EXPECT_EQ(2u, m_graph.getOutDegree(N2));
        EXPECT_EQ(0u, m_graph.getInDegree(N2));
        EXPECT_EQ(0u, m_graph.getOutDegree(N3));
        EXPECT_EQ(2u, m_graph.getInDegree(N3));

        // Confidence - check that removed node is really gone
        updateOrdering();
        EXPECT_THAT(getSortedTestNodes(), ::testing::UnorderedElementsAre(&N2, &N3));
    }

    TEST_F(ADirectedAcyclicGraph, RemovingOneTargetNode_DoesNotAffectEdgesOfOtherTargetNodes)
    {
        /*
        *        N2           <- remove this node
        *      /
        *  N1
        *      \          <- multiplicity = 2
        *        N3
        */
        addTestNodesToGraph(3);
        m_graph.addEdge(N1, N2);
        m_graph.addEdge(N1, N3);
        m_graph.addEdge(N1, N3);

        m_graph.removeNode(N2);

        EXPECT_EQ(2u, m_graph.getOutDegree(N1));
        EXPECT_EQ(0u, m_graph.getInDegree(N1));
        EXPECT_EQ(0u, m_graph.getOutDegree(N3));
        EXPECT_EQ(2u, m_graph.getInDegree(N3));

        // Confidence - check that removed node is really gone
        updateOrdering();
        EXPECT_THAT(getSortedTestNodes(), ::testing::UnorderedElementsAre(&N1, &N3));
    }

    TEST_F(ADirectedAcyclicGraph, ReturnsTwoNodesInRightOrder)
    {
        addTestNodesToGraph(2);
        m_graph.addEdge(N1, N2);

        updateOrdering();

        EXPECT_THAT(getSortedTestNodes(), ::testing::ElementsAre(&N1, &N2));
    }

    TEST_F(ADirectedAcyclicGraph, ReversesOrderIfLinkIsReversed)
    {
        addTestNodesToGraph(2);

        m_graph.addEdge(N1, N2);
        EXPECT_THAT(getSortedTestNodes(), ::testing::ElementsAre(&N1, &N2));

        m_graph.removeEdge(N1, N2);
        m_graph.addEdge(N2, N1);
        EXPECT_THAT(getSortedTestNodes(), ::testing::ElementsAre(&N2, &N1));
    }

    TEST_F(ADirectedAcyclicGraph, ComputesRightOrderForComplexGraph)
    {
        addTestNodesToGraph(6);

        /*     -----
         *   /        \
         * N2 -- N3 -- N6
         *     /    \
         *    /      \
         * N1 -- N4 -- N5
         */

        m_graph.addEdge(N1, N3);
        m_graph.addEdge(N1, N4);
        m_graph.addEdge(N2, N3);
        m_graph.addEdge(N2, N6);
        m_graph.addEdge(N3, N5);
        m_graph.addEdge(N3, N6);
        m_graph.addEdge(N4, N5);

        updateOrdering();

        EXPECT_LT(getRank(N1), getRank(N3));
        EXPECT_LT(getRank(N1), getRank(N4));
        EXPECT_LT(getRank(N2), getRank(N6));
        EXPECT_LT(getRank(N2), getRank(N3));
        EXPECT_LT(getRank(N3), getRank(N6));
        EXPECT_LT(getRank(N3), getRank(N5));
        EXPECT_LT(getRank(N4), getRank(N5));
    }

    TEST_F(ADirectedAcyclicGraph, ComputesRightOrderForComplexGraphAfterLinksAreChanged)
    {
        addTestNodesToGraph(6);

        /*     -----
         *   /        \
         * N2 -- N3 -- N6
         *     /    \
         *    /      \
         * N1 -- N4 -- N5
         */

        m_graph.addEdge(N1, N3);
        m_graph.addEdge(N1, N4);
        m_graph.addEdge(N2, N3);
        m_graph.addEdge(N2, N6);
        m_graph.addEdge(N3, N5);
        m_graph.addEdge(N3, N6);
        m_graph.addEdge(N4, N5);

        /*     -----
         *   /        \
         * N2 -- N3 -- N6 -- N1 -- N4 -- N5
         *          \                   /
         *           \ --------------- /
         *
         */

        m_graph.removeEdge(N1, N3);
        m_graph.addEdge(N6, N1);

        updateOrdering();

        EXPECT_LT(getRank(N2), getRank(N3));
        EXPECT_LT(getRank(N3), getRank(N6));
        EXPECT_LT(getRank(N6), getRank(N1));
        EXPECT_LT(getRank(N1), getRank(N4));
        EXPECT_LT(getRank(N2), getRank(N6));
        EXPECT_LT(getRank(N4), getRank(N5));
        EXPECT_LT(getRank(N3), getRank(N5));
    }

    TEST_F(ADirectedAcyclicGraph, ComputesRightOrderIfANodeIsRemovedFromBeginning)
    {
        addTestNodesToGraph(6);

        /*     -----
         *   /        \
         * N2 -- N3 -- N6
         *     /    \
         *    /      \
         * N1 -- N4 -- N5
         */

        m_graph.addEdge(N1, N3);
        m_graph.addEdge(N1, N4);
        m_graph.addEdge(N2, N3);
        m_graph.addEdge(N2, N6);
        m_graph.addEdge(N3, N5);
        m_graph.addEdge(N3, N6);
        m_graph.addEdge(N4, N5);

        /*
         * (removed)    N3 -- N6
         *               /    \
         *              /      \
         *           N1 -- N4 -- N5
         */

        m_graph.removeNode(N2);

        updateOrdering();

        EXPECT_LT(getRank(N1), getRank(N3));
        EXPECT_LT(getRank(N1), getRank(N4));
        EXPECT_LT(getRank(N3), getRank(N6));
        EXPECT_LT(getRank(N3), getRank(N5));
        EXPECT_LT(getRank(N4), getRank(N5));

        // Confidence - check that removed node is really gone
        EXPECT_THAT(getSortedTestNodes(), ::testing::UnorderedElementsAre(&N1, &N3, &N4, &N5, &N6));
    }

    TEST_F(ADirectedAcyclicGraph, ComputesRightOrderIfANodeIsRemovedFromEnd)
    {
        addTestNodesToGraph(6);

        /*     -----
         *   /        \
         * N2 -- N3 -- N6
         *     /    \
         *    /      \
         * N1 -- N4 -- N5
         */

        m_graph.addEdge(N1, N3);
        m_graph.addEdge(N1, N4);
        m_graph.addEdge(N2, N3);
        m_graph.addEdge(N2, N6);
        m_graph.addEdge(N3, N5);
        m_graph.addEdge(N3, N6);
        m_graph.addEdge(N4, N5);

        /*     -----
         *   /        \
         * N2 -- N3 -- N6
         *     /
         *    /
         * N1 -- N4    (removed)
         */

        m_graph.removeNode(N5);

        updateOrdering();

        EXPECT_LT(getRank(N1), getRank(N3));
        EXPECT_LT(getRank(N1), getRank(N4));
        EXPECT_LT(getRank(N2), getRank(N3));
        EXPECT_LT(getRank(N2), getRank(N6));
        EXPECT_LT(getRank(N3), getRank(N6));

        // Confidence - check that removed node is really gone
        EXPECT_THAT(getSortedTestNodes(), ::testing::UnorderedElementsAre(&N1, &N2, &N3, &N4, &N6));
    }

    TEST_F(ADirectedAcyclicGraph, ComputesRightOrderIfANodeIsRemovedFromTheMiddle)
    {
        addTestNodesToGraph(6);

        /*     -----
         *   /        \
         * N2 -- N3 -- N6
         *     /    \
         *    /      \
         * N1 -- N4 -- N5
         */

        m_graph.addEdge(N1, N3);
        m_graph.addEdge(N1, N4);
        m_graph.addEdge(N2, N3);
        m_graph.addEdge(N2, N6);
        m_graph.addEdge(N3, N5);
        m_graph.addEdge(N3, N6);
        m_graph.addEdge(N4, N5);


        /*     -----
         *   /        \
         * N2    N3    N6
         *
         *
         * N1 -- N4 -- N5
         */

        m_graph.removeNode(N3);

        updateOrdering();

        EXPECT_LT(getRank(N1), getRank(N4));
        EXPECT_LT(getRank(N2), getRank(N6));
        EXPECT_LT(getRank(N4), getRank(N5));

        // Confidence - check that removed node is really gone
        EXPECT_THAT(getSortedTestNodes(), ::testing::UnorderedElementsAre(&N1, &N2, &N4, &N5, &N6));
    }

    TEST_F(ADirectedAcyclicGraph, CycleDoesNotCauseStackOverflow_NoIdentifyableRootNode)
    {
        addTestNodesToGraph(3);

        // N1 -> N2 -> N3 -> N1 -> .... (infinity)
        m_graph.addEdge(N1, N2);
        m_graph.addEdge(N2, N3);
        m_graph.addEdge(N3, N1);

        EXPECT_FALSE(m_graph.getTopologicallySortedNodes().has_value());
        // Can call again, still reports false and does not crash
        EXPECT_FALSE(m_graph.getTopologicallySortedNodes().has_value());
    }

    TEST_F(ADirectedAcyclicGraph, CycleDoesNotCauseStackOverflow_WithRootNode)
    {
        addTestNodesToGraph(4);

        // N1 -> N2 -> N3 -> N4 -> N2 .... (infinity)
        m_graph.addEdge(N1, N2);
        m_graph.addEdge(N2, N3);
        m_graph.addEdge(N3, N4);
        m_graph.addEdge(N4, N2);

        EXPECT_FALSE(m_graph.getTopologicallySortedNodes().has_value());
        // Can call again, still reports false and does not crash
        EXPECT_FALSE(m_graph.getTopologicallySortedNodes().has_value());
    }

    TEST_F(ADirectedAcyclicGraph, RemovesMultiLinksBetweenTwoNodes_OneByOne)
    {
        addTestNodesToGraph(2);

        /*
         * N1   -x2->   N2
         */
        m_graph.addEdge(N1, N2);
        m_graph.addEdge(N1, N2);

        EXPECT_THAT(getSortedTestNodes(), ::testing::ElementsAre(&N1, &N2));

        m_graph.removeEdge(N1, N2);
        EXPECT_THAT(getSortedTestNodes(), ::testing::ElementsAre(&N1, &N2));
        /*
        * N1   -x1->   N2
        */

        m_graph.removeEdge(N1, N2);

        // Both nodes still here, but no ordering guarantees any more
        EXPECT_THAT(getSortedTestNodes(), ::testing::UnorderedElementsAre(&N1, &N2));
    }

    TEST_F(ADirectedAcyclicGraph, Confidence_ReAddingNode_WithMultiLinksToNeighbours_AndReversingOrderOfLinks_ReversesOrderOfNodes)
    {
        addTestNodesToGraph(3);

        /*
        * N1   -x2->   N2    -x3->    N3
        */
        m_graph.addEdge(N1, N2);
        m_graph.addEdge(N1, N2);
        m_graph.addEdge(N2, N3);
        m_graph.addEdge(N2, N3);
        m_graph.addEdge(N2, N3);

        EXPECT_THAT(getSortedTestNodes(), ::testing::ElementsAre(&N1, &N2, &N3));

        /*
        * N1      (removed)       N3
        */
        m_graph.removeNode(N2);

        // No links -> No ordering. Check that the removed node is not in the list now
        EXPECT_THAT(getSortedTestNodes(), ::testing::UnorderedElementsAre(&N1, &N3));

        // Re-add the node, and create links in the other direction
        m_graph.addNode(N2);

        /*
        * N3   ->   N2    ->    N1
        */
        m_graph.addEdge(N3, N2);
        m_graph.addEdge(N2, N1);

        EXPECT_THAT(getSortedTestNodes(), ::testing::ElementsAre(&N3, &N2, &N1));
    }
}
