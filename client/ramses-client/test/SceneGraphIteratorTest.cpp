//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/SceneGraphIterator.h"
#include "SimpleSceneTopology.h"
#include "NodeImpl.h"
#include "MeshNodeImpl.h"
#include "Collections/Vector.h"

using namespace testing;

namespace ramses
{
    class SceneGraphIteratorTest : public SimpleSceneTopology
    {
    protected:

        AssertionResult checkOrder(SceneGraphIterator& iterator, const std::vector<Node*>& expectedOrder) const
        {
            for (const auto expectedNode : expectedOrder)
            {
                const Node* actualNode = iterator.getNext();
                if (expectedNode != actualNode)
                {
                    return AssertionFailure() << expectedNode->getName() << " expected, but was " << actualNode->getName();
                }
            }

            if (iterator.getNext() != NULL)
            {
                return AssertionFailure() << "Retrieved more nodes than expected";
            }

            return AssertionSuccess();
        }
    };


    TEST_F(SceneGraphIteratorTest, traversesSceneGraph_DepthFirst)
    {
        SceneGraphIterator iterator(m_root, ETreeTraversalStyle_DepthFirst);

        std::vector<Node*> expectedOrder;
        expectedOrder.push_back(&m_root);
        expectedOrder.push_back(&m_vis1);
        expectedOrder.push_back(&m_mesh1a);
        expectedOrder.push_back(&m_mesh1b);
        expectedOrder.push_back(&m_vis2);
        expectedOrder.push_back(&m_mesh2a);
        expectedOrder.push_back(&m_mesh2b);

        EXPECT_TRUE(checkOrder(iterator, expectedOrder));
    }

    TEST_F(SceneGraphIteratorTest, traversesSceneGraph_BreadthFirst)
    {
        SceneGraphIterator iterator(m_root, ETreeTraversalStyle_BreadthFirst);

        std::vector<Node*> expectedOrder;
        expectedOrder.push_back(&m_root);
        expectedOrder.push_back(&m_vis1);
        expectedOrder.push_back(&m_vis2);
        expectedOrder.push_back(&m_mesh1a);
        expectedOrder.push_back(&m_mesh1b);
        expectedOrder.push_back(&m_mesh2a);
        expectedOrder.push_back(&m_mesh2b);

        EXPECT_TRUE(checkOrder(iterator, expectedOrder));
    }

    TEST_F(SceneGraphIteratorTest, traverseOnlyOneLeaf_DepthFirst)
    {
        SceneGraphIterator iter(m_mesh1b, ETreeTraversalStyle_DepthFirst);
        EXPECT_EQ(&m_mesh1b, iter.getNext());
        EXPECT_EQ(0, iter.getNext());
    }

    TEST_F(SceneGraphIteratorTest, traverseOnlyOneLeaf_BreathFirst)
    {
        SceneGraphIterator iter(m_mesh1b, ETreeTraversalStyle_BreadthFirst);
        EXPECT_EQ(&m_mesh1b, iter.getNext());
        EXPECT_EQ(0, iter.getNext());
    }
}
