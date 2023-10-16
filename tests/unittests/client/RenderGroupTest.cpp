//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ClientTestUtils.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/RamsesClient.h"
#include "ramses/client/Geometry.h"
#include "impl/RenderGroupImpl.h"
#include "impl/MeshNodeImpl.h"
#include "impl/RamsesClientImpl.h"
#include "internal/SceneGraph/SceneAPI/RenderGroup.h"
#include "internal/SceneGraph/SceneAPI/RenderGroupUtils.h"

using namespace testing;

namespace ramses::internal
{
    class ARenderGroup : public LocalTestClientWithScene, public testing::Test
    {
    protected:
        ARenderGroup()
            : renderGroup(*m_scene.createRenderGroup("RenderGroup"))
            , renderGroup2(*m_scene.createRenderGroup("RenderGroup2"))
        {
        }

        static void ExpectNumMeshesContained(uint32_t numMeshes, const ramses::RenderGroup& group)
        {
            EXPECT_EQ(numMeshes, static_cast<uint32_t>(group.impl().getAllMeshes().size()));
        }

        static void ExpectNumRenderGroupsContained(uint32_t numRenderGroups, const ramses::RenderGroup& group)
        {
            EXPECT_EQ(numRenderGroups, static_cast<uint32_t>(group.impl().getAllRenderGroups().size()));
        }

        void expectMeshContained(const MeshNode& mesh, int32_t order, const ramses::RenderGroup& group)
        {
            EXPECT_TRUE(group.containsMeshNode(mesh));
            int32_t actualOrder = 0;
            EXPECT_TRUE(group.getMeshNodeOrder(mesh, actualOrder));
            EXPECT_EQ(order, actualOrder);

            const auto& internalRg = m_internalScene.getRenderGroup(group.impl().getRenderGroupHandle());
            ASSERT_TRUE(ramses::internal::RenderGroupUtils::ContainsRenderable(mesh.impl().getRenderableHandle(), internalRg));
            EXPECT_EQ(order, ramses::internal::RenderGroupUtils::FindRenderableEntry(mesh.impl().getRenderableHandle(), internalRg)->order);
        }

        void expectRenderGroupContained(const ramses::RenderGroup& nestedRenderGroup, int32_t order, const ramses::RenderGroup& group)
        {
            EXPECT_TRUE(group.containsRenderGroup(nestedRenderGroup));
            int32_t actualOrder = 0;
            EXPECT_TRUE(group.getRenderGroupOrder(nestedRenderGroup, actualOrder));
            EXPECT_EQ(order, actualOrder);

            const auto& internalRg = m_internalScene.getRenderGroup(group.impl().getRenderGroupHandle());
            ASSERT_TRUE(ramses::internal::RenderGroupUtils::ContainsRenderGroup(nestedRenderGroup.impl().getRenderGroupHandle(), internalRg));
            EXPECT_EQ(order, ramses::internal::RenderGroupUtils::FindRenderGroupEntry(nestedRenderGroup.impl().getRenderGroupHandle(), internalRg)->order);
        }

        void expectMeshNotContained(const MeshNode& mesh, const ramses::RenderGroup& group)
        {
            EXPECT_FALSE(group.containsMeshNode(mesh));
            const auto& internalRg = m_internalScene.getRenderGroup(group.impl().getRenderGroupHandle());
            EXPECT_FALSE(ramses::internal::RenderGroupUtils::ContainsRenderable(mesh.impl().getRenderableHandle(), internalRg));
        }

        void expectRenderGroupNotContained(const ramses::RenderGroup& nestedRenderGroup, const ramses::RenderGroup& group)
        {
            EXPECT_FALSE(group.containsRenderGroup(nestedRenderGroup));
            const auto& internalRg = m_internalScene.getRenderGroup(group.impl().getRenderGroupHandle());
            EXPECT_FALSE(ramses::internal::RenderGroupUtils::ContainsRenderGroup(nestedRenderGroup.impl().getRenderGroupHandle(), internalRg));
        }

        void addValidMeshToRenderGroup()
        {
            MeshNode& mesh = createValidMeshNode();
            EXPECT_TRUE(renderGroup.addMeshNode(mesh, 3));
        }

