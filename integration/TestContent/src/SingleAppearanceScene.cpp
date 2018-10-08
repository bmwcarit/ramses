//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/SingleAppearanceScene.h"
#include "TestScenes/Triangle.h"
#include "ramses-utils.h"

#include "ramses-client-api/Scene.h"
#include "ramses-client-api/TransformationNode.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Effect.h"
#include "Math3d/Vector4.h"

namespace ramses_internal
{

    SingleAppearanceScene::SingleAppearanceScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-basic");

        ramses::Triangle redTriangle(m_client, m_scene, *effect, ramses::TriangleAppearance::EColor_Red);

        const float translation[] = {
            -1.0f,  0.0f, -12.0f,
            0.0f, -1.0f, -12.0f,
            1.0f,  0.0f, -12.0f };

        for (int i = 0; i < 3; ++i)
        {
            ramses::MeshNode* meshNode = m_scene.createMeshNode("triangle mesh node");
            addMeshNodeToDefaultRenderGroup(*meshNode);

            ramses::TransformationNode* trafoNode = m_scene.createTransformationNode("transformation node");
            trafoNode->setTranslation(translation[i * 3 + 0], translation[i * 3 + 1], translation[i * 3 + 2]);

            meshNode->setParent(*trafoNode);

            m_meshNodes.push_back(meshNode);
        }

        setAppearanceAndGeometryToAllMeshNodes(redTriangle.GetAppearance(), redTriangle.GetGeometry());

        switch (state)
        {
        case RED_TRIANGLES:
            setTriangleColor(redTriangle.GetAppearance(), *effect, Vector4(1.0f, 0.0f, 0.0f, 1.0f));
            break;
        case GREEN_TRIANGLES:
            setTriangleColor(redTriangle.GetAppearance(), *effect, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
            break;
        case BLUE_TRIANGLES:
            setTriangleColor(redTriangle.GetAppearance(), *effect, Vector4(0.0f, 0.0f, 1.0f, 1.0f));
            break;
        case CHANGE_APPEARANCE:
            ramses::Effect* effectExt = getTestEffect("ramses-test-client-basic-extended");
            ramses::Triangle blueTriangle(m_client, m_scene, *effectExt, ramses::TriangleAppearance::EColor_Blue);
            ramses::Appearance& appearance = blueTriangle.GetAppearance();

            ramses::UniformInput redgreenOffset;
            effectExt->findUniformInput("redgreen_offset", redgreenOffset);
            appearance.setInputValueVector2f(redgreenOffset, 0.5f, 0.5f);

            setAppearanceAndGeometryToAllMeshNodes(blueTriangle.GetAppearance(), blueTriangle.GetGeometry());
            setTriangleColor(blueTriangle.GetAppearance(), *effectExt, Vector4(0.0f, 0.0f, 1.0f, 1.0f));
            break;
        }
    }

    void SingleAppearanceScene::setAppearanceAndGeometryToAllMeshNodes(ramses::Appearance& appearance, ramses::GeometryBinding& geometry)
    {
        for (auto meshNode : m_meshNodes)
        {
            meshNode->removeAppearanceAndGeometry();
            meshNode->setAppearance(appearance);
            meshNode->setGeometryBinding(geometry);
        }
    }

    void SingleAppearanceScene::setTriangleColor(ramses::Appearance& appearance, const ramses::Effect& effect, const Vector4& color)
    {
        ramses::UniformInput colorInput;
        effect.findUniformInput("color", colorInput);
        appearance.setInputValueVector4f(colorInput, color.x, color.y, color.z, color.a);
    }
}
