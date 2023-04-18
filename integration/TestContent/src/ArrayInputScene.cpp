//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/ArrayInputScene.h"
#include "TestScenes/Triangle.h"
#include "ramses-utils.h"

#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Effect.h"
#include "Math3d/Vector4.h"

namespace ramses_internal
{

    ArrayInputScene::ArrayInputScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
    {
        ramses::UniformInput caseNumber;
        ramses::UniformInput colorVec4Array;
        ramses::UniformInput colorIntArray;
        ramses::UniformInput index;

        ramses::Effect* effect = getTestEffect("ramses-test-client-arrays");
        effect->findUniformInput("caseNumber", caseNumber);
        effect->findUniformInput("colorVec4Array", colorVec4Array);
        effect->findUniformInput("colorIntArray", colorIntArray);
        effect->findUniformInput("index", index);

        ramses::TriangleGeometry triangleGeometry(m_scene, *effect);
        ramses::Appearance& appearance = *scene.createAppearance(*effect, "appearance");

        ramses::Node* trafoNode = m_scene.createNode("transformation node");
        trafoNode->setTranslation({-1.0f, 0.0f, -12.0f});

        ramses::MeshNode* meshNode = m_scene.createMeshNode("triangle mesh node");
        meshNode->setParent(*trafoNode);
        meshNode->setAppearance(appearance);
        meshNode->setGeometryBinding(triangleGeometry.GetGeometry());
        addMeshNodeToDefaultRenderGroup(*meshNode);

        appearance.setInputValue(caseNumber, static_cast<int32_t>(state));

        switch (state)
        {
        case ARRAY_INPUT_VEC4:
        {
            const std::array<ramses::vec4f, 3u> floatValues = {
                ramses::vec4f{ 1.0f, 0.0f, 0.0f, 0.0f },
                ramses::vec4f{ 0.0f, 1.0f, 0.0f, 0.0f },
                ramses::vec4f{ 0.0f, 0.0f, 1.0f, 0.0f }
            };
            appearance.setInputValue(colorVec4Array, 3, floatValues.data());
            break;
        }
        case ARRAY_INPUT_INT32:
        {
            const std::array<int32_t, 3u> intValues = { 0, 1, 0 };
            appearance.setInputValue(colorIntArray, 3, intValues.data());
            break;
        }
        case ARRAY_INPUT_INT32_DYNAMIC_INDEX:
        {
            appearance.setInputValue(index, 1);
            const std::array<int32_t, 3u> intValues = { 0, 1, 0 };
            appearance.setInputValue(colorIntArray, 3, intValues.data());
            break;
        }
        }
    }
}
