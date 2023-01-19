//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "Collections/Guid.h"
#include "TransportCommon/ConnectionStatusUpdateNotifier.h"
#include "TransportCommon/SomeIPStackCommon.h"
#include "Utils/LogContext.h"
#include "Utils/LogMacros.h"
#include "absl/types/optional.h"
#include <chrono>
#include <unordered_map>
#include <random>

namespace ramses_internal
{
    /*
    Extension options
    - make initiator timeout configurable
    - detect too many init session on initiator and increase timeout

    Known issues/unsupported features
    - participant with same iid changes pid during LC
    - multiple participants with different iids have same pid
    */

    namespace internal
    {
        // non template base class with enum classes to allow fmt formatter
        class ConnectionSystemInitiatorResponderNonTemplateBase
        {
        public:
            enum class InitiatorState {
                Invalid,
                Unavailable,
                WaitForSessionReply,
                Connected,
            };

            enum class ResponderState {
                Invalid,
                Unavailable,
                WaitForUp,
                WaitForSession,
                Connected,
            };
        };

        static constexpr const char* const InitiatorStateNames[] = {
            "Invalid",
            "Unavailable",
            "WaitForSessionReply",
            "Connected",
        };

        static constexpr const char* const ResponderStateNames[] = {
            "Invalid",
            "Unavailable",
            "WaitForUp",
            "WaitForSession",
            "Connected",
        };
    }

    template <typename InstanceIdType>
    class ConnectionSystemInitiatorResponder : private internal::ConnectionSystemInitiatorResponderNonTemplateBase
    {
    public:
        ConnectionSystemInitiatorResponder(std::shared_ptr<ISomeIPStackCommon<InstanceIdType>> stack,
                                           uint32_t communicationUserID, Guid participantId,
                                           uint16_t protocolVersion,
                                           std::function<std::chrono::steady_clock::time_point(void)> steadyClockNow,
                                           const LogContext& logContext, const std::string& serviceTypeName,
                                           ConnectionStatusUpdateNotifier& connectionStatusUpdateNotifier,
                                           std::function<void()> wakeupKeepAliveThread);

        void connect();
        void disconnect();

        uint32_t getSupportedMinorProtocolVersion() const;
        bool isResponsibleForMinorProtocolVersion(uint32_t version) const;

        // lifecycle calls from stack: true result -> this class handled call
        bool handleServiceAvailable(InstanceIdType iid);
        bool handleServiceUnavailable(InstanceIdType iid);
        bool handleParticipantInfo(const SomeIPMsgHeader& header,
                                   uint16_t protocolVersion, uint32_t minorProtocolVersion,
                                   InstanceIdType senderInstanceId,
                                   uint64_t lastSentSessionId, uint64_t lastSentMessageId,
                                   std::chrono::steady_clock::time_point lastSentTime);
        bool handleKeepAlive(const SomeIPMsgHeader& header, bool usingPreviousMessageId);
        bool isInitiatorAndInvalid(InstanceIdType iid) const;

        // for sending
        bool isResponsibleForParticipant(const Guid& pid) const;

        template <typename F>
        bool sendUnicast(const char* callerMethod, const Guid& to, F&& sendFunc);
        template <typename F>
        bool sendBroadcast(const char* callerMethod, F&& sendFunc);

        // for receiving
        absl::optional<Guid*> processReceivedMessageHeader(const SomeIPMsgHeader& header, const char* callerMethod);

        bool isResponsibleForInstance(InstanceIdType iid) const;
        // keepalive thread, returns next wakeup
        std::chrono::steady_clock::time_point doOneThreadLoop(std::chrono::steady_clock::time_point now,
                                                              std::chrono::milliseconds keepAliveInterval,
                                                              std::chrono::milliseconds keepAliveTimeout);

        // logging
        void writeConnectionInfo(StringOutputStream& sos) const;
        void writePeriodicInfo(StringOutputStream& sos) const;

        // exposed for testing only
        using internal::ConnectionSystemInitiatorResponderNonTemplateBase::InitiatorState;
        using internal::ConnectionSystemInitiatorResponderNonTemplateBase::ResponderState;

        struct ParticipantState
        {
            Guid remotePid;
            InstanceIdType remoteIid;

            bool selfIsInitiator;

            std::chrono::steady_clock::time_point lastSentTime;
            std::chrono::steady_clock::time_point lastReceiveTime;

            uint64_t activeSessionId;
            uint64_t lastSentMessageId;
            uint64_t lastReceivedMessageId;

            InitiatorState initiatorState;
            ResponderState responderState;
        };

        const ParticipantState* getParticipantState(InstanceIdType iid) const;
        bool isInstanceAvailable(InstanceIdType iid) const;

    private:
        static constexpr const uint32_t MinorProtocolVersion = 1;

        bool checkConnected(const char* callerMethod) const;
        bool isParticipantConnected(const ParticipantState& pstate) const;
        bool isParticipantServiceAvailable(const ParticipantState& pstate) const;
        ParticipantState* addParticipantState(const Guid& pid, InstanceIdType iid);
        ParticipantState* findParticipantStateForIid(InstanceIdType iid);
        ParticipantState* getParticipantStateForSending(const Guid& pid, const char* callerMethod);
        bool isSelfInitiator(InstanceIdType remoteIid) const;
        bool handleSendResult(ParticipantState& pstate, const char* callerMethod, bool result);
        SomeIPMsgHeader generateHeaderForParticipant(ParticipantState& pstate);
        void disconnectParticipant(ParticipantState& pstate);
        void clearParticipantStateForReuse(ParticipantState& pstate, const char* reason);

        void initiatorInitSession(ParticipantState& pstate, const char* callerMethod, const char* reason);

        void responderSendError(ParticipantState& pstate, const char* callerMethod, const char* reason);
        void responderSendErrorForInvalidSid(ParticipantState& pstate, const char* callerMethod, uint64_t sessionId);
        void responderSendSessionReply(ParticipantState& pstate);

        std::shared_ptr<ISomeIPStackCommon<InstanceIdType>> m_stack;
        const InstanceIdType m_serviceIid;
        const uint32_t m_communicationUserID;
        const Guid m_participantId;
        const uint16_t m_protocolVersion;
        const std::function<std::chrono::steady_clock::time_point(void)> m_steadyClockNow;
        const LogContext& m_logContext;
        const std::string m_serviceTypeName;
        ConnectionStatusUpdateNotifier& m_connectionStatusUpdateNotifier;
        std::function<void()> m_wakeupKeepAliveThread;

        std::string m_logPrefix;
        bool m_connected = false;
        std::mt19937 m_randomGenerator{std::random_device{}()};

        std::vector<std::unique_ptr<ParticipantState>> m_participantStates;
        std::unordered_map<Guid, ParticipantState*> m_knownParticipants;
        std::unordered_map<Guid, ParticipantState*> m_connectedParticipants;
        std::unordered_map<InstanceIdType, ParticipantState*> m_availableInstances;
    };

    template <typename InstanceIdType>
    constexpr const uint32_t ConnectionSystemInitiatorResponder<InstanceIdType>::MinorProtocolVersion;

