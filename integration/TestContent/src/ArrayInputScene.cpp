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
        trafoNode->setTranslation(-1.0f, 0.0f, -12.0f);

        ramses::MeshNode* meshNode = m_scene.createMeshNode("triangle mesh node");
        meshNode->setParent(*trafoNode);
        meshNode->setAppearance(appearance);
        meshNode->setGeometryBinding(triangleGeometry.GetGeometry());
        addMeshNodeToDefaultRenderGroup(*meshNode);

        appearance.setInputValueInt32(caseNumber, state);

        switch (state)
        {
        case ARRAY_INPUT_VEC4:
        {
            const float floatValues[12] = {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f
            };
            appearance.setInputValueVector4f(colorVec4Array, 3, floatValues);
            break;
        }
        case ARRAY_INPUT_INT32:
        {
            const int32_t intValues[3] = { 0, 1, 0 };
            appearance.setInputValueInt32(colorIntArray, 3, intValues);
            break;
        }
        case ARRAY_INPUT_INT32_DYNAMIC_INDEX:
        {
            appearance.setInputValueInt32(index, 1);
            const int32_t intValues[3] = { 0, 1, 0 };
            appearance.setInputValueInt32(colorIntArray, 3, intValues);
            break;
        }
        }
    }
}
