//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/DataLinkScene.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/DataVector4f.h"
#include "ramses-client-api/Appearance.h"

#include "Scene/ClientScene.h"
#include "TestScenes/Triangle.h"

namespace ramses_internal
{
    DataLinkScene::DataLinkScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-basic");
        ramses::Triangle triangle1(scene, *effect, ramses::TriangleAppearance::EColor_Red);
        ramses::Triangle triangle2(scene, *effect, ramses::TriangleAppearance::EColor_Green);

        ramses::DataVector4f* colorData = scene.createDataVector4f("dataLinkColorData");

        triangle1.bindColor(*colorData);
        triangle2.bindColor(*colorData);

        ramses::MeshNode* mesh1 = scene.createMeshNode();
        ramses::MeshNode* mesh2 = scene.createMeshNode();

        ramses::Appearance& appearance1 = triangle1.GetAppearance();
        ramses::Appearance& appearance2 = triangle2.GetAppearance();
        appearance1.setName("dataLinkAppearance1");
        appearance2.setName("dataLinkAppearance2");

        mesh1->setAppearance(appearance1);
        mesh2->setAppearance(appearance2);

        mesh1->setGeometryBinding(triangle1.GetGeometry());
        mesh2->setGeometryBinding(triangle2.GetGeometry());

        ramses::Node* translate1 = scene.createNode();
        ramses::Node* translate2 = scene.createNode();

        mesh1->setParent(*translate1);
        mesh2->setParent(*translate2);

        addMeshNodeToDefaultRenderGroup(*mesh1);
        addMeshNodeToDefaultRenderGroup(*mesh2);

        translate1->setTranslation(-1.5f, 0.f, -15.f);
        translate2->setTranslation( 1.5f, 0.f, -15.f);

        switch (state)
        {
        case DATA_PROVIDER:
            scene.createDataProvider(*colorData, DataProviderId);
            colorData->setValue(1.f, 0.f, 0.f, 1.f);
            break;
        case DATA_CONSUMER:
            scene.createDataConsumer(*colorData, DataConsumerId);
            colorData->setValue(0.f, 1.f, 0.f, 1.f);
            break;
        case DATA_CONSUMER_AND_PROVIDER:
        {
            ramses::DataVector4f* colorData2 = scene.createDataVector4f();
            triangle2.bindColor(*colorData2);

            scene.createDataProvider(*colorData, DataProviderId);
            scene.createDataConsumer(*colorData2, DataConsumerId);

            colorData->setValue(0.f, 1.f, 1.f, 1.f);
            colorData2->setValue(0.f, 0.f, 1.f, 1.f);
        }
            break;
        default:
            assert(false && "invalid scene state");
        }
    }
}
