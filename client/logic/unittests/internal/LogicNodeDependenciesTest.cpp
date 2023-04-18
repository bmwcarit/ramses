//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "internals/LogicNodeDependencies.h"
#include "internals/ErrorReporting.h"
#include "LogicNodeDummy.h"

namespace rlogic::internal
{
    class ALogicNodeDependencies : public ::testing::Test
    {
    protected:
        void expectSortedNodeOrder(std::initializer_list<LogicNodeImpl*> nodes)
        {
            EXPECT_THAT(*m_dependencies.getTopologicallySortedNodes(), ::testing::ElementsAreArray(nodes));
        }

        void expectUnsortedNodeOrder(std::initializer_list<LogicNodeImpl*> nodes)
        {
            EXPECT_THAT(*m_dependencies.getTopologicallySortedNodes(), ::testing::UnorderedElementsAreArray(nodes));
        }

        static void expectLink(const PropertyImpl& output, std::initializer_list<PropertyImpl::Link> inputs)
        {
            // check that linked destinations all point to source
            for (auto& input : inputs)
            {
                EXPECT_EQ(&output, input.property->getIncomingLink().property);
                EXPECT_EQ(input.isWeakLink, input.property->getIncomingLink().isWeakLink);
            }
            // check that source points to all destinations
            ASSERT_EQ(output.getOutgoingLinks().size(), inputs.size());
            auto srcOutputIt = output.getOutgoingLinks().cbegin();
            for (auto& input : inputs)
            {
                EXPECT_EQ(srcOutputIt->property, input.property);
                EXPECT_EQ(srcOutputIt->isWeakLink, input.isWeakLink);
                ++srcOutputIt;
            }
        }

        static void expectNoLinks(const PropertyImpl& property)
        {
            if (property.isInput())
            {
                EXPECT_EQ(nullptr, property.getIncomingLink().property);
            }
            if (property.isOutput())
            {
                EXPECT_TRUE(property.getOutgoingLinks().empty());
            }
        }

        LogicNodeDummyImpl m_nodeA{ "A", false };
        LogicNodeDummyImpl m_nodeB{ "B", false };

        LogicNodeDependencies m_dependencies;
        ErrorReporting m_errorReporting;
    };

    TEST_F(ALogicNodeDependencies, IsEmptyAfterConstruction)
    {
        EXPECT_TRUE((*m_dependencies.getTopologicallySortedNodes()).empty());
    }

    TEST_F(ALogicNodeDependencies, RemovingNode_RemovesItFromTopologyList)
    {
        m_dependencies.addNode(m_nodeA);
        m_dependencies.addNode(m_nodeB);
        m_dependencies.removeNode(m_nodeA);

        expectSortedNodeOrder({&m_nodeB});
    }

    TEST_F(ALogicNodeDependencies, SingleDisconnectedNode)
    {
        m_dependencies.addNode(m_nodeA);
        expectSortedNodeOrder({&m_nodeA});
    }

    TEST_F(ALogicNodeDependencies, ConnectingTwoNodes_CreatesALink)
    {
        m_dependencies.addNode(m_nodeA);
        m_dependencies.addNode(m_nodeB);

        PropertyImpl& output = *m_nodeA.getOutputs()->getChild("output1")->m_impl;
        PropertyImpl& input = *m_nodeB.getInputs()->getChild("input1")->m_impl;

        EXPECT_TRUE(m_dependencies.link(output, input, false, m_errorReporting));

        // Sorted topologically
        expectSortedNodeOrder({ &m_nodeA, &m_nodeB });

        // Has exactly one link
        expectLink(output, { {&input, false} });
    }

    TEST_F(ALogicNodeDependencies, ConnectingTwoNodes_CreatesABackLink)
    {
        m_dependencies.addNode(m_nodeA);
        m_dependencies.addNode(m_nodeB);

        PropertyImpl& output = *m_nodeA.getOutputs()->getChild("output1")->m_impl;
        PropertyImpl& input = *m_nodeB.getInputs()->getChild("input1")->m_impl;

        EXPECT_TRUE(m_dependencies.link(output, input, true, m_errorReporting));

        // Back links do not affect topological order
        expectUnsortedNodeOrder({ &m_nodeA, &m_nodeB });

        // Has exactly one link
        expectLink(output, { {&input, true} });
    }

