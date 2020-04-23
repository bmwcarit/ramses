//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ClientTestUtils.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/GeometryBinding.h"
#include "RenderGroupImpl.h"
#include "MeshNodeImpl.h"
#include "RamsesClientImpl.h"
#include "SceneAPI/RenderGroup.h"
#include "SceneAPI/RenderGroupUtils.h"

using namespace testing;
using namespace ramses_internal;

namespace ramses
{
    class ARenderGroup : public LocalTestClientWithSceneAndAnimationSystem, public testing::Test
    {
    protected:
        ARenderGroup()
            : LocalTestClientWithSceneAndAnimationSystem()
            , renderGroup(*m_scene.createRenderGroup("RenderGroup"))
            , renderGroup2(*m_scene.createRenderGroup("RenderGroup2"))
        {
        }

        void expectNumMeshesContained(uint32_t numMeshes, const RenderGroup& group)
        {
            EXPECT_EQ(numMeshes, static_cast<uint32_t>(group.impl.getAllMeshes().size()));
        }

        void expectNumRenderGroupsContained(uint32_t numRenderGroups, const RenderGroup& group)
        {
            EXPECT_EQ(numRenderGroups, static_cast<uint32_t>(group.impl.getAllRenderGroups().size()));
        }

        void expectMeshContained(const MeshNode& mesh, int32_t order, const RenderGroup& group)
        {
            EXPECT_TRUE(group.containsMeshNode(mesh));
            int32_t actualOrder = 0;
            EXPECT_EQ(StatusOK, group.getMeshNodeOrder(mesh, actualOrder));
            EXPECT_EQ(order, actualOrder);

            const auto& internalRg = m_internalScene.getRenderGroup(group.impl.getRenderGroupHandle());
            ASSERT_TRUE(ramses_internal::RenderGroupUtils::ContainsRenderable(mesh.impl.getRenderableHandle(), internalRg));
            EXPECT_EQ(order, ramses_internal::RenderGroupUtils::FindRenderableEntry(mesh.impl.getRenderableHandle(), internalRg)->order);
        }

        void expectRenderGroupContained(const RenderGroup& nestedRenderGroup, int32_t order, const RenderGroup& group)
        {
            EXPECT_TRUE(group.containsRenderGroup(nestedRenderGroup));
            int32_t actualOrder = 0;
            EXPECT_EQ(StatusOK, group.getRenderGroupOrder(nestedRenderGroup, actualOrder));
            EXPECT_EQ(order, actualOrder);

            const auto& internalRg = m_internalScene.getRenderGroup(group.impl.getRenderGroupHandle());
            ASSERT_TRUE(ramses_internal::RenderGroupUtils::ContainsRenderGroup(nestedRenderGroup.impl.getRenderGroupHandle(), internalRg));
            EXPECT_EQ(order, ramses_internal::RenderGroupUtils::FindRenderGroupEntry(nestedRenderGroup.impl.getRenderGroupHandle(), internalRg)->order);
        }

        void expectMeshNotContained(const MeshNode& mesh, const RenderGroup& group)
        {
            EXPECT_FALSE(group.containsMeshNode(mesh));
            const auto& internalRg = m_internalScene.getRenderGroup(group.impl.getRenderGroupHandle());
            EXPECT_FALSE(ramses_internal::RenderGroupUtils::ContainsRenderable(mesh.impl.getRenderableHandle(), internalRg));
        }

        void expectRenderGroupNotContained(const RenderGroup& nestedRenderGroup, const RenderGroup& group)
        {
            EXPECT_FALSE(group.containsRenderGroup(nestedRenderGroup));
            const auto& internalRg = m_internalScene.getRenderGroup(group.impl.getRenderGroupHandle());
            EXPECT_FALSE(ramses_internal::RenderGroupUtils::ContainsRenderGroup(nestedRenderGroup.impl.getRenderGroupHandle(), internalRg));
        }

        void addValidMeshToRenderGroup()
        {
            MeshNode& mesh = createValidMeshNode();
            EXPECT_EQ(StatusOK, renderGroup.addMeshNode(mesh, 3));
        }

