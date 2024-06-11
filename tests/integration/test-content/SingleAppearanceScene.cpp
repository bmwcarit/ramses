//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/SingleAppearanceScene.h"
#include "TestScenes/Triangle.h"
#include "ramses/client/ramses-utils.h"

#include "ramses/client/Scene.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/Effect.h"

namespace ramses::internal
{

    SingleAppearanceScene::SingleAppearanceScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-basic");

        Triangle redTriangle(m_scene, *effect, TriangleAppearance::EColor::Red);

        const std::array translation = {
            -1.0f,  0.0f, -12.0f,
            0.0f, -1.0f, -12.0f,
            1.0f,  0.0f, -12.0f };

        for (int i = 0; i < 3; ++i)
        {
            ramses::MeshNode* meshNode = m_scene.createMeshNode("triangle mesh node");
            addMeshNodeToDefaultRenderGroup(*meshNode);

            ramses::Node* trafoNode = m_scene.createNode("transformation node");
            trafoNode->setTranslation({translation[i * 3 + 0], translation[i * 3 + 1], translation[i * 3 + 2]});

            meshNode->setParent(*trafoNode);

            m_meshNodes.push_back(meshNode);
        }

        setAppearanceAndGeometryToAllMeshNodes(redTriangle.GetAppearance(), redTriangle.GetGeometry());

        switch (state)
        {
        case RED_TRIANGLES:
            SetTriangleColor(redTriangle.GetAppearance(), *effect, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
            break;
        case GREEN_TRIANGLES:
            SetTriangleColor(redTriangle.GetAppearance(), *effect, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
            break;
        case BLUE_TRIANGLES:
            SetTriangleColor(redTriangle.GetAppearance(), *effect, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
            break;
        case CHANGE_APPEARANCE:
        {
            ramses::Effect* effectExt = getTestEffect("ramses-test-client-basic-extended");
            Triangle blueTriangle(m_scene, *effectExt, TriangleAppearance::EColor::Blue);
            ramses::Appearance& appearance = blueTriangle.GetAppearance();

            appearance.setInputValue(*effectExt->findUniformInput("redgreen_offset"), ramses::vec2f{ 0.5f, 0.5f });

            setAppearanceAndGeometryToAllMeshNodes(blueTriangle.GetAppearance(), blueTriangle.GetGeometry());
            SetTriangleColor(blueTriangle.GetAppearance(), *effectExt, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
            break;
        }
        default:
            break;
        }
    }

    void SingleAppearanceScene::setAppearanceAndGeometryToAllMeshNodes(ramses::Appearance& appearance, ramses::Geometry& geometry)
    {
        for (auto meshNode : m_meshNodes)
        {
            meshNode->removeAppearanceAndGeometry();
            meshNode->setAppearance(appearance);
            meshNode->setGeometry(geometry);
        }
    }

    void SingleAppearanceScene::SetTriangleColor(ramses::Appearance& appearance, const ramses::Effect& effect, const glm::vec4& color)
    {
        appearance.setInputValue(*effect.findUniformInput("color"), color);
    }
}