    template <typename InstanceIdType>
    ConnectionSystemInitiatorResponder<InstanceIdType>::ConnectionSystemInitiatorResponder(
            std::shared_ptr<ISomeIPStackCommon<InstanceIdType>>        stack,
            uint32_t                                                   communicationUserID,
            Guid                                                       participantId,
            uint16_t                                                   protocolVersion,
            std::function<std::chrono::steady_clock::time_point(void)> steadyClockNow,
            const LogContext&                                          logContext,
            const std::string&                                         serviceTypeName,
            ConnectionStatusUpdateNotifier&                            connectionStatusUpdateNotifier,
            std::function<void()>                                      wakeupKeepAliveThread)
        : m_stack(std::move(stack))
        , m_serviceIid(m_stack->getServiceInstanceId())
        , m_communicationUserID(communicationUserID)
        , m_participantId(participantId)
        , m_protocolVersion(protocolVersion)
        , m_steadyClockNow(std::move(steadyClockNow))
        , m_logContext(logContext)
        , m_serviceTypeName(serviceTypeName)
        , m_connectionStatusUpdateNotifier(connectionStatusUpdateNotifier)
        , m_wakeupKeepAliveThread(std::move(wakeupKeepAliveThread))
        , m_logPrefix(fmt::format("ConnectionSystemIR({}:{})::", m_communicationUserID, m_serviceTypeName))
    {
    }


    template <typename InstanceIdType>
    void ConnectionSystemInitiatorResponder<InstanceIdType>::connect()
    {
        LOG_INFO_P(m_logContext, "{}connect: MinorProtocolVersion {}",
                   m_logPrefix, MinorProtocolVersion);
        m_connected = true;
    }

    template <typename InstanceIdType>
    void ConnectionSystemInitiatorResponder<InstanceIdType>::disconnect()
    {
        LOG_INFO_P(m_logContext, "{}disconnect", m_logPrefix);
        if (!m_connected)
            return;

        for (const auto& p : m_connectedParticipants)
            m_connectionStatusUpdateNotifier.triggerNotification(p.first, EConnectionStatus::EConnectionStatus_NotConnected);

        m_knownParticipants.clear();
        m_availableInstances.clear();
        m_connectedParticipants.clear();
        m_participantStates.clear();

        m_connected = false;
    }

    template <typename InstanceIdType>
    uint32_t ConnectionSystemInitiatorResponder<InstanceIdType>::getSupportedMinorProtocolVersion() const
    {
        return MinorProtocolVersion;
    }

    template <typename InstanceIdType>
    bool ConnectionSystemInitiatorResponder<InstanceIdType>::isResponsibleForMinorProtocolVersion(uint32_t version) const
    {
        return version >= MinorProtocolVersion;
    }


    template <typename InstanceIdType>
    bool ConnectionSystemInitiatorResponder<InstanceIdType>::isSelfInitiator(InstanceIdType remoteIid) const
    {
        assert(remoteIid != InstanceIdType());
        assert(m_serviceIid != remoteIid);

        // smaller iid always is responder
        return remoteIid.getValue() < m_serviceIid.getValue();
    }

    template <typename InstanceIdType>
    auto ConnectionSystemInitiatorResponder<InstanceIdType>::addParticipantState(const Guid& pid, InstanceIdType iid) -> ParticipantState*
    {
        m_participantStates.push_back(std::make_unique<ParticipantState>(ParticipantState{
            pid, iid,
            isSelfInitiator(iid),
            {}, {},
            0, 0, 0,
            InitiatorState::Invalid,
            ResponderState::Invalid
        }));
        return m_participantStates.back().get();
    }

    template <typename InstanceIdType>
    auto ConnectionSystemInitiatorResponder<InstanceIdType>::findParticipantStateForIid(InstanceIdType iid) -> ParticipantState*
    {
        auto it = std::find_if(m_participantStates.begin(), m_participantStates.end(),
                               [&](const auto& pstate) { return pstate->remoteIid == iid; });
        return it == m_participantStates.end() ? nullptr : it->get();
    }

    template <typename InstanceIdType>
    bool ConnectionSystemInitiatorResponder<InstanceIdType>::handleServiceAvailable(InstanceIdType iid)
    {
        // early out if duplicate
        if (isInstanceAvailable(iid))
            return isResponsibleForInstance(iid);

        // find or create pstate
        ParticipantState* pstate = findParticipantStateForIid(iid);
        if (pstate)
        {
            LOG_INFO(m_logContext, m_logPrefix << "handleServiceAvailable: Use existing pstate for " << pstate->remotePid << ", iid " << iid);
        }
        else
        {
            LOG_INFO(m_logContext, m_logPrefix << "handleServiceAvailable: Create new pstate for iid " << iid);
            pstate = addParticipantState(Guid(), iid);
        }
        assert(pstate);
        assert(!pstate->remotePid.isValid() || !isParticipantConnected(*pstate));

        m_availableInstances[iid] = pstate;

        if (pstate->selfIsInitiator)
        {
            // not enough information yet if we are responsible
            if (pstate->initiatorState == InitiatorState::Invalid)
            {
                LOG_INFO_P(m_logContext, "{}handleServiceAvailable(I): Stay in {} for iid {} because no pinfo yet",
                           m_logPrefix, pstate->initiatorState, iid);
                return false;
            }

            // always start new session when service comes up
            initiatorInitSession(*pstate, "handleServiceAvailable", "up");
        }
        else
        {
            // not enough information yet if we are responsible
            if (pstate->responderState == ResponderState::Invalid)
            {
                LOG_INFO_P(m_logContext, "{}handleServiceAvailable(R): Stay in {} for iid {} because no pinfo yet",
                           m_logPrefix, pstate->responderState, iid);
                return false;
            }

            if (pstate->responderState == ResponderState::Unavailable)
            {
                LOG_INFO_P(m_logContext, "{}handleServiceAvailable(R): Change {} -> {} for {}",
                           m_logPrefix, pstate->responderState, ResponderState::WaitForSession, iid);
                pstate->responderState = ResponderState::WaitForSession;
            }

            else if (pstate->responderState == ResponderState::WaitForUp)
                responderSendSessionReply(*pstate);
            else
                assert(false);  // all other states imply was already up
        }

        return true;
    }

    template <typename InstanceIdType>
    bool ConnectionSystemInitiatorResponder<InstanceIdType>::handleServiceUnavailable(InstanceIdType iid)
    {
        // ignore if was never up (+ always let old code handle it too, extra down never hurts)
        auto it = m_availableInstances.find(iid);
        if (it == m_availableInstances.end())
            return false;

        ParticipantState* pstate = it->second;
        assert(pstate);

        m_availableInstances.erase(iid);

        if (!pstate->remotePid.isValid())
        {
            // Not responsible for iid
            assert(pstate->initiatorState == InitiatorState::Invalid && pstate->responderState == ResponderState::Invalid);
            return false;
        }

        // always disconnect when was connected
        if (isParticipantConnected(*pstate))
        {
            LOG_INFO(m_logContext, m_logPrefix << "handleServiceUnavailable: Disconnect pid " << pstate->remotePid);
            disconnectParticipant(*pstate);
        }
        else
        {
            LOG_INFO(m_logContext, m_logPrefix << "handleServiceUnavailable: For unconnected pid " << pstate->remotePid);
        }

        // update state
        if (pstate->selfIsInitiator)
            pstate->initiatorState = InitiatorState::Unavailable;
        else
            pstate->responderState = ResponderState::Unavailable;

        return true;
    }

