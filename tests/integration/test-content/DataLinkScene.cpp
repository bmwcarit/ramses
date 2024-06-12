//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/DataLinkScene.h"
#include "ramses/client/Scene.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/DataObject.h"
#include "ramses/client/Appearance.h"

#include "internal/SceneGraph/Scene/ClientScene.h"
#include "TestScenes/Triangle.h"

namespace ramses::internal
{
    DataLinkScene::DataLinkScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition)
        : IntegrationScene(scene, cameraPosition)
    {
        ramses::Effect* effect = getTestEffect("ramses-test-client-basic");
        Triangle triangle1(scene, *effect, TriangleAppearance::EColor::Red);
        Triangle triangle2(scene, *effect, TriangleAppearance::EColor::Green);

        ramses::DataObject* colorData = scene.createDataObject(ramses::EDataType::Vector4F, "dataLinkColorData");

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

        mesh1->setGeometry(triangle1.GetGeometry());
        mesh2->setGeometry(triangle2.GetGeometry());

        ramses::Node* translate1 = scene.createNode();
        ramses::Node* translate2 = scene.createNode();

        mesh1->setParent(*translate1);
        mesh2->setParent(*translate2);

        addMeshNodeToDefaultRenderGroup(*mesh1);
        addMeshNodeToDefaultRenderGroup(*mesh2);

        translate1->setTranslation({-1.5f, 0.f, -15.f});
        translate2->setTranslation({1.5f, 0.f, -15.f});

        switch (state)
        {
        case DATA_PROVIDER:
            scene.createDataProvider(*colorData, DataProviderId);
            colorData->setValue(ramses::vec4f{ 1.f, 0.f, 0.f, 1.f });
            break;
        case DATA_CONSUMER:
            scene.createDataConsumer(*colorData, DataConsumerId);
            colorData->setValue(ramses::vec4f{ 0.f, 1.f, 0.f, 1.f });
            break;
        case DATA_CONSUMER_AND_PROVIDER:
        {
            auto colorData2 = scene.createDataObject(ramses::EDataType::Vector4F);
            triangle2.bindColor(*colorData2);

            scene.createDataProvider(*colorData, DataProviderId);
            scene.createDataConsumer(*colorData2, DataConsumerId);

            colorData->setValue(ramses::vec4f{ 0.f, 1.f, 1.f, 1.f });
            colorData2->setValue(ramses::vec4f{ 0.f, 0.f, 1.f, 1.f });
        }
            break;
        default:
            assert(false && "invalid scene state");
        }
    }
}