    TEST_F(ALogicNodeDependencies, DisconnectingTwoNodes_RemovesLinks)
    {
        m_dependencies.addNode(m_nodeA);
        m_dependencies.addNode(m_nodeB);

        PropertyImpl& output = *m_nodeA.getOutputs()->getChild("output1")->m_impl;
        PropertyImpl& input = *m_nodeB.getInputs()->getChild("input1")->m_impl;

        EXPECT_TRUE(m_dependencies.link(output, input, false, m_errorReporting));
        EXPECT_TRUE(m_dependencies.unlink(output, input, m_errorReporting));

        // both nodes still there, but no ordering guarantees without the link
        expectUnsortedNodeOrder({ &m_nodeA, &m_nodeB });

        expectNoLinks(input);
        expectNoLinks(output);
    }

    TEST_F(ALogicNodeDependencies, RemovingSourceNode_RemovesLinks)
    {
        auto nodeToDelete = std::make_unique<LogicNodeDummyImpl>("node");

        m_dependencies.addNode(*nodeToDelete);
        m_dependencies.addNode(m_nodeB);

        PropertyImpl& input = *m_nodeB.getInputs()->getChild("input1")->m_impl;
        EXPECT_TRUE(m_dependencies.link(*nodeToDelete->getOutputs()->getChild("output1")->m_impl, input, false, m_errorReporting));

        m_dependencies.removeNode(*nodeToDelete);
        nodeToDelete = nullptr;

        // only target node left
        expectSortedNodeOrder({ &m_nodeB });
        expectNoLinks(input);
    }

    TEST_F(ALogicNodeDependencies, RemovingTargetNode_RemovesLinks)
    {
        auto nodeToDelete = std::make_unique<LogicNodeDummyImpl>("node");
        m_dependencies.addNode(m_nodeA);
        m_dependencies.addNode(*nodeToDelete);

        PropertyImpl& output = *m_nodeA.getOutputs()->getChild("output1")->m_impl;
        EXPECT_TRUE(m_dependencies.link(output, *nodeToDelete->getInputs()->getChild("input1")->m_impl, false, m_errorReporting));

        m_dependencies.removeNode(*nodeToDelete);
        nodeToDelete = nullptr;

        // only source node left
        expectSortedNodeOrder({ &m_nodeA });
        expectNoLinks(output);
    }

    TEST_F(ALogicNodeDependencies, RemovingMiddleNode_DoesNotAffectRelativeOrderOfOtherNodes)
    {
        auto nodeToDelete = std::make_unique<LogicNodeDummyImpl>("M", false);

        m_dependencies.addNode(m_nodeA);
        m_dependencies.addNode(*nodeToDelete);
        m_dependencies.addNode(m_nodeB);

        PropertyImpl& output1A = *m_nodeA.getOutputs()->getChild("output1")->m_impl;
        PropertyImpl& output2A = *m_nodeA.getOutputs()->getChild("output2")->m_impl;
        PropertyImpl& output1M = *nodeToDelete->getOutputs()->getChild("output1")->m_impl;
        PropertyImpl& input1M = *nodeToDelete->getInputs()->getChild("input1")->m_impl;
        PropertyImpl& input1B = *m_nodeB.getInputs()->getChild("input1")->m_impl;
        PropertyImpl& input2B = *m_nodeB.getInputs()->getChild("input2")->m_impl;

        // A   ->    M    ->   B
        //   \               /
        //      ---->-------
        EXPECT_TRUE(m_dependencies.link(output1A, input1M, false, m_errorReporting));
        EXPECT_TRUE(m_dependencies.link(output1M, input1B, false, m_errorReporting));
        EXPECT_TRUE(m_dependencies.link(output2A, input2B, false, m_errorReporting));

        expectSortedNodeOrder({ &m_nodeA, nodeToDelete.get(), &m_nodeB });

        m_dependencies.removeNode(*nodeToDelete);
        nodeToDelete = nullptr;

        // only other two nodes left (A and B). Their relative order is not changed
        expectSortedNodeOrder({ &m_nodeA, &m_nodeB });

        // Only link A->B remains
        expectLink(output2A, { { &input2B, false } });

        // Other links are gone
        expectNoLinks(output1A);
        expectNoLinks(input1B);
    }