    template <typename InstanceIdType>
    bool ConnectionSystemInitiatorResponder<InstanceIdType>::handleParticipantInfo(const SomeIPMsgHeader& header,
                                                                                   uint16_t protocolVersion, uint32_t minorProtocolVersion,
                                                                                   InstanceIdType senderInstanceId,
                                                                                   uint64_t lastSentSessionId, uint64_t lastSentMessageId,
                                                                                   std::chrono::steady_clock::time_point lastSentTime)
    {
        assert(protocolVersion == m_protocolVersion);

        //  check if responsible
        if (minorProtocolVersion < MinorProtocolVersion)
        {
            // check if we ever knew this pid (received and stored a pinfo with different minor)
            const auto existingIt = m_knownParticipants.find(Guid(header.participantId));
            if (existingIt != m_knownParticipants.end())
            {
                // remove from here and let old handle it from now on
                ParticipantState& existingPstate = *existingIt->second;
                LOG_WARN_P(m_logContext,
                           "{}handleParticipantInfo: Protocol version change for participant with iid {}, pid {}. already known with iid {}, pid {}. Remove here and hand over.",
                           m_logPrefix, senderInstanceId, header.participantId, existingPstate.remoteIid, existingPstate.remotePid);
                clearParticipantStateForReuse(existingPstate, "minor protocol version downgrade");
            }

            return false;
        }

        // detect when same pid is used by multiple instances
        {
            const auto existingIt = m_knownParticipants.find(Guid(header.participantId));
            if (existingIt != m_knownParticipants.end() && existingIt->second->remoteIid != senderInstanceId)
            {
                LOG_ERROR_P(m_logContext,
                            "{}handleParticipantInfo: Pid used by multiple instances. New with iid {}, pid {}. already known with iid {}, pid {}. Drop message and expect issues",
                            m_logPrefix, senderInstanceId, header.participantId, existingIt->second->remoteIid, existingIt->second->remotePid);

                // TODO: how should this case be handled? just ignore this conflicting message or throw previous one out? Drop conflicting message for now
                // better: remove it completely and start over with new information to not confuse logic
                return true;
            }
        }

        // header.messageId is not used for pinfo messages. the field is in the generic header but by definition a pinfo
        // is the first message with mid 1. this simplifies a lot of code checking for impossible conditions. in case it still does happen
        // log that case here, and rely on later error handling to detect when other messages are sent with wrong mid.
        if (header.messageId != 1)
        {
            LOG_INFO_P(m_logContext,  "{}handleParticipantInfo: Received header with invalid mid {}. Will be treated as mid 1 anyway, may cause followup errors.",
                       m_logPrefix, header);
        }

        if (isSelfInitiator(senderInstanceId))
        {
            ParticipantState* pstate = findParticipantStateForIid(senderInstanceId);
            const Guid senderPid(header.participantId);
            if (pstate)
            {
                LOG_INFO_P(m_logContext,
                           "{}handleParticipantInfo(I): Use existing for {}, protocolVersion {}/{}, iid {}, oldPid {}, oldSid {}, state {}, lastSentSessionId {}, lastSentMessageId {}",
                           m_logPrefix, header, protocolVersion, minorProtocolVersion, senderInstanceId,
                           pstate->remotePid, pstate->activeSessionId, pstate->initiatorState,
                           lastSentSessionId, lastSentMessageId);

                // detect pid change
                Guid currentPid(header.participantId);
                if (pstate->remotePid.isValid() &&
                    currentPid != pstate->remotePid)
                {
                    LOG_ERROR_P(m_logContext, "{}handleParticipantInfo(I): Pid for iid {} changed from {} to {}. Not supported, expect issues",
                                m_logPrefix, senderInstanceId, pstate->remotePid, currentPid);

                    // TODO currently broken when same instance has a new pid
                    // to handle properly must remove from known, must disconnect, start over fresh. exact handling depends on current initiator state
                }
            }
            else
            {
                LOG_INFO_P(m_logContext,  "{}handleParticipantInfo(I): New for {}, protocolVersion {}/{}, iid {}, lastSentSessionId {}, lastSentMessageId {}",
                           m_logPrefix, header, protocolVersion, minorProtocolVersion, senderInstanceId,
                           lastSentSessionId, lastSentMessageId);
                pstate = addParticipantState(senderPid, senderInstanceId);
            }

            assert(pstate);
            assert(pstate->responderState == ResponderState::Invalid);
            assert(pstate->remoteIid.isValid());

            const bool isServiceUp = isParticipantServiceAvailable(*pstate);

            if (pstate->initiatorState == InitiatorState::Invalid && !isServiceUp)
            {
                // initial info, store and mark responsible
                pstate->remotePid = senderPid;
                m_knownParticipants[pstate->remotePid] = pstate;

                LOG_DEBUG_P(m_logContext,  "{}handleParticipantInfo(I): Pid {} state change {} -> {}",
                            m_logPrefix, pstate->remotePid, pstate->initiatorState, InitiatorState::Unavailable);
                pstate->initiatorState = InitiatorState::Unavailable;
            }

            else if (pstate->initiatorState == InitiatorState::Invalid) // && isServiceUp
            {
                pstate->remotePid = senderPid;
                m_knownParticipants[pstate->remotePid] = pstate;

                if (header.sessionId == lastSentSessionId)
                {
                    // replying to "our" session -> can directly connect
                    pstate->activeSessionId = lastSentSessionId;
                    pstate->lastSentMessageId = lastSentMessageId;
                    pstate->lastReceivedMessageId = 1;

                    pstate->lastSentTime = lastSentTime;
                    pstate->lastReceiveTime = m_steadyClockNow();

                    pstate->initiatorState = InitiatorState::Connected;

                    LOG_INFO_P(m_logContext, "{}handleParticipantInfo(I): Connected pid {} from state {}", m_logPrefix, pstate->remotePid, InitiatorState::Invalid);
                    m_connectedParticipants[pstate->remotePid] = pstate;
                    m_connectionStatusUpdateNotifier.triggerNotification(pstate->remotePid, EConnectionStatus::EConnectionStatus_Connected);
                }
                else
                {
                    // wrong sid -> immediately initialize new session (because normal startup case)
                    initiatorInitSession(*pstate, "handleParticipantInfo(I)", "new pinfo");
                }
            }

            else if (pstate->initiatorState == InitiatorState::Unavailable)
            {
                assert(pstate->remotePid.isValid());
                assert(!isServiceUp);
                assert(!isParticipantConnected(*pstate));
                assert(m_knownParticipants.find(pstate->remotePid) != m_knownParticipants.end());

                LOG_INFO_P(m_logContext,  "{}handleParticipantInfo(I): Pid {} stay in state {}",
                            m_logPrefix, pstate->remotePid, pstate->initiatorState);
            }

            else if (pstate->initiatorState == InitiatorState::WaitForSessionReply)
            {
                assert(pstate->remotePid.isValid());
                assert(isServiceUp);
                assert(!isParticipantConnected(*pstate));
                assert(m_knownParticipants.find(pstate->remotePid) != m_knownParticipants.end());

                if (header.sessionId == pstate->activeSessionId)
                {
                    // get connected
                    pstate->lastReceivedMessageId = 1;
                    pstate->lastReceiveTime = m_steadyClockNow();

                    pstate->initiatorState = InitiatorState::Connected;

                    LOG_INFO_P(m_logContext, "{}handleParticipantInfo(I): Connected pid {} from state {}", m_logPrefix, pstate->remotePid, InitiatorState::WaitForSessionReply);
                    m_connectedParticipants[pstate->remotePid] = pstate;
                    m_connectionStatusUpdateNotifier.triggerNotification(pstate->remotePid, EConnectionStatus::EConnectionStatus_Connected);
                }
                else
                {
                    LOG_WARN_P(m_logContext,  "{}handleParticipantInfo(I): Ignore unexpected session {} from {}, active session {} in state {}",
                               m_logPrefix, header.sessionId, pstate->remotePid, pstate->activeSessionId, pstate->initiatorState);
                }
            }

            else if (pstate->initiatorState == InitiatorState::Connected)
            {
                assert(pstate->remotePid.isValid());
                assert(isServiceUp);
                assert(isParticipantConnected(*pstate));
                assert(m_knownParticipants.find(pstate->remotePid) != m_knownParticipants.end());

                if (header.sessionId == pstate->activeSessionId)
                {
                    // getting here is always a protocol violation even if mid and pid is good -> init new session
                    LOG_WARN_P(m_logContext,  "{}handleParticipantInfo(I): Disconnect pid {} because protocol violation",
                               m_logPrefix, pstate->remotePid);
                    disconnectParticipant(*pstate);
                    initiatorInitSession(*pstate, "handleParticipantInfo(I)", "protocol violation");
                }
                else
                {
                    LOG_WARN_P(m_logContext,  "{}handleParticipantInfo(I): Ignore unexpected session {} from {}, active session {} in state {}",
                               m_logPrefix, header.sessionId, pstate->remotePid, pstate->activeSessionId, pstate->initiatorState);
                }
            }

            else
                assert(false);
        }
        else  // Responder
        {
            ParticipantState* pstate = findParticipantStateForIid(senderInstanceId);
            if (pstate)
            {
                LOG_INFO_P(m_logContext,  "{}handleParticipantInfo(R): Use existing for {}, protocolVersion {}/{}, iid {}, oldPid {}, oldSid {}, state {}",
                           m_logPrefix, header, protocolVersion, minorProtocolVersion, senderInstanceId,
                           pstate->remotePid, pstate->activeSessionId, pstate->responderState);

                // detect pid change
                Guid currentPid(header.participantId);
                if (pstate->remotePid.isValid() &&
                    currentPid != pstate->remotePid)
                {
                    LOG_ERROR_P(m_logContext, "{}handleParticipantInfo(R): Pid for iid {} changed from {} to {}. Not supported, expect issues",
                                m_logPrefix, senderInstanceId, pstate->remotePid, currentPid);

                    // TODO currently broken when same instance has a new pid
                    // to handle properly must remove from known, must disconnect, start over fresh. exact handling depends on current responder state
                }
                pstate->remotePid = currentPid;
            }
            else
            {
                LOG_INFO_P(m_logContext,  "{}handleParticipantInfo(R): New for {}, protocolVersion {}/{}, iid {}",
                           m_logPrefix, header, protocolVersion, minorProtocolVersion, senderInstanceId);
                pstate = addParticipantState(Guid(header.participantId), senderInstanceId);
            }

            assert(pstate);
            assert(pstate->remotePid.isValid());
            assert(pstate->remoteIid.isValid());

            m_knownParticipants[pstate->remotePid] = pstate;
            const bool isServiceUp = isParticipantServiceAvailable(*pstate);

            auto storeValidDataInPState = [&]{
                pstate->lastReceiveTime = m_steadyClockNow();
                pstate->activeSessionId = header.sessionId;
                pstate->lastReceivedMessageId = 1;
                pstate->lastSentMessageId = 0; // new session
            };

            if (pstate->responderState == ResponderState::Invalid &&
                isServiceUp)
            {
                // first pinfo and up -> directly confirm
                storeValidDataInPState();
                responderSendSessionReply(*pstate);
            }

            else if (pstate->responderState == ResponderState::Invalid ||  //  and not up
                     pstate->responderState == ResponderState::Unavailable ||
                     pstate->responderState == ResponderState::WaitForUp)
            {
                assert(!isParticipantConnected(*pstate));
                assert(!isServiceUp);

                // cannot send yet but having active session now
                storeValidDataInPState();
                LOG_DEBUG_P(m_logContext,  "{}handleParticipantInfo(R): Pid {} state change {} -> {}",
                            m_logPrefix, pstate->remotePid, pstate->responderState, ResponderState::WaitForUp);
                pstate->responderState = ResponderState::WaitForUp;
            }

            else if (pstate->responderState == ResponderState::WaitForSession)
            {
                assert(!isParticipantConnected(*pstate));
                assert(isServiceUp);

                storeValidDataInPState();
                responderSendSessionReply(*pstate);
            }

            else if (pstate->responderState == ResponderState::Connected)
            {
                assert(isParticipantConnected(*pstate));
                assert(isServiceUp);

                // disconnect and reply
                storeValidDataInPState();
                LOG_WARN_P(m_logContext, "{}handleParticipantInfo(R): Disconnect pid {} because received new session {}",
                           m_logPrefix, pstate->remotePid, pstate->activeSessionId);
                disconnectParticipant(*pstate);
                responderSendSessionReply(*pstate);
            }

            else
                assert(false);
        }

        m_wakeupKeepAliveThread();

        return true;
    }

