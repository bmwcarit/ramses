//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/RenderPassScene.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/PerspectiveCamera.h"

namespace ramses_internal
{
    RenderPassScene::RenderPassScene(ramses::Scene& scene, UInt32 state, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
        , m_effect(*getTestEffect("ramses-test-client-basic"))
        , m_blueTriangle(scene, m_effect, ramses::TriangleAppearance::EColor_Blue)
        , m_whiteTriangle(scene, m_effect, ramses::TriangleAppearance::EColor_White)
    {
        ramses::MeshNode* meshNode1 = m_scene.createMeshNode();
        ramses::MeshNode* meshNode2 = m_scene.createMeshNode();
        meshNode1->setAppearance(m_blueTriangle.GetAppearance());
        meshNode2->setAppearance(m_whiteTriangle.GetAppearance());
        meshNode1->setGeometryBinding(m_blueTriangle.GetGeometry());
        meshNode2->setGeometryBinding(m_whiteTriangle.GetGeometry());

        ramses::Node* translateNode = m_scene.createNode();
        translateNode->addChild(*meshNode1);
        translateNode->addChild(*meshNode2);
        translateNode->translate({0.0f, 0.0f, -12.0f});

        ramses::Camera& camera1 = createCameraWithDefaultParameters();
        ramses::Camera& camera2 = createCameraWithDefaultParameters();

        ramses::Node* cameraTranslateNode = m_scene.createNode();
        camera2.setParent(*cameraTranslateNode);
        cameraTranslateNode->translate({-1.5f, -1.5f, 5.0f});

        ramses::RenderPass* renderPass1 = m_scene.createRenderPass();
        ramses::RenderPass* renderPass2 = m_scene.createRenderPass();
        renderPass1->setClearFlags(ramses::EClearFlags_None);
        renderPass2->setClearFlags(ramses::EClearFlags_None);
        ramses::RenderGroup* renderGroup1 = m_scene.createRenderGroup();
        ramses::RenderGroup* renderGroup2 = m_scene.createRenderGroup();

        switch (state)
        {
        case GROUPS_WITH_DIFFERENT_RENDER_ORDER:
        case NESTED_GROUPS:
            break;
        default:
            renderPass1->addRenderGroup(*renderGroup1);
            renderPass2->addRenderGroup(*renderGroup2);
            break;
        }

        switch (state)
        {
        case MESHES_NOT_IN_PASS:
            break;

        case ONE_MESH_PER_PASS:
            renderPass1->setCamera(camera1);
            renderPass2->setCamera(camera2);
            renderGroup1->addMeshNode(*meshNode1);
            renderGroup2->addMeshNode(*meshNode2);
            break;

        case MESH_IN_MULTIPLE_PASSES:
            renderPass1->setCamera(camera1);
            renderPass2->setCamera(camera2);
            renderGroup1->addMeshNode(*meshNode1);
            renderGroup2->addMeshNode(*meshNode1);
            break;

        case GROUPS_WITH_DIFFERENT_RENDER_ORDER:
        {
            renderGroup1->addMeshNode(*meshNode1);
            renderGroup2->addMeshNode(*meshNode2);

            renderPass1->setCamera(camera1);
            renderPass1->addRenderGroup(*renderGroup1, 2);
            renderPass1->addRenderGroup(*renderGroup2, 1);
        }
            break;

        case NESTED_GROUPS:
        {
            renderGroup1->addMeshNode(*meshNode1);

            renderGroup2->addRenderGroup(*renderGroup1, 2);
            renderGroup2->addMeshNode(*meshNode2, 1);

            renderPass1->setCamera(camera1);
            renderPass1->addRenderGroup(*renderGroup2);
        }
        break;

        case PASSES_WITH_DIFFERENT_RENDER_ORDER:
            renderPass1->setCamera(camera1);
            renderPass2->setCamera(camera1); // So both render passes render with same camera - to same screen location

            renderGroup1->addMeshNode(*meshNode1);
            renderGroup2->addMeshNode(*meshNode2);
            renderPass1->setRenderOrder(2);
            renderPass2->setRenderOrder(1);
            break;

        case PASSES_WITH_LEFT_AND_RIGHT_VIEWPORT:
            ramses::PerspectiveCamera* cameraLeft = m_scene.createPerspectiveCamera();
            cameraLeft->setFrustum(20.f, IntegrationScene::DefaultViewportWidth * 0.5f / IntegrationScene::DefaultViewportHeight, 0.1f, 100.f );
            cameraLeft->setViewport(0u, 0u, IntegrationScene::DefaultViewportWidth / 2u, IntegrationScene::DefaultViewportHeight);

            ramses::PerspectiveCamera* cameraRight = m_scene.createPerspectiveCamera();
            cameraRight->setFrustum(20.f, IntegrationScene::DefaultViewportWidth * 0.5f / (IntegrationScene::DefaultViewportHeight - 20u), 0.1f, 100.f);
            cameraRight->setViewport(IntegrationScene::DefaultViewportWidth / 2u, 20u, IntegrationScene::DefaultViewportWidth / 2u, IntegrationScene::DefaultViewportHeight - 20u);

            renderPass1->setCamera(*cameraLeft);
            renderPass2->setCamera(*cameraRight);
            renderGroup1->addMeshNode(*meshNode1);
            renderGroup2->addMeshNode(*meshNode2);
            break;
        }
    }
}