    TEST_F(ALogicNodeDependencies, ReversingDependencyOfTwoNodes_InvertsTopologicalOrder)
    {
        m_dependencies.addNode(m_nodeA);
        m_dependencies.addNode(m_nodeB);

        // Node A -> Node B  (output of node A linked to input of node B)
        PropertyImpl& inputB = *m_nodeB.getInputs()->getChild("input1")->m_impl;
        PropertyImpl& outputA = *m_nodeA.getOutputs()->getChild("output1")->m_impl;

        EXPECT_TRUE(m_dependencies.link(outputA, inputB, false, m_errorReporting));
        expectSortedNodeOrder({ &m_nodeA, &m_nodeB });

        // Reverse dependency
        // Node B -> Node A  (output of node B linked to input of node A)
        EXPECT_TRUE(m_dependencies.unlink(outputA, inputB, m_errorReporting));
        PropertyImpl& inputA = *m_nodeA.getInputs()->getChild("input1")->m_impl;
        PropertyImpl& outputB = *m_nodeB.getOutputs()->getChild("output1")->m_impl;

        EXPECT_TRUE(m_dependencies.link(outputB, inputA, false, m_errorReporting));

        // Still no disconnected nodes, but now topological order is B -> A
        expectSortedNodeOrder({ &m_nodeB, &m_nodeA });

        // Has exactly one link
        expectLink(outputB, { { &inputA, false } });
        // Other links are gone
        expectNoLinks(inputB);
        expectNoLinks(outputA);
    }

    TEST_F(ALogicNodeDependencies, ReversingDependencyOfTwoNodes_UsingBackLink_DoesNotAffectTopologicalOrder)
    {
        m_dependencies.addNode(m_nodeA);
        m_dependencies.addNode(m_nodeB);

        // Node A -> Node B  (output of node A linked to input of node B)
        PropertyImpl& inputB = *m_nodeB.getInputs()->getChild("input1")->m_impl;
        PropertyImpl& outputA = *m_nodeA.getOutputs()->getChild("output1")->m_impl;

        EXPECT_TRUE(m_dependencies.link(outputA, inputB, false, m_errorReporting));
        expectSortedNodeOrder({ &m_nodeA, &m_nodeB });

        // Reverse dependency but only using back link
        // Node B -> Node A  (output of node B linked to input of node A)
        EXPECT_TRUE(m_dependencies.unlink(outputA, inputB, m_errorReporting));
        PropertyImpl& inputA = *m_nodeA.getInputs()->getChild("input1")->m_impl;
        PropertyImpl& outputB = *m_nodeB.getOutputs()->getChild("output1")->m_impl;

        EXPECT_TRUE(m_dependencies.link(outputB, inputA, true, m_errorReporting));

        // Nodes are linked only using back link, this has no effect on topological order
        expectUnsortedNodeOrder({ &m_nodeB, &m_nodeA });

        // Has exactly one link
        expectLink(outputB, { { &inputA, true } });
        // Other links are gone
        expectNoLinks(inputB);
        expectNoLinks(outputA);
    }

    TEST_F(ALogicNodeDependencies, NodesConnectedInBothDirectionsFormingCycleWithBackLink)
    {
        m_dependencies.addNode(m_nodeA);
        m_dependencies.addNode(m_nodeB);

        PropertyImpl& inputA = *m_nodeA.getInputs()->getChild("input1")->m_impl;
        PropertyImpl& outputA = *m_nodeA.getOutputs()->getChild("output1")->m_impl;
        PropertyImpl& inputB = *m_nodeB.getInputs()->getChild("input1")->m_impl;
        PropertyImpl& outputB = *m_nodeB.getOutputs()->getChild("output1")->m_impl;

        // Node A -> Node B  (output of node A linked to input of node B)
        EXPECT_TRUE(m_dependencies.link(outputA, inputB, false, m_errorReporting));
        expectSortedNodeOrder({ &m_nodeA, &m_nodeB });

        // connect in reverse direction using back link
        EXPECT_TRUE(m_dependencies.link(outputB, inputA, true, m_errorReporting));
        expectSortedNodeOrder({ &m_nodeA, &m_nodeB });

        expectLink(outputA, { { &inputB, false } });
        expectLink(outputB, { { &inputA, true } });
    }

    class ALogicNodeDependencies_NestedLinks : public ALogicNodeDependencies
    {
    protected:

        ALogicNodeDependencies_NestedLinks()
        {
            m_dependencies.addNode(*m_nodeANested);
            m_dependencies.addNode(*m_nodeBNested);

            m_nestedOutputA = m_nodeANested->getOutputs()->getChild("outputStruct")->getChild("nested")->m_impl.get();
            m_nestedInputB = m_nodeBNested->getInputs()->getChild("inputStruct")->getChild("nested")->m_impl.get();
            m_arrayOutputA = m_nodeANested->getOutputs()->getChild("outputArray")->getChild(0)->m_impl.get();
            m_arrayInputB = m_nodeBNested->getInputs()->getChild("inputArray")->getChild(0)->m_impl.get();
        }

        std::unique_ptr<LogicNodeDummyImpl> m_nodeANested = std::make_unique<LogicNodeDummyImpl>( "A", true );
        std::unique_ptr<LogicNodeDummyImpl> m_nodeBNested = std::make_unique<LogicNodeDummyImpl>( "B", true );

