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
#include "Utils/ScopedPointer.h"
#include "PlatformAbstraction/PlatformLock.h"

namespace ramses
{
    class RamsesFrameworkConfigImpl;
}
namespace ramses_internal
{
    class StatisticCollectionFramework;

    class TcpDiscoveryDaemon final : public IDiscoveryDaemon
    {
    public:
        TcpDiscoveryDaemon(const ramses::RamsesFrameworkConfigImpl& config, PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection);
        ~TcpDiscoveryDaemon() override;

        bool start() override;
        bool stop() override;

    private:
        ScopedPointer<ICommunicationSystem> m_communicationSystem;
        Bool m_started;
    };
}

#endif
