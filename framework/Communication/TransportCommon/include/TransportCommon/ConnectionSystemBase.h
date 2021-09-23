//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CONNECTIONSYSTEMBASE_H
#define RAMSES_CONNECTIONSYSTEMBASE_H

#include "TransportCommon/ConnectionStatusUpdateNotifier.h"
#include "TransportCommon/SomeIPStackCommon.h"
#include "Common/ParticipantIdentifier.h"
#include "Collections/HashMap.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "Utils/LogMacros.h"
#include "Utils/StatisticCollection.h"
#include "PlatformAbstraction/PlatformTime.h"
#include <memory>
#include <condition_variable>
#include <random>
#include <type_traits>


namespace ramses_internal
{
    class StatisticCollectionFramework;

    template <typename Callbacks>
    class ConnectionSystemBase : public Callbacks, public Runnable
    {
        using InstanceIdType = typename Callbacks::InstanceIdType;
    public:
        ~ConnectionSystemBase() override;

        void writeStateForLog(StringOutputStream& sos);
        void logConnectionInfo();
        void logPeriodicInfo();

        bool connect();
        bool disconnect();

        IConnectionStatusUpdateNotifier& getConnectionStatusUpdateNotifier();

        struct ParticipantState
        {
            Guid pid;
            InstanceIdType iid;

            uint64_t sendSessionId;
            uint64_t sendMessageId;
            std::chrono::steady_clock::time_point lastSent;

            uint64_t expectedRecvSessionId;
            uint64_t expectedRecvMessageId;
            std::chrono::steady_clock::time_point lastRecv;

            // TODO(tobias) this is a hack to work around reconnection ping-pong. When set sending pinfo is
            // skipped on counter mismatch when receiving a pinfo. If it was falsely skipped the next message
            // or keepalive will trigger an error on remote initiating another connection reset that is then
            // handled regularly.
            // This flag is reset on each received pinfo.
            bool skipSendPinfoOnNextMismatch = false;
        };

        // for testing only!
        const ParticipantState* getParticipantState(InstanceIdType iid) const;
        std::chrono::steady_clock::time_point doOneThreadLoop(std::chrono::milliseconds keepAliveInterval,std::chrono::milliseconds keepAliveTimeout);

    protected:
        static bool CheckConstructorArguments(const std::shared_ptr<ISomeIPStackCommon<InstanceIdType>>& stack, UInt32 communicationUserID, const ParticipantIdentifier& namedPid,
                                              UInt32 protocolVersion,
                                              std::chrono::milliseconds keepAliveInterval, std::chrono::milliseconds keepAliveTimeout,
                                              const LogContext& logContext, const String& serviceTypeName);

        ConnectionSystemBase(std::shared_ptr<ISomeIPStackCommon<InstanceIdType>> stack, UInt32 communicationUserID, const ParticipantIdentifier& namedPid,
                             UInt32 protocolVersion, PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection,
                             std::chrono::milliseconds keepAliveInterval, std::chrono::milliseconds keepAliveTimeout,
                             std::function<std::chrono::steady_clock::time_point(void)> steadyClockNow,
                             const LogContext& logContext, const String& serviceTypeName);

        template <typename F>
        bool sendUnicast(const char* callerMethod, const Guid& to, F&& sendFunc);
        template <typename F>
        bool sendBroadcast(const char* callerMethod, F&& sendFunc);

        Guid* processReceivedMessageHeader(const SomeIPMsgHeader& header, const char* callerMethod);

        const UInt32 m_communicationUserID;

    private:
        bool trySendParticipantInfo(ParticipantState& pstate);
        void initNewSession(ParticipantState& pstate);
        ParticipantState* addParticipantState(const Guid& pid, InstanceIdType iid);
        bool checkConnected(const char* callerMethod) const;
        ParticipantState* getParticipantStateForSending(const Guid& pid, const char* callerMethod);
        bool handleSendResult(ParticipantState& pstate, const char* callerMethod, bool result);
        SomeIPMsgHeader generateHeaderForParticipant(ParticipantState& pstate);
        const HashMap<InstanceIdType, ParticipantState*>& availableInstances();
        bool isParticipantConnected(ParticipantState* pstate) const;
        ParticipantState* processMessageHeaderGeneric(const SomeIPMsgHeader& header, const char* callerMethod);

        virtual void run() override;

        // from ISomeIPCallbacks
        virtual void handleServiceAvailable(InstanceIdType iid) override final;
        virtual void handleServiceUnavailable(InstanceIdType iid) override final;

        virtual void handleParticipantInfo(const SomeIPMsgHeader& header, uint16_t protocolVersion, InstanceIdType senderInstanceId, uint64_t expectedReceiverPid, uint8_t clockType, uint64_t timestampNow) override final;
        virtual void handleKeepAlive(const SomeIPMsgHeader& header, uint64_t timestampNow) override final;

        const UInt32 m_protocolVersion;
        const ParticipantIdentifier m_participantId;
        const InstanceIdType m_serviceIID;
        const std::chrono::milliseconds m_keepAliveInterval;
        const std::chrono::milliseconds m_keepAliveTimeout;
        const std::function<std::chrono::steady_clock::time_point(void)> m_steadyClockNow;
        const LogContext& m_logContext;
        const String m_serviceTypeName;

        PlatformLock& m_frameworkLock;
        StatisticCollectionFramework& m_statisticCollection;

