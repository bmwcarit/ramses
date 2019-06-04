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
    ConnectionStatusUpdateNotifier::ConnectionStatusUpdateNotifier(const String& participantName, const String& usage, PlatformLock& frameworkLock)
        : m_participantName(participantName)
        , m_usage(usage)
        , m_lock(frameworkLock)
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
        std::vector<IConnectionStatusListener*>::iterator positionToDelete = find_c(m_listeners, listener);
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
            LOG_INFO(CONTEXT_COMMUNICATION, "ConnectionStatusUpdateNotifier(" << m_participantName << ":" << m_usage << ")::doStatusUpdate: newParticipantHasConnected " << participant);
            m_currentState.put(participant);
            for(auto listener : m_listeners)
            {
                listener->newParticipantHasConnected(participant);
            }
        }
        else
        {
            LOG_INFO(CONTEXT_COMMUNICATION, "ConnectionStatusUpdateNotifier(" << m_participantName << ":" << m_usage << ")::doStatusUpdate: participantHasDisconnected " << participant);
            m_currentState.remove(participant);
            for (auto listener : m_listeners)
            {
                listener->participantHasDisconnected(participant);
            }
        }
    }
}
