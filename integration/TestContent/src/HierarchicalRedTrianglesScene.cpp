//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/HierarchicalRedTrianglesScene.h"
#include "TestScenes/Triangle.h"
#include "ramses-utils.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"

namespace ramses_internal
{
    HierarchicalRedTrianglesScene::HierarchicalRedTrianglesScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
        , m_groupNode(0)
        , m_subGroup1Node(0)
        , m_subGroup2Node(0)
        , m_subGroup3Node(0)
        , m_rotateNode1(*m_scene.createNode())
        , m_rotateNode2(*m_scene.createNode())
        , m_scaleNode1(*m_scene.createNode())
        , m_scaleNode2(*m_scene.createNode())
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-basic");
        ramses::Triangle redTriangle(m_client, m_scene, *effect, ramses::TriangleAppearance::EColor_Red);

        m_groupNode = m_scene.createNode();
        ramses::Node* subGroups[3];
        for (int row = 0; row < 3; ++row)
        {
            ramses::Node* subGroupNode = m_scene.createNode();
            for (int column = 0; column < 3; ++column)
            {
                // create a mesh node to define the triangle with chosen appearance
                ramses::MeshNode* meshNode = m_scene.createMeshNode("red triangle mesh node");
                addMeshNodeToDefaultRenderGroup(*meshNode);
                meshNode->setAppearance(redTriangle.GetAppearance());
                meshNode->setGeometryBinding(redTriangle.GetGeometry());

                ramses::Node* transNode = m_scene.createNode();
                transNode->setTranslation(-1.f + column * 2, -3.f + row * 2, -30.f);
                transNode->setParent(*subGroupNode);

                if (row == 0 || row == 1)
                {
                    meshNode->setParent(*transNode);
                }
                else if (column == 0)
                {
                    m_scaleNode1.setScaling(1.f, 1.f, 1.f);
                    m_scaleNode1.setParent(*transNode);
                    meshNode->setParent(m_scaleNode1);
                }
                else if (column == 1)
                {
                    m_rotateNode1.setRotation(0.f, 0.f, 0.f);
                    m_rotateNode1.setParent(*transNode);
                    meshNode->setParent(m_rotateNode1);
                }
                else if (column == 2)
                {
                    m_scaleNode2.setScaling(1.f, 1.f, 1.f);
                    m_rotateNode2.setRotation(0.f, 0.f, 0.f);
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
            m_groupNode->setVisibility(true);
            break;
        case PARTIAL_VISIBILITY:
            m_groupNode->setVisibility(true);
            m_subGroup1Node->setVisibility(false);
            m_subGroup2Node->setVisibility(false);
            break;
        case NO_VISIBILITY:
            m_groupNode->setVisibility(false);
            break;
        case ROTATE_AND_SCALE:
            m_scaleNode1.setScaling(0.3f, 1.f, 1.f);
            m_rotateNode1.setRotation(0.f, 0.f, 90.f);
            m_scaleNode2.setScaling(0.3f, 1.f, 1.f);
            m_rotateNode2.setRotation(0.f, 0.f, 90.f);
            break;
        case DELETE_MESHNODE:
            destroySubTree(m_subGroup2Node);
            break;
        }
    }

    void HierarchicalRedTrianglesScene::destroySubTree(ramses::Node* rootNode)
    {
        if (rootNode != NULL)
        {
            while (rootNode && rootNode->getChildCount() > 0u)
            {
                destroySubTree(rootNode->getChild(0u));
            }

            m_scene.destroy(*rootNode);
        }
    }
}
