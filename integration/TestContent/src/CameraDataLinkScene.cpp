//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/CameraDataLinkScene.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/DataObject.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "ramses-utils.h"

#include "Scene/ClientScene.h"
#include "TestScenes/Triangle.h"

namespace ramses_internal
{
    CameraDataLinkScene::CameraDataLinkScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
    {
        switch (state)
        {
        case CAMERADATA_PROVIDER:
            setUpProviderScene();
            break;
        case CAMERADATA_CONSUMER:
            setUpConsumerScene();
            break;
        default:
            assert(false && "invalid scene state");
        }
    }

    void CameraDataLinkScene::setUpConsumerScene()
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-basic");
        ramses::Triangle triangle1(m_scene, *effect, ramses::TriangleAppearance::EColor_Red);
        ramses::Triangle triangle2(m_scene, *effect, ramses::TriangleAppearance::EColor_Green);

        ramses::MeshNode* mesh1 = m_scene.createMeshNode();
        ramses::MeshNode* mesh2 = m_scene.createMeshNode();

        ramses::Appearance& appearance1 = triangle1.GetAppearance();
        ramses::Appearance& appearance2 = triangle2.GetAppearance();
        appearance1.setName("dataLinkAppearance1");
        appearance2.setName("dataLinkAppearance2");

        mesh1->setAppearance(appearance1);
        mesh2->setAppearance(appearance2);

        mesh1->setGeometryBinding(triangle1.GetGeometry());
        mesh2->setGeometryBinding(triangle2.GetGeometry());

        addMeshNodeToDefaultRenderGroup(*mesh1);
        addMeshNodeToDefaultRenderGroup(*mesh2);

        mesh1->setTranslation({0.f, -0.25f, -1.f});
        mesh1->setScaling({0.3f, 0.5f, 1.f});
        mesh2->setTranslation({0.f, -10.f, -5.f});
        mesh2->setScaling({100.f, 100.f, 1.f});

        auto camera = m_scene.createPerspectiveCamera();
        camera->setViewport(0, 0, 16u, 16u);
        camera->setFrustum(19.f, static_cast<float>(ramses_internal::IntegrationScene::DefaultViewportWidth) / ramses_internal::IntegrationScene::DefaultViewportHeight, 0.1f, 100.f);
        setCameraToDefaultRenderPass(camera);

        auto dataVpOffset = m_scene.createDataObject(ramses::EDataType::Vector2I);
        auto dataVpSize = m_scene.createDataObject(ramses::EDataType::Vector2I);
        dataVpOffset->setValue(ramses::vec2i{ 0, 0 });
        dataVpSize->setValue(ramses::vec2i{ int32_t(ramses_internal::IntegrationScene::DefaultViewportWidth), int32_t(ramses_internal::IntegrationScene::DefaultViewportHeight) });
        camera->bindViewportOffset(*dataVpOffset);
        camera->bindViewportSize(*dataVpSize);
        auto dataFrustumPlanes = m_scene.createDataObject(ramses::EDataType::Vector4F);
        auto dataFrustumNearFarPlanes = m_scene.createDataObject(ramses::EDataType::Vector2F);
        ramses::RamsesUtils::SetPerspectiveCameraFrustumToDataObjects(camera->getVerticalFieldOfView(), camera->getAspectRatio(), camera->getNearPlane(), camera->getFarPlane(), *dataFrustumPlanes, *dataFrustumNearFarPlanes);
        camera->bindFrustumPlanes(*dataFrustumPlanes, *dataFrustumNearFarPlanes);

        m_scene.createDataConsumer(*dataVpOffset, ViewportOffsetConsumerId);
        m_scene.createDataConsumer(*dataVpSize, ViewportSizeConsumerId);
        m_scene.createDataConsumer(*dataFrustumPlanes, FrustumPlanesConsumerId);
        m_scene.createDataConsumer(*dataFrustumNearFarPlanes, FrustumPlanesNearFarConsumerId);
    }

    void CameraDataLinkScene::setUpProviderScene()
    {
        auto dataVpOffset = m_scene.createDataObject(ramses::EDataType::Vector2I);
        auto dataVpSize = m_scene.createDataObject(ramses::EDataType::Vector2I);
        auto dataFrustumPlanes = m_scene.createDataObject(ramses::EDataType::Vector4F);
        auto dataFrustumNearFarPlanes = m_scene.createDataObject(ramses::EDataType::Vector2F);
        dataVpOffset->setValue(ramses::vec2i{ 20, 20 });
        dataVpSize->setValue(ramses::vec2i{ int32_t(ramses_internal::IntegrationScene::DefaultViewportWidth / 2), int32_t(ramses_internal::IntegrationScene::DefaultViewportHeight / 2) });
        ramses::RamsesUtils::SetPerspectiveCameraFrustumToDataObjects(
            60.f, // much wider FOV will make content smaller when projected
            0.5f, // original aspect ratio is 1:1, this will make content wider
            0.1f,
            5.f, // far plane closer to camera will cause complete cull of 2nd triangle in background
            *dataFrustumPlanes,
            *dataFrustumNearFarPlanes);

        m_scene.createDataProvider(*dataVpOffset, ViewportOffsetProviderId);
        m_scene.createDataProvider(*dataVpSize, ViewportSizeProviderId);
        m_scene.createDataProvider(*dataFrustumPlanes, FrustumPlanesProviderId);
        m_scene.createDataProvider(*dataFrustumNearFarPlanes, FrustumPlanesNearFarProviderId);
    }
}
