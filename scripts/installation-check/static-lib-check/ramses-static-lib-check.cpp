//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
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
#include "ramses-text-bmw.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-framework-api/RamsesFramework.h"

int main(int argc, char* argv[])
{
    ramses::RamsesFramework framework(argc, argv);
    framework.isConnected();

    ramses::RendererConfig config;
    ramses::RamsesRenderer renderer(framework, config);
    renderer.flush();

    ramses::RamsesClient ramses("ramses-shared-lib-check", framework);
    ramses::Scene* scene = ramses.createScene(1u);
    scene->isPublished();

    ramses::BmwFontRegistry bmwFontRegistry;

    return 0;
}