        void addBrokenMeshToRenderGroup()
        {
            addBrokenMeshToRenderGroup(renderGroup);
        }

        void addBrokenMeshToRenderGroup(RenderGroup& renderGroupParam)
        {
            MeshNode* mesh = m_scene.createMeshNode();
            Appearance* appearance = m_scene.createAppearance(*TestEffects::CreateTestEffect(client), "appearance");
            mesh->setAppearance(*appearance);
            // missing geometry binding
            EXPECT_EQ(StatusOK, renderGroupParam.addMeshNode(*mesh, 3));
        }

        RenderGroup& renderGroup;
        RenderGroup& renderGroup2;
    };

    TEST_F(ARenderGroup, canValidate)
    {
        addValidMeshToRenderGroup();

        EXPECT_EQ(StatusOK, renderGroup.validate());
    }

    TEST_F(ARenderGroup, validationGivesWarningIfRenderGroupIsEmpty)
    {
        EXPECT_NE(StatusOK, renderGroup.validate());
    }

    TEST_F(ARenderGroup, validationGivesWarningIfNestedRenderGroupIsEmpty)
    {
        addValidMeshToRenderGroup();
        ASSERT_EQ(StatusOK, renderGroup.validate());

        renderGroup.addRenderGroup(renderGroup2);

        EXPECT_NE(StatusOK, renderGroup.validate());
    }

    TEST_F(ARenderGroup, validationGivesWarningIfRenderGroupContainsInvalidMesh)
    {
        addBrokenMeshToRenderGroup();

        EXPECT_NE(StatusOK, renderGroup.validate());
    }

    TEST_F(ARenderGroup, validationGivesWarningIfNestedRenderGroupContainsInvalidMesh)
    {
        addValidMeshToRenderGroup();
        ASSERT_EQ(StatusOK, renderGroup.validate());

        addBrokenMeshToRenderGroup(renderGroup2);

        ASSERT_EQ(StatusOK, renderGroup.validate());

        ASSERT_EQ(StatusOK, renderGroup.addRenderGroup(renderGroup2));

        EXPECT_NE(StatusOK, renderGroup.validate());
    }

    TEST_F(ARenderGroup, canAddMeshNodes)
    {
        MeshNode* mesh = m_scene.createMeshNode();
        MeshNode* mesh2 = m_scene.createMeshNode();

        EXPECT_EQ(StatusOK, renderGroup.addMeshNode(*mesh, 3));
        EXPECT_EQ(StatusOK, renderGroup.addMeshNode(*mesh2, -7));

        expectNumMeshesContained(2u, renderGroup);
        expectMeshContained(*mesh, 3, renderGroup);
        expectMeshContained(*mesh2, -7, renderGroup);
    }

    TEST_F(ARenderGroup, canAddRenderGroups)
    {
        addValidMeshToRenderGroup();
        MeshNode& mesh2 = createValidMeshNode();
        ASSERT_EQ(StatusOK, renderGroup2.addMeshNode(mesh2, 5));

        EXPECT_EQ(StatusOK, renderGroup.addRenderGroup(renderGroup2, 2));

        EXPECT_EQ(StatusOK, renderGroup.validate());
        EXPECT_EQ(StatusOK, renderGroup2.validate());
    }

    TEST_F(ARenderGroup, reportsErrorWhenAddMeshNodeFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(sceneId_t(12u));
        MeshNode* mesh = anotherScene.createMeshNode();

        EXPECT_NE(StatusOK, renderGroup.addMeshNode(*mesh));
    }

    TEST_F(ARenderGroup, reportsErrorWhenAddRenderGroupFromAnotherScene)
    {
        Scene& anotherScene = *client.createScene(sceneId_t(12u));
        RenderGroup* nestedRenderGroup = anotherScene.createRenderGroup();

        EXPECT_NE(StatusOK, renderGroup.addRenderGroup(*nestedRenderGroup));
    }

