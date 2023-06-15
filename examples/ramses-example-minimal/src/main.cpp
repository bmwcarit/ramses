//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"

#include <thread>

/**
 * @example ramses-example-minimal/src/main.cpp
 * @brief Minimal Example
 */

int main()
{
    /// [Minimal Example]
    // register at RAMSES daemon
    ramses::RamsesFrameworkConfig config{ramses::EFeatureLevel_Latest};
    ramses::RamsesFramework framework(config);
    ramses::RamsesClient& ramses(*framework.createClient("ramses-example-minimal"));
    framework.connect();

    // create a scene and register it at RAMSES daemon
    ramses::Scene* scene = ramses.createScene(ramses::sceneId_t(123u));

    // signal the scene it is in a state that can be rendered
    scene->flush();

    // distribute the scene to RAMSES
    scene->publish();

    // application logic
    uint32_t loops = 100;

    while (--loops)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // unregister and destroy scene
    scene->unpublish();
    ramses.destroy(*scene);

    // disconnect from RAMSES daemon
    framework.disconnect();
    /// [Minimal Example]

    return 0;
}
