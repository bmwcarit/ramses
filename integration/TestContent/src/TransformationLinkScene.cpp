//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/TransformationLinkScene.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"

#include "SceneImpl.h"
#include "Scene/ClientScene.h"

namespace ramses_internal
{
    TransformationLinkScene::TransformationLinkScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition, uint32_t vpWidth, uint32_t vpHeight)
        : IntegrationScene(scene, cameraPosition, vpWidth, vpHeight)
    {

        ramses::Node* centerProviderNode = m_scene.createNode("transform provider");
        scene.createTransformationDataProvider(*centerProviderNode, transformProviderDataId);

        ramses::Node* leftProviderNode = m_scene.createNode();
        leftProviderNode->setTranslation(-2.0f, 0.0f, 0.0f);
        scene.createTransformationDataProvider(*leftProviderNode, transformProviderDataId_Left);

        ramses::Node* rightProviderNode = m_scene.createNode();
        rightProviderNode->setTranslation(2.0f, 0.0f, 0.0f);
        scene.createTransformationDataProvider(*rightProviderNode, transformProviderDataId_Right);

        ramses::TriangleAppearance::EColor color = ramses::TriangleAppearance::EColor_Red;
        ramses::MeshNode* mesh = nullptr;
        switch (state)
        {
        case TRANSFORMATION_CONSUMER:
        case TRANSFORMATION_CONSUMER_OVERRIDEN:
        {
            ramses::Node* rotateNode = m_scene.createNode();
            rotateNode->setRotation(0.0f, 0.0f, 30.0f);
            ramses::Node* consumerGroupNode = m_scene.createNode("transform consumer");
            rotateNode->setParent(*consumerGroupNode);
            scene.createTransformationDataConsumer(*consumerGroupNode, transformConsumerDataId);

            if (TRANSFORMATION_CONSUMER_OVERRIDEN == state)
            {
                color = ramses::TriangleAppearance::EColor_Blue;
                ramses::Node* parentTransformWhichWillBeOverridden = m_scene.createNode();
                parentTransformWhichWillBeOverridden->setTranslation(1.2f, 1.2f, 0.0f);
                consumerGroupNode->setParent(*parentTransformWhichWillBeOverridden);
            }
            mesh = createTriangleMesh(color);
            mesh->setParent(*rotateNode);
            mesh->addChild(*centerProviderNode);

            break;
        }
        case TRANSFORMATION_CONSUMER_AND_PROVIDER:
        {
            color = ramses::TriangleAppearance::EColor_White;

            ramses::Node* consumerGroupNode = m_scene.createNode("transform consumer");
            scene.createTransformationDataConsumer(*consumerGroupNode, transformConsumerDataId);

            ramses::Node* translate = m_scene.createNode();
            translate->setTranslation(1.0f, 0.0f, 0.0f);
            translate->setParent(*consumerGroupNode);

            ramses::Node* rotateNode = m_scene.createNode();
            rotateNode->setRotation(0.0f, 0.0f, 15.0f);
            rotateNode->setParent(*translate);

            mesh = createTriangleMesh(color);
            mesh->setParent(*rotateNode);
            mesh->addChild(*centerProviderNode);

            break;
        }
        case TRANSFORMATION_PROVIDER:
        {
            color = ramses::TriangleAppearance::EColor_Green;
            ramses::Node* rotateNode = m_scene.createNode();
            rotateNode->setRotation(0.0f, 0.0f, 60.0f);

            mesh = createTriangleMesh(color);
            mesh->setParent(*rotateNode);
            mesh->addChild(*centerProviderNode);

            break;
        }
        case TRANSFORMATION_PROVIDER_WITHOUT_CONTENT:
        {
            //nothing to do
            break;
        }
        default:
            assert(false && "invalid scene state");
        }
    }

    ramses::MeshNode* TransformationLinkScene::createTriangleMesh(ramses::TriangleAppearance::EColor color)
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-basic");
        ramses::MeshNode* mesh = m_scene.createMeshNode();
        addMeshNodeToDefaultRenderGroup(*mesh);

        ramses::Triangle triangle(m_scene, *effect, color);
        mesh->setGeometryBinding(triangle.GetGeometry());
        mesh->setAppearance(triangle.GetAppearance());
        return mesh;
    }
}