    TEST_F(ARenderGroup, canCheckContainmentOfMeshes)
    {
        MeshNode* mesh = m_scene.createMeshNode();
        MeshNode* mesh2 = m_scene.createMeshNode();
        MeshNode* mesh3 = m_scene.createMeshNode();

        EXPECT_EQ(StatusOK, renderGroup.addMeshNode(*mesh, 1));
        EXPECT_EQ(StatusOK, renderGroup.addMeshNode(*mesh3, 3));

        expectNumMeshesContained(2u, renderGroup);
        expectMeshContained(*mesh, 1, renderGroup);
        expectMeshNotContained(*mesh2, renderGroup);
        expectMeshContained(*mesh3, 3, renderGroup);
    }

    TEST_F(ARenderGroup, canCheckContainmentOfNestedRenderGroups)
    {
        expectRenderGroupNotContained(renderGroup2, renderGroup);

        EXPECT_EQ(StatusOK, renderGroup.addRenderGroup(renderGroup2, 2));

        expectNumRenderGroupsContained(1u, renderGroup);
        expectRenderGroupContained(renderGroup2, 2, renderGroup);
    }

    TEST_F(ARenderGroup, doesNotDirectlyContainRenderablesOfNestedRenderGroups)
    {
        MeshNode* mesh = m_scene.createMeshNode();

        ASSERT_EQ(StatusOK, renderGroup2.addMeshNode(*mesh, 1));
        ASSERT_EQ(StatusOK, renderGroup.addRenderGroup(renderGroup2));

        expectMeshNotContained(*mesh, renderGroup);
    }

    TEST_F(ARenderGroup, canRemoveMesh)
    {
        MeshNode* mesh = m_scene.createMeshNode();
        ASSERT_EQ(StatusOK, renderGroup.addMeshNode(*mesh, 1));
        EXPECT_EQ(StatusOK, renderGroup.removeMeshNode(*mesh));

        expectNumMeshesContained(0u, renderGroup);
        expectMeshNotContained(*mesh, renderGroup);
    }

    TEST_F(ARenderGroup, canRemoveRenderGroup)
    {
        RenderGroup* nestedRenderGroup = m_scene.createRenderGroup();
        ASSERT_EQ(StatusOK, renderGroup.addRenderGroup(*nestedRenderGroup, 1));
        EXPECT_EQ(StatusOK, renderGroup.removeRenderGroup(*nestedRenderGroup));

        expectNumRenderGroupsContained(0u, renderGroup);
        expectRenderGroupNotContained(*nestedRenderGroup, renderGroup);
    }

    TEST_F(ARenderGroup, removingMeshDoesNotAffectOtherMeshes)
    {
        MeshNode* mesh = m_scene.createMeshNode();
        MeshNode* mesh2 = m_scene.createMeshNode();
        MeshNode* mesh3 = m_scene.createMeshNode();

        EXPECT_EQ(StatusOK, renderGroup.addMeshNode(*mesh, 1));
        EXPECT_EQ(StatusOK, renderGroup.addMeshNode(*mesh2, 2));
        EXPECT_EQ(StatusOK, renderGroup.addMeshNode(*mesh3, 3));

        EXPECT_EQ(StatusOK, renderGroup.removeMeshNode(*mesh2));

        expectNumMeshesContained(2u, renderGroup);
        expectMeshContained(*mesh, 1, renderGroup);
        expectMeshNotContained(*mesh2, renderGroup);
        expectMeshContained(*mesh3, 3, renderGroup);
    }

    TEST_F(ARenderGroup, removingRenderGroupDoesNotAffectOtherRenderGroups)
    {
        RenderGroup* nestedRenderGroup1 = m_scene.createRenderGroup();
        RenderGroup* nestedRenderGroup2 = m_scene.createRenderGroup();
        RenderGroup* nestedRenderGroup3 = m_scene.createRenderGroup();

        ASSERT_EQ(StatusOK, renderGroup.addRenderGroup(*nestedRenderGroup1, 1));
        ASSERT_EQ(StatusOK, renderGroup.addRenderGroup(*nestedRenderGroup2, 2));
        ASSERT_EQ(StatusOK, renderGroup.addRenderGroup(*nestedRenderGroup3, 3));

        ASSERT_EQ(StatusOK, renderGroup.removeRenderGroup(*nestedRenderGroup2));

        expectNumRenderGroupsContained(2u, renderGroup);
        expectRenderGroupContained(*nestedRenderGroup1, 1, renderGroup);
        expectRenderGroupNotContained(*nestedRenderGroup2, renderGroup);
        expectRenderGroupContained(*nestedRenderGroup3, 3, renderGroup);
    }

