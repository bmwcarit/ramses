//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CONNECTIONSTATUSUPDATENOTIFIER_H
#define RAMSES_CONNECTIONSTATUSUPDATENOTIFIER_H

#include "TransportCommon/IConnectionStatusUpdateNotifier.h"
#include "TransportCommon/EConnectionStatus.h"

#include "PlatformAbstraction/PlatformLock.h"
#include "Collections/Guid.h"
#include "Collections/Vector.h"
#include "Collections/HashSet.h"

namespace ramses_internal
{
    class ConnectionStatusUpdateNotifier : public IConnectionStatusUpdateNotifier
    {
    public:
        ConnectionStatusUpdateNotifier(const String& participantName, const String& usage, PlatformLock& frameworkLock);
        virtual ~ConnectionStatusUpdateNotifier() override;

        virtual void registerForConnectionUpdates(IConnectionStatusListener* listener) override;
        virtual void unregisterForConnectionUpdates(IConnectionStatusListener* listener) override;

        void triggerNotification(const Guid& participant, EConnectionStatus status);

    private:
        const String m_participantName;
        const String m_usage;

        PlatformLock& m_lock;
        std::vector<IConnectionStatusListener*> m_listeners;
        HashSet<Guid> m_currentState;
    };
}

#endif
