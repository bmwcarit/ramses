//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/AntiAliasingScene.h"
#include "ramses-utils.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/Effect.h"

#include "Collections/Vector.h"
#define _USE_MATH_DEFINES
#include <math.h>

namespace ramses_internal
{
    AntiAliasingScene::AntiAliasingScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 /*state*/, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
    {
        std::vector<Float> pos;
        std::vector<uint16_t> id;
        std::vector<Float> colors;

        Float zValue = -6.5f;

        pos.push_back(0.f);
        pos.push_back(0.f);
        pos.push_back(zValue);

        colors.push_back(1.f);
        colors.push_back(0.f);
        colors.push_back(0.f);

        pos.push_back(0.f);
        pos.push_back(0.f);
        pos.push_back(zValue);

        colors.push_back(1.f);
        colors.push_back(1.f);
        colors.push_back(1.f);

        const UInt32 slices = 28;
        const Float sliceAngle = 2.f * Float(M_PI) / slices;
        for (UInt16 i = 0; i <= slices; i++)
        {
            Float angle = sliceAngle * i;
            Float x = cosf(angle);
            Float y = sinf(angle);

            pos.push_back(x);
            pos.push_back(y);
            pos.push_back(zValue);

            colors.push_back(1.f);
            colors.push_back(0.f);
            colors.push_back(0.f);

            pos.push_back(x);
            pos.push_back(y);
            pos.push_back(zValue);

            colors.push_back(1.f);
            colors.push_back(1.f);
            colors.push_back(1.f);

            if (i > 0)
            {
                id.push_back(i % 2);
                id.push_back(2 * i + i % 2);
                id.push_back(2 * i + 2 + i % 2);
            }
        }

        const ramses::Vector3fArray* vertexPositions = m_client.createConstVector3fArray(uint32_t(pos.size() / 3), &pos[0]);
        const ramses::UInt16Array* indices = m_client.createConstUInt16Array(uint32_t(id.size()), &id[0]);
        const ramses::Vector3fArray* vertexColors = m_client.createConstVector3fArray(uint32_t(colors.size() / 3), &colors[0]);

        ramses::Effect* effectTex = getTestEffect("ramses-test-client-simple-color");
        ramses::Appearance* appearance = m_scene.createAppearance(*effectTex, "disk appearance");

        ramses::AttributeInput positionsInput;
        ramses::AttributeInput colorsInput;
        effectTex->findAttributeInput("a_position", positionsInput);
        effectTex->findAttributeInput("a_color", colorsInput);

        // set vertex positions directly in geometry
        ramses::GeometryBinding* geometry = m_scene.createGeometryBinding(*effectTex, "disk geometry");
        geometry->setIndices(*indices);
        geometry->setInputBuffer(positionsInput, *vertexPositions);
        geometry->setInputBuffer(colorsInput, *vertexColors);

        // create a mesh node to define the triangle with chosen appearance
        ramses::MeshNode* meshNode = m_scene.createMeshNode("disk mesh node");
        addMeshNodeToDefaultRenderGroup(*meshNode);
        meshNode->setAppearance(*appearance);
        meshNode->setGeometryBinding(*geometry);
    }
}