    TEST_F(ARenderGroup, doesNotContainMeshWhichWasDestroyed)
    {
        MeshNode* mesh = m_scene.createMeshNode();
        ASSERT_EQ(StatusOK, renderGroup.addMeshNode(*mesh));
        ASSERT_EQ(StatusOK, m_scene.destroy(*mesh));

        expectNumMeshesContained(0u, renderGroup);
    }

    TEST_F(ARenderGroup, doesNotContainRenderGroupWhichWasDestroyed)
    {
        RenderGroup* nestedRenderGroup = m_scene.createRenderGroup();
        ASSERT_EQ(StatusOK, renderGroup.addRenderGroup(*nestedRenderGroup));
        ASSERT_EQ(StatusOK, m_scene.destroy(*nestedRenderGroup));

        expectNumRenderGroupsContained(0u, renderGroup);
    }

    TEST_F(ARenderGroup, destroyingMeshDoesNotAffectOtherMeshes)
    {
        MeshNode* mesh = m_scene.createMeshNode();
        MeshNode* mesh2 = m_scene.createMeshNode();
        MeshNode* mesh3 = m_scene.createMeshNode();

        EXPECT_EQ(StatusOK, renderGroup.addMeshNode(*mesh, 1));
        EXPECT_EQ(StatusOK, renderGroup.addMeshNode(*mesh2, 2));
        EXPECT_EQ(StatusOK, renderGroup.addMeshNode(*mesh3, 3));

        EXPECT_EQ(StatusOK, m_scene.destroy(*mesh2));

        expectNumMeshesContained(2u, renderGroup);
        expectMeshContained(*mesh, 1, renderGroup);
        expectMeshContained(*mesh3, 3, renderGroup);
    }

    TEST_F(ARenderGroup, destroyingRenderGroupDoesNotAffectOtherRenderGroups)
    {
        RenderGroup* nestedRenderGroup1 = m_scene.createRenderGroup();
        RenderGroup* nestedRenderGroup2 = m_scene.createRenderGroup();
        RenderGroup* nestedRenderGroup3 = m_scene.createRenderGroup();

        ASSERT_EQ(StatusOK, renderGroup.addRenderGroup(*nestedRenderGroup1, 1));
        ASSERT_EQ(StatusOK, renderGroup.addRenderGroup(*nestedRenderGroup2, 2));
        ASSERT_EQ(StatusOK, renderGroup.addRenderGroup(*nestedRenderGroup3, 3));

        EXPECT_EQ(StatusOK, m_scene.destroy(*nestedRenderGroup2));

        expectNumRenderGroupsContained(2u, renderGroup);
        expectRenderGroupContained(*nestedRenderGroup1, 1, renderGroup);
        expectRenderGroupContained(*nestedRenderGroup3, 3, renderGroup);
    }

    TEST_F(ARenderGroup, canRemoveAllMeshes)
    {
        MeshNode* mesh = m_scene.createMeshNode();
        MeshNode* mesh2 = m_scene.createMeshNode();
        MeshNode* mesh3 = m_scene.createMeshNode();

        EXPECT_EQ(StatusOK, renderGroup.addMeshNode(*mesh, 1));
        EXPECT_EQ(StatusOK, renderGroup.addMeshNode(*mesh2, 2));
        EXPECT_EQ(StatusOK, renderGroup.addMeshNode(*mesh3, 3));

        EXPECT_EQ(StatusOK, renderGroup.removeAllRenderables());

        expectNumMeshesContained(0u, renderGroup);
        expectMeshNotContained(*mesh, renderGroup);
        expectMeshNotContained(*mesh2, renderGroup);
        expectMeshNotContained(*mesh3, renderGroup);
    }

