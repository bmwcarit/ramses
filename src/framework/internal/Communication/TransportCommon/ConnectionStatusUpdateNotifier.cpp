//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <utility>

#include "internal/Communication/TransportCommon/ConnectionStatusUpdateNotifier.h"
#include "internal/Core/TaskFramework/ITask.h"
#include "internal/Communication/TransportCommon/IConnectionStatusListener.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    ConnectionStatusUpdateNotifier::ConnectionStatusUpdateNotifier(std::string  participantName, const LogContext& logContext, std::string usage, PlatformLock& frameworkLock)
        : m_participantName(std::move(participantName))
        , m_logContext(logContext)
        , m_usage(std::move(usage))
        , m_lock(frameworkLock)
    {
    }

    ConnectionStatusUpdateNotifier::~ConnectionStatusUpdateNotifier() = default;

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
        auto positionToDelete = find_c(m_listeners, listener);
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
            LOG_INFO(m_logContext, "ConnectionStatusUpdateNotifier({}:{})::doStatusUpdate: newParticipantHasConnected {}", m_participantName, m_usage, participant);
            m_currentState.put(participant);
            for(auto listener : m_listeners)
            {
                listener->newParticipantHasConnected(participant);
            }
        }
        else
        {
            LOG_INFO(m_logContext, "ConnectionStatusUpdateNotifier({}:{})::doStatusUpdate: participantHasDisconnected {}", m_participantName, m_usage, participant);
            m_currentState.remove(participant);
            for (auto listener : m_listeners)
            {
                listener->participantHasDisconnected(participant);
            }
        }
    }
}