        ConnectionStatusUpdateNotifier m_connectionStatusUpdateNotifier;
        std::shared_ptr<ISomeIPStackCommon<InstanceIdType>> m_stack;
        bool m_connected = false;
        std::mt19937 m_randomGenerator{std::random_device{}()};

        std::vector<std::unique_ptr<ParticipantState>> m_participantStates;
        HashMap<Guid, ParticipantState*> m_knownParticipants;
        HashMap<Guid, ParticipantState*> m_connectedParticipants;
        HashMap<InstanceIdType, ParticipantState*> m_availableInstances;

        PlatformThread m_thread;
        std::condition_variable_any m_wakeupThread;
    };


    template <typename Callbacks>
    bool ConnectionSystemBase<Callbacks>::CheckConstructorArguments(const std::shared_ptr<ISomeIPStackCommon<InstanceIdType>>& stack, UInt32 communicationUserID, const ParticipantIdentifier& namedPid,
                                                                    UInt32 protocolVersion,
                                                                    std::chrono::milliseconds keepAliveInterval, std::chrono::milliseconds keepAliveTimeout,
                                                                    const LogContext& logContext, const String& serviceTypeName)
    {
        if (communicationUserID == 0)
        {
            LOG_ERROR(logContext, "ConnectionSystemBase(" << communicationUserID << ":" << serviceTypeName << ")::CheckConstructorArguments: Invalid communicationUserID "<< communicationUserID);
            return false;
        }
        if (!stack)
        {
            LOG_ERROR(logContext, "ConnectionSystemBase(" << communicationUserID << ":" << serviceTypeName << ")::CheckConstructorArguments: Invalid stack");
            return false;
        }
        if (!stack->getServiceInstanceId().isValid())
        {
            LOG_ERROR(logContext, "ConnectionSystemBase(" << communicationUserID << ":" << serviceTypeName << ")::CheckConstructorArguments: Invalid service IID");
            return false;
        }
        if (protocolVersion == 0)
        {
            LOG_ERROR(logContext, "ConnectionSystemBase(" << communicationUserID << ":" << serviceTypeName << ")::CheckConstructorArguments: Invalid protocol version " << protocolVersion);
            return false;
        }
        if (!namedPid.getParticipantId().isValid())
        {
            LOG_ERROR(logContext, "ConnectionSystemBase(" << communicationUserID << ":" << serviceTypeName << ")::CheckConstructorArguments: Invalid pid");
            return false;
        }

        if (keepAliveInterval == std::chrono::milliseconds{0} && keepAliveTimeout == std::chrono::milliseconds{0})
        {
            LOG_WARN(logContext, "ConnectionSystemBase(" << communicationUserID << ":" << serviceTypeName << ")::CheckConstructorArguments: Disabling keepalive. For testing only");
        }
        else if (keepAliveTimeout <= keepAliveInterval || keepAliveInterval == std::chrono::milliseconds{0})
        {
            LOG_ERROR(logContext, "ConnectionSystemBase(" << communicationUserID << ":" << serviceTypeName << ")::CheckConstructorArguments: Keepalive value mismatch. Timeout must be > Interval and Interval != 0");
            return false;
        }
        return true;
    }

    template <typename Callbacks>
    ConnectionSystemBase<Callbacks>::ConnectionSystemBase(std::shared_ptr<ISomeIPStackCommon<InstanceIdType>>        stack,
                                                          UInt32                                                     communicationUserID,
                                                          const ParticipantIdentifier&                               namedPid,
                                                          UInt32                                                     protocolVersion,
                                                          PlatformLock&                                              frameworkLock,
                                                          StatisticCollectionFramework&                              statisticCollection,
                                                          std::chrono::milliseconds                                  keepAliveInterval,
                                                          std::chrono::milliseconds                                  keepAliveTimeout,
                                                          std::function<std::chrono::steady_clock::time_point(void)> steadyClockNow,
                                                          const LogContext&                                          logContext,
                                                          const String&                                              serviceTypeName)
        : m_communicationUserID(communicationUserID)
        , m_protocolVersion(protocolVersion)
        , m_participantId(namedPid)
        , m_serviceIID(stack->getServiceInstanceId())
        , m_keepAliveInterval(keepAliveInterval)
        , m_keepAliveTimeout(keepAliveTimeout)
        , m_steadyClockNow(std::move(steadyClockNow))
        , m_logContext(logContext)
        , m_serviceTypeName(serviceTypeName)
        , m_frameworkLock(frameworkLock)
        , m_statisticCollection(statisticCollection)
        , m_connectionStatusUpdateNotifier(fmt::to_string(m_communicationUserID), serviceTypeName.stdRef(), frameworkLock)
        , m_stack(std::move(stack))
        , m_thread(String(fmt::format("R_CONN_{}", serviceTypeName)))
    {
        assert(CheckConstructorArguments(m_stack, m_communicationUserID, m_participantId, m_protocolVersion, m_keepAliveInterval, m_keepAliveTimeout, m_logContext, m_serviceTypeName));
    }

    template <typename Callbacks>
    ConnectionSystemBase<Callbacks>::~ConnectionSystemBase()
    {
        LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::~ConnectionSystemBase: connected " << m_connected);
        if (m_connected)
            disconnect();
    }

