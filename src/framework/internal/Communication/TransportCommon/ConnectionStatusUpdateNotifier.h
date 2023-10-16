//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Communication/TransportCommon/IConnectionStatusUpdateNotifier.h"
#include "internal/Communication/TransportCommon/EConnectionStatus.h"

#include "internal/PlatformAbstraction/PlatformLock.h"
#include "internal/PlatformAbstraction/Collections/Guid.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "internal/PlatformAbstraction/Collections/HashSet.h"
#include "internal/Core/Utils/LogContext.h"

namespace ramses::internal
{
    class ConnectionStatusUpdateNotifier : public IConnectionStatusUpdateNotifier
    {
    public:
        ConnectionStatusUpdateNotifier(std::string  participantName, const LogContext& logContext, std::string  usage, PlatformLock& frameworkLock);
        ~ConnectionStatusUpdateNotifier() override;

        void registerForConnectionUpdates(IConnectionStatusListener* listener) override;
        void unregisterForConnectionUpdates(IConnectionStatusListener* listener) override;

        void triggerNotification(const Guid& participant, EConnectionStatus status);

    private:
        const std::string m_participantName;
        const LogContext& m_logContext;
        const std::string m_usage;

        PlatformLock& m_lock;
        std::vector<IConnectionStatusListener*> m_listeners;
        HashSet<Guid> m_currentState;
    };
}