        PropertyImpl* m_nestedOutputA;
        PropertyImpl* m_nestedInputB;
        PropertyImpl* m_arrayOutputA;
        PropertyImpl* m_arrayInputB;
    };

    TEST_F(ALogicNodeDependencies_NestedLinks, ReportsErrorWhenUnlinkingStructInputs_BasedOnTheirType)
    {
        PropertyImpl* structProperty = m_nodeBNested->getInputs()->getChild("inputStruct")->m_impl.get();
        EXPECT_FALSE(m_dependencies.unlink(*m_nestedOutputA, *structProperty, m_errorReporting));
        EXPECT_EQ("Can't unlink properties of complex types directly!", m_errorReporting.getErrors()[0].message);
    }

    TEST_F(ALogicNodeDependencies_NestedLinks, ReportsErrorWhenUnlinkingArrayInputs_BasedOnTheirType)
    {
        PropertyImpl* arrayProperty = m_nodeBNested->getInputs()->getChild("inputArray")->m_impl.get();
        EXPECT_FALSE(m_dependencies.unlink(*m_nestedOutputA, *arrayProperty, m_errorReporting));
        EXPECT_EQ("Can't unlink properties of complex types directly!", m_errorReporting.getErrors()[0].message);
    }

    TEST_F(ALogicNodeDependencies_NestedLinks, ReportsErrorWhenUnlinkingStructs_WithLinkedChildren)
    {
        EXPECT_TRUE(m_dependencies.link(*m_nestedOutputA, *m_nestedInputB, false, m_errorReporting));
        EXPECT_TRUE(m_errorReporting.getErrors().empty());

        // Still can't unlink complex type
        PropertyImpl* outputParentStruct = m_nodeANested->getOutputs()->getChild("outputStruct")->m_impl.get();
        PropertyImpl* inputParentStruct = m_nodeBNested->getInputs()->getChild("inputStruct")->m_impl.get();
        EXPECT_FALSE(m_dependencies.unlink(*outputParentStruct, *inputParentStruct, m_errorReporting));
        EXPECT_EQ("Can't unlink properties of complex types directly!", m_errorReporting.getErrors()[0].message);
    }

    TEST_F(ALogicNodeDependencies_NestedLinks, ConnectingTwoNodes_CreatesALink)
    {
        EXPECT_TRUE(m_dependencies.link(*m_nestedOutputA, *m_nestedInputB, false, m_errorReporting));

        // Sorted topologically
        expectSortedNodeOrder({m_nodeANested.get(), m_nodeBNested.get()});

        // Has exactly one link
        expectLink(*m_nestedOutputA, { { m_nestedInputB, false } });
    }

    TEST_F(ALogicNodeDependencies_NestedLinks, DisconnectingTwoNodes_RemovesLinks)
    {
        EXPECT_TRUE(m_dependencies.link(*m_nestedOutputA, *m_nestedInputB, false, m_errorReporting));
        EXPECT_TRUE(m_dependencies.unlink(*m_nestedOutputA, *m_nestedInputB, m_errorReporting));

        // both nodes still there, but no ordering guarantees without the link
        expectUnsortedNodeOrder({ m_nodeANested.get(), m_nodeBNested.get() });

        expectNoLinks(*m_nestedOutputA);
        expectNoLinks(*m_nestedInputB);
    }

    TEST_F(ALogicNodeDependencies_NestedLinks, RemovingSourceNode_RemovesLinks)
    {
        EXPECT_TRUE(m_dependencies.link(*m_nestedOutputA, *m_nestedInputB, false, m_errorReporting));

        m_dependencies.removeNode(*m_nodeANested);
        m_nodeANested = nullptr;

        // only target node left
        expectSortedNodeOrder({ m_nodeBNested.get() });
        expectNoLinks(*m_nestedInputB);
    }

    TEST_F(ALogicNodeDependencies_NestedLinks, RemovingTargetNode_RemovesLinks)
    {
        EXPECT_TRUE(m_dependencies.link(*m_nestedOutputA, *m_nestedInputB, false, m_errorReporting));

        m_dependencies.removeNode(*m_nodeBNested);
        m_nodeBNested = nullptr;

        // only source node left
        expectSortedNodeOrder({ m_nodeANested.get() });

        // No links
        expectNoLinks(*m_nestedOutputA);
    }