    template <typename Callbacks>
    bool ConnectionSystemBase<Callbacks>::connect()
    {
        LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::connect: keepAliveInterval " << m_keepAliveInterval.count()
                 << "ms, keepAliveTimeout " << m_keepAliveTimeout.count() << "ms");

        if (m_connected)
        {
            LOG_ERROR(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::connect: Already connected");
            return false;
        }

        if (!m_stack->connect())
            return false;

        if (m_keepAliveInterval != std::chrono::milliseconds{0})
        {
            assert(!Runnable::isCancelRequested());
            m_thread.start(*this);
        }

        m_connected = true;
        return true;
    }

    template <typename Callbacks>
    bool ConnectionSystemBase<Callbacks>::disconnect()
    {
        if (!m_connected)
        {
            LOG_ERROR(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::disconnect: Not connected");
            return false;
        }

        if (m_thread.joinable())
        {
            m_thread.cancel();
            {
                m_wakeupThread.notify_one();
                m_frameworkLock.unlock();
                m_thread.join();
                m_frameworkLock.lock();
            }
            resetCancel();
        }

        if (!m_stack->disconnect())
            return false;

        for (const auto& p : m_connectedParticipants)
            m_connectionStatusUpdateNotifier.triggerNotification(p.key, EConnectionStatus::EConnectionStatus_NotConnected);

        m_knownParticipants.clear();
        m_connectedParticipants.clear();
        m_availableInstances.clear();
        m_participantStates.clear();

        m_connected = false;
        return true;
    }

    template <typename Callbacks>
    IConnectionStatusUpdateNotifier& ConnectionSystemBase<Callbacks>::getConnectionStatusUpdateNotifier()
    {
        return m_connectionStatusUpdateNotifier;
    }

    template <typename Callbacks>
    auto ConnectionSystemBase<Callbacks>::availableInstances() -> const HashMap<InstanceIdType, ParticipantState*>&
    {
        return m_availableInstances;
    }

    template <typename Callbacks>
    bool ConnectionSystemBase<Callbacks>::isParticipantConnected(ParticipantState* pstate) const
    {
        assert(pstate);
        return m_connectedParticipants.contains(pstate->pid);
    }

    template <typename Callbacks>
    SomeIPMsgHeader ConnectionSystemBase<Callbacks>::generateHeaderForParticipant(ParticipantState& pstate)
    {
        return SomeIPMsgHeader{ m_participantId.getParticipantId().get(), pstate.sendSessionId, pstate.sendMessageId };
    }

    template <typename Callbacks>
    auto ConnectionSystemBase<Callbacks>::getParticipantStateForSending(const Guid& pid, const char* callerMethod) -> ParticipantState*
    {
        if (!checkConnected(callerMethod))
            return nullptr;

        auto it = m_connectedParticipants.find(pid);
        if (it == m_connectedParticipants.end())
        {
            if (m_knownParticipants.contains(pid))
                LOG_ERROR(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::" << callerMethod << ": participant " << pid << " is not connected");
            else
                LOG_ERROR(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::" << callerMethod << ": unknown participant " << pid);
            return nullptr;
        }

        return it->value;
    }

    template <typename Callbacks>
    bool ConnectionSystemBase<Callbacks>::handleSendResult(ParticipantState& pstate, const char* callerMethod, bool result)
    {
        if (result)
        {
            ++pstate.sendMessageId;
            pstate.lastSent = m_steadyClockNow();
            m_statisticCollection.statMessagesSent.incCounter(1);
        }
        else
        {
            if (m_connectedParticipants.contains(pstate.pid))
            {
                // Remove from connected
                LOG_WARN(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleSendResult(" << callerMethod << "): Disconnect pid " << pstate.pid << " because sending failed");

                m_connectionStatusUpdateNotifier.triggerNotification(pstate.pid, EConnectionStatus::EConnectionStatus_NotConnected);
                m_connectedParticipants.remove(pstate.pid);
            }
            else
                LOG_WARN(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleSendResult(" << callerMethod << "): Sending to pid " << pstate.pid << " failed");

            // must start new session because we do not resend message
            initNewSession(pstate);

            // no trySendParticipantInfo because sending just failed. recover by keepalive thread or incoming message (in next thread loop)
        }
        return result;
    }

    template <typename Callbacks>
    template <typename F>
    bool ConnectionSystemBase<Callbacks>::sendUnicast(const char* callerMethod, const Guid& to, F&& sendFunc)
    {
        if (ParticipantState* pstate = getParticipantStateForSending(to, callerMethod))
            return handleSendResult(*pstate, callerMethod, sendFunc(pstate->iid, generateHeaderForParticipant(*pstate)));
        return false;
    }

    template <typename Callbacks>
    template <typename F>
    bool ConnectionSystemBase<Callbacks>::sendBroadcast(const char* callerMethod, F&& sendFunc)
    {
        if (!checkConnected(callerMethod))
            return false;

        // prevent iter over connected because modified on send failure
        for (auto& p : availableInstances())
        {
            if (isParticipantConnected(p.value))
                if (ParticipantState* pstate = getParticipantStateForSending(p.value->pid, callerMethod))
                    handleSendResult(*pstate, callerMethod, sendFunc(pstate->iid, generateHeaderForParticipant(*pstate)));
        }
        return true;
    }

    template <typename Callbacks>
    bool ConnectionSystemBase<Callbacks>::checkConnected(const char* callerMethod) const
    {
        if (m_connected)
            return true;
        LOG_ERROR(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::" << callerMethod << " called without beeing connected");
        return false;
    }

    template <typename Callbacks>
    auto ConnectionSystemBase<Callbacks>::processMessageHeaderGeneric(const SomeIPMsgHeader& header, const char* callerMethod) -> ParticipantState*
    {
        m_statisticCollection.statMessagesReceived.incCounter(1);

        // find pstate
        const Guid pid(header.participantId);
        auto it = m_knownParticipants.find(pid);
        if (it == m_knownParticipants.end())
        {
            LOG_WARN(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::processMessageHeaderGeneric(" << callerMethod << "): got message from unknown pid " << pid);
            return nullptr;
        }
        assert(pid.isValid());

        // update keepalive also for not fully connected or invalid messages (no need to wake up thread, previous keepalive value will do)
        ParticipantState* pstate = it->value;
        assert(pstate);

        const auto now = m_steadyClockNow();
        LOG_TRACE(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::processMessageHeaderGeneric: Update lastRecv for pid " << pid << " from "
                 << asMilliseconds(pstate->lastRecv) << " -> " << asMilliseconds(now));
        pstate->lastRecv = now;

        // check header counters
        if (header.sessionId == 0)
        {
            LOG_FATAL(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::processMessageHeaderGeneric(" << callerMethod << "): received impossible sessionId 0 from " << pid);
            return nullptr;
        }
        if (header.messageId <= 1)
        {
            LOG_FATAL(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::processMessageHeaderGeneric(" << callerMethod << "): received impossible messageId "
                      << header.messageId << " on non-participantInfo from " << pid);
            return nullptr;
        }
        return pstate;
    }

    template <typename Callbacks>
    Guid* ConnectionSystemBase<Callbacks>::processReceivedMessageHeader(const SomeIPMsgHeader& header, const char* callerMethod)
    {
        ParticipantState* pstate = processMessageHeaderGeneric(header, callerMethod);
        if (!pstate)
            return nullptr;

        if (!m_connectedParticipants.contains(pstate->pid))
        {

            LOG_WARN(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::processReceivedMessageHeader(" << callerMethod <<
                     "): ignore from not connected pid " << pstate->pid << " " << header);
            return nullptr;
        }
        assert(pstate->expectedRecvSessionId != 0);  // expected == 0 can only happen when not connected (anymore)

        if (header.sessionId == pstate->expectedRecvSessionId &&
            header.messageId == pstate->expectedRecvMessageId)
        {
            // everything ok
            ++pstate->expectedRecvMessageId;
            return &pstate->pid;
        }

        LOG_WARN(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::processReceivedMessageHeader(" << callerMethod << "): counter mismatch from pid " << pstate->pid <<
                 ". Expected " << pstate->expectedRecvSessionId << ":" << pstate->expectedRecvMessageId << ", got " <<
                 header.sessionId << ":" << header.messageId);

        // handle counter failure on connected participant: disconnect + reinit
        m_connectedParticipants.remove(pstate->pid);
        m_connectionStatusUpdateNotifier.triggerNotification(pstate->pid, EConnectionStatus::EConnectionStatus_NotConnected);

        initNewSession(*pstate);
        trySendParticipantInfo(*pstate);

        return nullptr;
    }

    template <typename Callbacks>
    void ConnectionSystemBase<Callbacks>::initNewSession(ParticipantState& pstate)
    {
        // do not throw away remote sid and mid when never sent anything yet
        if (pstate.sendMessageId > 1)
        {
            // expect other side to start new session
            pstate.expectedRecvMessageId = 1;
            pstate.expectedRecvSessionId = 0;
        }

        // start new send session with random session id
        std::uniform_int_distribution<uint64_t> dis(1);
        pstate.sendMessageId = 1;
        pstate.sendSessionId = dis(m_randomGenerator);
    }

    template <typename Callbacks>
    auto ConnectionSystemBase<Callbacks>::addParticipantState(const Guid& pid, InstanceIdType iid) -> ParticipantState*
    {
        assert(iid != InstanceIdType());

        std::uniform_int_distribution<uint64_t> dis(1);
        const auto now = m_steadyClockNow();
        const ParticipantState pstate{pid, iid,
                                      dis(m_randomGenerator), 1, now,
                                      0, 1, now};

        m_participantStates.push_back(std::make_unique<ParticipantState>(pstate));
        return m_participantStates.back().get();
    }

    template <typename Callbacks>
    bool ConnectionSystemBase<Callbacks>::trySendParticipantInfo(ParticipantState& pstate)
    {
        assert(pstate.iid != InstanceIdType());
        assert(m_availableInstances.contains(pstate.iid));

        const SomeIPMsgHeader header = generateHeaderForParticipant(pstate);
        LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::sendParticipantInfo: to " << pstate.pid << ", protocolVersion " << m_protocolVersion
                 << ", senderInstanceId " << m_serviceIID << ", " << header);
        const bool result = handleSendResult(pstate, "trySendParticipantInfo",
                                             m_stack->sendParticipantInfo(pstate.iid, header, static_cast<uint16_t>(m_protocolVersion), m_serviceIID, pstate.pid.get(), 0, 0));  // TODO(tobias): clocktype + timestamp

        // record send timestamp to prevent flooding on error
        if (!result)
            pstate.lastSent = m_steadyClockNow();

        // pick up potentially new resend / keepalive timeout
        m_wakeupThread.notify_one();

        return result;
    }

    template <typename Callbacks>
    void ConnectionSystemBase<Callbacks>::handleServiceAvailable(InstanceIdType iid)
    {
        if (iid == InstanceIdType())
        {
            LOG_ERROR(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleServiceAvailable: for invalid iid");
            return;
        }
        if (iid == m_serviceIID)
        {
            LOG_ERROR(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleServiceAvailable: for own iid " << iid);
            return;
        }

        // check for duplicate callback
        if (m_availableInstances.contains(iid))
            return;

        // try find pstate for iid
        ParticipantState* pstate = nullptr;
        auto it = std::find_if(m_participantStates.begin(), m_participantStates.end(),
                               [&](auto& up) { return up->iid == iid; });
        if (it != m_participantStates.end())
        {
            pstate = it->get();
            LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleServiceAvailable: Use existing pstate for " << pstate->pid << ", iid " << iid);
        }
        else
        {
            LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleServiceAvailable: Create new pstate for iid " << iid);
            pstate = addParticipantState(Guid(), iid);
        }
        assert(pstate);
        assert(!pstate->pid.isValid() || !m_connectedParticipants.contains(pstate->pid));

        m_availableInstances.put(iid, pstate);

        // try send participantInfo to inform other side we are up
        if (trySendParticipantInfo(*pstate))
        {
            LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleServiceAvailable: Successfully sent initial participantInfo to " << iid << "/" << pstate->pid << ". Waiting for confirmation");

            // become connected when already received message
            if (pstate->expectedRecvSessionId != 0)
            {
                LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleServiceAvailable: Connected pid " << pstate->pid << ", iid " << iid);
                m_connectedParticipants.put(pstate->pid, pstate);
                m_connectionStatusUpdateNotifier.triggerNotification(pstate->pid, EConnectionStatus::EConnectionStatus_Connected);
            }
        }
    }

    template <typename Callbacks>
    void ConnectionSystemBase<Callbacks>::handleServiceUnavailable(InstanceIdType iid)
    {
        ParticipantState** maybePstate = m_availableInstances.get(iid);
        if (!maybePstate)
            return;

        ParticipantState* pstate = *maybePstate;
        m_availableInstances.remove(iid);
        assert(pstate);

        // init new state to inform other side when up again that we want need reinit
        initNewSession(*pstate);

        if (m_connectedParticipants.contains(pstate->pid))
        {
            LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleServiceUnavailable: Disconnect pid " << pstate->pid);

            m_connectionStatusUpdateNotifier.triggerNotification(pstate->pid, EConnectionStatus::EConnectionStatus_NotConnected);
            m_connectedParticipants.remove(pstate->pid);
        }
        else
        {
            LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleServiceUnavailable: For unconnected pid " << pstate->pid);
        }
    }

    template <typename Callbacks>
    void ConnectionSystemBase<Callbacks>::handleParticipantInfo(const SomeIPMsgHeader& header, uint16_t protocolVersion, InstanceIdType senderInstanceId, uint64_t /*expectedReceiverPid*/, uint8_t /*clockType*/, uint64_t /*timestampNow*/)
    {
        m_statisticCollection.statMessagesReceived.incCounter(1);

        Guid pid(header.participantId);
        if (protocolVersion != m_protocolVersion)
        {
            LOG_WARN(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: Ignore iid " << senderInstanceId << ", pid " << pid << " because protocol version mismatch. " <<
                     "Expected " << m_protocolVersion << ", got " << protocolVersion);
            return;
        }

        if (!pid.isValid())
        {
            LOG_ERROR(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: message from invalid pid with iid " << senderInstanceId);
            return;
        }
        if (pid == m_participantId.getParticipantId())
        {
            LOG_ERROR(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: message from own pid with iid " << senderInstanceId);
            return;
        }
        if (senderInstanceId == InstanceIdType())
        {
            LOG_ERROR(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: message from invalid iid with pid " << pid);
            return;
        }
        if (senderInstanceId == m_serviceIID)
        {
            LOG_ERROR(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: message from own iid with pid " << pid);
            return;
        }
        if (header.sessionId == 0)
        {
            LOG_ERROR(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: received impossible sessionId 0 from " << pid);
            return;
        }
        if (header.messageId == 0)
        {
            LOG_ERROR(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: received impossible messageId "
                      << header.messageId << " from " << pid);
            return;
        }

        // try to find pstate by different ways
        ParticipantState* pstate = nullptr;

        // find by pid (when already received participantInfo from this pid)
        if (m_knownParticipants.contains(pid))
        {
            pstate = m_knownParticipants[pid];
            assert(pstate);
            assert(pstate->iid != InstanceIdType());
            pstate->lastRecv = m_steadyClockNow();

            if (pstate->iid == senderInstanceId)
            {
                LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: got again with same iid " << senderInstanceId << " and pid " << pid);
            }
            else
            {
                const bool prevUp = m_availableInstances.contains(pstate->iid);
                const bool newUp = m_availableInstances.contains(senderInstanceId);

                LOG_FATAL(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: pid " << pid << " changed from iid "
                          << pstate->iid << " (up:" << prevUp << ") -> " << senderInstanceId << " (up:" << newUp << ") -> NOT SUPPORTED YET");
                return;

                // TODO(tobias):
                // on prev
                // if connected: disconnect
                // if available -> remove pid -> send participantinfo
                // remove from known

                // on new
                //    if was connected -> disconnect
                //    if was known -> remove
                // if has pstate: update pid to new
                // add new known
                // if nothing found -> add new pstate
            }
        }
        // find by instance (if service available but first participantInfo)
        else if (m_availableInstances.contains(senderInstanceId))
        {
            pstate = m_availableInstances[senderInstanceId];
            assert(pstate);

            if (pstate->pid.isValid())
            {
                LOG_WARN(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: iid " << senderInstanceId << " changed from pid "
                         << pstate->pid << " -> " << pid);

                // instance now has new pid (still unknown pid; known pids take first branch)
                assert(pid != pstate->pid);

                // make old pstate.pid disconnected and unknown
                if (m_connectedParticipants.contains(pstate->pid))
                {
                    LOG_WARN(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: disconnect " << pstate->pid << " because instance changed pid");
                    m_connectionStatusUpdateNotifier.triggerNotification(pstate->pid, EConnectionStatus::EConnectionStatus_NotConnected);
                    m_connectedParticipants.remove(pstate->pid);
                }
                m_knownParticipants.remove(pstate->pid);

                // no new session to let failure recovery kick in later in function
                pstate->pid = Guid();
            }

            assert(!pstate->pid.isValid());

            LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: iid " << senderInstanceId << " now linked to pid " << pid);

            m_knownParticipants.put(pid, pstate);
            pstate->pid = pid;
            pstate->lastRecv = m_steadyClockNow();
        }
        else
        {
            // find in participantStates (when service was once up but currently not)
            auto it = std::find_if(m_participantStates.begin(), m_participantStates.end(),
                                   [&](auto& up) { return up->iid == senderInstanceId; });
            if (it != m_participantStates.end())
            {
                LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: reuse pstate for unavailable iid " << senderInstanceId);
                pstate = it->get();
                pstate->pid = pid;
                pstate->lastRecv = m_steadyClockNow();
            }
            else
            {
                // completely new pid and iid (never up, never received anything)
                LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: add new pstate for pid " << pid << ", iid " << senderInstanceId);
                pstate = addParticipantState(pid, senderInstanceId);
            }
            m_knownParticipants.put(pid, pstate);
        }
        assert(pstate);

        // store skip state if needed and always reset to ensure regular (pinfo) messages reset it
        bool prevSkipSendPinfoOnNextMismatch = pstate->skipSendPinfoOnNextMismatch;
        pstate->skipSendPinfoOnNextMismatch = false;

        // check if we are open for new session and the message starts a new session
        if (pstate->expectedRecvSessionId == 0 &&
            pstate->expectedRecvMessageId == 1 &&
            pstate->expectedRecvMessageId == header.messageId)
        {
            // take over expected values
            pstate->expectedRecvSessionId = header.sessionId;
            ++pstate->expectedRecvMessageId;

            // remote service must also be available and we must have sent participantInfo already to get fully connected
            if (!m_availableInstances.contains(pstate->iid))
            {
                // accept but service must be up first to connect
                LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: Received initial participantInfo from "
                         << pid << " but sender iid " << pstate->iid << " not up");
            }
            else if (pstate->sendMessageId <= 1)
            {
                // accept but participantInfo send failed
                LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: Received initial participantInfo from "
                         << pid << " but could not successfully send participantInfo to sender iid " << pstate->iid);
            }
            else
            {
                LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: Connected pid " << pstate->pid << ", iid " << senderInstanceId);
                m_connectedParticipants.put(pstate->pid, pstate);
                m_connectionStatusUpdateNotifier.triggerNotification(pstate->pid, EConnectionStatus::EConnectionStatus_Connected);
            }
        }
        else if (pstate->expectedRecvSessionId == header.sessionId &&
                 pstate->expectedRecvMessageId == header.messageId)
        {
            LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: received valid from pid " << pid);
            ++pstate->expectedRecvMessageId;
        }
        else
        {
            LOG_WARN(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: counter mismatch from pid " << pid <<
                     ". Expected " << pstate->expectedRecvSessionId << ":" << pstate->expectedRecvMessageId << ", got " <<
                     header.sessionId << ":" << header.messageId);

            // mismatch: disconnect and new session
            if (m_connectedParticipants.contains(pid))
            {
                LOG_WARN(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: Disconnect pid " << pstate->pid);
                m_connectionStatusUpdateNotifier.triggerNotification(pstate->pid, EConnectionStatus::EConnectionStatus_NotConnected);
                m_connectedParticipants.remove(pstate->pid);
            }

            const uint64_t previousSendSessionId = pstate->sendSessionId;
            const uint64_t previousSendMessageId = pstate->sendMessageId;
            initNewSession(*pstate);

            // try to directly connect again
            if (!m_availableInstances.contains(senderInstanceId))
            {
                LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: New session for pid " << pstate->pid << " but sender iid " << senderInstanceId << " not up");
            }
            else
            {
                // may not skip when has sent anything else than pinfo to remote. otherwise state on remote might get mixed up
                if (prevSkipSendPinfoOnNextMismatch && previousSendMessageId == 2)
                {
                    // skip sending and keep last announced session because we assume last pinfo reached receiver and is treated as valid there
                    LOG_WARN(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: Skip sending pinfo to " << pstate->pid <<
                             ", iid " << senderInstanceId << ", keep sendSessionId " << previousSendSessionId << ", sendMessageId " << previousSendMessageId);
                    pstate->sendSessionId = previousSendSessionId;
                    pstate->sendMessageId = previousSendMessageId;
                }
                else
                {
                    if (prevSkipSendPinfoOnNextMismatch)
                        LOG_WARN(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: Will not skip send pinfo to " << pstate->pid <<
                                 ", iid " << senderInstanceId << ", because data already sent: sendSessionId " << previousSendSessionId << ", sendMessageId " << previousSendMessageId);

                    if (!trySendParticipantInfo(*pstate))
                    {
                        LOG_INFO(m_logContext,
                                 "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: New session for pid " << pstate->pid
                                                         << " but could not successfully send participantInfo to sender iid " << senderInstanceId);
                        return;
                    }

                    // successfully sent pinfo, skip next one
                    pstate->skipSendPinfoOnNextMismatch = true;
                }

                // must take over expected values on connect (regular messages can never start new session)
                pstate->expectedRecvSessionId = header.sessionId;
                ++pstate->expectedRecvMessageId;

                LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleParticipantInfo: Connected pid " << pstate->pid << ", iid " << senderInstanceId);
                m_connectedParticipants.put(pstate->pid, pstate);
                m_connectionStatusUpdateNotifier.triggerNotification(pstate->pid, EConnectionStatus::EConnectionStatus_Connected);
            }
        }
    }

    template <typename Callbacks>
    void ConnectionSystemBase<Callbacks>::run()
    {
#ifdef __ghs__
#   ifdef RAMSES_CONN_KEEPALIVE_THREAD_PRIORITY
        setThreadPriorityIntegrity(RAMSES_CONN_KEEPALIVE_THREAD_PRIORITY, "keepalive thread");
#   endif
#   ifdef RAMSES_CONN_KEEPALIVE_THREAD_CORE_BINDING
        setThreadCoreBindingIntegrity(RAMSES_CONN_KEEPALIVE_THREAD_CORE_BINDING, "keepalive thread");
#   endif
#endif

        LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::run: Keepalive thread started");
        assert(m_keepAliveInterval > std::chrono::milliseconds{0});

        std::unique_lock<std::recursive_mutex> l(m_frameworkLock);
        while (!isCancelRequested())
        {
            const auto next_wakeup = doOneThreadLoop(m_keepAliveInterval, m_keepAliveTimeout);
            m_wakeupThread.wait_until(l, next_wakeup);
        }
        LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::run: Keepalive thread canceled");
    }

    template <typename Callbacks>
    std::chrono::steady_clock::time_point ConnectionSystemBase<Callbacks>::doOneThreadLoop(std::chrono::milliseconds keepAliveInterval, std::chrono::milliseconds keepAliveTimeout)
    {
        assert(keepAliveInterval.count() > 0);
        assert(keepAliveInterval < keepAliveTimeout);

        const auto now = m_steadyClockNow();
        std::chrono::steady_clock::time_point nextWakeup = now + keepAliveInterval;  // maximum keepalive interval

        LOG_TRACE_F(m_logContext, ([&](ramses_internal::StringOutputStream& sos) {
            sos << "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::doOneThreadLoop: Loop at " << asMilliseconds(now) << " ";
            for (const auto& a : m_availableInstances)
                sos << a.key << ",";
        }));

        // iterate all available instances
        // (never modified in this loop and only available might be connected and only available are interesting for sending keepalive)
        for (auto& p : m_availableInstances)
        {
            ParticipantState& pstate = *p.value;
            assert(pstate.iid != InstanceIdType());

            // check all connected for lastRecv. disconnect if too old
            if (pstate.pid.isValid() &&
                m_connectedParticipants.contains(pstate.pid))
            {
                if (pstate.lastRecv + keepAliveTimeout <= now)
                {
                    // Remove from connected
                    LOG_WARN(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::doOneThreadLoop: Disconnect pid " << pstate.pid <<
                             " because lastRecv " << (std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(now - pstate.lastRecv).count()) <<
                             "ms ago, expected " << (std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(now - pstate.lastRecv - keepAliveInterval).count()) <<
                             "ms ago, latest " << (std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(now - pstate.lastRecv - keepAliveTimeout).count()) << "ms ago");

                    m_connectionStatusUpdateNotifier.triggerNotification(pstate.pid, EConnectionStatus::EConnectionStatus_NotConnected);
                    m_connectedParticipants.remove(pstate.pid);

                    initNewSession(pstate);
                }
                else
                {
                    // update wakup with next receive timeout
                    nextWakeup = std::min(nextWakeup, pstate.lastRecv + keepAliveTimeout);
                }
            }

            // check if we must send something
            if (pstate.lastSent + keepAliveInterval <= now)
            {
                if (pstate.sendMessageId == 1)
                {
                    // initial participantInfo already failed, try to send again
                    LOG_WARN(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::doOneThreadLoop: Try recover connection to " << pstate.iid << "/" << pstate.pid);

                    if (trySendParticipantInfo(pstate))
                    {
                        LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::doOneThreadLoop: Successfully sent initial participantInfo to " << pstate.iid << "/" << pstate.pid << ". Waiting for confirmation");

                        // become connected when already received message
                        if (pstate.expectedRecvSessionId != 0)
                        {
                            LOG_INFO(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::doOneThreadLoop: Connected pid " << pstate.pid << ", iid " << pstate.iid);
                            m_connectedParticipants.put(pstate.pid, &pstate);
                            m_connectionStatusUpdateNotifier.triggerNotification(pstate.pid, EConnectionStatus::EConnectionStatus_Connected);
                        }
                    }
                }
                else
                {
                    LOG_TRACE(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::doOneThreadLoop: Send keepalive to " << pstate.iid);
                    // already successfully sent something, send keepalive
                    handleSendResult(pstate, "keepAlive/thread",
                                     m_stack->sendKeepAlive(pstate.iid, generateHeaderForParticipant(pstate), 0));
                }
            }

            // update wakup with next send timeout
            nextWakeup = std::min(nextWakeup, pstate.lastSent + keepAliveInterval);
        }

        LOG_TRACE(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::doOneThreadLoop: next wakeup at " << asMilliseconds(nextWakeup) << " (dt " <<
                 (std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(nextWakeup - now).count()) << ")");

        return nextWakeup;
    }

    // test getters
    template <typename Callbacks>
    auto ConnectionSystemBase<Callbacks>::getParticipantState(InstanceIdType iid) const -> const ParticipantState*
    {
        const auto it = std::find_if(m_participantStates.cbegin(), m_participantStates.cend(), [&](auto& ps) { return ps->iid == iid; });
        if (it != m_participantStates.cend())
            return it->get();
        return nullptr;
    }

    // Logging functions

    template <typename Callbacks>
    void ConnectionSystemBase<Callbacks>::writeStateForLog(StringOutputStream& sos)
    {
        std::unique_lock<std::recursive_mutex> l(m_frameworkLock);
        sos << "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::logConnectionInfo:\n";
        sos << "  ProtocolVersion: " << m_protocolVersion << "\n";
        sos << "  ParticipantId: " << m_participantId.getParticipantId() << "/" << m_participantId.getParticipantName() << "\n";
        sos << "  Known participants:";
        if (m_connected)
        {
            for (const auto& p : m_knownParticipants)
            {
                const auto& pstate = *p.value;
                sos << "\n  - Pid " << pstate.pid << ", iid " << pstate.iid;
                if (m_availableInstances.contains(pstate.iid))
                    sos << "+";
                else
                    sos << "-";
                if (m_connectedParticipants.contains(pstate.pid))
                    sos << ", Connected";
                else
                    sos << ", Not connected";
            }
        }
        else
            sos << "  Not connected";
        sos << "\n";
        m_stack->logConnectionState(sos);
    }

    template <typename Callbacks>
    void ConnectionSystemBase<Callbacks>::logConnectionInfo()
    {
        LOG_INFO_F(m_logContext, ([&](StringOutputStream& sos) {
            writeStateForLog(sos);
        }));
    }

    template <typename Callbacks>
    void ConnectionSystemBase<Callbacks>::logPeriodicInfo()
    {
        std::unique_lock<std::recursive_mutex> l(m_frameworkLock);
        LOG_INFO_F(CONTEXT_PERIODIC, ([&](StringOutputStream& sos) {
            sos << m_serviceTypeName << " Participants: ";
            if (m_connected)
            {
                if (m_knownParticipants.size() == 0)
                    sos << "None";

                bool first = true;
                for (const auto& p : m_knownParticipants)
                {
                    if (first)
                        first = false;
                    else
                        sos << ", ";
                    const auto& pstate = *p.value;
                    sos << pstate.pid << "/" << pstate.iid << ":";
                    if (m_connectedParticipants.contains(pstate.pid))
                        sos << "conn";
                    else if (m_availableInstances.contains(pstate.iid))
                        sos << "avail";
                    else
                        sos << "unavail";
                }
            }
            else
                sos << "Not connected";
        }));
    }

    // handlers for generic ISomeIPCallbacks callbacks
    template <typename Callbacks>
    void ConnectionSystemBase<Callbacks>::handleKeepAlive(const SomeIPMsgHeader& header, uint64_t /*timestampNow*/)
    {
        ParticipantState* pstate = processMessageHeaderGeneric(header, "handleKeepAlive");
        if (!pstate)
            return;

        // check if sid/mid expected
        if (header.sessionId == pstate->expectedRecvSessionId &&
            header.messageId == pstate->expectedRecvMessageId)
        {
            // everything ok
            ++pstate->expectedRecvMessageId;
            return;
        }

        LOG_WARN(m_logContext, "ConnectionSystemBase(" << m_communicationUserID << ":" << m_serviceTypeName << ")::handleKeepAlive: counter mismatch from pid " << pstate->pid <<
                 ". Expected " << pstate->expectedRecvSessionId << ":" << pstate->expectedRecvMessageId << ", got " <<
                 header.sessionId << ":" << header.messageId);

        // handle counter failure depending on connection/availability state

        if (m_connectedParticipants.contains(pstate->pid))
        {
            m_connectionStatusUpdateNotifier.triggerNotification(pstate->pid, EConnectionStatus::EConnectionStatus_NotConnected);
            m_connectedParticipants.remove(pstate->pid);
        }

        initNewSession(*pstate);

        if (m_availableInstances.contains(pstate->iid))
            trySendParticipantInfo(*pstate);
    }
}

#endif
