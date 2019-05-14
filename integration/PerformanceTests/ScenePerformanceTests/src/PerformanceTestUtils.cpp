//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PerformanceTestUtils.h"
#include "ramses-client-api/DataMatrix44f.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/RamsesObject.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/SceneObject.h"
#include "Collections/String.h"
#include "Utils/StringUtils.h"

ramses::Node& PerformanceTestUtils::BuildGraphOfNodes(ramses::Scene& scene, uint32_t childrenPerNode, uint32_t depth, ramses::ERamsesObjectType nodeType)
{
    ramses::Node& rootNode = *CreateNode(scene, nodeType, 0xffffffff);
    BuildGraphLevel(scene, rootNode, childrenPerNode, depth, nodeType);
    return rootNode;
}

void PerformanceTestUtils::BuildGraphLevel(ramses::Scene& scene, ramses::Node& targetNode, uint32_t childrenPerNode, uint32_t currentDepth, ramses::ERamsesObjectType nodeType)
{
    if (currentDepth > 0)
    {
        for (uint32_t i = 0; i < childrenPerNode; i++)
        {
            ramses::Node& child = *CreateNode(scene, nodeType, currentDepth * 1000000u + i);
            targetNode.addChild(child);
            BuildGraphLevel(scene, child, childrenPerNode, currentDepth - 1, nodeType);
        }
    }
}

void PerformanceTestUtils::BuildNodesOfVariousTypes(std::vector<ramses::SceneObject*>& outputList, ramses::Scene& scene, uint32_t objectCount)
{
    for (uint32_t i = 0; i < objectCount; i++)
    {
        outputList.push_back(CreateObjectOfType(scene, i));
    }
}

ramses::SceneObject* PerformanceTestUtils::CreateObjectOfType(ramses::Scene& scene, uint32_t i)
{
    const uint32_t type = i % 25;

    // Try to come up with a somewhat realistic distribution of object types
    if (type < 20)
        return CreateNode(scene, ramses::ERamsesObjectType_Node, i);
    return CreateNode(scene, ramses::ERamsesObjectType_MeshNode, i);
}

void PerformanceTestUtils::BuildBranchOfNodes(ramses::Scene& scene, uint32_t nodeCount, ramses::Node*& createdRoot, ramses::Node*& createdMiddle, ramses::Node*& createdLeaf, ramses::ERamsesObjectType nodeType)
{
    // Expect references to null pointers
    assert(NULL == createdRoot);
    assert(NULL == createdMiddle);
    assert(NULL == createdLeaf);

    createdRoot = CreateNode(scene, nodeType, 0xffffffff);
    ramses::Node* currentNode = createdRoot;

    // Build up the hierarchy
    for (uint32_t i = 0; i < nodeCount; i++)
    {
        ramses::Node* newNode = CreateNode(scene, nodeType, i);
        currentNode->addChild(*newNode);

        // Pick a node halfway down the hierarchy
        if (i == nodeCount / 2)
        {
            createdMiddle = newNode;
        }

        currentNode = newNode;
    }

    createdLeaf = currentNode;

    // All references should point to non-null pointers now
    assert(NULL != createdRoot);
    assert(NULL != createdMiddle);
    assert(NULL != createdLeaf);
}

uint32_t PerformanceTestUtils::GetNextRandom(uint32_t seed)
{
    // Linear congruential generator. These constants are used by GCC.
    const uint32_t a = 1103515245;
    const uint32_t c = 12345;
    seed = (seed * a + c) & 0x7fffffff;
    return seed;
}

// Map 31 bits of random input to a given range [min...max]
uint32_t PerformanceTestUtils::MapToRange(uint32_t input, uint32_t min, uint32_t max)
{
    const double normalized = input / static_cast<double>(0x7fffffff);
    const double range = static_cast<double>(max - min);

    return static_cast<uint32_t>(normalized * range) + min;
}

ramses::Node* PerformanceTestUtils::CreateNode(ramses::Scene& scene, ramses::ERamsesObjectType type, uint32_t idxForNameGen)
{
    const ramses_internal::String nodeName = ramses_internal::StringUtils::HexFromNumber(idxForNameGen);

    switch (type)
    {
    case ramses::ERamsesObjectType_Node:
        return scene.createNode(nodeName.c_str());
    case ramses::ERamsesObjectType_MeshNode:
        return scene.createMeshNode(nodeName.c_str());
    default:
        assert(false && "Unsupported type");
        return 0;
    }
}
