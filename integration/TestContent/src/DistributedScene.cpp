//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/DistributedScene.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/TranslateNode.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/RenderPass.h"
#include "RamsesFrameworkImpl.h"
#include "RamsesClientImpl.h"

namespace ramses_internal
{
    DistributedScene::DistributedScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
    {
        createContent();

        switch (state)
        {
        case SHOW_SCENE:
            break;
        case UNPUBLISH:
            scene.unpublish();
            break;
        case DESTROY:
            m_client.destroy(scene);
            break;
        case DISCONNECT:
            ramsesClient.impl.getFramework().disconnect();
            break;
        }
    }

    void DistributedScene::createContent()
    {
        ramses::Triangle triangle(m_client, m_scene, *getTestEffect("ramses-test-client-basic"), ramses::TriangleAppearance::EColor_Red);

        ramses::MeshNode& meshNode = *m_scene.createMeshNode("red triangle mesh node");
        addMeshNodeToDefaultRenderGroup(meshNode);
        meshNode.setGeometryBinding(triangle.GetGeometry());
        meshNode.setAppearance(triangle.GetAppearance());

        ramses::TranslateNode& transNode = *m_scene.createTranslateNode();
        transNode.setTranslation(0.f, 0.f, -10.f);
        meshNode.setParent(transNode);
    }
}
