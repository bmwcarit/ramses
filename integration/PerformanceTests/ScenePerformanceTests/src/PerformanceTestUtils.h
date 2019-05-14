//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PERFORMANCETESTUTILS_H
#define RAMSES_PERFORMANCETESTUTILS_H

#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-client-api/RamsesObjectTypes.h"
#include "Collections/Vector.h"

namespace ramses
{
    class Node;
    class Scene;
    class SceneObject;
}

class PerformanceTestUtils
{
public:

    static ramses::Node& BuildGraphOfNodes(ramses::Scene& scene, uint32_t childrenPerNode, uint32_t depth, ramses::ERamsesObjectType nodeType);

    static void BuildNodesOfVariousTypes(std::vector<ramses::SceneObject*>& outputList, ramses::Scene& scene, uint32_t objectCount);

    // Create a connected graph where all nodes has exactly one child (essentially a linked list). For convenience the most relevant nodes are returned as pointers.
    static void BuildBranchOfNodes(ramses::Scene& scene, uint32_t nodeCount, ramses::Node*& createdRoot, ramses::Node*& createdMiddle, ramses::Node*& createdLeaf, ramses::ERamsesObjectType nodeType);

    template<typename T> static void ShuffleObjectList(std::vector<T>& list);

private:
    static void BuildGraphLevel(ramses::Scene& scene, ramses::Node& targetNode, uint32_t childrenPerNode, uint32_t currentDepth, ramses::ERamsesObjectType nodeType);

    static ramses::SceneObject* CreateObjectOfType(ramses::Scene& scene, uint32_t i);

    static uint32_t GetNextRandom(uint32_t seed);
    static uint32_t MapToRange(uint32_t input, uint32_t min, uint32_t max);

    // It would be nicer with a templated approach, but this is simpler (for now)
    static ramses::Node* CreateNode(ramses::Scene& scene, ramses::ERamsesObjectType type, uint32_t idxForNameGen);
};

template<typename T>
void PerformanceTestUtils::ShuffleObjectList(std::vector<T>& list)
{
    // ramses_capu::random does not support setting the seed value, so we need to re-invent the wheel a bit here, in order to get deterministic values across platforms.
    uint32_t randomSeed = 0;
    uint32_t listSize = static_cast<uint32_t>(list.size());

    // Fisher-Yates shuffle
    for (uint32_t i = 0; i < listSize; i++)
    {
        // Generate next random number in sequence (will return 31 bits of random data)
        randomSeed = GetNextRandom(randomSeed);

        // Map the 31 bits to a random number between 'i' and 'list.size'
        uint32_t j = MapToRange(randomSeed, i, listSize);

        // Swap
        T temp = list[i];
        list[i] = list[j];
        list[j] = temp;
    }
}

#endif
