//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "ramses/renderer/RendererConfig.h"
#include "ramses/client/ramses-client.h"
#include "ramses/client/text/ramses-text.h"
#include "ramses/renderer/RamsesRenderer.h"
#include "ramses/framework/RamsesFramework.h"
#include "ramses/client/ramses-utils.h"

int main(int argc, char* argv[])
{
    ramses::RamsesFrameworkConfig frameworkConfig{ramses::EFeatureLevel_Latest};

    printf("Start ramses-shared-lib-check\n");
    ramses::RamsesFramework framework{frameworkConfig};
    const auto flag = framework.isConnected();

    ramses::RendererConfig config;
    ramses::RamsesRenderer* renderer(framework.createRenderer(config));
    renderer->flush();

    ramses::RamsesClient* ramses(framework.createClient("ramses-shared-lib-check"));
    ramses::Scene* scene = ramses->createScene(ramses::sceneId_t(1u));
    ramses::Node* node = scene->createNode();
    ramses::MeshNode* meshNode = scene->createMeshNode();
    const auto flag2 = scene->isPublished();

    ramses::Node* meshNodeAsNode = meshNode;
    ramses::MeshNode* meshNodeDynamic = dynamic_cast<ramses::MeshNode*>(meshNodeAsNode);
    if(!meshNodeDynamic)
    {
        printf("Could not find dynamic casted mesh node!\n");
        exit(1);
    }

    ramses::FontRegistry fontRegistry;

    ramses::nodeId_t nid = ramses::RamsesUtils::GetNodeId(*node);
    const auto val = nid.getValue();

    printf("End ramses-shared-lib-check\n");
    return 0;
}
