//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMMUNICATIONSYSTEMFACTORY_H
#define RAMSES_COMMUNICATIONSYSTEMFACTORY_H

#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses
{
    class RamsesFrameworkConfigImpl;
}

namespace ramses_internal
{
    class ParticipantIdentifier;
    class IDiscoveryDaemon;
    class ICommunicationSystem;
    class PlatformLock;
    class StatisticCollectionFramework;

    class CommunicationSystemFactory
    {
    public:
        static ICommunicationSystem* ConstructCommunicationSystem(const ramses::RamsesFrameworkConfigImpl& config,
            const ParticipantIdentifier& participantIdentifier, PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection);

        static IDiscoveryDaemon* ConstructDiscoveryDaemon(const ramses::RamsesFrameworkConfigImpl& config, PlatformLock& frameworkLoc,
            StatisticCollectionFramework& statisticCollectionk);
    };
}

#endif
