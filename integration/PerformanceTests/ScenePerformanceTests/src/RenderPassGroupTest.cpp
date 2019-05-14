//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RenderPassGroupTest.h"
#include "PerformanceTestUtils.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/RamsesObjectTypes.h"
#include "ramses-client-api/RamsesObject.h"
#include "ramses-client-api/SceneObject.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/Node.h"

RenderPassGroupTest::RenderPassGroupTest(ramses_internal::String testName, uint32_t testState) : PerformanceTestBase(testName, testState) {};

void RenderPassGroupTest::initTest(ramses::RamsesClient& client, ramses::Scene& scene)
{
    UNUSED(client);
    m_scene = &scene;

    m_meshes.reserve(MeshCount);

    for (uint32_t i = 0; i < MeshCount; i++)
    {
        m_meshes.push_back(scene.createMeshNode());
    }

    // Keep meshes in a different order than created on the scene (probably not that important)
    PerformanceTestUtils::ShuffleObjectList(m_meshes);

    switch (m_testState)
    {
    case RenderPassGroupTest_FlatGroups_Creation:
    case RenderPassGroupTest_FlatGroups_Destruction:
    {
        m_useNestedRenderGroups = false;
        break;
    }
    case RenderPassGroupTest_NestedGroups_Creation:
    case RenderPassGroupTest_NestedGroups_Destruction:
    {
        m_useNestedRenderGroups = true;
        break;
    }
    default:
    {
        assert(false);
        break;
    }
    }
}

void RenderPassGroupTest::preUpdate()
{
    switch (m_testState)
    {
    case RenderPassGroupTest_FlatGroups_Creation:
    case RenderPassGroupTest_NestedGroups_Creation:
    {
        // Destroy stuff from last frame
        destroyThings();
        break;
    }
    case RenderPassGroupTest_FlatGroups_Destruction:
    case RenderPassGroupTest_NestedGroups_Destruction:
    {
        createThings();
        break;
    }
    default:
    {
        assert(false);
        break;
    }
    }
}

void RenderPassGroupTest::update()
{
    switch (m_testState)
    {
    case RenderPassGroupTest_FlatGroups_Creation:
    case RenderPassGroupTest_NestedGroups_Creation:
    {
        createThings();
        break;
    }
    case RenderPassGroupTest_FlatGroups_Destruction:
    case RenderPassGroupTest_NestedGroups_Destruction:
    {
        destroyThings();
        break;
    }
    default:
    {
        assert(false);
        break;
    }
    }
}

void RenderPassGroupTest::addRenderGroups()
{
    assert(m_groups.empty());

    for (uint32_t i = 0; i < RenderGroupCount; i++)
    {
        m_groups.push_back(m_scene->createRenderGroup());
    }
}

void RenderPassGroupTest::addRenderPasses()
{
    assert(m_passes.empty());

    for (uint32_t i = 0; i < RenderPassCount; i++)
    {
        m_passes.push_back(m_scene->createRenderPass());
    }
}

void RenderPassGroupTest::shuffleGroupsAndPasses()
{
    PerformanceTestUtils::ShuffleObjectList(m_groups);
    PerformanceTestUtils::ShuffleObjectList(m_passes);
}

void RenderPassGroupTest::assignGroupsAndPasses()
{
    static_assert(MeshCount > RenderGroupCount, "Need more meshes than render groups");
    static_assert(RenderGroupCount > RenderPassCount, "Need more render groups than render passes");
    assert(m_meshes.size() == MeshCount);
    assert(m_groups.size() == RenderGroupCount);
    assert(m_passes.size() == RenderPassCount);

    if (m_useNestedRenderGroups)
    {
        std::vector<ramses::RenderGroup*> unassigned(m_groups);

        ramses::RenderGroup& parent = *unassigned.back();
        unassigned.pop_back();
        assignNestedGroup(parent, unassigned);
        assert(unassigned.empty());
    }

    for (uint32_t i = 0; i < m_meshes.size(); i++)
    {
        uint32_t groupIndex = i % m_groups.size();
        m_groups[groupIndex]->addMeshNode(*m_meshes[i]);
    }

    for (uint32_t i = 0; i < m_groups.size(); i++)
    {
        uint32_t passIndex = i % m_passes.size();
        m_passes[passIndex]->addRenderGroup(*m_groups[i]);
    }
}

void RenderPassGroupTest::assignNestedGroup(ramses::RenderGroup& parent, std::vector<ramses::RenderGroup*>& unassigned)
{
    ramses::RenderGroup* currentGroup;

    for (uint32_t i = 0; i < 10; i++)
    {
        if (!unassigned.empty())
        {
            currentGroup = unassigned.back();
            unassigned.pop_back();

            parent.addRenderGroup(*currentGroup);
        }
        else
        {
            return;
        }
    }

    if (!unassigned.empty())
    {
        assert(currentGroup != NULL);
        assignNestedGroup(*currentGroup, unassigned);
    }
}

void RenderPassGroupTest::createThings()
{
    addRenderGroups();
    addRenderPasses();
    shuffleGroupsAndPasses();
    assignGroupsAndPasses();
}

void RenderPassGroupTest::destroyThings()
{
    for (uint32_t i = 0; i < m_groups.size(); i++)
    {
        m_scene->destroy(*m_groups[i]);
    }

    for (uint32_t i = 0; i < m_passes.size(); i++)
    {
        m_scene->destroy(*m_passes[i]);
    }

    m_groups.clear();
    m_passes.clear();
}
