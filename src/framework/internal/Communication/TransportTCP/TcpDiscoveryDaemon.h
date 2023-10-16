//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Communication/TransportCommon/IDiscoveryDaemon.h"
#include "internal/Communication/TransportCommon/ICommunicationSystem.h"
#include "internal/Communication/TransportCommon/LogConnectionInfo.h"
#include "internal/PlatformAbstraction/PlatformLock.h"
#include <memory>

namespace ramses::internal
{
    class StatisticCollectionFramework;
    class Ramsh;
    class RamsesFrameworkConfigImpl;

    class TcpDiscoveryDaemon final : public IDiscoveryDaemon
    {
    public:
        TcpDiscoveryDaemon(const RamsesFrameworkConfigImpl& config, PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection, Ramsh* optionalRamsh);
        ~TcpDiscoveryDaemon() override;

        bool start() override;
        bool stop() override;

    private:
        std::unique_ptr<ICommunicationSystem> m_communicationSystem;
        bool m_started;
        std::shared_ptr<LogConnectionInfo> m_commandLogConnectionInformation;
    };
}
