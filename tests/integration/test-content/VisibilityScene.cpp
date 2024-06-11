//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/VisibilityScene.h"
#include "TestScenes/Triangle.h"
#include "ramses/client/Scene.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/ramses-utils.h"

namespace ramses::internal
{
    VisibilityScene::VisibilityScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition, uint32_t vpWidth, uint32_t vpHeight)
        : IntegrationScene(scene, cameraPosition, vpWidth, vpHeight)
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-basic");
        Triangle triangle1(m_scene, *effect, TriangleAppearance::EColor::Red, 1.f, TriangleGeometry::EVerticesOrder_CCW);
        Triangle triangle2(m_scene, *effect, TriangleAppearance::EColor::Blue, 1.f, TriangleGeometry::EVerticesOrder_CW); // different vertices order forces different indices resource used
        triangle2.GetAppearance().setCullingMode(ramses::ECullMode::Disabled);

        auto triangle1mesh = m_scene.createMeshNode("triangle1");
        auto triangle2mesh = m_scene.createMeshNode("triangle2");
        triangle1mesh->setAppearance(triangle1.GetAppearance());
        triangle1mesh->setGeometry(triangle1.GetGeometry());
        triangle2mesh->setAppearance(triangle2.GetAppearance());
        triangle2mesh->setGeometry(triangle2.GetGeometry());
        addMeshNodeToDefaultRenderGroup(*triangle1mesh);
        addMeshNodeToDefaultRenderGroup(*triangle2mesh);
        triangle1mesh->setTranslation({-1.f, 0.f, -1.f});
        triangle2mesh->setTranslation({1.f, 0.f, -1.f});

        setState(state);
    }

    void VisibilityScene::setState(uint32_t state)
    {
        auto triangle1mesh = m_scene.findObject<ramses::MeshNode>("triangle1");
        auto triangle2mesh = m_scene.findObject<ramses::MeshNode>("triangle2");
        assert(triangle1mesh && triangle2mesh);

        switch (state)
        {
        case VISIBILITY_ALL_OFF:
            triangle1mesh->setVisibility(ramses::EVisibilityMode::Off);
            triangle2mesh->setVisibility(ramses::EVisibilityMode::Off);
            break;
        case VISIBILITY_OFF_AND_INVISIBLE:
            triangle1mesh->setVisibility(ramses::EVisibilityMode::Invisible);
            triangle2mesh->setVisibility(ramses::EVisibilityMode::Off);
            break;
        case VISIBILITY_OFF_AND_VISIBLE:
            triangle1mesh->setVisibility(ramses::EVisibilityMode::Visible);
            triangle2mesh->setVisibility(ramses::EVisibilityMode::Off);
            break;
        case VISIBILITY_ALL_VISIBLE:
            triangle1mesh->setVisibility(ramses::EVisibilityMode::Visible);
            triangle2mesh->setVisibility(ramses::EVisibilityMode::Visible);
            break;
        default:
            break;
        }
    }
}