    template <typename InstanceIdType>
    bool ConnectionSystemInitiatorResponder<InstanceIdType>::isInitiatorAndInvalid(InstanceIdType iid) const
    {
        auto it = m_availableInstances.find(iid);
        if (it != m_availableInstances.end())
        {
            ParticipantState* pstate = it->second;
            assert(pstate);
            return (pstate->selfIsInitiator && pstate->initiatorState == InitiatorState::Invalid);
        }
        return false;
    }

    template <typename InstanceIdType>
    bool ConnectionSystemInitiatorResponder<InstanceIdType>::handleKeepAlive(const SomeIPMsgHeader& header, bool usingPreviousMessageId)
    {
        auto pstateIt = m_knownParticipants.find(Guid(header.participantId));
        if (pstateIt == m_knownParticipants.end())
            return false;

        ParticipantState* pstate = pstateIt->second;
        assert(pstate);

        const uint64_t expectedMessageId = usingPreviousMessageId ?
            pstate->lastReceivedMessageId :
            pstate->lastReceivedMessageId+1;

        if (pstate->selfIsInitiator)
        {
            if (pstate->initiatorState == InitiatorState::Invalid ||  // this case cannot really happen because of known check
                pstate->initiatorState == InitiatorState::Unavailable)
            {
                // ignore
            }

            else if (pstate->initiatorState == InitiatorState::WaitForSessionReply) // in case pinfo got lost
            {
                assert(isParticipantServiceAvailable(*pstate));

                // ignore invalid sid
                if (header.sessionId == pstate->activeSessionId)
                {
                    // always an error in this state, only pinfo is valid message
                    LOG_WARN_P(m_logContext, "{}handleKeepAlive(I): Unexpected in state {} with {}. Last received mid {}, expected {}",
                               m_logPrefix, pstate->initiatorState, header, pstate->lastReceivedMessageId, expectedMessageId);
                    initiatorInitSession(*pstate, "handleKeepAlive", "keepAlive in WaitForSessionReply");
                }
            }

            else if (pstate->initiatorState == InitiatorState::Connected)
            {
                assert(isParticipantServiceAvailable(*pstate));
                assert(isParticipantConnected(*pstate));
                assert(pstate->lastReceivedMessageId > 0);

                // ignore invalid sid
                if (header.sessionId == pstate->activeSessionId)
                {
                    if (header.messageId == expectedMessageId)
                    {
                        pstate->lastReceivedMessageId = header.messageId;
                        pstate->lastReceiveTime = m_steadyClockNow();
                    }
                    else
                    {
                        if (header.messageId == 0)
                            LOG_WARN_P(m_logContext, "{}handleKeepAlive(I): Received error from {} in state {}",
                                       m_logPrefix, header, pstate->initiatorState);
                        else
                            LOG_WARN_P(m_logContext, "{}handleKeepAlive(I): Wrong mid from {} in state {}. Last received mid {}, expected {}",
                                       m_logPrefix, header, pstate->initiatorState, pstate->lastReceivedMessageId, expectedMessageId);
                        initiatorInitSession(*pstate, "handleKeepAlive", header.messageId == 0 ? "received error" : "keepalive mid mismatch");
                    }
                }
            }

            else
                assert(false);
        }
        else
        {
            if (pstate->responderState == ResponderState::Invalid ||  // this case cannot really happen because of known check
                pstate->responderState == ResponderState::Unavailable)
            {
                // ignore
            }

            else if (pstate->responderState == ResponderState::WaitForSession)
            {
                // every message/session is wrong here, also reply with error and stay in current state.
                // this speeds up error detection on initiator when pinfo got lost
                responderSendErrorForInvalidSid(*pstate, "handleKeepAlive", header.sessionId);
            }

            else if (pstate->responderState == ResponderState::WaitForUp)
            {
                assert(!isParticipantServiceAvailable(*pstate));

                if (header.sessionId == pstate->activeSessionId &&
                    header.messageId == expectedMessageId)
                {
                    pstate->lastReceivedMessageId = header.messageId;
                    pstate->lastReceiveTime = m_steadyClockNow();
                }
                else
                {
                    LOG_WARN_P(m_logContext, "{}handleKeepAlive(R): Invalid message {} in state {}. Expected sid {}, mid {}",
                               m_logPrefix, header, pstate->responderState, pstate->activeSessionId, expectedMessageId);
                    pstate->activeSessionId = 0;
                    pstate->responderState = ResponderState::Unavailable;
                }
            }

            else if (pstate->responderState == ResponderState::Connected)
            {
                assert(isParticipantServiceAvailable(*pstate));
                assert(isParticipantConnected(*pstate));

                if (header.sessionId == pstate->activeSessionId &&
                    header.messageId == expectedMessageId)
                {
                    pstate->lastReceivedMessageId = header.messageId;
                    pstate->lastReceiveTime = m_steadyClockNow();
                }
                else if (header.sessionId == pstate->activeSessionId)
                {
                    // good sid, bad mid
                    LOG_WARN_P(m_logContext, "{}handleKeepAlive(R): Invalid message {} in state {}. Expected sid {}, mid {}",
                               m_logPrefix, header, pstate->responderState, pstate->activeSessionId, expectedMessageId);
                    responderSendError(*pstate, "handleKeepAlive", "sid or mid mismatch");
                }
                else
                {
                    // bad sid, special error handling to send error only but otherwise ignore
                    responderSendErrorForInvalidSid(*pstate, "handleKeepAlive", header.sessionId);
                }
            }

            else
                assert(false);
        }

        return true;
    }