        void addBrokenMeshToRenderGroup()
        {
            addBrokenMeshToRenderGroup(renderGroup);
        }

        void addBrokenMeshToRenderGroup(ramses::RenderGroup& renderGroupParam)
        {
            MeshNode* mesh = m_scene.createMeshNode();
            Appearance* appearance = m_scene.createAppearance(*TestEffects::CreateTestEffect(m_scene), "appearance");
            mesh->setAppearance(*appearance);
            // missing geometry binding
            EXPECT_TRUE(renderGroupParam.addMeshNode(*mesh, 3));
        }

        ramses::RenderGroup& renderGroup;
        ramses::RenderGroup& renderGroup2;
    };

    TEST_F(ARenderGroup, canValidate)
    {
        addValidMeshToRenderGroup();

        ValidationReport report;
        renderGroup.validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(ARenderGroup, validationGivesWarningIfRenderGroupIsEmpty)
    {
        ValidationReport report;
        renderGroup.validate(report);
        EXPECT_TRUE(report.hasIssue());
    }

    TEST_F(ARenderGroup, validationGivesWarningIfNestedRenderGroupIsEmpty)
    {
        addValidMeshToRenderGroup();
        ValidationReport report;
        renderGroup.validate(report);
        ASSERT_FALSE(report.hasIssue());

        renderGroup.addRenderGroup(renderGroup2);

        report.clear();
        renderGroup.validate(report);
        EXPECT_TRUE(report.hasIssue());
    }

    TEST_F(ARenderGroup, validatesIfEmptyButNestedRenderGroupIsNot)
    {
        // empty -> invalid
        ValidationReport report;
        renderGroup.validate(report);
        EXPECT_TRUE(report.hasIssue());

        // nested group also empty -> invalid
        renderGroup.addRenderGroup(renderGroup2);
        report.clear();
        renderGroup.validate(report);
        EXPECT_TRUE(report.hasIssue());

        // add mesh to nested group -> valid
        EXPECT_TRUE(renderGroup2.addMeshNode(createValidMeshNode(), 3));
        report.clear();
        renderGroup.validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(ARenderGroup, validationGivesWarningIfRenderGroupContainsInvalidMesh)
    {
        addBrokenMeshToRenderGroup();

        ValidationReport report;
        renderGroup.validate(report);
        EXPECT_TRUE(report.hasIssue());
    }

    TEST_F(ARenderGroup, validationGivesWarningIfNestedRenderGroupContainsInvalidMesh)
    {
        addValidMeshToRenderGroup();
        ValidationReport report;
        renderGroup.validate(report);
        ASSERT_FALSE(report.hasIssue());

        addBrokenMeshToRenderGroup(renderGroup2);

        renderGroup.validate(report);
        ASSERT_FALSE(report.hasIssue());

        ASSERT_TRUE(renderGroup.addRenderGroup(renderGroup2));

        report.clear();
        renderGroup.validate(report);
        EXPECT_TRUE(report.hasIssue());
    }

    TEST_F(ARenderGroup, canAddMeshNodes)
    {
        MeshNode* mesh = m_scene.createMeshNode();
        MeshNode* mesh2 = m_scene.createMeshNode();

        EXPECT_TRUE(renderGroup.addMeshNode(*mesh, 3));
        EXPECT_TRUE(renderGroup.addMeshNode(*mesh2, -7));

        ExpectNumMeshesContained(2u, renderGroup);
        expectMeshContained(*mesh, 3, renderGroup);
        expectMeshContained(*mesh2, -7, renderGroup);
    }

    TEST_F(ARenderGroup, canAddRenderGroups)
    {
        addValidMeshToRenderGroup();
        MeshNode& mesh2 = createValidMeshNode();
        ASSERT_TRUE(renderGroup2.addMeshNode(mesh2, 5));

        EXPECT_TRUE(renderGroup.addRenderGroup(renderGroup2, 2));

        ValidationReport report;
        renderGroup.validate(report);
        EXPECT_FALSE(report.hasIssue());
        renderGroup2.validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(ARenderGroup, reportsErrorWhenAddMeshNodeFromAnotherScene)
    {
        ramses::Scene& anotherScene = *client.createScene(sceneId_t(12u));
        MeshNode* mesh = anotherScene.createMeshNode();

        EXPECT_FALSE(renderGroup.addMeshNode(*mesh));
    }

    TEST_F(ARenderGroup, reportsErrorWhenAddRenderGroupFromAnotherScene)
    {
        ramses::Scene& anotherScene = *client.createScene(sceneId_t(12u));
        ramses::RenderGroup* nestedRenderGroup = anotherScene.createRenderGroup();

        EXPECT_FALSE(renderGroup.addRenderGroup(*nestedRenderGroup));
    }

    TEST_F(ARenderGroup, canCheckContainmentOfMeshes)
    {
        MeshNode* mesh = m_scene.createMeshNode();
        MeshNode* mesh2 = m_scene.createMeshNode();
        MeshNode* mesh3 = m_scene.createMeshNode();

        EXPECT_TRUE(renderGroup.addMeshNode(*mesh, 1));
        EXPECT_TRUE(renderGroup.addMeshNode(*mesh3, 3));

        ExpectNumMeshesContained(2u, renderGroup);
        expectMeshContained(*mesh, 1, renderGroup);
        expectMeshNotContained(*mesh2, renderGroup);
        expectMeshContained(*mesh3, 3, renderGroup);
    }

    TEST_F(ARenderGroup, canCheckContainmentOfNestedRenderGroups)
    {
        expectRenderGroupNotContained(renderGroup2, renderGroup);

        EXPECT_TRUE(renderGroup.addRenderGroup(renderGroup2, 2));

        ExpectNumRenderGroupsContained(1u, renderGroup);
        expectRenderGroupContained(renderGroup2, 2, renderGroup);
    }

    TEST_F(ARenderGroup, doesNotDirectlyContainRenderablesOfNestedRenderGroups)
    {
        MeshNode* mesh = m_scene.createMeshNode();

        ASSERT_TRUE(renderGroup2.addMeshNode(*mesh, 1));
        ASSERT_TRUE(renderGroup.addRenderGroup(renderGroup2));

        expectMeshNotContained(*mesh, renderGroup);
    }

    TEST_F(ARenderGroup, canRemoveMesh)
    {
        MeshNode* mesh = m_scene.createMeshNode();
        ASSERT_TRUE(renderGroup.addMeshNode(*mesh, 1));
        EXPECT_TRUE(renderGroup.removeMeshNode(*mesh));

        ExpectNumMeshesContained(0u, renderGroup);
        expectMeshNotContained(*mesh, renderGroup);
    }

    TEST_F(ARenderGroup, canRemoveRenderGroup)
    {
        ramses::RenderGroup* nestedRenderGroup = m_scene.createRenderGroup();
        ASSERT_TRUE(renderGroup.addRenderGroup(*nestedRenderGroup, 1));
        EXPECT_TRUE(renderGroup.removeRenderGroup(*nestedRenderGroup));

        ExpectNumRenderGroupsContained(0u, renderGroup);
        expectRenderGroupNotContained(*nestedRenderGroup, renderGroup);
    }

    TEST_F(ARenderGroup, removingMeshDoesNotAffectOtherMeshes)
    {
        MeshNode* mesh = m_scene.createMeshNode();
        MeshNode* mesh2 = m_scene.createMeshNode();
        MeshNode* mesh3 = m_scene.createMeshNode();

        EXPECT_TRUE(renderGroup.addMeshNode(*mesh, 1));
        EXPECT_TRUE(renderGroup.addMeshNode(*mesh2, 2));
        EXPECT_TRUE(renderGroup.addMeshNode(*mesh3, 3));

        EXPECT_TRUE(renderGroup.removeMeshNode(*mesh2));

        ExpectNumMeshesContained(2u, renderGroup);
        expectMeshContained(*mesh, 1, renderGroup);
        expectMeshNotContained(*mesh2, renderGroup);
        expectMeshContained(*mesh3, 3, renderGroup);
    }

    TEST_F(ARenderGroup, removingRenderGroupDoesNotAffectOtherRenderGroups)
    {
        ramses::RenderGroup* nestedRenderGroup1 = m_scene.createRenderGroup();
        ramses::RenderGroup* nestedRenderGroup2 = m_scene.createRenderGroup();
        ramses::RenderGroup* nestedRenderGroup3 = m_scene.createRenderGroup();

        ASSERT_TRUE(renderGroup.addRenderGroup(*nestedRenderGroup1, 1));
        ASSERT_TRUE(renderGroup.addRenderGroup(*nestedRenderGroup2, 2));
        ASSERT_TRUE(renderGroup.addRenderGroup(*nestedRenderGroup3, 3));

        ASSERT_TRUE(renderGroup.removeRenderGroup(*nestedRenderGroup2));

        ExpectNumRenderGroupsContained(2u, renderGroup);
        expectRenderGroupContained(*nestedRenderGroup1, 1, renderGroup);
        expectRenderGroupNotContained(*nestedRenderGroup2, renderGroup);
        expectRenderGroupContained(*nestedRenderGroup3, 3, renderGroup);
    }

    TEST_F(ARenderGroup, doesNotContainMeshWhichWasDestroyed)
    {
        MeshNode* mesh = m_scene.createMeshNode();
        ASSERT_TRUE(renderGroup.addMeshNode(*mesh));
        ASSERT_TRUE(m_scene.destroy(*mesh));

        ExpectNumMeshesContained(0u, renderGroup);
    }

    TEST_F(ARenderGroup, doesNotContainRenderGroupWhichWasDestroyed)
    {
        ramses::RenderGroup* nestedRenderGroup = m_scene.createRenderGroup();
        ASSERT_TRUE(renderGroup.addRenderGroup(*nestedRenderGroup));
        ASSERT_TRUE(m_scene.destroy(*nestedRenderGroup));

        ExpectNumRenderGroupsContained(0u, renderGroup);
    }

    TEST_F(ARenderGroup, destroyingMeshDoesNotAffectOtherMeshes)
    {
        MeshNode* mesh = m_scene.createMeshNode();
        MeshNode* mesh2 = m_scene.createMeshNode();
        MeshNode* mesh3 = m_scene.createMeshNode();

        EXPECT_TRUE(renderGroup.addMeshNode(*mesh, 1));
        EXPECT_TRUE(renderGroup.addMeshNode(*mesh2, 2));
        EXPECT_TRUE(renderGroup.addMeshNode(*mesh3, 3));

        EXPECT_TRUE(m_scene.destroy(*mesh2));

        ExpectNumMeshesContained(2u, renderGroup);
        expectMeshContained(*mesh, 1, renderGroup);
        expectMeshContained(*mesh3, 3, renderGroup);
    }

    TEST_F(ARenderGroup, destroyingRenderGroupDoesNotAffectOtherRenderGroups)
    {
        ramses::RenderGroup* nestedRenderGroup1 = m_scene.createRenderGroup();
        ramses::RenderGroup* nestedRenderGroup2 = m_scene.createRenderGroup();
        ramses::RenderGroup* nestedRenderGroup3 = m_scene.createRenderGroup();

        ASSERT_TRUE(renderGroup.addRenderGroup(*nestedRenderGroup1, 1));
        ASSERT_TRUE(renderGroup.addRenderGroup(*nestedRenderGroup2, 2));
        ASSERT_TRUE(renderGroup.addRenderGroup(*nestedRenderGroup3, 3));

        EXPECT_TRUE(m_scene.destroy(*nestedRenderGroup2));

        ExpectNumRenderGroupsContained(2u, renderGroup);
        expectRenderGroupContained(*nestedRenderGroup1, 1, renderGroup);
        expectRenderGroupContained(*nestedRenderGroup3, 3, renderGroup);
    }

    TEST_F(ARenderGroup, canRemoveAllMeshes)
    {
        MeshNode* mesh = m_scene.createMeshNode();
        MeshNode* mesh2 = m_scene.createMeshNode();
        MeshNode* mesh3 = m_scene.createMeshNode();

        EXPECT_TRUE(renderGroup.addMeshNode(*mesh, 1));
        EXPECT_TRUE(renderGroup.addMeshNode(*mesh2, 2));
        EXPECT_TRUE(renderGroup.addMeshNode(*mesh3, 3));

        EXPECT_TRUE(renderGroup.removeAllRenderables());

        ExpectNumMeshesContained(0u, renderGroup);
        expectMeshNotContained(*mesh, renderGroup);
        expectMeshNotContained(*mesh2, renderGroup);
        expectMeshNotContained(*mesh3, renderGroup);
    }

    TEST_F(ARenderGroup, canRemoveAllRenderGroups)
    {
        ramses::RenderGroup* nestedRenderGroup1 = m_scene.createRenderGroup();
        ramses::RenderGroup* nestedRenderGroup2 = m_scene.createRenderGroup();
        ramses::RenderGroup* nestedRenderGroup3 = m_scene.createRenderGroup();

        ASSERT_TRUE(renderGroup.addRenderGroup(*nestedRenderGroup1, 1));
        ASSERT_TRUE(renderGroup.addRenderGroup(*nestedRenderGroup2, 2));
        ASSERT_TRUE(renderGroup.addRenderGroup(*nestedRenderGroup3, 3));

        EXPECT_TRUE(renderGroup.removeAllRenderGroups());

        ExpectNumRenderGroupsContained(0u, renderGroup);
        expectRenderGroupNotContained(*nestedRenderGroup1, renderGroup);
        expectRenderGroupNotContained(*nestedRenderGroup2, renderGroup);
        expectRenderGroupNotContained(*nestedRenderGroup3, renderGroup);
    }

    TEST_F(ARenderGroup, reportsErrorWhenTryingToRemoveMeshWhichWasNotAdded)
    {
        MeshNode* mesh = m_scene.createMeshNode();
        bool status = renderGroup.removeMeshNode(*mesh);

        EXPECT_FALSE(status);
    }

    TEST_F(ARenderGroup, reportsErrorWhenTryingToRemoveRenderGroupWhichWasNotAdded)
    {
        ramses::RenderGroup* nestedRenderGroup = m_scene.createRenderGroup();
        EXPECT_FALSE(renderGroup.removeRenderGroup(*nestedRenderGroup));
    }

    TEST_F(ARenderGroup, canChangeMeshOrderByReaddingIt)
    {
        MeshNode* mesh = m_scene.createMeshNode();

        EXPECT_TRUE(renderGroup.addMeshNode(*mesh, 1));
        EXPECT_TRUE(renderGroup.addMeshNode(*mesh, 999));

        expectMeshContained(*mesh, 999, renderGroup);
    }

    TEST_F(ARenderGroup, canChangeRenderGroupOrderByReaddingIt)
    {
        ramses::RenderGroup* nestedRenderGroup = m_scene.createRenderGroup();

        ASSERT_TRUE(renderGroup.addRenderGroup(*nestedRenderGroup, 1));
        ASSERT_TRUE(renderGroup.addRenderGroup(*nestedRenderGroup, 999));

        expectRenderGroupContained(*nestedRenderGroup, 999, renderGroup);
    }

    TEST_F(ARenderGroup, canChangeMeshOrderByRemovingItAndAddingAgain)
    {
        MeshNode* mesh = m_scene.createMeshNode();

        EXPECT_TRUE(renderGroup.addMeshNode(*mesh, 1));
        EXPECT_TRUE(renderGroup.removeMeshNode(*mesh));
        EXPECT_TRUE(renderGroup.addMeshNode(*mesh, 999));

        expectMeshContained(*mesh, 999, renderGroup);
    }

    TEST_F(ARenderGroup, canChangeRenderGroupOrderByRemovingItAndAddingAgain)
    {
        ramses::RenderGroup* nestedRenderGroup = m_scene.createRenderGroup();

        ASSERT_TRUE(renderGroup.addRenderGroup(*nestedRenderGroup, 1));
        ASSERT_TRUE(renderGroup.removeRenderGroup(*nestedRenderGroup));
        ASSERT_TRUE(renderGroup.addRenderGroup(*nestedRenderGroup, 999));

        expectRenderGroupContained(*nestedRenderGroup, 999, renderGroup);
    }

    TEST_F(ARenderGroup, meshCanBeAddedToTwoGroupsWithDifferentOrder)
    {
        MeshNode* mesh = m_scene.createMeshNode();

        EXPECT_TRUE(renderGroup.addMeshNode(*mesh, 3));
        EXPECT_TRUE(renderGroup2.addMeshNode(*mesh, 999));

        ExpectNumMeshesContained(1u, renderGroup);
        ExpectNumMeshesContained(1u, renderGroup2);
        expectMeshContained(*mesh, 3, renderGroup);
        expectMeshContained(*mesh, 999, renderGroup2);
    }

    TEST_F(ARenderGroup, renderGroupCanBeAddedToTwoGroupsWithDifferentOrder)
    {
        ramses::RenderGroup* nestedRenderGroup = m_scene.createRenderGroup();

        EXPECT_TRUE(renderGroup.addRenderGroup(*nestedRenderGroup, 3));
        EXPECT_TRUE(renderGroup2.addRenderGroup(*nestedRenderGroup, 999));

        ExpectNumRenderGroupsContained(1u, renderGroup);
        ExpectNumRenderGroupsContained(1u, renderGroup2);
        expectRenderGroupContained(*nestedRenderGroup, 3, renderGroup);
        expectRenderGroupContained(*nestedRenderGroup, 999, renderGroup2);
    }

    TEST_F(ARenderGroup, removingMeshFromGroupDoesNotAffectItsPlaceInAnotherGroup)
    {
        MeshNode* mesh = m_scene.createMeshNode();

        EXPECT_TRUE(renderGroup.addMeshNode(*mesh, 3));
        EXPECT_TRUE(renderGroup2.addMeshNode(*mesh, 999));

        EXPECT_TRUE(renderGroup.removeMeshNode(*mesh));

        ExpectNumMeshesContained(0u, renderGroup);
        ExpectNumMeshesContained(1u, renderGroup2);
        expectMeshNotContained(*mesh, renderGroup);
        expectMeshContained(*mesh, 999, renderGroup2);
    }

    TEST_F(ARenderGroup, removingRenderGroupFromGroupDoesNotAffectItsPlaceInAnotherGroup)
    {
        ramses::RenderGroup* nestedRenderGroup = m_scene.createRenderGroup();

        ASSERT_TRUE(renderGroup.addRenderGroup(*nestedRenderGroup, 3));
        ASSERT_TRUE(renderGroup2.addRenderGroup(*nestedRenderGroup, 999));

        EXPECT_TRUE(renderGroup.removeRenderGroup(*nestedRenderGroup));

        ExpectNumRenderGroupsContained(0u, renderGroup);
        ExpectNumRenderGroupsContained(1u, renderGroup2);
        expectRenderGroupNotContained(*nestedRenderGroup, renderGroup);
        expectRenderGroupContained(*nestedRenderGroup, 999, renderGroup2);
    }

    TEST_F(ARenderGroup, destroyedMeshIsRemovedFromAllItsGroups)
    {
        MeshNode* mesh = m_scene.createMeshNode();

        EXPECT_TRUE(renderGroup.addMeshNode(*mesh, 3));
        EXPECT_TRUE(renderGroup2.addMeshNode(*mesh, 999));

        EXPECT_TRUE(m_scene.destroy(*mesh));

        ExpectNumMeshesContained(0u, renderGroup);
        ExpectNumMeshesContained(0u, renderGroup2);
    }

    TEST_F(ARenderGroup, destroyedNestedRenderGroupIsRemovedFromAllItsGroups)
    {
        ramses::RenderGroup* nestedRenderGroup = m_scene.createRenderGroup();

        ASSERT_TRUE(renderGroup.addRenderGroup(*nestedRenderGroup, 3));
        ASSERT_TRUE(renderGroup2.addRenderGroup(*nestedRenderGroup, 999));

        ASSERT_TRUE(m_scene.destroy(*nestedRenderGroup));

        ExpectNumRenderGroupsContained(0u, renderGroup);
        ExpectNumRenderGroupsContained(0u, renderGroup2);
    }
}
