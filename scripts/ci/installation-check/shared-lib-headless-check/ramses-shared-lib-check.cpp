//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "ramses/client/ramses-client.h"
#include "ramses/framework/RamsesFramework.h"
#include "ramses/client/text/ramses-text.h"

int main(int argc, char* argv[])
{
    ramses::RamsesFrameworkConfig config{ramses::EFeatureLevel_Latest};

    printf("Start ramses-shared-lib-check headless\n");
    ramses::RamsesFramework framework{config};
    const auto flag = framework.isConnected();

    ramses::RamsesClient* ramses(framework.createClient("ramses-shared-lib-check"));
    ramses::Scene* scene = ramses->createScene(ramses::sceneId_t(1u));
    const auto flag2 = scene->isPublished();

    ramses::FontRegistry fontRegistry;

    printf("End ramses-shared-lib-check headless\n");
    return 0;
}