    template <typename InstanceIdType>
    bool ConnectionSystemInitiatorResponder<InstanceIdType>::isResponsibleForParticipant(const Guid& pid) const
    {
        return m_knownParticipants.find(pid) != m_knownParticipants.end();
    }

    template <typename InstanceIdType>
    template <typename F>
    bool ConnectionSystemInitiatorResponder<InstanceIdType>::sendUnicast(const char* callerMethod, const Guid& to, F&& sendFunc)
    {
        if (ParticipantState* pstate = getParticipantStateForSending(to, callerMethod))
            return handleSendResult(*pstate, callerMethod, sendFunc(pstate->remoteIid, generateHeaderForParticipant(*pstate)));
        return false;
    }

    template <typename InstanceIdType>
    template <typename F>
    bool ConnectionSystemInitiatorResponder<InstanceIdType>::sendBroadcast(const char* callerMethod, F&& sendFunc)
    {
        if (!checkConnected(callerMethod))
            return false;

        // prevent iter over connected because modified on send failure
        for (auto& p : m_availableInstances)
        {
            if (isParticipantConnected(*p.second))
                if (ParticipantState* pstate = getParticipantStateForSending(p.second->remotePid, callerMethod))
                    handleSendResult(*pstate, callerMethod, sendFunc(pstate->remoteIid, generateHeaderForParticipant(*pstate)));
        }
        return true;
    }

