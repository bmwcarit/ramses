//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/Citymodel.h"
#include "RamsesFrameworkImpl.h"
#include "Ramsh/RamshCommandExit.h"
#include "thread"

int main(int argc, char* argv[])
{
    ramses::RamsesFrameworkConfig frameworkConfig(argc, argv);
    frameworkConfig.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);

    ramses::RamsesFramework framework(frameworkConfig);
    ramses_internal::RamshCommandExit commandExit;
    framework.impl.getRamsh().add(commandExit);

    Citymodel citymodel(argc, argv, framework);

    framework.connect();

    uint32_t frame = 0;
    Timer timer;

    while (!citymodel.shouldExit() && !commandExit.exitRequested())
    {
        citymodel.doFrame();
        frame++;

        const float frameTime = 1.0f / 60.0f;
        float       sleepTime = (frameTime * frame - timer.getTime());
        if (sleepTime >= 0)
        {
            std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime));
        }
    }

    return 0;
}