    TEST_F(ARenderGroup, canRemoveAllRenderGroups)
    {
        RenderGroup* nestedRenderGroup1 = m_scene.createRenderGroup();
        RenderGroup* nestedRenderGroup2 = m_scene.createRenderGroup();
        RenderGroup* nestedRenderGroup3 = m_scene.createRenderGroup();

        ASSERT_EQ(StatusOK, renderGroup.addRenderGroup(*nestedRenderGroup1, 1));
        ASSERT_EQ(StatusOK, renderGroup.addRenderGroup(*nestedRenderGroup2, 2));
        ASSERT_EQ(StatusOK, renderGroup.addRenderGroup(*nestedRenderGroup3, 3));

        EXPECT_EQ(StatusOK, renderGroup.removeAllRenderGroups());

        expectNumRenderGroupsContained(0u, renderGroup);
        expectRenderGroupNotContained(*nestedRenderGroup1, renderGroup);
        expectRenderGroupNotContained(*nestedRenderGroup2, renderGroup);
        expectRenderGroupNotContained(*nestedRenderGroup3, renderGroup);
    }

    TEST_F(ARenderGroup, reportsErrorWhenTryingToRemoveMeshWhichWasNotAdded)
    {
        MeshNode* mesh = m_scene.createMeshNode();
        status_t status = renderGroup.removeMeshNode(*mesh);

        EXPECT_NE(StatusOK, status);
    }

    TEST_F(ARenderGroup, reportsErrorWhenTryingToRemoveRenderGroupWhichWasNotAdded)
    {
        RenderGroup* nestedRenderGroup = m_scene.createRenderGroup();
        EXPECT_NE(StatusOK, renderGroup.removeRenderGroup(*nestedRenderGroup));
    }

    TEST_F(ARenderGroup, canChangeMeshOrderByReaddingIt)
    {
        MeshNode* mesh = m_scene.createMeshNode();

        EXPECT_EQ(StatusOK, renderGroup.addMeshNode(*mesh, 1));
        EXPECT_EQ(StatusOK, renderGroup.addMeshNode(*mesh, 999));

        expectMeshContained(*mesh, 999, renderGroup);
    }

    TEST_F(ARenderGroup, canChangeRenderGroupOrderByReaddingIt)
    {
        RenderGroup* nestedRenderGroup = m_scene.createRenderGroup();

        ASSERT_EQ(StatusOK, renderGroup.addRenderGroup(*nestedRenderGroup, 1));
        ASSERT_EQ(StatusOK, renderGroup.addRenderGroup(*nestedRenderGroup, 999));

        expectRenderGroupContained(*nestedRenderGroup, 999, renderGroup);
    }

    TEST_F(ARenderGroup, canChangeMeshOrderByRemovingItAndAddingAgain)
    {
        MeshNode* mesh = m_scene.createMeshNode();

        EXPECT_EQ(StatusOK, renderGroup.addMeshNode(*mesh, 1));
        EXPECT_EQ(StatusOK, renderGroup.removeMeshNode(*mesh));
        EXPECT_EQ(StatusOK, renderGroup.addMeshNode(*mesh, 999));

        expectMeshContained(*mesh, 999, renderGroup);
    }

    TEST_F(ARenderGroup, canChangeRenderGroupOrderByRemovingItAndAddingAgain)
    {
        RenderGroup* nestedRenderGroup = m_scene.createRenderGroup();

        ASSERT_EQ(StatusOK, renderGroup.addRenderGroup(*nestedRenderGroup, 1));
        ASSERT_EQ(StatusOK, renderGroup.removeRenderGroup(*nestedRenderGroup));
        ASSERT_EQ(StatusOK, renderGroup.addRenderGroup(*nestedRenderGroup, 999));

        expectRenderGroupContained(*nestedRenderGroup, 999, renderGroup);
    }