    TEST_F(ALogicNodeDependencies_NestedLinks, ReversingDependencyOfTwoNodes_InvertsTopologicalOrder)
    {
        EXPECT_TRUE(m_dependencies.link(*m_nestedOutputA, *m_nestedInputB, false, m_errorReporting));
        expectSortedNodeOrder({ m_nodeANested.get(), m_nodeBNested.get() });

        // Reverse dependency
        // Node B -> Node A  (output of node B linked to input of node A)
        EXPECT_TRUE(m_dependencies.unlink(*m_nestedOutputA, *m_nestedInputB, m_errorReporting));
        PropertyImpl& nestedInputA = *m_nodeANested->getInputs()->getChild("inputStruct")->getChild("nested")->m_impl;
        PropertyImpl& nestedOutputB = *m_nodeBNested->getOutputs()->getChild("outputStruct")->getChild("nested")->m_impl;

        EXPECT_TRUE(m_dependencies.link(nestedOutputB, nestedInputA, false, m_errorReporting));

        // Still no disconnected nodes, but now topological order is B -> A
        expectSortedNodeOrder({ m_nodeBNested.get(), m_nodeANested.get() });

        // Has exactly one link
        expectLink(nestedOutputB, { { &nestedInputA, false } });
        expectNoLinks(*m_nestedOutputA);
        expectNoLinks(*m_nestedInputB);
    }

    TEST_F(ALogicNodeDependencies, AddsDependencyToBinding)
    {
        RamsesBindingDummyImpl binding;
        m_dependencies.addNode(m_nodeA);
        m_dependencies.addNode(binding);

        m_dependencies.addBindingDependency(binding, m_nodeA);

        expectSortedNodeOrder({ &binding, &m_nodeA });
    }

    TEST_F(ALogicNodeDependencies, AddsDependencyToBinding_WithLinkedNodes)
    {
        RamsesBindingDummyImpl binding;
        m_dependencies.addNode(m_nodeA);
        m_dependencies.addNode(m_nodeB);
        m_dependencies.addNode(binding);

        PropertyImpl& output = *m_nodeA.getOutputs()->getChild("output1")->m_impl;
        PropertyImpl& input = *m_nodeB.getInputs()->getChild("input1")->m_impl;
        EXPECT_TRUE(m_dependencies.link(output, input, false, m_errorReporting));

        m_dependencies.addBindingDependency(binding, m_nodeA);

        // Sorted topologically
        expectSortedNodeOrder({ &binding, &m_nodeA, &m_nodeB });

        // Has exactly one link
        expectLink(output, { {&input, false} });
    }

    TEST_F(ALogicNodeDependencies, RemovesDependencyToBinding)
    {
        RamsesBindingDummyImpl binding;
        m_dependencies.addNode(m_nodeA);
        m_dependencies.addNode(binding);

        m_dependencies.addBindingDependency(binding, m_nodeA);
        expectSortedNodeOrder({ &binding, &m_nodeA });

        m_dependencies.removeBindingDependency(binding, m_nodeA);
        expectUnsortedNodeOrder({ &binding, &m_nodeA });
    }

    TEST_F(ALogicNodeDependencies, RemovesDependencyToBindingAfterNodeRemoved)
    {
        RamsesBindingDummyImpl binding;
        m_dependencies.addNode(m_nodeA);
        m_dependencies.addNode(binding);

        m_dependencies.addBindingDependency(binding, m_nodeA);
        expectSortedNodeOrder({ &binding, &m_nodeA });

        m_dependencies.removeNode(m_nodeA);
        expectUnsortedNodeOrder({ &binding });
    }

    TEST_F(ALogicNodeDependencies, RemovesDependencyToBindingAfterBindingRemoved)
    {
        RamsesBindingDummyImpl binding;
        m_dependencies.addNode(m_nodeA);
        m_dependencies.addNode(binding);

        m_dependencies.addBindingDependency(binding, m_nodeA);
        expectSortedNodeOrder({ &binding, &m_nodeA });

        m_dependencies.removeNode(binding);
        expectUnsortedNodeOrder({ &m_nodeA });
    }

    TEST_F(ALogicNodeDependencies, RespectsSwappedDependencyBetweenBindings)
    {
        RamsesBindingDummyImpl binding1;
        RamsesBindingDummyImpl binding2;
        m_dependencies.addNode(binding1);
        m_dependencies.addNode(binding2);

        m_dependencies.addBindingDependency(binding1, binding2);
        expectSortedNodeOrder({ &binding1, &binding2 });

        m_dependencies.removeBindingDependency(binding1, binding2);

        m_dependencies.addBindingDependency(binding2, binding1);
        expectSortedNodeOrder({ &binding2, &binding1 });
    }
}