    template <typename InstanceIdType>
    absl::optional<Guid*> ConnectionSystemInitiatorResponder<InstanceIdType>::processReceivedMessageHeader(const SomeIPMsgHeader& header, const char* callerMethod)
    {
        // check if responsible first (if not return absl::nullopt, on error return optional will nullptr instead)
        const Guid pid(header.participantId);
        if (!m_connected || !isResponsibleForParticipant(pid))
            return absl::nullopt;

        auto it = m_knownParticipants.find(pid);
        assert(it != m_knownParticipants.end()); // must be known if is responsible
        ParticipantState* pstate = it->second;
        assert(pstate);

        const uint64_t expectedMessageId = pstate->lastReceivedMessageId + 1;

        if (pstate->selfIsInitiator)
        {
            assert(pstate->responderState == ResponderState::Invalid);

            if (pstate->initiatorState == InitiatorState::Connected)
            {
                assert(isInstanceAvailable(pstate->remoteIid));
                assert(isParticipantConnected(*pstate));

                // ignore invalid sid
                if (header.sessionId == pstate->activeSessionId)
                {
                    if (header.messageId == expectedMessageId)
                    {
                        // received valid message
                        pstate->lastReceivedMessageId = header.messageId;
                        pstate->lastReceiveTime = m_steadyClockNow();
                        return &pstate->remotePid;
                    }
                    else
                    {
                        // wrong mid
                        LOG_WARN_P(m_logContext, "{}processReceivedMessageHeader(I/{}): Received {} with invalid mid, expected {}",
                                   m_logPrefix, callerMethod, header, expectedMessageId);
                        initiatorInitSession(*pstate, callerMethod, "mid mismatch");
                    }
                }
            }
            else if (pstate->initiatorState == InitiatorState::WaitForSessionReply)
            {
                assert(isInstanceAvailable(pstate->remoteIid));
                assert(!isParticipantConnected(*pstate));

                // getting here with valid sid is always an error
                if (header.sessionId == pstate->activeSessionId)
                {
                    // wrong mid
                    LOG_WARN_P(m_logContext, "{}processReceivedMessageHeader(I/{}): Unexpected {} in state {}",
                               m_logPrefix, callerMethod, header, pstate->initiatorState);
                    initiatorInitSession(*pstate, callerMethod, "mid mismatch");
                }
            }
        }
        else
        {
            assert(pstate->initiatorState == InitiatorState::Invalid);

            if (pstate->responderState == ResponderState::Connected)
            {
                assert(isInstanceAvailable(pstate->remoteIid));
                assert(isParticipantConnected(*pstate));

                if (header.sessionId == pstate->activeSessionId)
                {
                    if (header.messageId == expectedMessageId)
                    {
                        // received valid message
                        pstate->lastReceivedMessageId = header.messageId;
                        pstate->lastReceiveTime = m_steadyClockNow();
                        return &pstate->remotePid;
                    }
                    else
                    {
                        // wrong mid
                        LOG_WARN_P(m_logContext, "{}processReceivedMessageHeader(R/{}): Received {} with invalid mid, expected {}",
                                   m_logPrefix, callerMethod, header, expectedMessageId);
                        responderSendError(*pstate, callerMethod, "mid mismatch");
                    }
                }
                else
                {
                    // bad sid, special error handling to send error only but otherwise ignore
                    responderSendErrorForInvalidSid(*pstate, "processReceivedMessageHeader", header.sessionId);
                }
            }

            else if (pstate->responderState == ResponderState::WaitForSession)
            {
                // every message/session is wrong here, also reply with error and stay in current state.
                // this speeds up error detection on initiator when pinfo got lost
                responderSendErrorForInvalidSid(*pstate, "processReceivedMessageHeader", header.sessionId);
            }

            else if (pstate->responderState == ResponderState::WaitForUp)
            {
                // always an error, cannot directly connect on up when received unexpected message
                LOG_WARN_P(m_logContext, "{}processReceivedMessageHeader(R/{}): Unexpected {} in state {}, change to {}",
                           m_logPrefix, callerMethod, header, pstate->responderState, ResponderState::Unavailable);
                pstate->responderState = ResponderState::Unavailable;
            }
        }

        // always an error if gets here
        return nullptr;
    }


    template <typename InstanceIdType>
    bool ConnectionSystemInitiatorResponder<InstanceIdType>::isResponsibleForInstance(InstanceIdType iid) const
    {
        // is responsible if has an entry with valid PID (i.e. received pinfo for it and stored info)
        auto it = m_availableInstances.find(iid);
        return it != m_availableInstances.end() && it->second->remotePid.isValid();
    }

    template <typename InstanceIdType>
    std::chrono::steady_clock::time_point ConnectionSystemInitiatorResponder<InstanceIdType>::doOneThreadLoop(std::chrono::steady_clock::time_point now,
                                                                                                              std::chrono::milliseconds keepAliveInterval,
                                                                                                              std::chrono::milliseconds keepAliveTimeout)
    {
        assert(now != std::chrono::steady_clock::time_point{});
        assert(keepAliveInterval.count() > 0);
        assert(keepAliveInterval < keepAliveTimeout);

        std::chrono::steady_clock::time_point nextWakeup = now + keepAliveInterval;  // maximum keepalive interval

        for (auto& pstatePtr : m_participantStates)
        {
            assert(pstatePtr);
            ParticipantState& pstate = *pstatePtr;

            // check for receive timeouts
            auto TimeoutMsg = [&](auto state) {
                return fmt::format("Receive timeout from {} in state {} because lastRecv {}ms ago, expected {}ms ago, latest {}ms ago",
                                   pstate.remotePid, state,
                                   std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(now - pstate.lastReceiveTime).count(),
                                   std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(now - pstate.lastReceiveTime - keepAliveInterval).count(),
                                   std::chrono::duration_cast<std::chrono::duration<int64_t, std::milli>>(now - pstate.lastReceiveTime - keepAliveTimeout).count());
            };

            if (pstate.lastReceiveTime + keepAliveTimeout <= now)
            {
                // initiator
                if (pstate.initiatorState == InitiatorState::WaitForSessionReply ||
                    pstate.initiatorState == InitiatorState::Connected)
                {
                    LOG_WARN_P(m_logContext, "{}doOneThreadLoop(I): {}", m_logPrefix, TimeoutMsg(pstate.initiatorState));
                    initiatorInitSession(pstate, "doOneThreadLoop", "receive timeout");
                }

                // responder
                else if (pstate.responderState == ResponderState::WaitForUp)
                {
                    LOG_WARN_P(m_logContext, "{}doOneThreadLoop(R): {}. Change to {}", m_logPrefix, TimeoutMsg(pstate.responderState), ResponderState::Unavailable);
                    pstate.responderState = ResponderState::Unavailable;
                }

                else if (pstate.responderState == ResponderState::Connected)
                {
                    LOG_WARN_P(m_logContext, "{}doOneThreadLoop(R): {}", m_logPrefix, TimeoutMsg(pstate.responderState));
                    responderSendError(pstate, "doOneThreadLoop", "receive timeout");
                }
            }

            // update wakeup if still in a state that requires receive timeout tracking
            if (pstate.initiatorState == InitiatorState::WaitForSessionReply ||
                pstate.initiatorState == InitiatorState::Connected ||
                pstate.responderState == ResponderState::WaitForUp ||
                pstate.responderState == ResponderState::Connected)
            {
                nextWakeup = std::min(nextWakeup, pstate.lastReceiveTime + keepAliveTimeout);
            }

            // state in which keepalives should be sent
            if ((pstate.initiatorState == InitiatorState::WaitForSessionReply ||
                 pstate.initiatorState == InitiatorState::Connected ||
                 pstate.responderState == ResponderState::Connected))
            {
                assert(pstate.remotePid.isValid());
                assert(pstate.remoteIid.isValid());
                assert(isParticipantServiceAvailable(pstate));
                assert(pstate.initiatorState == InitiatorState::Invalid || pstate.responderState == ResponderState::Invalid);

                // check if must send something to fulfil keepalive interval
                if (pstate.lastSentTime + keepAliveInterval <= now)
                {
                    SomeIPMsgHeader header{m_participantId.get(), pstate.activeSessionId, pstate.lastSentMessageId};  // same mid
                    if (m_stack->sendKeepAlive(pstate.remoteIid, header, 0, true))
                    {
                        LOG_TRACE_PF(m_logContext, ([&](auto& buf) {
                            fmt::format_to(buf, "{}doOneThreadLoop: Sent keepalive to {} {} state ",
                                           m_logPrefix, pstate.remoteIid, header);
                            if (pstate.selfIsInitiator)
                                fmt::format_to(buf, "{}", pstate.initiatorState);
                            else
                                fmt::format_to(buf, "{}", pstate.responderState);
                        }));
                    }
                    else
                    {
                        // only log but do nothing else, a failed keepalive send should not lead to connection loss
                        LOG_WARN_PF(m_logContext, ([&](auto& buf) {
                            fmt::format_to(buf, "{}doOneThreadLoop: Failed to send keepalive to {} {} state ",
                                           m_logPrefix, pstate.remoteIid, header);
                            if (pstate.selfIsInitiator)
                                fmt::format_to(buf, "{}", pstate.initiatorState);
                            else
                                fmt::format_to(buf, "{}", pstate.responderState);
                        }));
                    }

                    // always update lastSentTime (even on failed sending) to avoid spamming keepalives
                    pstate.lastSentTime = now;
                }

                // update wakeup based on all participants in states that are relevant for keepalives
                nextWakeup = std::min(nextWakeup, pstate.lastSentTime + keepAliveInterval);
            }
        }

        return nextWakeup;
    }

