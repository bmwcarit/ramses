//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportCommon/ConnectionStatusUpdateNotifier.h"
#include "TaskFramework/ITask.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "TransportCommon/IConnectionStatusListener.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    ConnectionStatusUpdateNotifier::ConnectionStatusUpdateNotifier(PlatformLock& frameworkLock)
        : m_lock(frameworkLock)
    {
    }

    ConnectionStatusUpdateNotifier::~ConnectionStatusUpdateNotifier()
    {
    }

    void ConnectionStatusUpdateNotifier::registerForConnectionUpdates(IConnectionStatusListener* listener)
    {
        PlatformGuard g(m_lock);
        m_listeners.push_back(listener);
        for(const auto& participant : m_currentState)
        {
            listener->newParticipantHasConnected(participant);
        }
    }

    void ConnectionStatusUpdateNotifier::unregisterForConnectionUpdates(IConnectionStatusListener* listener)
    {
        PlatformGuard g(m_lock);
        Vector<IConnectionStatusListener*>::Iterator positionToDelete = m_listeners.find(listener);
        if (positionToDelete != m_listeners.end())
        {
            m_listeners.erase(positionToDelete);
        }
    }

    void ConnectionStatusUpdateNotifier::triggerNotification(const Guid& participant, EConnectionStatus status)
    {
        PlatformGuard g(m_lock);
        assert(status == EConnectionStatus_Connected || status == EConnectionStatus_NotConnected);
        if (status == EConnectionStatus_Connected)
        {
            LOG_INFO(CONTEXT_COMMUNICATION, "ConnectionStatusUpdateNotifier::doStatusUpdate: newParticipantHasConnected " << participant);
            m_currentState.put(participant);
            for(auto listener : m_listeners)
            {
                listener->newParticipantHasConnected(participant);
            }
        }
        else
        {
            LOG_INFO(CONTEXT_COMMUNICATION, "ConnectionStatusUpdateNotifier::doStatusUpdate: participantHasDisconnected " << participant);
            m_currentState.remove(participant);
            for (auto listener : m_listeners)
            {
                listener->participantHasDisconnected(participant);
            }
        }
    }
}
