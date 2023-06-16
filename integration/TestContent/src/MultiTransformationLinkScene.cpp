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
#include <cassert>

namespace ramses_internal
{
    MultiTransformationLinkScene::MultiTransformationLinkScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
        , m_scaleFactor(10.f / NumRows)
        , m_dummyEffect(*getTestEffect("ramses-test-client-basic"))
        , m_redTriangle(m_scene, m_dummyEffect, ramses::TriangleAppearance::EColor_Red)
        , m_greenTriangle(m_scene, m_dummyEffect, ramses::TriangleAppearance::EColor_Green)
        , m_blueTriangle(m_scene, m_dummyEffect, ramses::TriangleAppearance::EColor_Blue)
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
        ramses::Node* rootNode = m_scene.createNode();
        rootNode->setTranslation({-0.5f * m_scaleFactor * NumRows, -5.f, -32.f});

        uint32_t meshIndex = 0;

        ramses::Node* lastRowNode = nullptr;
        for (uint32_t row = 0; row < NumRows; ++row)
        {
            ramses::Node* rowNode = m_scene.createNode();
            if (lastRowNode)
            {
                rowNode->setTranslation({0.f, m_scaleFactor, 0.f});
                rowNode->setParent(*lastRowNode);
            }
            else
            {
                rowNode->setParent(*rootNode);
            }
            lastRowNode = rowNode;
            m_scene.createTransformationDataProvider(*rowNode, ramses::dataProviderId_t{DataIdRowStart + row});

            ramses::Node* lastTransNode = nullptr;
            for (uint32_t column = 0; column < NumRows && meshIndex < NumMeshes; ++column, ++meshIndex)
            {
                // create a mesh node to define the triangle with chosen appearance
                ramses::MeshNode* meshNode = m_scene.createMeshNode("red triangle mesh node");
                addMeshNodeToDefaultRenderGroup(*meshNode);
                meshNode->setAppearance(m_redTriangle.GetAppearance());
                meshNode->setGeometryBinding(m_redTriangle.GetGeometry());

                // scale and translate each triangle to fit in the square
                ramses::Node* transNode = m_scene.createNode();
                if (lastTransNode)
                {
                    transNode->setTranslation({m_scaleFactor, 0.f, 0.f});
                    transNode->setParent(*lastTransNode);
                }
                else
                {
                    transNode->setParent(*rowNode);
                }
                lastTransNode = transNode;

                ramses::Node* scaleNode = m_scene.createNode();
                scaleNode->setScaling({m_scaleFactor * 0.3f, m_scaleFactor * 0.3f, m_scaleFactor});

                meshNode->setParent(*scaleNode);
                scaleNode->setParent(*transNode);
            }
        }
    }

    void MultiTransformationLinkScene::createSceneProviderConsumer()
    {
        uint32_t meshIndex = 0;
        for (uint32_t row = 0; row < NumRows; ++row)
        {
            ramses::Node* rowNode = m_scene.createNode();
            ramses::Node* rowGroupNode = m_scene.createNode();
            rowNode->setParent(*rowGroupNode);

            m_scene.createTransformationDataConsumer(*rowGroupNode, ramses::dataConsumerId_t{DataIdRowStart + row});
            rowNode->setTranslation({m_scaleFactor * 0.2f, m_scaleFactor * 0.2f, m_scaleFactor * 0.2f});

            ramses::Node* lastTransNode = nullptr;
            for (uint32_t column = 0; column < NumRows && meshIndex < NumMeshes; ++column, ++meshIndex)
            {
                // create a mesh node to define the triangle with chosen appearance
                ramses::MeshNode* meshNode = m_scene.createMeshNode("red triangle mesh node");
                addMeshNodeToDefaultRenderGroup(*meshNode);
                meshNode->setAppearance(m_greenTriangle.GetAppearance());
                meshNode->setGeometryBinding(m_greenTriangle.GetGeometry());

                // scale and translate each triangle to fit in the square
                ramses::Node* transNode = m_scene.createNode();
                if (lastTransNode)
                {
                    transNode->setTranslation({m_scaleFactor, 0.f, 0.f});
                    transNode->setParent(*lastTransNode);
                }
                else
                {
                    transNode->setParent(*rowNode);
                }
                lastTransNode = transNode;

                ramses::Node* scaleNode = m_scene.createNode();
                scaleNode->setScaling({m_scaleFactor * 0.3f, m_scaleFactor * 0.3f, m_scaleFactor});

                meshNode->setParent(*scaleNode);
                scaleNode->setParent(*transNode);

                m_scene.createTransformationDataProvider(*meshNode, ramses::dataProviderId_t{DataIdMeshStart + meshIndex});
            }
        }
    }

    void MultiTransformationLinkScene::createSceneConsumer()
    {
        for (uint32_t meshIndex = 0; meshIndex < NumMeshes; ++meshIndex)
        {
            ramses::Node* meshGroupNode = m_scene.createNode();
            m_scene.createTransformationDataConsumer(*meshGroupNode, ramses::dataConsumerId_t{DataIdMeshStart + meshIndex});

            ramses::Node* transNode = m_scene.createNode();
            transNode->setTranslation({m_scaleFactor * 0.3f, m_scaleFactor * 0.3f, m_scaleFactor * 0.3f});

            ramses::MeshNode* meshNode = m_scene.createMeshNode();
            addMeshNodeToDefaultRenderGroup(*meshNode);
            meshNode->setAppearance(m_blueTriangle.GetAppearance());
            meshNode->setGeometryBinding(m_blueTriangle.GetGeometry());

            meshNode->setParent(*transNode);
            transNode->setParent(*meshGroupNode);
        }
    }
}