    template <typename InstanceIdType>
    bool ConnectionSystemInitiatorResponder<InstanceIdType>::checkConnected(const char* callerMethod) const
    {
        if (m_connected)
            return true;
        LOG_ERROR(m_logContext, m_logPrefix << callerMethod << " called without beeing connected");
        return false;
    }

    template <typename InstanceIdType>
    bool ConnectionSystemInitiatorResponder<InstanceIdType>::isParticipantConnected(const ParticipantState& pstate) const
    {
        return pstate.remotePid.isValid() && m_connectedParticipants.find(pstate.remotePid) != m_connectedParticipants.end();
    }

    template <typename InstanceIdType>
    bool ConnectionSystemInitiatorResponder<InstanceIdType>::isParticipantServiceAvailable(const ParticipantState& pstate) const
    {
        assert(pstate.remoteIid.isValid());
        return isInstanceAvailable(pstate.remoteIid);
    }

    template <typename InstanceIdType>
    auto ConnectionSystemInitiatorResponder<InstanceIdType>::getParticipantStateForSending(const Guid& pid, const char* callerMethod) -> ParticipantState*
    {
        if (!checkConnected(callerMethod))
            return nullptr;

        auto it = m_connectedParticipants.find(pid);
        if (it == m_connectedParticipants.end())
        {
            if (m_knownParticipants.find(pid) != m_knownParticipants.end())
                LOG_ERROR(m_logContext, m_logPrefix << callerMethod << ": participant " << pid << " is not connected");
            else
                LOG_ERROR(m_logContext, m_logPrefix << callerMethod << ": unknown participant " << pid);
            return nullptr;
        }

        return it->second;
    }

    template <typename InstanceIdType>
    bool ConnectionSystemInitiatorResponder<InstanceIdType>::handleSendResult(ParticipantState& pstate, const char* callerMethod, bool result)
    {
        assert(isParticipantConnected(pstate));

        if (result)
        {
            pstate.lastSentTime = m_steadyClockNow();
            return true;
        }

        // send failed
        if (pstate.selfIsInitiator)
            initiatorInitSession(pstate, callerMethod, "sending failed");
        else
            responderSendError(pstate, callerMethod, "sending failed");
        return false;
    }

    template <typename InstanceIdType>
    SomeIPMsgHeader ConnectionSystemInitiatorResponder<InstanceIdType>::generateHeaderForParticipant(ParticipantState& pstate)
    {
        return SomeIPMsgHeader{m_participantId.get(), pstate.activeSessionId, ++pstate.lastSentMessageId};
    }

    template <typename InstanceIdType>
    void ConnectionSystemInitiatorResponder<InstanceIdType>::disconnectParticipant(ParticipantState& pstate)
    {
        assert(isParticipantConnected(pstate));
        assert(pstate.initiatorState == InitiatorState::Connected || pstate.responderState == ResponderState::Connected);

        m_connectedParticipants.erase(pstate.remotePid);
        m_connectionStatusUpdateNotifier.triggerNotification(pstate.remotePid, EConnectionStatus::EConnectionStatus_NotConnected);
    }


    template <typename InstanceIdType>
    void ConnectionSystemInitiatorResponder<InstanceIdType>::initiatorInitSession(ParticipantState& pstate, const char* callerMethod, const char* reason)
    {
        assert(pstate.selfIsInitiator);
        assert(pstate.remoteIid.isValid());

        // disconnect if was connected
        if (isParticipantConnected(pstate))
        {
            LOG_WARN(m_logContext, m_logPrefix << "initiatorInitSession(" << callerMethod << "): Disconnect pid " << pstate.remotePid << " because " << reason);
            assert(pstate.initiatorState == InitiatorState::Connected);
            disconnectParticipant(pstate);
        }

        // prepare pstate for new session
        std::uniform_int_distribution<uint64_t> dis(1, std::numeric_limits<uint64_t>::max()-1);
        pstate.activeSessionId = dis(m_randomGenerator);
        pstate.lastSentMessageId = 0;
        pstate.lastReceivedMessageId = 0;  // expect 1 next

        const auto now = m_steadyClockNow();
        pstate.lastSentTime = now;  // always set to now even if send fails to prevent init session spam
        pstate.lastReceiveTime = now; // give full timeout for reply

        // send init message
        const SomeIPMsgHeader header = generateHeaderForParticipant(pstate);
        LOG_INFO_P(m_logContext, "{}initiatorInitSession: to {}/{}, protocolVersion {}/{}, senderInstanceId {}, {}, state {:s} -> {:s}",
                   m_logPrefix, pstate.remoteIid, pstate.remotePid,
                   m_protocolVersion, MinorProtocolVersion, m_serviceIid, header, pstate.initiatorState, InitiatorState::WaitForSessionReply);

        if (!m_stack->sendParticipantInfo(pstate.remoteIid, header,
                                          m_protocolVersion, MinorProtocolVersion,
                                          m_serviceIid, pstate.remotePid.get(),
                                          0, 0))
        {
            // no special handling, recover by timeout and sending new session to avoid spamming
            LOG_WARN(m_logContext, m_logPrefix << "initiatorInitSession: sendParticipantInfo to " << pstate.remoteIid << "/" << pstate.remotePid << " failed. Will try again after timeout");
        }

        pstate.initiatorState = InitiatorState::WaitForSessionReply;

        // wakeup keepalive thread to pick up new timeout values
        m_wakeupKeepAliveThread();
    }

    template <typename InstanceIdType>
    void ConnectionSystemInitiatorResponder<InstanceIdType>::responderSendError(ParticipantState& pstate, const char* callerMethod, const char* reason)
    {
        assert(!pstate.selfIsInitiator);
        assert(pstate.responderState == ResponderState::Connected);

        // Remove from connected
        LOG_WARN(m_logContext, m_logPrefix << "responderSendError(" << callerMethod << "): Disconnect pid " << pstate.remotePid
                 << " because " << reason);
        disconnectParticipant(pstate);

        // send error and update state
        // only log send result, has no effect on state
        const SomeIPMsgHeader header{m_participantId.get(), pstate.activeSessionId, 0};  // use keepalive with MID=0 as error
        if (m_stack->sendKeepAlive(pstate.remoteIid, header, 0, true))
            LOG_INFO(m_logContext, m_logPrefix << "responderSendError: Send error to " << pstate.remoteIid << ", " << header);
        else
            LOG_WARN(m_logContext, m_logPrefix << "responderSendError: Failed to send error to " << pstate.remoteIid << ", " << header);

        pstate.activeSessionId = 0;
        pstate.responderState = ResponderState::WaitForSession;
    }

