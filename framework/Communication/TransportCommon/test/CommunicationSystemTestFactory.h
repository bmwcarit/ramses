//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMMUNICATION_COMMUNICATIONSYSTEMTESTFACTORY_H
#define RAMSES_COMMUNICATION_COMMUNICATIONSYSTEMTESTFACTORY_H

#include "CommunicationSystemTestWrapper.h"

namespace ramses_internal
{
    namespace CommunicationSystemTestFactory
    {
        inline std::unique_ptr<CommunicationSystemTestState> ConstructTestState(ECommunicationSystemType commType, EServiceType serviceType)
        {
            return std::unique_ptr<CommunicationSystemTestState>{new CommunicationSystemTestState(commType, serviceType)};
        }

        inline std::unique_ptr<CommunicationSystemDiscoveryDaemonTestWrapper> ConstructDiscoveryDaemonTestWrapper(CommunicationSystemTestState& state)
        {
            return std::unique_ptr<CommunicationSystemDiscoveryDaemonTestWrapper>{new CommunicationSystemDiscoveryDaemonTestWrapper(state)};
        }

        inline std::unique_ptr<CommunicationSystemTestWrapper> ConstructTestWrapper(CommunicationSystemTestState& state_, const String& name = String(), const Guid& id_ = Guid(true),
            ECommunicationSystemTestConfiguration commSysConfig_ = ECommunicationSystemTestConfiguration_Default,
            bool allowObjectDestructionICOnly = true)
        {
            UNUSED(allowObjectDestructionICOnly);
            return std::unique_ptr<CommunicationSystemTestWrapper>{new CommunicationSystemTestWrapper(state_, name, id_, commSysConfig_)};
        }
    }
}

#endif
