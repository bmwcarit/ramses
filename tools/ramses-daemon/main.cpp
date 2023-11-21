//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Communication/TransportCommon/CommunicationSystemFactory.h"
#include "internal/Communication/TransportCommon/IDiscoveryDaemon.h"
#include "ramses-sdk-build-config.h"
#include "internal/Core/Common/ParticipantIdentifier.h"
#include "impl/RamsesFrameworkConfigImpl.h"
#include "internal/Ramsh/RamshStandardSetup.h"
#include "internal/Ramsh/RamshCommandExit.h"
#include "internal/Core/Utils/RamsesLogger.h"
#include "internal/Core/Utils/StatisticCollection.h"
#include "internal/Core/Utils/LogMacros.h"
#include <memory>
#include "ramses-framework-cli.h"

int main(int argc, const char* argv[])
{
    using namespace ramses::internal;

    ramses::RamsesFrameworkConfig config{ramses::EFeatureLevel_Latest};
    config.setDLTApplicationDescription("ramses-daemon");
    config.setDLTApplicationID("SMGR");

    CLI::App cli;
    try
    {
        ramses::registerOptions(cli, config);
    }
    catch (const CLI::Error& error)
    {
        // configuration error
        std::cerr << error.what();
        return -1;
    }
    CLI11_PARSE(cli, argc, argv);

    GetRamsesLogger().initialize(config.impl().loggerConfig, false, true); // no framework used

    auto commandExit = std::make_shared<RamshCommandExit>();
    RamshStandardSetup ramsh(ramses::ERamsesShellType::Console, "Daemon");
    ramsh.add(commandExit);
    ramsh.start();

    LOG_INFO(CONTEXT_CLIENT, "Daemon::main  Starting Ramses Daemon");
    LOG_INFO(CONTEXT_CLIENT, "Daemon::main  Version: {} Hash:{} Commit:{}",
        ::ramses_sdk::RAMSES_SDK_RAMSES_VERSION, ::ramses_sdk::RAMSES_SDK_GIT_COMMIT_HASH, ::ramses_sdk::RAMSES_SDK_GIT_COMMIT_COUNT);

    PlatformLock frameworkLock;
    StatisticCollectionFramework statisticCollection;
    std::unique_ptr<IDiscoveryDaemon> discoveryDaemon(CommunicationSystemFactory::ConstructDiscoveryDaemon(config.impl(), frameworkLock, statisticCollection, &ramsh));
    discoveryDaemon->start();
    LOG_DEBUG(CONTEXT_SMOKETEST, "Ramsh commands registered");

    commandExit->waitForExitRequest();

    ramsh.stop();
    return 0;
}
