//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/PlatformLock.h"

#include <cstdint>
#include <memory>

namespace ramses::internal
{
    class ParticipantIdentifier;
    class IDiscoveryDaemon;
    class ICommunicationSystem;
    class StatisticCollectionFramework;
    class Ramsh;
    class RamsesFrameworkConfigImpl;

    class CommunicationSystemFactory
    {
    public:
        static std::unique_ptr<ICommunicationSystem> ConstructCommunicationSystem(const RamsesFrameworkConfigImpl& config,
            const ParticipantIdentifier& participantIdentifier, PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection);

        static std::unique_ptr<IDiscoveryDaemon> ConstructDiscoveryDaemon(const RamsesFrameworkConfigImpl& config, PlatformLock& frameworkLoc,
                                                                          StatisticCollectionFramework& statisticCollectionk, Ramsh* optionalRamsh = nullptr);
    };
}
