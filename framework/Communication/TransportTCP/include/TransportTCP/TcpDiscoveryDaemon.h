//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TCPDISCOVERYDAEMON_H
#define RAMSES_TCPDISCOVERYDAEMON_H

#include "TransportCommon/IDiscoveryDaemon.h"
#include "TransportCommon/ICommunicationSystem.h"
#include "TransportCommon/LogConnectionInfo.h"
#include "PlatformAbstraction/PlatformLock.h"
#include <memory>

namespace ramses
{
    class RamsesFrameworkConfigImpl;
}
namespace ramses_internal
{
    class StatisticCollectionFramework;
    class Ramsh;

    class TcpDiscoveryDaemon final : public IDiscoveryDaemon
    {
    public:
        TcpDiscoveryDaemon(const ramses::RamsesFrameworkConfigImpl& config, PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection, Ramsh* optionalRamsh);
        ~TcpDiscoveryDaemon() override;

        bool start() override;
        bool stop() override;

    private:
        std::unique_ptr<ICommunicationSystem> m_communicationSystem;
        Bool m_started;
        std::unique_ptr<LogConnectionInfo> m_commandLogConnectionInformation;
    };
}

#endif
