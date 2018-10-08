//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/MultiTransformationLinkScene.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/GroupNode.h"

namespace ramses_internal
{
    MultiTransformationLinkScene::MultiTransformationLinkScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
        , m_scaleFactor(10.f / NumRows)
        , m_dummyEffect(*getTestEffect("ramses-test-client-basic"))
        , m_redTriangle(m_client, m_scene, m_dummyEffect, ramses::TriangleAppearance::EColor_Red)
        , m_greenTriangle(m_client, m_scene, m_dummyEffect, ramses::TriangleAppearance::EColor_Green)
        , m_blueTriangle(m_client, m_scene, m_dummyEffect, ramses::TriangleAppearance::EColor_Blue)
    {
        switch (state)
        {
        case PROVIDER_SCENE:
            createSceneProvider();
            break;
        case PROVIDER_CONSUMER_SCENE:
            createSceneProviderConsumer();
            break;
        case CONSUMER_SCENE:
            createSceneConsumer();
            break;
        default:
            assert(false);
        }
    }

    void MultiTransformationLinkScene::createSceneProvider()
    {
        ramses::GroupNode* rootNode = m_scene.createGroupNode();
        rootNode->setTranslation(-0.5f * m_scaleFactor * NumRows, -5.f, -32.f);

        UInt32 meshIndex = 0;

        ramses::Node* lastRowNode = NULL;
        for (UInt32 row = 0; row < NumRows; ++row)
        {
            ramses::GroupNode* rowNode = m_scene.createGroupNode();
            if (lastRowNode)
            {
                rowNode->setTranslation(0.f, m_scaleFactor, 0.f);
                rowNode->setParent(*lastRowNode);
            }
            else
            {
                rowNode->setParent(*rootNode);
            }
            lastRowNode = rowNode;
            m_scene.createTransformationDataProvider(*rowNode, DataIdRowStart + row);

            ramses::Node* lastTransNode = NULL;
            for (UInt32 column = 0; column < NumRows && meshIndex < NumMeshes; ++column, ++meshIndex)
            {
                // create a mesh node to define the triangle with chosen appearance
                ramses::MeshNode* meshNode = m_scene.createMeshNode("red triangle mesh node");
                addMeshNodeToDefaultRenderGroup(*meshNode);
                meshNode->setAppearance(m_redTriangle.GetAppearance());
                meshNode->setGeometryBinding(m_redTriangle.GetGeometry());

                // scale and translate each triangle to fit in the square
                ramses::GroupNode* transNode = m_scene.createGroupNode();
                if (lastTransNode)
                {
                    transNode->setTranslation(m_scaleFactor, 0.f, 0.f);
                    transNode->setParent(*lastTransNode);
                }
                else
                {
                    transNode->setParent(*rowNode);
                }
                lastTransNode = transNode;

                ramses::GroupNode* scaleNode = m_scene.createGroupNode();
                scaleNode->setScaling(m_scaleFactor * 0.3f, m_scaleFactor * 0.3f, m_scaleFactor);

                meshNode->setParent(*scaleNode);
                scaleNode->setParent(*transNode);
            }
        }
    }

    void MultiTransformationLinkScene::createSceneProviderConsumer()
    {
        UInt32 meshIndex = 0;
        for (UInt32 row = 0; row < NumRows; ++row)
        {
            ramses::GroupNode* rowNode = m_scene.createGroupNode();
            ramses::GroupNode* rowGroupNode = m_scene.createGroupNode();
            rowNode->setParent(*rowGroupNode);

            m_scene.createTransformationDataConsumer(*rowGroupNode, DataIdRowStart + row);
            rowNode->setTranslation(m_scaleFactor * 0.2f, m_scaleFactor * 0.2f, m_scaleFactor * 0.2f);

            ramses::Node* lastTransNode = NULL;
            for (UInt32 column = 0; column < NumRows && meshIndex < NumMeshes; ++column, ++meshIndex)
            {
                // create a mesh node to define the triangle with chosen appearance
                ramses::MeshNode* meshNode = m_scene.createMeshNode("red triangle mesh node");
                addMeshNodeToDefaultRenderGroup(*meshNode);
                meshNode->setAppearance(m_greenTriangle.GetAppearance());
                meshNode->setGeometryBinding(m_greenTriangle.GetGeometry());

                // scale and translate each triangle to fit in the square
                ramses::GroupNode* transNode = m_scene.createGroupNode();
                if (lastTransNode)
                {
                    transNode->setTranslation(m_scaleFactor, 0.f, 0.f);
                    transNode->setParent(*lastTransNode);
                }
                else
                {
                    transNode->setParent(*rowNode);
                }
                lastTransNode = transNode;

                ramses::GroupNode* scaleNode = m_scene.createGroupNode();
                scaleNode->setScaling(m_scaleFactor * 0.3f, m_scaleFactor * 0.3f, m_scaleFactor);

                meshNode->setParent(*scaleNode);
                scaleNode->setParent(*transNode);

                m_scene.createTransformationDataProvider(*meshNode, DataIdMeshStart + meshIndex);
            }
        }
    }

    void MultiTransformationLinkScene::createSceneConsumer()
    {
        for (UInt32 meshIndex = 0; meshIndex < NumMeshes; ++meshIndex)
        {
            ramses::GroupNode* meshGroupNode = m_scene.createGroupNode();
            m_scene.createTransformationDataConsumer(*meshGroupNode, DataIdMeshStart + meshIndex);

            ramses::GroupNode* transNode = m_scene.createGroupNode();
            transNode->setTranslation(m_scaleFactor * 0.3f, m_scaleFactor * 0.3f, m_scaleFactor * 0.3f);

            ramses::MeshNode* meshNode = m_scene.createMeshNode();
            addMeshNodeToDefaultRenderGroup(*meshNode);
            meshNode->setAppearance(m_blueTriangle.GetAppearance());
            meshNode->setGeometryBinding(m_blueTriangle.GetGeometry());

            meshNode->setParent(*transNode);
            transNode->setParent(*meshGroupNode);
        }
    }
}
