//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "BoundingSphereTest.h"
#include "PerformanceTestUtils.h"
#include "ramses-client-api/Node.h"
#include "ramses-hmi-utils.h"

BoundingSphereTest::BoundingSphereTest(ramses_internal::String testName, uint32_t testState) : PerformanceTestBase(testName, testState) {};

void BoundingSphereTest::initTest(ramses::RamsesClient& client, ramses::Scene& scene)
{
    UNUSED(client);

    m_spheres = new ramses::BoundingSphereCollection(scene);

    switch (m_testState)
    {
    case BoundingSphereTest_1K_BinaryTree:
    {
        m_rootNode = &PerformanceTestUtils::BuildGraphOfNodes(scene, 2u, 10u, ramses::ERamsesObjectType_MeshNode);
        break;
    }
    case BoundingSphereTest_4K_BinaryTree:
    {
        m_rootNode = &PerformanceTestUtils::BuildGraphOfNodes(scene, 2u, 12u, ramses::ERamsesObjectType_MeshNode);
        break;
    }
    case BoundingSphereTest_16K_BinaryTree:
    {
        m_rootNode = &PerformanceTestUtils::BuildGraphOfNodes(scene, 2u, 14u, ramses::ERamsesObjectType_MeshNode);
        break;
    }
    case BoundingSphereTest_1K_SpreadOutTree:
    {
        m_rootNode = &PerformanceTestUtils::BuildGraphOfNodes(scene, 4u, 5u, ramses::ERamsesObjectType_MeshNode);
        break;
    }
    case BoundingSphereTest_4K_SpreadOutTree:
    {
        m_rootNode = &PerformanceTestUtils::BuildGraphOfNodes(scene, 4u, 6u, ramses::ERamsesObjectType_MeshNode);
        break;
    }
    case BoundingSphereTest_16K_SpreadOutTree:
    {
        m_rootNode = &PerformanceTestUtils::BuildGraphOfNodes(scene, 4u, 7u, ramses::ERamsesObjectType_MeshNode);
        break;
    }
    default:
        break;
    }
}

BoundingSphereTest::~BoundingSphereTest()
{
    delete m_spheres;
}

void BoundingSphereTest::update()
{
    m_spheres->computeBoundingSphereInWorldSpace(*m_rootNode);
}
