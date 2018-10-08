//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PerformanceTestData.h"
#include "PerformanceTestBase.h"
#include "PerformanceTestAssert.h"

#include "TransformationHierarchyTest.h"
#include "BoundingSphereTest.h"
#include "GetObjectTypeTest.h"
#include "ObjectRegistryTest.h"
#include "NodeVisibilityPerfTest.h"
#include "RenderPassGroupTest.h"
#include "DefaultRendererCacheTest.h"
#include "MemoryPoolTest.h"
#include "NodeTopologyTest.h"
#include "StringLayoutingPerformanceTest.h"

namespace ramses_internal {

PerformanceTestData::PerformanceTestData(ramses_internal::String filterIn, ramses_internal::String filterOut)
{
    filterIn.toLowerCase();
    filterOut.toLowerCase();

    m_filterIn = filterIn;
    m_filterOut = filterOut;

    init();
}

void PerformanceTestData::init()
{
    // Testing cache of transformation data on nodes. The basic idea is to modify a single node in different locations in a scene with deeply nested hierarchical nodes.
    // If the root node is modified, it will cause propagated changes all the way down (thus expensive). If a leaf node is modified, the upper part of the scene nodes
    // should remain unchanged (thus cheap).
    {
        PerformanceTestBase* rootNodeTest = createTest<TransformationHierarchyTest>("TransformationHierarchyTest_RootNode", TransformationHierarchyTest::TransformationHierarchyTest_Root);
        PerformanceTestBase* halfWayNodeTest = createTest<TransformationHierarchyTest>("TransformationHierarchyTest_HalfWayNode", TransformationHierarchyTest::TransformationHierarchyTest_HalfWayNode);
        PerformanceTestBase* leafNodeTest = createTest<TransformationHierarchyTest>("TransformationHierarchyTest_LeafNode", TransformationHierarchyTest::TransformationHierarchyTest_Leaf);

        createAssert(rootNodeTest).isSlowerThan(halfWayNodeTest);
        createAssert(leafNodeTest).isMuchFasterThan(halfWayNodeTest, 100.f);
    }

    {
        PerformanceTestBase* binary_1K = createTest<BoundingSphereTest>("BoundingSphereTest_1K_BinaryTree", BoundingSphereTest::BoundingSphereTest_1K_BinaryTree);
        PerformanceTestBase* binary_4K = createTest<BoundingSphereTest>("BoundingSphereTest_4K_BinaryTree", BoundingSphereTest::BoundingSphereTest_4K_BinaryTree);
        PerformanceTestBase* binary_16K = createTest<BoundingSphereTest>("BoundingSphereTest_16K_BinaryTree", BoundingSphereTest::BoundingSphereTest_16K_BinaryTree);

        PerformanceTestBase* spreadOut_1K = createTest<BoundingSphereTest>("BoundingSphereTest_1K_SpreadOutTree", BoundingSphereTest::BoundingSphereTest_1K_SpreadOutTree);
        PerformanceTestBase* spreadOut_4K = createTest<BoundingSphereTest>("BoundingSphereTest_4K_SpreadOutTree", BoundingSphereTest::BoundingSphereTest_4K_SpreadOutTree);
        PerformanceTestBase* spreadOut_16K = createTest<BoundingSphereTest>("BoundingSphereTest_16K_SpreadOutTree", BoundingSphereTest::BoundingSphereTest_16K_SpreadOutTree);

        createAssert(binary_1K).isMuchFasterThan(binary_4K, 2.f);
        createAssert(binary_4K).isMuchFasterThan(binary_16K, 2.f);
        createAssert(spreadOut_1K).isMuchFasterThan(spreadOut_4K, 2.f);
        createAssert(spreadOut_4K).isMuchFasterThan(spreadOut_16K, 2.f);
    }

    {
        PerformanceTestBase* getTypeTest = createTest<GetObjectTypeTest>("GetObjectTypeTest_GetType", GetObjectTypeTest::GetObjectTypeTest_GetType);
        PerformanceTestBase* isOfTypeTest = createTest<GetObjectTypeTest>("GetObjectTypeTest_IsOfType", GetObjectTypeTest::GetObjectTypeTest_IsOfType);

        createAssert(getTypeTest).isMuchFasterThan(isOfTypeTest, 1.2f);
    }

    {
        // A "deep" graph means that every node has exactly one child. This results in a long list/branch of nodes.
        PerformanceTestBase* deepRoot = createTest<NodeVisibilityPerfTest>("NodeVisibilityTest_DeepGraph_RootNode", NodeVisibilityPerfTest::NodeVisibilityTest_DeepGraph_RootNode);
        PerformanceTestBase* deepMiddle = createTest<NodeVisibilityPerfTest>("NodeVisibilityTest_DeepGraph_MiddleNode", NodeVisibilityPerfTest::NodeVisibilityTest_DeepGraph_MiddleNode);
        PerformanceTestBase* deepLeaf = createTest<NodeVisibilityPerfTest>("NodeVisibilityTest_DeepGraph_LeafNode", NodeVisibilityPerfTest::NodeVisibilityTest_DeepGraph_LeafNode);
        createAssert(deepRoot).isSlowerThan(deepMiddle);
        createAssert(deepMiddle).isSlowerThan(deepLeaf);

        // A "wide" graph is in this context a binary tree, meaning that every node has exactly two children. The node count is the same as in the wide graph tests.
        createTest<NodeVisibilityPerfTest>("NodeVisibilityTest_WideGraph_RootNode", NodeVisibilityPerfTest::NodeVisibilityTest_WideGraph_RootNode);
    }

    {
        PerformanceTestBase* flatCreation = createTest<RenderPassGroupTest>("RenderPassGroupTest_FlatGroups_Creation", RenderPassGroupTest::RenderPassGroupTest_FlatGroups_Creation);
        PerformanceTestBase* nestedCreation = createTest<RenderPassGroupTest>("RenderPassGroupTest_NestedGroups_Creation", RenderPassGroupTest::RenderPassGroupTest_NestedGroups_Creation);
        createAssert(flatCreation).isSameSpeedAs(nestedCreation, 1.3f);

        PerformanceTestBase* flatDestruction = createTest<RenderPassGroupTest>("RenderPassGroupTest_FlatGroups_Destruction", RenderPassGroupTest::RenderPassGroupTest_FlatGroups_Destruction);
        PerformanceTestBase* nestedDestruction = createTest<RenderPassGroupTest>("RenderPassGroupTest_NestedGroups_Destruction", RenderPassGroupTest::RenderPassGroupTest_NestedGroups_Destruction);
        createAssert(flatDestruction).isSameSpeedAs(nestedDestruction, 1.3f);
    }

    {
        // Currently it makes no sense to try to assert any performance characteristics
        createTest<DefaultRendererCacheTest>("DefaultRendererCacheTest_HasResource_Positive", DefaultRendererCacheTest::DefaultRendererCacheTest_HasResource_Positive);
        createTest<DefaultRendererCacheTest>("DefaultRendererCacheTest_HasResource_Negative", DefaultRendererCacheTest::DefaultRendererCacheTest_HasResource_Negative);
        createTest<DefaultRendererCacheTest>("DefaultRendererCacheTest_GetResource", DefaultRendererCacheTest::DefaultRendererCacheTest_GetResource);
        createTest<DefaultRendererCacheTest>("DefaultRendererCacheTest_StoreResource", DefaultRendererCacheTest::DefaultRendererCacheTest_StoreResource);
    }

    {
        PerformanceTestBase* addTest = createTest<ObjectRegistryTest>("ObjectRegistryTest_Add", ObjectRegistryTest::ObjectRegistryTest_Add);
        PerformanceTestBase* containsTest = createTest<ObjectRegistryTest>("ObjectRegistryTest_FindByName", ObjectRegistryTest::ObjectRegistryTest_FindByName);
        PerformanceTestBase* deleteTest = createTest<ObjectRegistryTest>("ObjectRegistryTest_Delete", ObjectRegistryTest::ObjectRegistryTest_Delete);

        createAssert(containsTest).isFasterThan(addTest);
        createAssert(containsTest).isFasterThan(deleteTest);
    }

    {
        PerformanceTestBase* sequential        = createTest<MemoryPoolTest>("MemoryPoolTest_AddRemoveSequential",               MemoryPoolTest::MemoryPoolTest_AddRemoveSequential);
        PerformanceTestBase* sequential_alloc  = createTest<MemoryPoolTest>("MemoryPoolTest_AddRemoveSequential_Preallocated",  MemoryPoolTest::MemoryPoolTest_AddRemoveSequential_Preallocated);
        PerformanceTestBase* shuffled          = createTest<MemoryPoolTest>("MemoryPoolTest_AddRemoveShuffled",                 MemoryPoolTest::MemoryPoolTest_AddRemoveShuffled);
        PerformanceTestBase* shuffled_alloc    = createTest<MemoryPoolTest>("MemoryPoolTest_AddRemoveShuffled_Preallocated",    MemoryPoolTest::MemoryPoolTest_AddRemoveShuffled_Preallocated);
        PerformanceTestBase* interleaved       = createTest<MemoryPoolTest>("MemoryPoolTest_AddRemoveInterleaved",              MemoryPoolTest::MemoryPoolTest_AddRemoveInterleaved);
        PerformanceTestBase* interleaved_alloc = createTest<MemoryPoolTest>("MemoryPoolTest_AddRemoveInterleaved_Preallocated", MemoryPoolTest::MemoryPoolTest_AddRemoveInterleaved_Preallocated);
        PerformanceTestBase* shuffled_explicit          = createTest<MemoryPoolTest>("MemoryPoolTest_Explicit_AddRemoveShuffled",                 MemoryPoolTest::MemoryPoolTest_Explicit_AddRemoveShuffled);
        PerformanceTestBase* shuffled_explicit_alloc    = createTest<MemoryPoolTest>("MemoryPoolTest_Explicit_AddRemoveShuffled_Preallocated",    MemoryPoolTest::MemoryPoolTest_Explicit_AddRemoveShuffled_Preallocated);
        PerformanceTestBase* interleaved_explicit       = createTest<MemoryPoolTest>("MemoryPoolTest_Explicit_AddRemoveInterleaved",              MemoryPoolTest::MemoryPoolTest_Explicit_AddRemoveInterleaved);
        PerformanceTestBase* interleaved_explicit_alloc = createTest<MemoryPoolTest>("MemoryPoolTest_Explicit_AddRemoveInterleaved_Preallocated", MemoryPoolTest::MemoryPoolTest_Explicit_AddRemoveInterleaved_Preallocated);

        createAssert(sequential_alloc).isFasterThan(sequential);
        createAssert(shuffled_alloc).isFasterThan(shuffled);
        createAssert(interleaved_alloc).isFasterThan(interleaved);

        createAssert(shuffled_explicit_alloc).isSameSpeedAs(shuffled_explicit, 1.5f);
        createAssert(interleaved_explicit_alloc).isSameSpeedAs(interleaved_explicit, 1.5f);
        createAssert(shuffled_explicit).isFasterThan(shuffled);
        createAssert(interleaved_explicit).isFasterThan(interleaved);
    }

    {
        PerformanceTestBase* removeIndividuallyTest = createTest<NodeTopologyTest>("NodeTopologyTest_RemoveNodesIndividually", NodeTopologyTest::NodeTopologyTest_RemoveNodesIndividually);
        PerformanceTestBase* removebyDestroyTest = createTest<NodeTopologyTest>("NodeTopologyTest_RemoveNodesbyDestroyingParent", NodeTopologyTest::NodeTopologyTest_RemoveNodesbyDestroyingParent);

        createAssert(removebyDestroyTest).isFasterThan(removeIndividuallyTest);
    }

    {
        createTest<StringLayoutingPerformanceTest>("StringLayoutingPerformanceTest_LayoutBigString", StringLayoutingPerformanceTest::StringLayoutingPerformanceTest_LayoutBigString);
    }
}

PerformanceTestData::~PerformanceTestData()
{
    for (uint32_t i = 0; i < m_tests.size(); i++)
    {
        delete m_tests[i];
    }

    for (uint32_t i = 0; i < m_testAsserts.size(); i++)
    {
        delete m_testAsserts[i];
    }
}

PerformanceTestAssert& PerformanceTestData::createAssert(PerformanceTestBase* inputTest)
{
    PerformanceTestAssert* result = new PerformanceTestAssert(inputTest);
    m_testAsserts.push_back(result);
    return *result;
}

uint32_t PerformanceTestData::getTestCount() const
{
    return static_cast<uint32_t>(m_tests.size());
}

const PerformanceTestBaseVector& PerformanceTestData::getTests() const
{
    return m_tests;
}

bool PerformanceTestData::isMatchedByFilter(ramses_internal::String testName) const
{
    static String notProvided("*");
    testName.toLowerCase();

    bool filteredIn = m_filterIn == notProvided || testName.find(m_filterIn) != -1;
    bool filteredOut = m_filterOut != notProvided && testName.find(m_filterOut) != -1;

    return filteredIn && !filteredOut;
}
}
