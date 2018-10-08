//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransformationHierarchyTest.h"
#include "PerformanceTestUtils.h"
#include "ramses-client-api/RotateNode.h"
#include "ramses-client-api/TransformationNode.h"
#include "ramses-client-api/TranslateNode.h"
#include "ramses-client-api/ScaleNode.h"
#include "ramses-client-api/Node.h"

TransformationHierarchyTest::TransformationHierarchyTest(ramses_internal::String testName, uint32_t testState) : PerformanceTestBase(testName, testState) {};

void TransformationHierarchyTest::initTest(ramses::RamsesClient& client, ramses::Scene& scene)
{
    UNUSED(client);

    const uint32_t HierarchyDepth = 10 * 1000;

    ramses::Node *root = NULL;
    ramses::Node *middle = NULL;
    ramses::Node *leaf = NULL;
    PerformanceTestUtils::BuildBranchOfNodes(scene, HierarchyDepth, root, middle, leaf, ramses::ERamsesObjectType_TransformationNode);

    assert(root);
    assert(middle);
    assert(leaf);

    m_leafNode = static_cast<ramses::TransformationNode*>(leaf);

    switch (m_testState)
    {
        case TransformationHierarchyTest_Root:
        {
            m_targetNode = static_cast<ramses::TransformationNode*>(root);
            break;
        }
        case TransformationHierarchyTest_HalfWayNode:
        {
            m_targetNode = static_cast<ramses::TransformationNode*>(middle);
            break;
        }
        case TransformationHierarchyTest_Leaf:
        {
            m_targetNode = static_cast<ramses::TransformationNode*>(leaf);
            break;
        }
        default:
        {
            assert(false);
            break;
        }
    }
}

void TransformationHierarchyTest::preUpdate()
{

}

void TransformationHierarchyTest::update()
{
    m_targetNode->rotate(0.1f, 0.2f, 0.3f);

    float mat[16] = { 0.f };
    m_leafNode->getModelMatrix(mat);
}