    template <typename InstanceIdType>
    void ConnectionSystemInitiatorResponder<InstanceIdType>::responderSendErrorForInvalidSid(ParticipantState& pstate, const char* callerMethod, uint64_t sessionId)
    {
        assert(!pstate.selfIsInitiator);
        assert(pstate.responderState == ResponderState::Connected ||
               pstate.responderState == ResponderState::WaitForSession);

        // send error for given session but does not touch any state. failed send is ignored
        const SomeIPMsgHeader header{m_participantId.get(), sessionId, 0};  // use keepalive with MID=0 as error
        if (m_stack->sendKeepAlive(pstate.remoteIid, header, 0, true))
            LOG_WARN_P(m_logContext, "{}responderSendErrorForInvalidSid({}): Drop mesage and send error to {} because invalid session {} in state {}. Expected {}",
                       m_logPrefix, callerMethod, pstate.remoteIid, sessionId, pstate.responderState, pstate.activeSessionId);
        else
            LOG_WARN_P(m_logContext, "{}responderSendErrorForInvalidSid({}): Drop message but failed to send error to {} because invalid session {} in state {}. Expected {}",
                       m_logPrefix, callerMethod, pstate.remoteIid, sessionId, pstate.responderState, pstate.activeSessionId);
    }

    template <typename InstanceIdType>
    void ConnectionSystemInitiatorResponder<InstanceIdType>::responderSendSessionReply(ParticipantState& pstate)
    {
        LOG_INFO(m_logContext, m_logPrefix << "responderSendSessionReply: to " << pstate.remoteIid << "/" << pstate.remotePid
                 << " for session " << pstate.activeSessionId << ", state " << pstate.responderState);

        assert(!pstate.selfIsInitiator);
        assert(pstate.activeSessionId != 0);
        assert(!isParticipantConnected(pstate));

        pstate.lastSentMessageId = 0;
        const SomeIPMsgHeader header = generateHeaderForParticipant(pstate);
        if (m_stack->sendParticipantInfo(pstate.remoteIid, header,
                                         m_protocolVersion, MinorProtocolVersion,
                                         m_serviceIid, pstate.remotePid.get(),
                                         0, 0))
        {
            pstate.lastSentTime = m_steadyClockNow();
            pstate.responderState = ResponderState::Connected;

            LOG_INFO(m_logContext, m_logPrefix << "responderSendSessionReply: Connected pid " << pstate.remotePid);
            m_connectedParticipants[pstate.remotePid] = &pstate;
            m_connectionStatusUpdateNotifier.triggerNotification(pstate.remotePid, EConnectionStatus::EConnectionStatus_Connected);
        }
        else
        {
            LOG_WARN(m_logContext, m_logPrefix << "responderSendSessionReply: Failed to send to "
                     << pstate.remoteIid << "/" << pstate.remotePid << " for session " << pstate.activeSessionId);

            pstate.responderState = ResponderState::WaitForSession;
            pstate.activeSessionId = 0;
        }
    }


    template <typename InstanceIdType>
    auto  ConnectionSystemInitiatorResponder<InstanceIdType>::getParticipantState(InstanceIdType iid) const -> const ParticipantState*
    {
        const auto it = std::find_if(m_participantStates.cbegin(), m_participantStates.cend(), [&](auto& ps) { return ps->remoteIid == iid; });
        if (it != m_participantStates.cend())
            return it->get();
        return nullptr;
    }


    template <typename InstanceIdType>
    bool  ConnectionSystemInitiatorResponder<InstanceIdType>::isInstanceAvailable(InstanceIdType iid) const
    {
        return m_availableInstances.find(iid) != m_availableInstances.end();
    }

    template <typename InstanceIdType>
    void ConnectionSystemInitiatorResponder<InstanceIdType>::clearParticipantStateForReuse(ParticipantState& pstate, const char* reason)
    {
        assert(pstate.remotePid.isValid());
        assert(pstate.remoteIid.isValid());
        assert(m_knownParticipants.find(pstate.remotePid) != m_knownParticipants.end());

        LOG_WARN_P(m_logContext, "{}clearParticipantStateForReuse: Invalidate participant with pid {}, iid {}, state {} because {}",
                   m_logPrefix, pstate.remotePid, pstate.remoteIid,
                   pstate.selfIsInitiator ? fmt::to_string(pstate.initiatorState) : fmt::to_string(pstate.responderState),
                   reason);

        if (isParticipantConnected(pstate))
            disconnectParticipant(pstate);

        m_knownParticipants.erase(pstate.remotePid);

        pstate.remotePid = Guid();
        pstate.lastSentTime = std::chrono::steady_clock::time_point{};
        pstate.lastReceiveTime = std::chrono::steady_clock::time_point{};
        pstate.activeSessionId = 0;
        pstate.lastSentMessageId = 0;
        pstate.lastReceivedMessageId = 0;
        pstate.initiatorState = InitiatorState::Invalid;
        pstate.responderState = ResponderState::Invalid;
    }


    // Logging

    template <typename InstanceIdType>
    void ConnectionSystemInitiatorResponder<InstanceIdType>::writeConnectionInfo(StringOutputStream& sos) const
    {
        sos << "  ConnectionSystemIR Participants:";
        if (m_participantStates.empty())
            sos << " None";

        for (const auto& pstate : m_participantStates)
        {
            sos.formatTo("\n  - Pid {}, iid {} {}, ",
                         pstate->remotePid, pstate->remoteIid,
                         isParticipantServiceAvailable(*pstate) ? "Up" : "Down");
            if (pstate->selfIsInitiator)
                sos << pstate->initiatorState;
            else
                sos << pstate->responderState;
            sos.formatTo("\n    Sid:{} RecvdMid:{} SentMid:{}",
                         pstate->activeSessionId, pstate->lastReceivedMessageId, pstate->lastSentMessageId);
        }
        sos << "\n";
    }

    template <typename InstanceIdType>
    void ConnectionSystemInitiatorResponder<InstanceIdType>::writePeriodicInfo(StringOutputStream& sos) const
    {
        sos << "; IR: ";
        if (m_participantStates.empty())
            sos << "None";

        bool first = true;
        for (const auto& pstate : m_participantStates)
        {
            if (first)
                first = false;
            else
                sos << ", ";
            sos.formatTo("{}/{}{}:", pstate->remotePid, pstate->remoteIid, isParticipantServiceAvailable(*pstate) ? "+" : "-");
            if (pstate->selfIsInitiator)
                sos.formatTo("I/{:s}", pstate->initiatorState);
            else
                sos.formatTo("R/{:s}", pstate->responderState);
        }
    }
}

MAKE_ENUM_CLASS_PRINTABLE_NO_EXTRA_LAST(ramses_internal::internal::ConnectionSystemInitiatorResponderNonTemplateBase::InitiatorState,
                                        "InitiatorState",
                                        ramses_internal::internal::InitiatorStateNames,
                                        ramses_internal::internal::ConnectionSystemInitiatorResponderNonTemplateBase::InitiatorState::Connected);
MAKE_ENUM_CLASS_PRINTABLE_NO_EXTRA_LAST(ramses_internal::internal::ConnectionSystemInitiatorResponderNonTemplateBase::ResponderState,
                                        "ResponderState",
                                        ramses_internal::internal::ResponderStateNames,
                                        ramses_internal::internal::ConnectionSystemInitiatorResponderNonTemplateBase::ResponderState::Connected);