    TEST_F(ARenderGroup, meshCanBeAddedToTwoGroupsWithDifferentOrder)
    {
        MeshNode* mesh = m_scene.createMeshNode();

        EXPECT_EQ(StatusOK, renderGroup.addMeshNode(*mesh, 3));
        EXPECT_EQ(StatusOK, renderGroup2.addMeshNode(*mesh, 999));

        expectNumMeshesContained(1u, renderGroup);
        expectNumMeshesContained(1u, renderGroup2);
        expectMeshContained(*mesh, 3, renderGroup);
        expectMeshContained(*mesh, 999, renderGroup2);
    }

    TEST_F(ARenderGroup, renderGroupCanBeAddedToTwoGroupsWithDifferentOrder)
    {
        RenderGroup* nestedRenderGroup = m_scene.createRenderGroup();

        EXPECT_EQ(StatusOK, renderGroup.addRenderGroup(*nestedRenderGroup, 3));
        EXPECT_EQ(StatusOK, renderGroup2.addRenderGroup(*nestedRenderGroup, 999));

        expectNumRenderGroupsContained(1u, renderGroup);
        expectNumRenderGroupsContained(1u, renderGroup2);
        expectRenderGroupContained(*nestedRenderGroup, 3, renderGroup);
        expectRenderGroupContained(*nestedRenderGroup, 999, renderGroup2);
    }

    TEST_F(ARenderGroup, removingMeshFromGroupDoesNotAffectItsPlaceInAnotherGroup)
    {
        MeshNode* mesh = m_scene.createMeshNode();

        EXPECT_EQ(StatusOK, renderGroup.addMeshNode(*mesh, 3));
        EXPECT_EQ(StatusOK, renderGroup2.addMeshNode(*mesh, 999));

        EXPECT_EQ(StatusOK, renderGroup.removeMeshNode(*mesh));

        expectNumMeshesContained(0u, renderGroup);
        expectNumMeshesContained(1u, renderGroup2);
        expectMeshNotContained(*mesh, renderGroup);
        expectMeshContained(*mesh, 999, renderGroup2);
    }

    TEST_F(ARenderGroup, removingRenderGroupFromGroupDoesNotAffectItsPlaceInAnotherGroup)
    {
        RenderGroup* nestedRenderGroup = m_scene.createRenderGroup();

        ASSERT_EQ(StatusOK, renderGroup.addRenderGroup(*nestedRenderGroup, 3));
        ASSERT_EQ(StatusOK, renderGroup2.addRenderGroup(*nestedRenderGroup, 999));

        EXPECT_EQ(StatusOK, renderGroup.removeRenderGroup(*nestedRenderGroup));

        expectNumRenderGroupsContained(0u, renderGroup);
        expectNumRenderGroupsContained(1u, renderGroup2);
        expectRenderGroupNotContained(*nestedRenderGroup, renderGroup);
        expectRenderGroupContained(*nestedRenderGroup, 999, renderGroup2);
    }

    TEST_F(ARenderGroup, destroyedMeshIsRemovedFromAllItsGroups)
    {
        MeshNode* mesh = m_scene.createMeshNode();

        EXPECT_EQ(StatusOK, renderGroup.addMeshNode(*mesh, 3));
        EXPECT_EQ(StatusOK, renderGroup2.addMeshNode(*mesh, 999));

        EXPECT_EQ(StatusOK, m_scene.destroy(*mesh));

        expectNumMeshesContained(0u, renderGroup);
        expectNumMeshesContained(0u, renderGroup2);
    }

    TEST_F(ARenderGroup, destroyedNestedRenderGroupIsRemovedFromAllItsGroups)
    {
        RenderGroup* nestedRenderGroup = m_scene.createRenderGroup();

        ASSERT_EQ(StatusOK, renderGroup.addRenderGroup(*nestedRenderGroup, 3));
        ASSERT_EQ(StatusOK, renderGroup2.addRenderGroup(*nestedRenderGroup, 999));

        ASSERT_EQ(StatusOK, m_scene.destroy(*nestedRenderGroup));

        expectNumRenderGroupsContained(0u, renderGroup);
        expectNumRenderGroupsContained(0u, renderGroup2);
    }
}
