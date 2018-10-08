//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "NodeTopologyTest.h"
#include "PerformanceTestUtils.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/MeshNode.h"

NodeTopologyTest::NodeTopologyTest(ramses_internal::String testName, uint32_t testState) : PerformanceTestBase(testName, testState) {};

void NodeTopologyTest::initTest(ramses::RamsesClient& client, ramses::Scene& scene)
{
    UNUSED(client);
    m_scene = &scene;
    m_parent = NULL;
    m_children.reserve(NodeCount);

    for (uint32_t i = 0; i < NodeCount; i++)
    {
        m_children.push_back(scene.createMeshNode());
    }

    m_childrenShuffled = m_children;

    PerformanceTestUtils::ShuffleObjectList(m_childrenShuffled);
}

void NodeTopologyTest::preUpdate()
{
    if (!m_parent)
    {
        m_parent = m_scene->createMeshNode();
    }

    addChildren();
}

void NodeTopologyTest::update()
{
    switch (m_testState)
    {
    case NodeTopologyTest_RemoveNodesIndividually:
    {
        removeChildren();
    }
    case NodeTopologyTest_RemoveNodesbyDestroyingParent:
    {
        m_scene->destroy(*m_parent);
        m_parent = NULL;
        break;
    }
    default:
    {
        assert(false);
        break;
    }
    }
}

void NodeTopologyTest::addChildren()
{
    for (auto it : m_children)
    {
        m_parent->addChild(*it);
    }
}

void NodeTopologyTest::removeChildren()
{
    for (auto it : m_childrenShuffled)
    {
        m_parent->removeChild(*it);
    }
}

