//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/HierarchicalRedTrianglesScene.h"
#include "TestScenes/Triangle.h"
#include "ramses/client/ramses-utils.h"
#include "ramses/client/Scene.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Effect.h"

namespace ramses::internal
{
    HierarchicalRedTrianglesScene::HierarchicalRedTrianglesScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition, uint32_t vpWidth, uint32_t vpHeight)
        : IntegrationScene(scene, cameraPosition, vpWidth, vpHeight)
        , m_groupNode(nullptr)
        , m_subGroup1Node(nullptr)
        , m_subGroup2Node(nullptr)
        , m_subGroup3Node(nullptr)
        , m_rotateNode1(*m_scene.createNode())
        , m_rotateNode2(*m_scene.createNode())
        , m_scaleNode1(*m_scene.createNode())
        , m_scaleNode2(*m_scene.createNode())
    {
        ramses::Effect* effect = createTestEffect(state);
        Triangle redTriangle(m_scene, *effect, TriangleAppearance::EColor::Red);

        m_groupNode = m_scene.createNode();
        std::array<ramses::Node*, 3> subGroups{};
        for (int row = 0; row < 3; ++row)
        {
            ramses::Node* subGroupNode = m_scene.createNode();
            for (int column = 0; column < 3; ++column)
            {
                // create a mesh node to define the triangle with chosen appearance
                ramses::MeshNode* meshNode = m_scene.createMeshNode("red triangle mesh node");
                addMeshNodeToDefaultRenderGroup(*meshNode);
                meshNode->setAppearance(redTriangle.GetAppearance());
                meshNode->setGeometry(redTriangle.GetGeometry());

                ramses::Node* transNode = m_scene.createNode();
                transNode->setTranslation({-1.f + column * 2, -3.f + row * 2, -30.f});
                transNode->setParent(*subGroupNode);

                if (row == 0 || row == 1)
                {
                    meshNode->setParent(*transNode);
                }
                else if (column == 0)
                {
                    m_scaleNode1.setScaling({1.f, 1.f, 1.f});
                    m_scaleNode1.setParent(*transNode);
                    meshNode->setParent(m_scaleNode1);
                }
                else if (column == 1)
                {
                    m_rotateNode1.setRotation({0.f, 0.f, 0.f}, ramses::ERotationType::Euler_XYZ);
                    m_rotateNode1.setParent(*transNode);
                    meshNode->setParent(m_rotateNode1);
                }
                else if (column == 2)
                {
                    m_scaleNode2.setScaling({1.f, 1.f, 1.f});
                    m_rotateNode2.setRotation({0.f, 0.f, 0.f}, ramses::ERotationType::Euler_XYZ);
                    m_rotateNode2.setParent(*transNode);
                    m_scaleNode2.setParent(m_rotateNode2);
                    meshNode->setParent(m_scaleNode2);
                }
            }
            subGroupNode->setParent(*m_groupNode);
            subGroups[row] = subGroupNode;
        }
        m_subGroup1Node = subGroups[0];
        m_subGroup2Node = subGroups[1];
        m_subGroup3Node = subGroups[2];

        switch (state)
        {
        case THREE_ROWS_TRIANGLES:
        case REENABLED_FULL_VISIBILITY:
            m_groupNode->setVisibility(ramses::EVisibilityMode::Visible);
            break;
        case PARTIAL_VISIBILITY:
            m_groupNode->setVisibility(ramses::EVisibilityMode::Visible);
            m_subGroup1Node->setVisibility(ramses::EVisibilityMode::Invisible);
            m_subGroup2Node->setVisibility(ramses::EVisibilityMode::Invisible);
            break;
        case INVISIBLE:
            m_groupNode->setVisibility(ramses::EVisibilityMode::Invisible);
            break;
        case VISIBILITY_OFF:
            m_groupNode->setVisibility(ramses::EVisibilityMode::Off);
            break;
        case ROTATE_AND_SCALE:
        case ROTATE_AND_SCALE_UBO1:
        case ROTATE_AND_SCALE_UBO2:
        case ROTATE_AND_SCALE_UBO3:
            m_scaleNode1.setScaling({ 0.3f, 1.f, 1.f });
            m_rotateNode1.setRotation({ 0.f, 0.f, -90.f }, ramses::ERotationType::Euler_XYZ);
            m_scaleNode2.setScaling({ 0.3f, 1.f, 1.f });
            m_rotateNode2.setRotation({ 0.f, 0.f, -90.f }, ramses::ERotationType::Euler_XYZ);
            if (state != ROTATE_AND_SCALE)
            {
                const auto uniform = redTriangle.GetAppearance().getEffect().findUniformInput("generalUbo.variant");
                assert(uniform);
                redTriangle.GetAppearance().setInputValue(*uniform, static_cast<int32_t>(state - ROTATE_AND_SCALE_UBO1 + 1));
            }
            break;
        case DELETE_MESHNODE:
            destroySubTree(m_subGroup2Node);
            break;
        default:
            break;
        }
    }

    Effect* HierarchicalRedTrianglesScene::createTestEffect(uint32_t state)
    {
        switch (state)
        {
        case ROTATE_AND_SCALE_UBO1:
        case ROTATE_AND_SCALE_UBO2:
        case ROTATE_AND_SCALE_UBO3:
            return getTestEffect("ramses-test-client-basic-ubo");
        default:
            return getTestEffect("ramses-test-client-basic");
        }
    }

    void HierarchicalRedTrianglesScene::destroySubTree(ramses::Node* rootNode)
    {
        if (rootNode != nullptr)
        {
            while (rootNode && rootNode->getChildCount() > 0u)
            {
                destroySubTree(rootNode->getChild(0u));
            }

            m_scene.destroy(*rootNode);
        }
    }
}
