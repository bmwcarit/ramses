//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/Citymodel.h"
#include "ramses-demoLib/DisplayManager.h"

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "RendererLib/RendererConfig.h"
#include "RendererConfigImpl.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "RamsesFrameworkImpl.h"
#include "Ramsh/RamshCommandExit.h"


int main(int argc, char* argv[])
{
    ramses::RamsesFrameworkConfig frameworkConfig(argc, argv);
    frameworkConfig.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);
    ramses::RamsesFramework framework(frameworkConfig);
    ramses_internal::RamshCommandExit commandExit;
    framework.impl.getRamsh().add(commandExit);

    ramses::RendererConfig rendererConfig(argc, argv);

    ramses_internal::RendererConfig& internalRendererConfig = const_cast<ramses_internal::RendererConfig&>(rendererConfig.impl.getInternalRendererConfig());

    std::chrono::microseconds frameCallbackMaxPollTime{1000000u};
    internalRendererConfig.setFrameCallbackMaxPollTime(frameCallbackMaxPollTime);

    ramses::RamsesRenderer renderer(framework, rendererConfig);
    renderer.setSkippingOfUnmodifiedBuffers(false);

    Citymodel client(argc, argv, framework);
    if (client.shouldExit())
    {
        return 0;
    }
    framework.connect();

    DisplayManager displayManager(renderer, framework, &client);

    ramses::DisplayConfig displayConfig(argc, argv);
    const ramses::displayId_t displayId = displayManager.createDisplay(displayConfig);

    ramses::sceneId_t sceneId = client.getSceneId();

    const uint32_t renderOrder = 0;
    displayManager.showSceneOnDisplay(sceneId, displayId, renderOrder);

    while (!client.shouldExit() && displayManager.isRunning() && !commandExit.exitRequested())
    {
        client.doFrame();
        renderer.doOneLoop();
        displayManager.dispatchAndFlush();
    }

    renderer.hideScene(sceneId);
    renderer.unmapScene(sceneId);
    renderer.destroyDisplay(displayId);
    renderer.flush();
}
