//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "NodeVisibilityPerfTest.h"
#include "PerformanceTestUtils.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/VisibilityNode.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/RamsesObjectTypes.h"
#include "ramses-client-api/Scene.h"

NodeVisibilityPerfTest::NodeVisibilityPerfTest(ramses_internal::String testName, uint32_t testState) : PerformanceTestBase(testName, testState) {};

void NodeVisibilityPerfTest::initTest(ramses::RamsesClient& client, ramses::Scene& scene)
{
    UNUSED(client);

    m_isVisible = true;
    m_scene = &scene;

    if (m_testState == NodeVisibilityTest_WideGraph_RootNode)
    {
        m_targetNode = m_scene->createVisibilityNode();
        m_targetNode->addChild(PerformanceTestUtils::BuildGraphOfNodes(scene, 2u, 12, ramses::ERamsesObjectType_MeshNode)); // 2^12 => 4096 nodes
    }
    else
    {
        ramses::Node* root = NULL;
        ramses::Node* middle = NULL;
        ramses::Node* leaf = NULL;

        PerformanceTestUtils::BuildBranchOfNodes(scene, 4096u, root, middle, leaf, ramses::ERamsesObjectType_MeshNode);

        switch (m_testState)
        {
        case NodeVisibilityTest_DeepGraph_RootNode:
        {
            m_targetNode = injectVisibilityNodeAsParent(root);
            break;
        }
        case NodeVisibilityTest_DeepGraph_MiddleNode:
        {
            m_targetNode = injectVisibilityNodeAsParent(middle);
            break;
        }
        case NodeVisibilityTest_DeepGraph_LeafNode:
        {
            m_targetNode = injectVisibilityNodeAsParent(leaf);
            break;
        }
        default:
        {
            assert(false);
            break;
        }
        }
    }

    // Initial flush
    m_scene->flush();
}

ramses::VisibilityNode* NodeVisibilityPerfTest::injectVisibilityNodeAsParent(ramses::Node* target)
{
    ramses::VisibilityNode* newNode = m_scene->createVisibilityNode();

    if (target->hasParent())
    {
        ramses::Node* oldParent = target->getParent();
        newNode->setParent(*oldParent);
    }

    target->setParent(*newNode);
    return newNode;
}

void NodeVisibilityPerfTest::preUpdate()
{
    m_isVisible = !m_isVisible;
}

void NodeVisibilityPerfTest::update()
{
    m_targetNode->setVisibility(m_isVisible);
    m_scene->flush();
}
