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
#include "ramses-client-api/RotateNode.h"
#include "ramses-client-api/GroupNode.h"
#include "ramses-client-api/TranslateNode.h"

#include "SceneImpl.h"
#include "Scene/ClientScene.h"

namespace ramses_internal
{
    TransformationLinkScene::TransformationLinkScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
    {

        ramses::GroupNode* centerProviderNode = m_scene.createGroupNode("transform provider");
        scene.createTransformationDataProvider(*centerProviderNode, transformProviderDataId);

        ramses::TranslateNode* leftProviderNode = m_scene.createTranslateNode();
        leftProviderNode->setTranslation(-2.0f, 0.0f, 0.0f);
        scene.createTransformationDataProvider(*leftProviderNode, transformProviderDataId_Left);

        ramses::TranslateNode* rightProviderNode = m_scene.createTranslateNode();
        rightProviderNode->setTranslation(2.0f, 0.0f, 0.0f);
        scene.createTransformationDataProvider(*rightProviderNode, transformProviderDataId_Right);

        ramses::TriangleAppearance::EColor color = ramses::TriangleAppearance::EColor_Red;
        ramses::MeshNode* mesh = NULL;
        switch (state)
        {
        case TRANSFORMATION_CONSUMER:
        case TRANSFORMATION_CONSUMER_OVERRIDEN:
        {
            ramses::RotateNode* rotateNode = m_scene.createRotateNode();
            rotateNode->setRotation(0.0f, 0.0f, 30.0f);
            ramses::GroupNode* consumerGroupNode = m_scene.createGroupNode("transform consumer");
            rotateNode->setParent(*consumerGroupNode);
            scene.createTransformationDataConsumer(*consumerGroupNode, transformConsumerDataId);

            if (TRANSFORMATION_CONSUMER_OVERRIDEN == state)
            {
                color = ramses::TriangleAppearance::EColor_Blue;
                ramses::TranslateNode* parentTransformWhichWillBeOverridden = m_scene.createTranslateNode();
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

            ramses::GroupNode* consumerGroupNode = m_scene.createGroupNode("transform consumer");
            scene.createTransformationDataConsumer(*consumerGroupNode, transformConsumerDataId);

            ramses::TranslateNode* translate = m_scene.createTranslateNode();
            translate->setTranslation(1.0f, 0.0f, 0.0f);
            translate->setParent(*consumerGroupNode);

            ramses::RotateNode* rotateNode = m_scene.createRotateNode();
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
            ramses::RotateNode* rotateNode = m_scene.createRotateNode();
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

        ramses::Triangle triangle(m_client, m_scene, *effect, color);
        mesh->setGeometryBinding(triangle.GetGeometry());
        mesh->setAppearance(triangle.GetAppearance());
        return mesh;
    }
}
