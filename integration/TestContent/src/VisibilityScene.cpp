//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/VisibilityScene.h"
#include "TestScenes/Triangle.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-utils.h"

namespace ramses_internal
{
    VisibilityScene::VisibilityScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-basic");
        ramses::Triangle triangle1(m_scene, *effect, ramses::TriangleAppearance::EColor_Red, 1.f, ramses::TriangleGeometry::EVerticesOrder_CCW);
        ramses::Triangle triangle2(m_scene, *effect, ramses::TriangleAppearance::EColor_Blue, 1.f, ramses::TriangleGeometry::EVerticesOrder_CW); // different vertices order forces different indices resource used
        triangle2.GetAppearance().setCullingMode(ramses::ECullMode_Disabled);

        auto triangle1mesh = m_scene.createMeshNode("triangle1");
        auto triangle2mesh = m_scene.createMeshNode("triangle2");
        triangle1mesh->setAppearance(triangle1.GetAppearance());
        triangle1mesh->setGeometryBinding(triangle1.GetGeometry());
        triangle2mesh->setAppearance(triangle2.GetAppearance());
        triangle2mesh->setGeometryBinding(triangle2.GetGeometry());
        addMeshNodeToDefaultRenderGroup(*triangle1mesh);
        addMeshNodeToDefaultRenderGroup(*triangle2mesh);
        triangle1mesh->setTranslation(-1.f, 0.f, -1.f);
        triangle2mesh->setTranslation(1.f, 0.f, -1.f);

        setState(state);
    }

    void VisibilityScene::setState(UInt32 state)
    {
        auto triangle1mesh = ramses::RamsesUtils::TryConvert<ramses::MeshNode>(*m_scene.findObjectByName("triangle1"));
        auto triangle2mesh = ramses::RamsesUtils::TryConvert<ramses::MeshNode>(*m_scene.findObjectByName("triangle2"));
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
        }
    }
}
