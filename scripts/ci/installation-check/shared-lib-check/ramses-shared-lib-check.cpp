//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "ramses-renderer-api/RendererConfig.h"
#include "ramses-client.h"
#include "ramses-text.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-utils.h"
#include "CLI/CLI.hpp"

int main(int argc, char* argv[])
{
    CLI::App cli;
    ramses::RamsesFrameworkConfig frameworkConfig;
    frameworkConfig.registerOptions(cli);
    CLI11_PARSE(cli, argc, argv);

    printf("Start ramses-shared-lib-check\n");
    ramses::RamsesFramework framework{frameworkConfig};
    framework.isConnected();

    ramses::RendererConfig config;
    ramses::RamsesRenderer* renderer(framework.createRenderer(config));
    renderer->flush();

    ramses::RamsesClient* ramses(framework.createClient("ramses-shared-lib-check"));
    ramses::Scene* scene = ramses->createScene(ramses::sceneId_t(1u));
    ramses::Node* node = scene->createNode();
    scene->isPublished();

    ramses::FontRegistry fontRegistry;

    ramses::nodeId_t nid = ramses::RamsesUtils::GetNodeId(*node);
    nid.getValue();

    printf("End ramses-shared-lib-check\n");
    return 0;
}
