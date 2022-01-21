//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportCommon/ConnectionSystemInitiatorResponder.h"
#include "Collections/StringOutputStream.h"
#include "ConnectionSystemTestCommon.h"
#include "TransportCommon/SomeIPStackCommon.h"
#include "ScopedLogContextLevel.h"
#include "absl/types/optional.h"
#include "gmock/gmock.h"
#include <chrono>

namespace ramses_internal
{
    using namespace TestConnectionSystemBase;

    namespace internal
    {
        void PrintTo(const ConnectionSystemInitiatorResponderNonTemplateBase::InitiatorState& state, ::std::ostream* os)
        {
            *os << fmt::to_string(state);
        }

        void PrintTo(const ConnectionSystemInitiatorResponderNonTemplateBase::ResponderState& state, ::std::ostream* os)
        {
            *os << fmt::to_string(state);
        }
    }

    /*
     * Test ConnectionSytemIR through ConnectionSystemBase
     *
     * many tests change state transitions and have a specific naming pattern:
     * X_<state before>_<events>_<state after>_<extra description>
     * X = I: self is initiator, remote is responder
     * X = R: self is responder, remote is initiator
     *
     * some tests use a short form for the names when there are no events and no transition
     * X_<state>_<things tests>
     *
     * possible actions during each state
     *   up / down / pinfo(ok+bad) / recv-keepalive(ok+bad) / send(ok+fail) / receive(ok+bad) / recv-timeout / send-timeout
     */
    class AConnectionSystemInitiatorResponder : public Test
    {
    public:
        using InitiatorState = internal::ConnectionSystemInitiatorResponderNonTemplateBase::InitiatorState;
        using ResponderState = internal::ConnectionSystemInitiatorResponderNonTemplateBase::ResponderState;
        using ParticipantState = ConnectionSystemInitiatorResponder<TestInstanceId>::ParticipantState;

        AConnectionSystemInitiatorResponder()
            : stackPtr(std::make_shared<StrictMock<StackMock>>())
            , stack(*stackPtr)
            , connsys(construct(3, TestInstanceId(5), ParticipantIdentifier(Guid(pid), "foobar"), 99, 0, 0, [this]() { return currentTime; }))
            , connsysIR(connsys->getConnectionSystemIR())
            , fromStack(*connsys)
        {
        }

        std::unique_ptr<TestConnectionSystem> construct(uint32_t commUserId, TestInstanceId serviceIid, const ParticipantIdentifier& namedPid, uint32_t protocolVersion,
                                                        uint64_t keepAliveInterval, uint64_t keepAliveTimeout,
                                                        const std::function<std::chrono::steady_clock::time_point(void)>& steadyClockNow)
        {
            EXPECT_CALL(*stackPtr, getServiceInstanceId()).WillRepeatedly(Return(serviceIid));
            return std::make_unique<TestConnectionSystem>(stackPtr, commUserId, namedPid, protocolVersion, lock,
                                                          std::chrono::milliseconds(keepAliveInterval), std::chrono::milliseconds(keepAliveTimeout),
                                                          steadyClockNow, true);
        }

        void connect()
        {
            lock.lock();
            EXPECT_CALL(stack, connect()).WillOnce(Return(true));
            ASSERT_TRUE(connsys->connect());
        }

        void disconnect()
        {
            EXPECT_CALL(stack, disconnect()).WillOnce(Return(true));
            ASSERT_TRUE(connsys->disconnect());
            lock.unlock();
        }

        static std::chrono::steady_clock::time_point Tp(uint64_t tpMilliseconds)
        {
            return std::chrono::steady_clock::time_point{std::chrono::milliseconds(tpMilliseconds)};
        }

        void expectRemoteDisconnects(std::initializer_list<uint64_t> guids)
        {
            for (auto g : guids)
                EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(g)));
        }

        void connectRemoteBaseOnly(TestInstanceId remoteIid, const Guid& remotePidArg, uint64_t sessionIdIn, uint64_t* sessionIdOut)
        {
            EXPECT_CALL(stack, sendParticipantInfo(remoteIid, ValidHdr(pid, 1u), 99, 1, iid, 0, _, _)).WillOnce(StoreSessionAndReturn(sessionIdOut, true));
            fromStack.handleServiceAvailable(remoteIid);
            EXPECT_CALL(connsys->connections, newParticipantHasConnected(remotePidArg));
            fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePidArg.get(), sessionIdIn, 1}, 99, SomeIPConstants::FallbackMinorProtocolVersion, remoteIid, 0, 0, 0);
        }

        void initiatorToState(InitiatorState state, TestInstanceId remoteIid, uint64_t remotePidArg, uint64_t* sessionId)
        {
            assert(sessionId);
            assert(state != InitiatorState::Invalid);
            if (state == InitiatorState::Unavailable)
            {
                *sessionId = 123;
                fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePidArg, *sessionId, 1}, 99, 1, remoteIid, 0, 0, 0);
            }
            else if (state == InitiatorState::Connected)
            {
                EXPECT_CALL(stack, sendParticipantInfo(remoteIid, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0)).WillOnce(StoreSessionAndReturn(sessionId, true));
                fromStack.handleServiceAvailable(remoteIid);
                EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePidArg)));
                fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePidArg, *sessionId, 1}, 99, 1, remoteIid, pid, 0, 0);
            }
            else if (state == InitiatorState::WaitForSessionReply)
            {
                EXPECT_CALL(stack, sendParticipantInfo(remoteIid, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0)).WillOnce(StoreSessionAndReturn(sessionId, true));
                fromStack.handleServiceAvailable(remoteIid);
                EXPECT_CALL(stack, sendParticipantInfo(remoteIid, ValidHdr(pid, 1u), 99, 1, iid, remotePidArg, 0, 0)).WillOnce(StoreSessionAndReturn(sessionId, true));
                fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePidArg, *sessionId+1, 1}, 99, 1, remoteIid, pid, 0, 0);
            }

            const auto* pstate = connsysIR.getParticipantState(remoteIid);
            ASSERT_TRUE(pstate);
            ASSERT_EQ(state, pstate->initiatorState);
        }

        void responderToState(ResponderState state, TestInstanceId remoteIid, uint64_t remotePidArg, uint64_t sessionId)
        {
            assert(state != ResponderState::Invalid);
            if (state == ResponderState::WaitForUp)
            {
                fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePidArg, sessionId, 1}, 99, 1, remoteIid, 0, 0, 0);
            }
            else if (state == ResponderState::Unavailable)
            {
                fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePidArg, sessionId, 1}, 99, 1, remoteIid, 0, 0, 0);
                fromStack.handleKeepAlive(SomeIPMsgHeader{remotePidArg, sessionId, 2}, 0, true);
            }
            else if (state == ResponderState::WaitForSession)
            {
                fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePidArg, sessionId, 1}, 99, 1, remoteIid, 0, 0, 0);
                fromStack.handleKeepAlive(SomeIPMsgHeader{remotePidArg, sessionId, 2}, 0, true);
                fromStack.handleServiceAvailable(remoteIid);

            }
            else if (state == ResponderState::Connected)
            {
                EXPECT_CALL(stack, sendParticipantInfo(remoteIid, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0)).WillOnce(Return(true));
                fromStack.handleServiceAvailable(remoteIid);
                EXPECT_CALL(stack, sendParticipantInfo(remoteIid, SomeIPMsgHeader{pid, sessionId, 1u}, 99, 1, iid, remotePidArg, 0, 0)).WillOnce(Return(true));
                EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePidArg)));
                fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePidArg, sessionId, 1}, 99, 1, remoteIid, remotePidArg, 0, 0);
            }

            const auto* pstate = connsysIR.getParticipantState(remoteIid);
            ASSERT_TRUE(pstate);
            ASSERT_EQ(state, pstate->responderState);
        }

        PlatformLock lock;
        std::shared_ptr<StrictMock<StackMock>> stackPtr;
        StrictMock<StackMock>& stack;
        uint64_t pid{4};
        TestInstanceId iid{5};
        std::unique_ptr<TestConnectionSystem> connsys;
        ConnectionSystemInitiatorResponder<TestInstanceId>& connsysIR;
        Callbacks& fromStack;
        std::chrono::steady_clock::time_point currentTime{std::chrono::milliseconds(10)};
        uint64_t remotePid{3};
        TestInstanceId remoteIidI{10};
        TestInstanceId remoteIidR{1};
    };

    static bool operator==(const ConnectionSystemInitiatorResponder<TestInstanceId>::ParticipantState& pa,
                           const ConnectionSystemInitiatorResponder<TestInstanceId>::ParticipantState& pb)
    {
        return std::tie(pa.remotePid, pa.remoteIid, pa.selfIsInitiator, pa.lastSentTime, pa.lastReceiveTime,
                        pa.activeSessionId, pa.lastSentMessageId, pa.lastReceivedMessageId, pa.initiatorState, pa.responderState) ==
            std::tie(pb.remotePid, pb.remoteIid, pb.selfIsInitiator, pb.lastSentTime, pb.lastReceiveTime,
                     pb.activeSessionId, pb.lastSentMessageId, pb.lastReceivedMessageId, pb.initiatorState, pb.responderState);
    }


    TEST_F(AConnectionSystemInitiatorResponder, cannotSendWhenNotConnected)
    {
        EXPECT_FALSE(connsysIR.sendUnicast("test", Guid(10), [](const auto& /*iid*/, const auto& /*header*/) {
            throw std::exception();
            return true;
        }));
        EXPECT_FALSE(connsysIR.sendBroadcast("test", [](const auto& /*iid*/, const auto& /*header*/) {
            throw std::exception();
            return true;
        }));
    }

    TEST_F(AConnectionSystemInitiatorResponder, cannotReceiveWhenNotConnected){
        EXPECT_FALSE(connsysIR.processReceivedMessageHeader(SomeIPMsgHeader{1, 2, 3}, "test"));
    }

    TEST_F(AConnectionSystemInitiatorResponder, doesNotHandleKeepaliveWhenNotConnected)
    {
        EXPECT_FALSE(connsysIR.handleKeepAlive(SomeIPMsgHeader{1, 2, 3}, false));
    }

    TEST_F(AConnectionSystemInitiatorResponder, isNotResponsibleWhenNotConnected)
    {
        EXPECT_FALSE(connsysIR.isResponsibleForInstance(TestInstanceId{1}));
        EXPECT_FALSE(connsysIR.isResponsibleForInstance(TestInstanceId{2}));
        EXPECT_FALSE(connsysIR.isResponsibleForInstance(TestInstanceId{5}));
        EXPECT_FALSE(connsysIR.isResponsibleForParticipant(Guid{1}));
        EXPECT_FALSE(connsysIR.isResponsibleForParticipant(Guid{2}));
        EXPECT_FALSE(connsysIR.isResponsibleForParticipant(Guid{4}));
    }

    TEST_F(AConnectionSystemInitiatorResponder, hasProperMinorProtocolVersion)
    {
        EXPECT_TRUE(connsysIR.getSupportedMinorProtocolVersion() > SomeIPConstants::FallbackMinorProtocolVersion);
    }


    class AConnectionSystemInitiatorResponderConnected : public AConnectionSystemInitiatorResponder
    {
    public:
        AConnectionSystemInitiatorResponderConnected()
        {
            connect();
        }

        ~AConnectionSystemInitiatorResponderConnected()
        {
            disconnect();
        }
    };

    TEST_F(AConnectionSystemInitiatorResponderConnected, basicInitiatorLifecycle){
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, 123, 1}, 99, 1, remoteIidR, 0, 0, 0);
        EXPECT_EQ(InitiatorState::Unavailable, connsysIR.getParticipantState(remoteIidR)->initiatorState);

        uint64_t activeSession = 0;
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(StoreSessionAndReturn(&activeSession, true));
        fromStack.handleServiceAvailable(remoteIidR);

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, activeSession, 1}, 99, 1, remoteIidR, 0, 0, 0);
        EXPECT_EQ(InitiatorState::Connected, connsysIR.getParticipantState(remoteIidR)->initiatorState);

        EXPECT_CALL(stack, sendTestMessage(remoteIidR, SomeIPMsgHeader{pid, activeSession, 2u}, 456)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendTestMessage(Guid(remotePid), 456));

        EXPECT_CALL(stack, sendTestMessage(remoteIidR, SomeIPMsgHeader{pid, activeSession, 3u}, 789)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->broadcastTestMessage(789));

        EXPECT_CALL(connsys->consumer, handleTestMessage(44, Guid(remotePid)));
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, activeSession, 2u}, 44);

        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        fromStack.handleServiceUnavailable(remoteIidR);
        EXPECT_EQ(InitiatorState::Unavailable, connsysIR.getParticipantState(remoteIidR)->initiatorState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, basicResponderLifecycle)
    {
        const uint64_t activeSession = 123;
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, activeSession, 1}, 99, 1, remoteIidI, 0, 0, 0);
        EXPECT_EQ(ResponderState::WaitForUp, connsysIR.getParticipantState(remoteIidI)->responderState);

        EXPECT_CALL(stack, sendParticipantInfo(remoteIidI, SomeIPMsgHeader{pid, activeSession, 1u}, 99, 1, iid, remotePid, 0, 0)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid)));
        fromStack.handleServiceAvailable(remoteIidI);
        EXPECT_EQ(ResponderState::Connected, connsysIR.getParticipantState(remoteIidI)->responderState);

        EXPECT_CALL(stack, sendTestMessage(remoteIidI, SomeIPMsgHeader{pid, activeSession, 2u}, 456)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendTestMessage(Guid(remotePid), 456));

        EXPECT_CALL(stack, sendTestMessage(remoteIidI, SomeIPMsgHeader{pid, activeSession, 3u}, 789)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->broadcastTestMessage(789));

        EXPECT_CALL(connsys->consumer, handleTestMessage(44, Guid(remotePid)));
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, activeSession, 2u}, 44);

        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        fromStack.handleServiceUnavailable(remoteIidI);
        EXPECT_EQ(ResponderState::Unavailable, connsysIR.getParticipantState(remoteIidI)->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Invalid_upDown_Invalid_storeEntryOnly)
    {
        EXPECT_FALSE(connsysIR.isInstanceAvailable(remoteIidR));
        EXPECT_EQ(nullptr, connsysIR.getParticipantState(remoteIidR));

        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(remoteIidR);

        EXPECT_TRUE(connsysIR.isInstanceAvailable(remoteIidR));
        EXPECT_FALSE(connsysIR.isResponsibleForInstance(remoteIidR));

        const auto* pstate = connsysIR.getParticipantState(remoteIidR);
        ASSERT_TRUE(pstate);
        EXPECT_EQ(remoteIidR, pstate->remoteIid);
        EXPECT_FALSE(pstate->remotePid.isValid());
        EXPECT_TRUE(pstate->selfIsInitiator);
        EXPECT_EQ(Tp(0), pstate->lastSentTime);
        EXPECT_EQ(Tp(0), pstate->lastReceiveTime);
        EXPECT_EQ(0, pstate->activeSessionId);
        EXPECT_EQ(0, pstate->lastSentMessageId);
        EXPECT_EQ(0, pstate->lastReceivedMessageId);
        EXPECT_EQ(InitiatorState::Invalid, pstate->initiatorState);
        EXPECT_EQ(ResponderState::Invalid, pstate->responderState);

        fromStack.handleServiceUnavailable(remoteIidR);

        EXPECT_FALSE(connsysIR.isInstanceAvailable(remoteIidR));
        EXPECT_FALSE(connsysIR.isResponsibleForInstance(remoteIidR));
        ASSERT_EQ(pstate, connsysIR.getParticipantState(remoteIidR));
        EXPECT_EQ(remoteIidR, pstate->remoteIid);
        EXPECT_FALSE(pstate->remotePid.isValid());
        EXPECT_EQ(InitiatorState::Invalid, pstate->initiatorState);
        EXPECT_EQ(ResponderState::Invalid, pstate->responderState);

        fromStack.handleServiceUnavailable(remoteIidR); // do nothing on extra down
        EXPECT_FALSE(connsysIR.isInstanceAvailable(remoteIidR));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Invalid_upPinfoGood_Connected_directConnect)
    {
        currentTime = Tp(2);
        uint64_t session = 0;
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        fromStack.handleServiceAvailable(remoteIidR);

        currentTime = Tp(3);
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session, 1}, 99, 1, remoteIidR, pid, 0, 0);

        EXPECT_TRUE(connsysIR.isResponsibleForInstance(remoteIidR));
        EXPECT_TRUE(connsysIR.isResponsibleForParticipant(Guid(remotePid)));

        const auto* pstate = connsysIR.getParticipantState(remoteIidR);
        ASSERT_TRUE(pstate);
        EXPECT_EQ(remoteIidR, pstate->remoteIid);
        EXPECT_TRUE(pstate->remotePid.isValid());
        EXPECT_TRUE(pstate->selfIsInitiator);
        EXPECT_EQ(Tp(2), pstate->lastSentTime);
        EXPECT_EQ(Tp(3), pstate->lastReceiveTime);
        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(1, pstate->lastSentMessageId);
        EXPECT_EQ(1, pstate->lastReceivedMessageId);
        EXPECT_EQ(InitiatorState::Connected, pstate->initiatorState);
        EXPECT_EQ(ResponderState::Invalid, pstate->responderState);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Invalid_upPinfoBadSid_WaitForSessionReply_initNewSession)
    {
        uint64_t session_a = 0;
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0)).WillOnce(StoreSessionAndReturn(&session_a, true));
        fromStack.handleServiceAvailable(remoteIidR);

        uint64_t session_b = 0;
        currentTime = Tp(2);
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(StoreSessionAndReturn(&session_b, true));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session_b+1, 1}, 99, 1, remoteIidR, pid, 0, 0);

        EXPECT_TRUE(connsysIR.isResponsibleForInstance(remoteIidR));
        EXPECT_TRUE(connsysIR.isResponsibleForParticipant(Guid(remotePid)));

        const auto* pstate = connsysIR.getParticipantState(remoteIidR);
        ASSERT_TRUE(pstate);
        EXPECT_EQ(remoteIidR, pstate->remoteIid);
        EXPECT_EQ(Guid(remotePid), pstate->remotePid);
        EXPECT_TRUE(pstate->selfIsInitiator);
        EXPECT_EQ(Tp(2), pstate->lastSentTime);
        EXPECT_EQ(Tp(2), pstate->lastReceiveTime);
        EXPECT_EQ(session_b, pstate->activeSessionId);
        EXPECT_EQ(1, pstate->lastSentMessageId);
        EXPECT_EQ(0, pstate->lastReceivedMessageId);
        EXPECT_EQ(InitiatorState::WaitForSessionReply, pstate->initiatorState);
        EXPECT_EQ(ResponderState::Invalid, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Invalid_Pinfo_Unavailable_storeAndMarkResponsible)
    {
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, 123, 1}, 99, 1, remoteIidR, 0, 0, 0);

        EXPECT_FALSE(connsysIR.isResponsibleForInstance(remoteIidR)); // not yet because not up
        EXPECT_TRUE(connsysIR.isResponsibleForParticipant(Guid(remotePid)));

        const auto* pstate = connsysIR.getParticipantState(remoteIidR);
        ASSERT_TRUE(pstate);
        EXPECT_EQ(remoteIidR, pstate->remoteIid);
        EXPECT_EQ(Guid(remotePid), pstate->remotePid);
        EXPECT_TRUE(pstate->selfIsInitiator);
        EXPECT_EQ(Tp(0), pstate->lastSentTime);
        EXPECT_EQ(Tp(0), pstate->lastReceiveTime);
        EXPECT_EQ(0, pstate->activeSessionId); // no handover
        EXPECT_EQ(0, pstate->lastSentMessageId);
        EXPECT_EQ(0, pstate->lastReceivedMessageId);
        EXPECT_EQ(InitiatorState::Unavailable, pstate->initiatorState);
        EXPECT_EQ(ResponderState::Invalid, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Invalid_upDownUpPinfoBadSid_WaitForSessionReply_multipleAvailabilityChangesBeforePinfo)
    {
        uint64_t session_a = 0;
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0)).WillOnce(StoreSessionAndReturn(&session_a, true));
        fromStack.handleServiceAvailable(remoteIidR);
        fromStack.handleServiceUnavailable(remoteIidR);

        EXPECT_FALSE(connsysIR.isResponsibleForInstance(remoteIidR)); // not up yet
        EXPECT_FALSE(connsysIR.isResponsibleForParticipant(Guid(remotePid)));

        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0)).WillOnce(StoreSessionAndReturn(&session_a, true));
        fromStack.handleServiceAvailable(remoteIidR);
        EXPECT_FALSE(connsysIR.isResponsibleForInstance(remoteIidR)); // no pinfo yet

        uint64_t session_b = 0;
        currentTime = Tp(2);
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(StoreSessionAndReturn(&session_b, true));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session_a+1, 1}, 99, 1, remoteIidR, pid, 0, 0); // bad sid

        EXPECT_TRUE(connsysIR.isResponsibleForInstance(remoteIidR));
        EXPECT_TRUE(connsysIR.isResponsibleForParticipant(Guid(remotePid)));

        const auto* pstate = connsysIR.getParticipantState(remoteIidR);
        ASSERT_TRUE(pstate);
        EXPECT_EQ(Tp(2), pstate->lastSentTime);
        EXPECT_EQ(Tp(2), pstate->lastReceiveTime);
        EXPECT_EQ(session_b, pstate->activeSessionId);
        EXPECT_EQ(1, pstate->lastSentMessageId);
        EXPECT_EQ(0, pstate->lastReceivedMessageId);
        EXPECT_EQ(InitiatorState::WaitForSessionReply, pstate->initiatorState);

        // Note: down+up is only way to make old code send new session *without* sending pinfo. all other messages are ignored before pinfo
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Invalid_up_sendFails)
    {
        EXPECT_FALSE(connsys->sendTestMessage(Guid(remotePid), 456));
        EXPECT_TRUE(connsys->broadcastTestMessage(789)); // does not fail when no receiver

        uint64_t session_a = 0;
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0)).WillOnce(StoreSessionAndReturn(&session_a, true));
        fromStack.handleServiceAvailable(remoteIidR);

        EXPECT_FALSE(connsys->sendTestMessage(Guid(remotePid), 456));
        EXPECT_TRUE(connsys->broadcastTestMessage(789));
        // receive does not reach connsysIR yet, would only test old code
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Invalid_upKeepalivePinfo_Invalid_handoverWithKeepaliveTsAndMid)
    {
        uint64_t session = 0;
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        fromStack.handleServiceAvailable(remoteIidR);

        currentTime = Tp(2000);
        EXPECT_CALL(stack, sendKeepAlive(remoteIidR, SomeIPMsgHeader{pid, session, 2}, _, false)).WillOnce(Return(true)); // from old
        connsys->doOneThreadLoop(std::chrono::milliseconds{10}, std::chrono::milliseconds{50});

        currentTime = Tp(2001);
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session, 1}, 99, 1, remoteIidR, pid, 0, 0);

        const auto* pstate = connsysIR.getParticipantState(remoteIidR);
        ASSERT_TRUE(pstate);
        EXPECT_EQ(Tp(2000), pstate->lastSentTime);
        EXPECT_EQ(Tp(2001), pstate->lastReceiveTime);
        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(2, pstate->lastSentMessageId);
        EXPECT_EQ(1, pstate->lastReceivedMessageId);
        EXPECT_EQ(InitiatorState::Connected, pstate->initiatorState);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Invalid_upTempPinfoSendFailPinfoGood_Connected_handoverAfterOldSendFail)
    {
        uint64_t session = 0;
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0)).WillOnce(StoreSessionAndReturn(&session, false));
        fromStack.handleServiceAvailable(remoteIidR);

        currentTime = Tp(2000);
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        connsys->doOneThreadLoop(std::chrono::milliseconds{10}, std::chrono::milliseconds{50});

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session, 1}, 99, 1, remoteIidR, pid, 0, 0);

        const auto* pstate = connsysIR.getParticipantState(remoteIidR);
        ASSERT_TRUE(pstate);
        EXPECT_EQ(Tp(2000), pstate->lastSentTime);
        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(1, pstate->lastSentMessageId);
        EXPECT_EQ(1, pstate->lastReceivedMessageId);
        EXPECT_EQ(InitiatorState::Connected, pstate->initiatorState);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Invalid_upPinfoSendFailPinfoGood_WaitForSessionReply_doesNotTakeValuesFromFailedOldPinfoSend)
    {
        uint64_t session = 0;
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0)).WillOnce(StoreSessionAndReturn(&session, false));
        fromStack.handleServiceAvailable(remoteIidR);

        currentTime = Tp(2000);
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0)).WillOnce(StoreSessionAndReturn(&session, false));
        connsys->doOneThreadLoop(std::chrono::milliseconds{10}, std::chrono::milliseconds{50});

        currentTime = Tp(3000);
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, 123, 1}, 99, 1, remoteIidR, pid, 0, 0); // other session

        // nothing taken from old code
        const auto* pstate = connsysIR.getParticipantState(remoteIidR);
        ASSERT_TRUE(pstate);
        EXPECT_EQ(Tp(3000), pstate->lastSentTime);
        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(1, pstate->lastSentMessageId);
        EXPECT_EQ(0, pstate->lastReceivedMessageId);
        EXPECT_EQ(InitiatorState::WaitForSessionReply, pstate->initiatorState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Unavailable_receive_Unavailable_ignoresAllIncomingMessages)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::Unavailable, remoteIidR, remotePid, &session);

        const ParticipantState pstateBefore = *connsysIR.getParticipantState(remoteIidR);

        // incoming correct session
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, pstateBefore.lastReceivedMessageId}, 0, true);
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 0}, 0, true);
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session, pstateBefore.lastReceivedMessageId+1}, 123);
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session, 1}, 99, 1, remoteIidR, pid, 0, 0);
        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidR));

        // incoming other session
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session+1, 1}, 99, 1, remoteIidR, pid, 0, 0);
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session+1, 2}, 99, 1, remoteIidR, pid, 0, 0);
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session+1, 2}, 0, true);
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session+1, 2}, 123);
        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidR));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Unavailable_sendFails)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::Unavailable, remoteIidR, remotePid, &session);
        const ParticipantState pstateBefore = *connsysIR.getParticipantState(remoteIidR);

        EXPECT_FALSE(connsys->sendTestMessage(Guid(remotePid), 456));
        EXPECT_TRUE(connsys->broadcastTestMessage(789));
        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidR));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Unavailable_doesNotSendAnythingOnTimeout)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::Unavailable, remoteIidR, remotePid, &session);

        const ParticipantState pstateBefore = *connsysIR.getParticipantState(remoteIidR);
        currentTime = Tp(2000);
        connsys->doOneThreadLoop(std::chrono::milliseconds{10}, std::chrono::milliseconds{50});
        // expect nothing
        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidR));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Unavailable_down_Unavailable_ignoreServiceDown)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::Unavailable, remoteIidR, remotePid, &session);

        const ParticipantState pstateBefore = *connsysIR.getParticipantState(remoteIidR);
        fromStack.handleServiceUnavailable(remoteIidR);
        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidR));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Unavailable_upInit_WaitForSession_sendNewSessionWhenPossible)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::Unavailable, remoteIidR, remotePid, &session);

        currentTime = Tp(2000);
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        fromStack.handleServiceAvailable(remoteIidR);

        const auto* pstate = connsysIR.getParticipantState(remoteIidR);
        EXPECT_EQ(Tp(2000), pstate->lastSentTime);
        EXPECT_EQ(Tp(2000), pstate->lastReceiveTime);
        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(1, pstate->lastSentMessageId);
        EXPECT_EQ(0, pstate->lastReceivedMessageId);
        EXPECT_EQ(InitiatorState::WaitForSessionReply, pstate->initiatorState);
        EXPECT_TRUE(connsysIR.isResponsibleForInstance(remoteIidR));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Unavailable_upInitFail_WaitForSession_newSessionSendFailStillWaitsForSession)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::Unavailable, remoteIidR, remotePid, &session);

        currentTime = Tp(2000);
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(StoreSessionAndReturn(&session, false));
        fromStack.handleServiceAvailable(remoteIidR);

        const auto* pstate = connsysIR.getParticipantState(remoteIidR);
        EXPECT_EQ(Tp(2000), pstate->lastSentTime);
        EXPECT_EQ(Tp(2000), pstate->lastReceiveTime);
        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(1, pstate->lastSentMessageId);
        EXPECT_EQ(0, pstate->lastReceivedMessageId);
        EXPECT_EQ(InitiatorState::WaitForSessionReply, pstate->initiatorState);
        EXPECT_TRUE(connsysIR.isResponsibleForInstance(remoteIidR));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_WaitForSession_down_Unavailable_handleDown)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::WaitForSessionReply, remoteIidR, remotePid, &session);
        fromStack.handleServiceUnavailable(remoteIidR);
        EXPECT_EQ(InitiatorState::Unavailable, connsysIR.getParticipantState(remoteIidR)->initiatorState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_WaitForSession_confirm_Connected_getsConnectedOnCorrectSession)
    {
        currentTime = Tp(1000);
        uint64_t session = 0;
        initiatorToState(InitiatorState::WaitForSessionReply, remoteIidR, remotePid, &session);

        currentTime = Tp(2000);
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session, 1}, 99, 1, remoteIidR, pid, 0, 0);

        const auto* pstate = connsysIR.getParticipantState(remoteIidR);
        EXPECT_EQ(Tp(2000), pstate->lastReceiveTime);
        EXPECT_EQ(Tp(1000), pstate->lastSentTime);
        EXPECT_EQ(1, pstate->lastReceivedMessageId);
        EXPECT_EQ(1, pstate->lastSentMessageId);
        EXPECT_EQ(InitiatorState::Connected, pstate->initiatorState);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_WaitForSession_wrongMid_WaitForSession_newSessionWhenNotPinfoWithMidOne)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::WaitForSessionReply, remoteIidR, remotePid, &session);

        // keepalive without pinfo
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 1}, 0, true);
        EXPECT_EQ(InitiatorState::WaitForSessionReply, connsysIR.getParticipantState(remoteIidR)->initiatorState);

        // keepalive without pinfo, incrementing
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 1}, 0, false);
        EXPECT_EQ(InitiatorState::WaitForSessionReply, connsysIR.getParticipantState(remoteIidR)->initiatorState);

        // keepalive without pinfo, wrong mid
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 2}, 0, true);
        EXPECT_EQ(InitiatorState::WaitForSessionReply, connsysIR.getParticipantState(remoteIidR)->initiatorState);

        // keepalive error mid
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 0}, 0, true);
        EXPECT_EQ(InitiatorState::WaitForSessionReply, connsysIR.getParticipantState(remoteIidR)->initiatorState);

        // regular message wrong mid
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session, 2}, 123);
        EXPECT_EQ(InitiatorState::WaitForSessionReply, connsysIR.getParticipantState(remoteIidR)->initiatorState);

        // regular message correct mid
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session, 1}, 123);
        EXPECT_EQ(InitiatorState::WaitForSessionReply, connsysIR.getParticipantState(remoteIidR)->initiatorState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_WaitForSession_otherSession_WaitForSession_ignoreUnrelatedSession)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::WaitForSessionReply, remoteIidR, remotePid, &session);
        const ParticipantState pstateBefore = *connsysIR.getParticipantState(remoteIidR);

        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session+1, 1}, 99, 1, remoteIidR, pid, 0, 0);
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session+1, 1}, 99, 2, remoteIidR, pid, 0, 0);
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session+1, 1}, 0, true);
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session+1, 1}, 0, false);
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session+1, 0}, 0, true);
        EXPECT_EQ(nullptr, connsysIR.processReceivedMessageHeader(SomeIPMsgHeader{remotePid, session+1, 1}, "test"));
        EXPECT_EQ(nullptr, connsysIR.processReceivedMessageHeader(SomeIPMsgHeader{remotePid, session+1, 2}, "test"));

        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidR));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_WaitForSession_up_WaitForSession_ignoreUp)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::WaitForSessionReply, remoteIidR, remotePid, &session);
        const ParticipantState pstateBefore = *connsysIR.getParticipantState(remoteIidR);
        fromStack.handleServiceAvailable(remoteIidR);
        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidR));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_WaitForSession_updateTime_sendKeepaliveOnInterval)
    {
        uint64_t session = 0;
        currentTime = Tp(2000);
        initiatorToState(InitiatorState::WaitForSessionReply, remoteIidR, remotePid, &session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidR);
        EXPECT_EQ(Tp(2000), pstate->lastSentTime);

        EXPECT_EQ(Tp(2100), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));

        currentTime = Tp(2099); // not time for keepalive yet
        EXPECT_EQ(Tp(2100), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(Tp(2000), pstate->lastSentTime);

        currentTime = Tp(2100); // exact fit
        EXPECT_CALL(stack, sendKeepAlive(remoteIidR, SomeIPMsgHeader{pid, session, 1}, _, true)).WillOnce(Return(true));
        EXPECT_EQ(Tp(2200), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(Tp(2100), pstate->lastSentTime);

        currentTime = Tp(2250); // a bit late
        EXPECT_CALL(stack, sendKeepAlive(remoteIidR, SomeIPMsgHeader{pid, session, 1}, _, true)).WillOnce(Return(true));
        EXPECT_EQ(Tp(2350), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(Tp(2250), pstate->lastSentTime);
        EXPECT_EQ(1, pstate->lastSentMessageId);
        EXPECT_EQ(Tp(2000), pstate->lastReceiveTime); // untouched
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_WaitForSession_updateTime_ignoresKeepaliveSendFails)
    {
        uint64_t session = 0;
        currentTime = Tp(2000);
        initiatorToState(InitiatorState::WaitForSessionReply, remoteIidR, remotePid, &session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidR);
        EXPECT_EQ(Tp(2000), pstate->lastSentTime);

        currentTime = Tp(2100);
        EXPECT_CALL(stack, sendKeepAlive(remoteIidR, SomeIPMsgHeader{pid, session, 1}, _, true)).WillOnce(Return(false));
        EXPECT_EQ(Tp(2200), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(Tp(2100), pstate->lastSentTime); // still updated to prevent spam
        EXPECT_EQ(InitiatorState::WaitForSessionReply, pstate->initiatorState);

        // will not try to send prematurely again after fail
        currentTime = Tp(2101);
        EXPECT_EQ(Tp(2200), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        currentTime = Tp(2199);
        EXPECT_EQ(Tp(2200), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));

        currentTime = Tp(2200); // regular next time
        EXPECT_CALL(stack, sendKeepAlive(remoteIidR, SomeIPMsgHeader{pid, session, 1}, _, true)).WillOnce(Return(true));
        EXPECT_EQ(Tp(2300), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(Tp(2200), pstate->lastSentTime);
        EXPECT_EQ(1, pstate->lastSentMessageId);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_WaitForSession_updateTime_sendNewSessionOnTimeout)
    {
        uint64_t session = 0;
        currentTime = Tp(2000);
        initiatorToState(InitiatorState::WaitForSessionReply, remoteIidR, remotePid, &session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidR);

        currentTime = Tp(2499); // before timeout
        EXPECT_CALL(stack, sendKeepAlive(remoteIidR, SomeIPMsgHeader{pid, session, 1}, _, true)).WillOnce(Return(true)); // unrelated
        EXPECT_EQ(Tp(2500), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(Tp(2000), pstate->lastReceiveTime);

        currentTime = Tp(2500); // exact match
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, _, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        EXPECT_EQ(Tp(2600), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(Tp(2500), pstate->lastReceiveTime);
        EXPECT_EQ(Tp(2500), pstate->lastSentTime);

        currentTime = Tp(3500); // a bit late, no additional keepalive sent when doing new session anyway
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, _, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        EXPECT_EQ(Tp(3600), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(Tp(3500), pstate->lastReceiveTime);
        EXPECT_EQ(Tp(3500), pstate->lastSentTime);

        // init send fails
        currentTime = Tp(4000);
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, _, 0, 0)).WillOnce(StoreSessionAndReturn(&session, false));
        EXPECT_EQ(Tp(4100), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(Tp(4000), pstate->lastReceiveTime);
        EXPECT_EQ(Tp(4000), pstate->lastSentTime);

        // tries again after timeout
        currentTime = Tp(4500);
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, _, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        EXPECT_EQ(Tp(4600), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(Tp(4500), pstate->lastReceiveTime);
        EXPECT_EQ(Tp(4500), pstate->lastSentTime);

        EXPECT_EQ(InitiatorState::WaitForSessionReply, pstate->initiatorState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Connected_canSendNormal)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::Connected, remoteIidR, remotePid, &session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidR);

        currentTime = Tp(2000);
        EXPECT_CALL(stack, sendTestMessage(remoteIidR, SomeIPMsgHeader{pid, session, 2u}, 456)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendTestMessage(Guid(remotePid), 456));
        EXPECT_EQ(2u, pstate->lastSentMessageId);
        EXPECT_EQ(Tp(2000), pstate->lastSentTime);
        EXPECT_EQ(InitiatorState::Connected, pstate->initiatorState);

        currentTime = Tp(2100);
        EXPECT_CALL(stack, sendTestMessage(remoteIidR, SomeIPMsgHeader{pid, session, 3u}, 789)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->broadcastTestMessage(789));
        EXPECT_EQ(3u, pstate->lastSentMessageId);
        EXPECT_EQ(Tp(2100), pstate->lastSentTime);
        EXPECT_EQ(InitiatorState::Connected, pstate->initiatorState);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Connected_unicastFail_WaitForSessionReply_handleFailedUnicast)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::Connected, remoteIidR, remotePid, &session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidR);

        currentTime = Tp(2000);
        InSequence seq;
        EXPECT_CALL(stack, sendTestMessage(remoteIidR, SomeIPMsgHeader{pid, session, 2u}, 456)).WillOnce(Return(false));
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        EXPECT_FALSE(connsys->sendTestMessage(Guid(remotePid), 456));

        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(1u, pstate->lastSentMessageId);
        EXPECT_EQ(0u, pstate->lastReceivedMessageId);
        EXPECT_EQ(Tp(2000), pstate->lastSentTime);
        EXPECT_EQ(Tp(2000), pstate->lastReceiveTime);
        EXPECT_EQ(InitiatorState::WaitForSessionReply, pstate->initiatorState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Connected_broaadcastFail_WaitForSessionReply_handleFailedBroadcast)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::Connected, remoteIidR, remotePid, &session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidR);

        currentTime = Tp(2100);
        InSequence seq;
        EXPECT_CALL(stack, sendTestMessage(remoteIidR, SomeIPMsgHeader{pid, session, 2u}, 789)).WillOnce(Return(false));
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        EXPECT_TRUE(connsys->broadcastTestMessage(789));

        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(1u, pstate->lastSentMessageId);
        EXPECT_EQ(Tp(2100), pstate->lastSentTime);
        EXPECT_EQ(InitiatorState::WaitForSessionReply, pstate->initiatorState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Connected_receiveNormalUpdatesValues)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::Connected, remoteIidR, remotePid, &session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidR);

        currentTime = Tp(2000);
        EXPECT_CALL(connsys->consumer, handleTestMessage(345, Guid(remotePid)));
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session, 2u}, 345);
        EXPECT_EQ(2u, pstate->lastReceivedMessageId);
        EXPECT_EQ(Tp(2000), pstate->lastReceiveTime);

        currentTime = Tp(2100);
        EXPECT_CALL(connsys->consumer, handleTestMessage(678, Guid(remotePid)));
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session, 3u}, 678);
        EXPECT_EQ(3u, pstate->lastReceivedMessageId);
        EXPECT_EQ(Tp(2100), pstate->lastReceiveTime);

        EXPECT_EQ(InitiatorState::Connected, pstate->initiatorState);
        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Connected_receiveKeepAliveUpdatesValues)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::Connected, remoteIidR, remotePid, &session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidR);

        // incrementing mid
        currentTime = Tp(2000);
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 2u}, 0, false);
        EXPECT_EQ(2u, pstate->lastReceivedMessageId);
        EXPECT_EQ(Tp(2000), pstate->lastReceiveTime);

        // same mid, still update ts
        currentTime = Tp(2100);
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 2u}, 0, true);
        EXPECT_EQ(2u, pstate->lastReceivedMessageId);
        EXPECT_EQ(Tp(2100), pstate->lastReceiveTime);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Connected_sendKeepAliveOnlyWhenNeeded)
    {
        uint64_t session = 0;
        currentTime = Tp(2000);
        initiatorToState(InitiatorState::Connected, remoteIidR, remotePid, &session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidR);

        currentTime = Tp(2050);
        EXPECT_CALL(stack, sendTestMessage(remoteIidR, SomeIPMsgHeader{pid, session, 2u}, 456)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendTestMessage(Guid(remotePid), 456));

        currentTime = Tp(2110); // would send keepalive without normal send
        EXPECT_EQ(Tp(2150), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));

        currentTime = Tp(2200); // now needs keepalive
        EXPECT_CALL(stack, sendKeepAlive(remoteIidR, SomeIPMsgHeader{pid, session, 2u}, _, true)).WillOnce(Return(true));
        EXPECT_EQ(Tp(2300), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));

        currentTime = Tp(2290);
        EXPECT_CALL(stack, sendTestMessage(remoteIidR, SomeIPMsgHeader{pid, session, 3u}, 789)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->broadcastTestMessage(789));

        currentTime = Tp(2300); // would send keepalive without normal broadcast
        EXPECT_EQ(Tp(2390), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));

        currentTime = Tp(2400); // now needs keepalive
        EXPECT_CALL(stack, sendKeepAlive(remoteIidR, SomeIPMsgHeader{pid, session, 3u}, _, true)).WillOnce(Return(true));
        EXPECT_EQ(Tp(2500), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));

        EXPECT_EQ(Tp(2400), pstate->lastSentTime);
        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Connected_staysConnectedWhenSendingKeepAliveFails)
    {
        uint64_t session = 0;
        currentTime = Tp(2000);
        initiatorToState(InitiatorState::Connected, remoteIidR, remotePid, &session);

        const auto* pstate = connsysIR.getParticipantState(remoteIidR);
        EXPECT_EQ(Tp(2000), pstate->lastSentTime);

        currentTime = Tp(2200);
        EXPECT_CALL(stack, sendKeepAlive(remoteIidR, SomeIPMsgHeader{pid, session, 1u}, _, true)).WillOnce(Return(false));
        EXPECT_EQ(Tp(2300), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(Tp(2200), pstate->lastSentTime); // still updates time

        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 1u}, 0, true); // needed to prevent timeout

        currentTime = Tp(2600);
        EXPECT_CALL(stack, sendKeepAlive(remoteIidR, SomeIPMsgHeader{pid, session, 1u}, _, true)).WillOnce(Return(false));
        EXPECT_EQ(Tp(2700), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(Tp(2600), pstate->lastSentTime);

        EXPECT_EQ(InitiatorState::Connected, pstate->initiatorState);
        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Connected_receiveTimeout_WaitForSessionReply_newSessionOnTimeout)
    {
        uint64_t session = 0;
        currentTime = Tp(2000);
        initiatorToState(InitiatorState::Connected, remoteIidR, remotePid, &session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidR);

        currentTime = Tp(2500);
        InSequence seq;
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        EXPECT_EQ(Tp(2600), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));

        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(1u, pstate->lastSentMessageId);
        EXPECT_EQ(Tp(2500), pstate->lastSentTime);
        EXPECT_EQ(InitiatorState::WaitForSessionReply, pstate->initiatorState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Connected_normalMidMismatch_WaitForSessionReply_newSessionOnLostMessage)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::Connected, remoteIidR, remotePid, &session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidR);

        InSequence seq;
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session, 10u}, 678);

        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(1u, pstate->lastSentMessageId);
        EXPECT_EQ(InitiatorState::WaitForSessionReply, pstate->initiatorState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Connected_keepaliveMidMismatch_WaitForSessionReply_newSessionOnLostMessageDetectedByKeepAlive)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::Connected, remoteIidR, remotePid, &session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidR);

        InSequence seq;
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 2u}, 0, true);

        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(1u, pstate->lastSentMessageId);
        EXPECT_EQ(InitiatorState::WaitForSessionReply, pstate->initiatorState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Connected_receivedError_WaitForSessionReply_newSessionOnErrorMessage)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::Connected, remoteIidR, remotePid, &session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidR);

        InSequence seq;
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 0u}, 0, true);

        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(1u, pstate->lastSentMessageId);
        EXPECT_EQ(InitiatorState::WaitForSessionReply, pstate->initiatorState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Connected_down_Unavailable_disconnectOnUnavailable)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::Connected, remoteIidR, remotePid, &session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidR);

        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        fromStack.handleServiceUnavailable(remoteIidR);
        EXPECT_EQ(InitiatorState::Unavailable, pstate->initiatorState);

        fromStack.handleServiceUnavailable(remoteIidR); // no change
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Connected_up_Connected_ignoresDuplicateUp)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::Connected, remoteIidR, remotePid, &session);
        const auto pstateBefore = *connsysIR.getParticipantState(remoteIidR);

        fromStack.handleServiceAvailable(remoteIidR);
        fromStack.handleServiceAvailable(remoteIidR);
        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidR));

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Connected_ignoresUnrelatedSid)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::Connected, remoteIidR, remotePid, &session);
        const ParticipantState pstateBefore = *connsysIR.getParticipantState(remoteIidR);

        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session+1, 1}, 99, 1, remoteIidR, pid, 0, 0);
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session+1, 1}, 99, 2, remoteIidR, pid, 0, 0);
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session+1, 1}, 0, true);
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session+1, 1}, 0, false);
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session+1, 0}, 0, true);
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session+1, 1u}, 345);
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session+1, 2u}, 345);
        EXPECT_EQ(nullptr, connsysIR.processReceivedMessageHeader(SomeIPMsgHeader{remotePid, session+1, 1}, "test"));
        EXPECT_EQ(nullptr, connsysIR.processReceivedMessageHeader(SomeIPMsgHeader{remotePid, session+1, 2}, "test"));

        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidR));
        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Connected_processReceivedMessageHeaderDifferentiatesBetweenNotResponsibleAndError)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::Connected, remoteIidR, remotePid, &session);

        // receive from unknown fails with not responsible
        EXPECT_EQ(absl::nullopt, connsysIR.processReceivedMessageHeader(SomeIPMsgHeader{remotePid+100, session, 2u}, "test"));
        EXPECT_EQ(nullptr, connsysIR.processReceivedMessageHeader(SomeIPMsgHeader{remotePid, 0u, 0u}, "test").value());

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, sendToUnknownFails)
    {
        EXPECT_FALSE(connsysIR.sendUnicast("test", Guid(remotePid+100), [](const auto& /*iid*/, const auto& /*header*/) {
            throw std::exception();
            return true;
        }));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Connected_pinfo_WaitForSessionReply_pinfoInConnectIsProtocolViolation)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::Connected, remoteIidR, remotePid, &session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidR);

        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session, 2u}, 99, 1, remoteIidR, pid, 0, 0); // correct sid+mid but still wrong

        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(1u, pstate->lastSentMessageId);
        EXPECT_EQ(InitiatorState::WaitForSessionReply, pstate->initiatorState);
    }

    TEST_F(AConnectionSystemInitiatorResponder, I_Connected_disconnectNotifyerOnDisconnectConnSys)
    {
        connect();  // manual connect and disconnect

        uint64_t session = 0;
        initiatorToState(InitiatorState::Connected, remoteIidR, remotePid, &session);

        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));

        disconnect();
        connsysIR.disconnect();  // does nothing
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, canBroadcastToMultiple)
    {
        uint64_t session_a = 0;
        uint64_t remotePid_a{1};
        TestInstanceId remoteIid_a{1};
        initiatorToState(InitiatorState::Connected, remoteIid_a, remotePid_a, &session_a);
        const auto* pstate_a = connsysIR.getParticipantState(remoteIid_a);

        uint64_t session_b = 0;
        uint64_t remotePid_b{2};
        TestInstanceId remoteIid_b{2};
        initiatorToState(InitiatorState::Connected, remoteIid_b, remotePid_b, &session_b);
        const auto* pstate_b = connsysIR.getParticipantState(remoteIid_b);

        // not connected, should not get anything
        uint64_t session_c = 0;
        uint64_t remotePid_c{3};
        TestInstanceId remoteIid_c{3};
        initiatorToState(InitiatorState::WaitForSessionReply, remoteIid_c, remotePid_c, &session_c);
        const auto* pstate_c = connsysIR.getParticipantState(remoteIid_c);

        const uint64_t session_d = 1234;
        uint64_t remotePid_d{10};
        TestInstanceId remoteIid_d{10};
        responderToState(ResponderState::Connected, remoteIid_d, remotePid_d, session_d);
        const auto* pstate_d = connsysIR.getParticipantState(remoteIid_d);

        // send message in b only to get different mids
        EXPECT_CALL(stack, sendTestMessage(remoteIid_b, SomeIPMsgHeader{pid, session_b, 2u}, 456)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendTestMessage(Guid(remotePid_b), 456));

        currentTime = Tp(2000);
        EXPECT_CALL(stack, sendTestMessage(remoteIid_a, SomeIPMsgHeader{pid, session_a, 2u}, 123)).WillOnce(Return(true));
        EXPECT_CALL(stack, sendTestMessage(remoteIid_b, SomeIPMsgHeader{pid, session_b, 3u}, 123)).WillOnce(Return(true));
        EXPECT_CALL(stack, sendTestMessage(remoteIid_d, SomeIPMsgHeader{pid, session_d, 2u}, 123)).WillOnce(Return(true));
        connsys->broadcastTestMessage(123);

        EXPECT_EQ(2u, pstate_a->lastSentMessageId);
        EXPECT_EQ(Tp(2000), pstate_a->lastSentTime);
        EXPECT_EQ(3u, pstate_b->lastSentMessageId);
        EXPECT_EQ(Tp(2000), pstate_b->lastSentTime);
        EXPECT_EQ(*pstate_c, *connsysIR.getParticipantState(remoteIid_c)); // fully unchanged
        EXPECT_EQ(2u, pstate_d->lastSentMessageId);
        EXPECT_EQ(Tp(2000), pstate_d->lastSentTime);

        expectRemoteDisconnects({remotePid_a, remotePid_b, remotePid_d});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, canBroadcastToMultipleAndOneFails)
    {
        uint64_t session_a = 0;
        uint64_t remotePid_a{1};
        TestInstanceId remoteIid_a{1};
        initiatorToState(InitiatorState::Connected, remoteIid_a, remotePid_a, &session_a);
        const auto* pstate_a = connsysIR.getParticipantState(remoteIid_a);

        uint64_t session_b = 0;
        uint64_t remotePid_b{2};
        TestInstanceId remoteIid_b{2};
        initiatorToState(InitiatorState::Connected, remoteIid_b, remotePid_b, &session_b);
        const auto* pstate_b = connsysIR.getParticipantState(remoteIid_b);

        EXPECT_CALL(stack, sendTestMessage(remoteIid_a, SomeIPMsgHeader{pid, session_a, 2u}, 123)).WillOnce(Return(false));
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid_a)));
        EXPECT_CALL(stack, sendParticipantInfo(remoteIid_a, ValidHdr(pid, 1u), 99, 1, iid, remotePid_a, 0, 0)).WillOnce(Return(true));

        EXPECT_CALL(stack, sendTestMessage(remoteIid_b, SomeIPMsgHeader{pid, session_b, 2u}, 123)).WillOnce(Return(true));
        connsys->broadcastTestMessage(123);

        EXPECT_EQ(InitiatorState::WaitForSessionReply, pstate_a->initiatorState);
        EXPECT_EQ(1u, pstate_a->lastSentMessageId);
        EXPECT_EQ(InitiatorState::Connected, pstate_b->initiatorState);
        EXPECT_EQ(2u, pstate_b->lastSentMessageId);

        expectRemoteDisconnects({remotePid_b});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, threadLoopNextWakeUpReturnsEarliestTime)
    {
        currentTime = Tp(2000);
        uint64_t session_a = 0;
        uint64_t remotePid_a{1};
        TestInstanceId remoteIid_a{1};
        initiatorToState(InitiatorState::Connected, remoteIid_a, remotePid_a, &session_a);
        const auto* pstate_a = connsysIR.getParticipantState(remoteIid_a);

        currentTime = Tp(2010);
        uint64_t session_b = 0;
        uint64_t remotePid_b{2};
        TestInstanceId remoteIid_b{2};
        initiatorToState(InitiatorState::Connected, remoteIid_b, remotePid_b, &session_b);
        const auto* pstate_b = connsysIR.getParticipantState(remoteIid_b);

        // next wakup for keepalive is from a
        EXPECT_EQ(Tp(2100), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));

        // only send for a and make b earliest
        currentTime = Tp(2100);
        EXPECT_CALL(stack, sendKeepAlive(remoteIid_a, SomeIPMsgHeader{pid, session_a, 1u}, _, true)).WillOnce(Return(true));
        EXPECT_EQ(Tp(2110), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(Tp(2100), pstate_a->lastSentTime);
        EXPECT_EQ(Tp(2010), pstate_b->lastSentTime);

        // make a send and receive data short before b receive timeout at 2510
        currentTime = Tp(2500);
        EXPECT_CALL(stack, sendTestMessage(remoteIid_a, SomeIPMsgHeader{pid, session_a, 2u}, 456)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendTestMessage(Guid(remotePid_a), 456));
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid_a, session_a, 1u}, 0, true);
        EXPECT_EQ(Tp(2500), pstate_a->lastSentTime);
        EXPECT_EQ(Tp(2500), pstate_a->lastReceiveTime);

        // make b send too
        EXPECT_CALL(stack, sendTestMessage(remoteIid_b, SomeIPMsgHeader{pid, session_b, 2u}, 456)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendTestMessage(Guid(remotePid_b), 456));
        EXPECT_EQ(Tp(2500), pstate_b->lastSentTime);
        EXPECT_EQ(Tp(2010), pstate_b->lastReceiveTime);

        // next event is b receive timeout
        EXPECT_EQ(Tp(2510), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));

        expectRemoteDisconnects({remotePid_a, remotePid_b});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, broadcastToOldAndNew)
    {
        uint64_t session_a = 0;
        uint64_t remotePid_a{1};
        TestInstanceId remoteIid_a{1};
        initiatorToState(InitiatorState::Connected, remoteIid_a, remotePid_a, &session_a);

        uint64_t session_b = 0;
        uint64_t remotePid_b{2};
        TestInstanceId remoteIid_b{2};
        connectRemoteBaseOnly(remoteIid_b, Guid(remotePid_b), 123, &session_b);

        EXPECT_CALL(stack, sendTestMessage(remoteIid_a, SomeIPMsgHeader{pid, session_a, 2u}, 123)).WillOnce(Return(true));
        EXPECT_CALL(stack, sendTestMessage(remoteIid_b, SomeIPMsgHeader{pid, session_b, 2u}, 123)).WillOnce(Return(true));
        connsys->broadcastTestMessage(123);

        expectRemoteDisconnects({remotePid_a, remotePid_b});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, instanceBecomesUnavailableInOldWhenHandledByNew)
    {
        EXPECT_FALSE(connsys->isInstanceAvailable(remoteIidR));
        EXPECT_FALSE(connsysIR.isInstanceAvailable(remoteIidR));

        uint64_t session = 0;
        currentTime = Tp(2000);
        initiatorToState(InitiatorState::WaitForSessionReply, remoteIidR, remotePid, &session);

        EXPECT_TRUE(connsys->isInstanceAvailable(remoteIidR));
        EXPECT_TRUE(connsysIR.isInstanceAvailable(remoteIidR));

        fromStack.handleServiceUnavailable(remoteIidR);

        EXPECT_FALSE(connsys->isInstanceAvailable(remoteIidR));
        EXPECT_FALSE(connsysIR.isInstanceAvailable(remoteIidR));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_WaitForSession_sendsMultipleInitSessionsGetsAnswerToAllLater)
    {
        uint64_t session_1 = 0;
        uint64_t session_2 = 0;
        uint64_t session_3 = 0;

        currentTime = Tp(2000);
        initiatorToState(InitiatorState::WaitForSessionReply, remoteIidR, remotePid, &session_1);

        currentTime = Tp(2500);
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, _, 0, 0)).WillOnce(StoreSessionAndReturn(&session_2, true));
        connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500});

        currentTime = Tp(3000);
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, _, 0, 0)).WillOnce(StoreSessionAndReturn(&session_3, true));
        connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500});

        // answer them all
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session_1, 1}, 99, 1, remoteIidR, pid, 0, 0);
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session_2, 1}, 99, 1, remoteIidR, pid, 0, 0);
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid))); // finally a matching one
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session_3, 1}, 99, 1, remoteIidR, pid, 0, 0);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_WaitForSession_sendFails)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::WaitForSessionReply, remoteIidR, remotePid, &session);
        const auto pstateBefore = *connsysIR.getParticipantState(remoteIidR);

        EXPECT_FALSE(connsys->sendTestMessage(Guid(remotePid), 456));
        EXPECT_TRUE(connsys->broadcastTestMessage(789));

        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidR));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Invalid_upDown_Invalid_storeEntryOnly)
    {
        EXPECT_FALSE(connsysIR.isInstanceAvailable(remoteIidI));
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidI, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(remoteIidI);

        EXPECT_TRUE(connsysIR.isInstanceAvailable(remoteIidI));
        EXPECT_FALSE(connsysIR.isResponsibleForInstance(remoteIidI));

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        ASSERT_TRUE(pstate);
        EXPECT_EQ(remoteIidI, pstate->remoteIid);
        EXPECT_FALSE(pstate->remotePid.isValid());
        EXPECT_FALSE(pstate->selfIsInitiator);
        EXPECT_EQ(Tp(0), pstate->lastSentTime);
        EXPECT_EQ(Tp(0), pstate->lastReceiveTime);
        EXPECT_EQ(0, pstate->activeSessionId);
        EXPECT_EQ(0, pstate->lastSentMessageId);
        EXPECT_EQ(0, pstate->lastReceivedMessageId);
        EXPECT_EQ(InitiatorState::Invalid, pstate->initiatorState);
        EXPECT_EQ(ResponderState::Invalid, pstate->responderState);

        fromStack.handleServiceUnavailable(remoteIidI);

        EXPECT_FALSE(connsysIR.isInstanceAvailable(remoteIidI));
        EXPECT_FALSE(connsysIR.isResponsibleForInstance(remoteIidI));
        ASSERT_EQ(pstate, connsysIR.getParticipantState(remoteIidI));
        EXPECT_EQ(remoteIidI, pstate->remoteIid);
        EXPECT_FALSE(pstate->remotePid.isValid());
        EXPECT_EQ(InitiatorState::Invalid, pstate->initiatorState);
        EXPECT_EQ(ResponderState::Invalid, pstate->responderState);

        fromStack.handleServiceUnavailable(remoteIidI); // do nothing
        EXPECT_FALSE(connsysIR.isInstanceAvailable(remoteIidI));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Invalid_upPinfoReply_Connected_directReplyAndConnectOnPinfo)
    {
        currentTime = Tp(2);
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidI, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(remoteIidI);

        const uint64_t session = 123;
        currentTime = Tp(3);
        InSequence seq;
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidI, SomeIPMsgHeader{pid, session, 1u}, 99, 1, iid, remotePid, 0, 0)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session, 1}, 99, 1, remoteIidI, pid, 0, 0);

        EXPECT_TRUE(connsysIR.isResponsibleForInstance(remoteIidI));
        EXPECT_TRUE(connsysIR.isResponsibleForParticipant(Guid(remotePid)));

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        ASSERT_TRUE(pstate);
        EXPECT_EQ(remoteIidI, pstate->remoteIid);
        EXPECT_EQ(Guid(remotePid), pstate->remotePid);
        EXPECT_FALSE(pstate->selfIsInitiator);
        EXPECT_EQ(Tp(3), pstate->lastSentTime);
        EXPECT_EQ(Tp(3), pstate->lastReceiveTime);
        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(1, pstate->lastSentMessageId);
        EXPECT_EQ(1, pstate->lastReceivedMessageId);
        EXPECT_EQ(InitiatorState::Invalid, pstate->initiatorState);
        EXPECT_EQ(ResponderState::Connected, pstate->responderState);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Invalid_upPinfoReplyFail_WaitForSession_waitForNewSessionWhenReplyFails)
    {
        currentTime = Tp(2);
        // will not use anything from this sent out pinfo
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidI, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(remoteIidI);

        const uint64_t session = 123;
        currentTime = Tp(3);
        InSequence seq;
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidI, SomeIPMsgHeader{pid, session, 1u}, 99, 1, iid, remotePid, 0, 0)).WillOnce(Return(false));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session, 1}, 99, 1, remoteIidI, pid, 0, 0);

        EXPECT_TRUE(connsysIR.isResponsibleForInstance(remoteIidI));
        EXPECT_TRUE(connsysIR.isResponsibleForParticipant(Guid(remotePid)));

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        ASSERT_TRUE(pstate);
        EXPECT_EQ(remoteIidI, pstate->remoteIid);
        EXPECT_EQ(Guid(remotePid), pstate->remotePid);
        EXPECT_FALSE(pstate->selfIsInitiator);
        EXPECT_EQ(Tp(0), pstate->lastSentTime);
        EXPECT_EQ(Tp(3), pstate->lastReceiveTime);
        EXPECT_EQ(0u, pstate->activeSessionId);
        EXPECT_EQ(1, pstate->lastSentMessageId);
        EXPECT_EQ(1, pstate->lastReceivedMessageId);
        EXPECT_EQ(InitiatorState::Invalid, pstate->initiatorState);
        EXPECT_EQ(ResponderState::WaitForSession, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Invalid_Pinfo_WaitForUp_storeSessionAndWaitForUp)
    {
        currentTime = Tp(3);
        const uint64_t session = 123;
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session, 1}, 99, 1, remoteIidI, 0, 0, 0);

        EXPECT_FALSE(connsysIR.isResponsibleForInstance(remoteIidI)); // not yet because not up
        EXPECT_TRUE(connsysIR.isResponsibleForParticipant(Guid(remotePid)));

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        ASSERT_TRUE(pstate);
        EXPECT_EQ(remoteIidI, pstate->remoteIid);
        EXPECT_EQ(Guid(remotePid), pstate->remotePid);
        EXPECT_FALSE(pstate->selfIsInitiator);
        EXPECT_EQ(Tp(0), pstate->lastSentTime);
        EXPECT_EQ(Tp(3), pstate->lastReceiveTime);
        EXPECT_EQ(session, pstate->activeSessionId); // received session stored for later reply
        EXPECT_EQ(0, pstate->lastSentMessageId);
        EXPECT_EQ(1, pstate->lastReceivedMessageId);
        EXPECT_EQ(InitiatorState::Invalid, pstate->initiatorState);
        EXPECT_EQ(ResponderState::WaitForUp, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForUp_pinfo_WaitForUp_updatesValuesForValidPinfo)
    {
        currentTime = Tp(2);
        const uint64_t session = 123;
        responderToState(ResponderState::WaitForUp, remoteIidI, remotePid, session);

        currentTime = Tp(4);
        const uint64_t newSession = 456;
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, newSession, 1}, 99, 1, remoteIidI, 0, 0, 0);

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(remoteIidI, pstate->remoteIid);
        EXPECT_EQ(Guid(remotePid), pstate->remotePid);
        EXPECT_FALSE(pstate->selfIsInitiator);
        EXPECT_EQ(Tp(0), pstate->lastSentTime);
        EXPECT_EQ(Tp(4), pstate->lastReceiveTime);
        EXPECT_EQ(newSession, pstate->activeSessionId);
        EXPECT_EQ(0, pstate->lastSentMessageId);
        EXPECT_EQ(1, pstate->lastReceivedMessageId);
        EXPECT_EQ(ResponderState::WaitForUp, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForUp_keepalive_WaitForUp_updatesValuesForValidKeepAlive)
    {
        currentTime = Tp(2);
        const uint64_t session = 123;
        responderToState(ResponderState::WaitForUp, remoteIidI, remotePid, session);

        currentTime = Tp(4);
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 1}, 0, true);

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(remoteIidI, pstate->remoteIid);
        EXPECT_EQ(Guid(remotePid), pstate->remotePid);
        EXPECT_FALSE(pstate->selfIsInitiator);
        EXPECT_EQ(Tp(0), pstate->lastSentTime);
        EXPECT_EQ(Tp(4), pstate->lastReceiveTime);
        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(0, pstate->lastSentMessageId);
        EXPECT_EQ(1, pstate->lastReceivedMessageId);
        EXPECT_EQ(ResponderState::WaitForUp, pstate->responderState);

        currentTime = Tp(5);
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 2}, 0, false); // inc mid
        EXPECT_EQ(Tp(5), pstate->lastReceiveTime);
        EXPECT_EQ(2, pstate->lastReceivedMessageId);
        EXPECT_EQ(ResponderState::WaitForUp, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForUp_keepaliveBadSid_Unavailable_forgetSessionOnBadKeepAliveSid)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::WaitForUp, remoteIidI, remotePid, session);

        const uint64_t newSession = 456;
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, newSession, 1}, 0, true);

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(0, pstate->activeSessionId);
        EXPECT_EQ(ResponderState::Unavailable, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForUp_keepaliveBadMid_Unavailable_forgetSessionOnBadKeepAliveMid)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::WaitForUp, remoteIidI, remotePid, session);

        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 2}, 0, true);

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(0, pstate->activeSessionId);
        EXPECT_EQ(ResponderState::Unavailable, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForUp_normalReceive_Unavailable_receiveNeverAllowedinState)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::WaitForUp, remoteIidI, remotePid, session);

        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session, 2}, 123);

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(ResponderState::Unavailable, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForUp_down_WaitForUp_ignoreDuplicateDown)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::WaitForUp, remoteIidI, remotePid, session);
        const auto pstateBefore = *connsysIR.getParticipantState(remoteIidI);

        fromStack.handleServiceUnavailable(remoteIidI);
        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidI));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForUp_sendFails)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::WaitForUp, remoteIidI, remotePid, session);
        const auto pstateBefore = *connsysIR.getParticipantState(remoteIidI);

        EXPECT_FALSE(connsys->sendTestMessage(Guid(remotePid), 456));
        EXPECT_TRUE(connsys->broadcastTestMessage(789));

        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidI));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForUp_up_Connected_directConnect)
    {
        currentTime = Tp(2);
        const uint64_t session = 123;
        responderToState(ResponderState::WaitForUp, remoteIidI, remotePid, session);

        currentTime = Tp(3);
        InSequence seq;
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidI, SomeIPMsgHeader{pid, session, 1u}, 99, 1, iid, remotePid, 0, 0)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid)));
        fromStack.handleServiceAvailable(remoteIidI);

        EXPECT_TRUE(connsysIR.isResponsibleForInstance(remoteIidI));
        EXPECT_TRUE(connsysIR.isResponsibleForParticipant(Guid(remotePid)));

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        ASSERT_TRUE(pstate);
        EXPECT_EQ(remoteIidI, pstate->remoteIid);
        EXPECT_EQ(Guid(remotePid), pstate->remotePid);
        EXPECT_FALSE(pstate->selfIsInitiator);
        EXPECT_EQ(Tp(3), pstate->lastSentTime);
        EXPECT_EQ(Tp(2), pstate->lastReceiveTime);
        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(1, pstate->lastSentMessageId);
        EXPECT_EQ(1, pstate->lastReceivedMessageId);
        EXPECT_EQ(InitiatorState::Invalid, pstate->initiatorState);
        EXPECT_EQ(ResponderState::Connected, pstate->responderState);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForUp_timeout_Unavailable_preventDirectConnectWhenPinfoOutdated)
    {
        currentTime = Tp(2000);
        const uint64_t session = 123;
        responderToState(ResponderState::WaitForUp, remoteIidI, remotePid, session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidI);

        currentTime = Tp(2499); // not yet timeout
        EXPECT_EQ(Tp(2500), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(ResponderState::WaitForUp, pstate->responderState);

        currentTime = Tp(2500);
        EXPECT_EQ(Tp(2600), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(ResponderState::Unavailable, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForUp_extraPinfoUp_Connected_usesNewValuesWhenReceivingExtraPinfo)
    {
        currentTime = Tp(2);
        responderToState(ResponderState::WaitForUp, remoteIidI, remotePid, 22);

        currentTime = Tp(4);
        const uint64_t session = 123;
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session, 1}, 99, 1, remoteIidI, 0, 0, 0);

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(Tp(0), pstate->lastSentTime);
        EXPECT_EQ(Tp(4), pstate->lastReceiveTime);
        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(0, pstate->lastSentMessageId);
        EXPECT_EQ(1, pstate->lastReceivedMessageId);
        EXPECT_EQ(ResponderState::WaitForUp, pstate->responderState);

        EXPECT_CALL(stack, sendParticipantInfo(remoteIidI, SomeIPMsgHeader{pid, session, 1u}, 99, 1, iid, remotePid, 0, 0)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid)));
        fromStack.handleServiceAvailable(remoteIidI);

        EXPECT_EQ(ResponderState::Connected, pstate->responderState);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForUp_extraKeepalive_WaitForUp_preventTimeoutByKeepAlives)
    {
        currentTime = Tp(2000);
        const uint64_t session = 123;
        responderToState(ResponderState::WaitForUp, remoteIidI, remotePid, session);

        currentTime = Tp(2400);
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 1}, 0, true);

        currentTime = Tp(2600); // would have receive timeout
        connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500});

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(Tp(2400), pstate->lastReceiveTime);
        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(1, pstate->lastReceivedMessageId);
        EXPECT_EQ(ResponderState::WaitForUp, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Unavailable_up_WaitForSession)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::Unavailable, remoteIidI, remotePid, session);

        fromStack.handleServiceAvailable(remoteIidI);

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(0, pstate->activeSessionId);
        EXPECT_EQ(ResponderState::WaitForSession, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Unavailable_down_Unavailable_ignoresDown)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::Unavailable, remoteIidI, remotePid, session);
        const ParticipantState pstateBefore = *connsysIR.getParticipantState(remoteIidI);

        fromStack.handleServiceUnavailable(remoteIidI);
        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidI));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Unavailable_keepalive_Unavailable_ignoresKeepalive)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::Unavailable, remoteIidI, remotePid, session);
        const ParticipantState pstateBefore = *connsysIR.getParticipantState(remoteIidI);

        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 0}, 0, true);
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 1}, 0, true);
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 2}, 0, true);

        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidI));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Unavailable_timeout_Unavailable_timeoutDoesNothing)
    {
        currentTime = Tp(2000);
        const uint64_t session = 123;
        responderToState(ResponderState::Unavailable, remoteIidI, remotePid, session);
        const ParticipantState pstateBefore = *connsysIR.getParticipantState(remoteIidI);

        currentTime = Tp(3000);
        connsys->doOneThreadLoop(std::chrono::milliseconds{10}, std::chrono::milliseconds{50});

        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidI));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Unavailable_sendFails)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::Unavailable, remoteIidI, remotePid, session);
        const auto pstateBefore = *connsysIR.getParticipantState(remoteIidI);

        EXPECT_FALSE(connsys->sendTestMessage(Guid(remotePid), 456));
        EXPECT_TRUE(connsys->broadcastTestMessage(789));

        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidI));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Unavailable_receiveFailsInState)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::Unavailable, remoteIidI, remotePid, session);
        const auto pstateBefore = *connsysIR.getParticipantState(remoteIidI);

        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session, 2}, 123);

        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidI));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Unavailable_pinfoGood_WaitForUp)
    {
        currentTime = Tp(2);
        responderToState(ResponderState::Unavailable, remoteIidI, remotePid, 22);

        currentTime = Tp(4);
        const uint64_t session = 123;
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session, 1}, 99, 1, remoteIidI, 0, 0, 0);

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(Tp(0), pstate->lastSentTime);
        EXPECT_EQ(Tp(4), pstate->lastReceiveTime);
        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(0, pstate->lastSentMessageId);
        EXPECT_EQ(1, pstate->lastReceivedMessageId);
        EXPECT_EQ(ResponderState::WaitForUp, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForSession_up_WaitForSession_ignoreRedundantUp)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::WaitForSession, remoteIidI, remotePid, session);
        const auto pstateBefore = *connsysIR.getParticipantState(remoteIidI);

        fromStack.handleServiceAvailable(remoteIidI);

        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidI));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForSession_down_Unavailable)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::WaitForSession, remoteIidI, remotePid, session);

        fromStack.handleServiceUnavailable(remoteIidI);
        EXPECT_EQ(ResponderState::Unavailable, connsysIR.getParticipantState(remoteIidI)->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForSession_keepalive_WaitForSession_sendsErrorAndStaysInState)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::WaitForSession, remoteIidI, remotePid, session);
        const ParticipantState pstateBefore = *connsysIR.getParticipantState(remoteIidI);

        // old session different mids
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 0}, _, _)).WillOnce(Return(true)); // error
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 0}, 0, true);
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 0}, _, _)).WillOnce(Return(true)); // error
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 1}, 0, true);
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 0}, _, _)).WillOnce(Return(true)); // error
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 2}, 0, true);

        // error send fail does not change state
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 0}, _, _)).WillOnce(Return(false)); // error fails
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 2}, 0, true);

        // other session replies with received session
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, 23, 0}, _, _)).WillOnce(Return(true)); // error
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, 23, 1}, 0, true);

        // error send fail does not change state
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, 23, 0}, _, _)).WillOnce(Return(false)); // error fails
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, 23, 2}, 0, true);

        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidI));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForSession_sendFails)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::WaitForSession, remoteIidI, remotePid, session);
        const auto pstateBefore = *connsysIR.getParticipantState(remoteIidI);

        EXPECT_FALSE(connsys->sendTestMessage(Guid(remotePid), 456));
        EXPECT_TRUE(connsys->broadcastTestMessage(789));

        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidI));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForSession_receiveNormal_WaitForSession_sendErrorAndStaysInState)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::WaitForSession, remoteIidI, remotePid, session);
        const auto pstateBefore = *connsysIR.getParticipantState(remoteIidI);

        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 0}, _, _)).WillOnce(Return(true)); // error
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session, 2}, 123);
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 0}, _, _)).WillOnce(Return(false)); //error fails
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session, 2}, 123);

        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, 43, 0}, _, _)).WillOnce(Return(true)); // error
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, 43, 2}, 123);
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, 43, 0}, _, _)).WillOnce(Return(false)); //error fails
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, 43, 2}, 123);

        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidI));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForSession_timeout_WaitForSession_timeoutDoesNothing)
    {
        currentTime = Tp(2000);
        const uint64_t session = 123;
        responderToState(ResponderState::WaitForSession, remoteIidI, remotePid, session);
        const ParticipantState pstateBefore = *connsysIR.getParticipantState(remoteIidI);

        currentTime = Tp(3000);
        connsys->doOneThreadLoop(std::chrono::milliseconds{10}, std::chrono::milliseconds{50});

        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidI));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForSession_pinfoGood_Connected_getsConnectedWhenSendReplyOk)
    {
        currentTime = Tp(2);
        responderToState(ResponderState::WaitForSession, remoteIidI, remotePid, 22);

        currentTime = Tp(4);
        const uint64_t session = 123;
        InSequence seq;
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidI, SomeIPMsgHeader{pid, session, 1u}, 99, 1, iid, remotePid, 0, 0)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session, 1}, 99, 1, remoteIidI, 0, 0, 0);

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(Tp(4), pstate->lastSentTime);
        EXPECT_EQ(Tp(4), pstate->lastReceiveTime);
        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(1, pstate->lastSentMessageId);
        EXPECT_EQ(1, pstate->lastReceivedMessageId);
        EXPECT_EQ(ResponderState::Connected, pstate->responderState);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForSession_pinfoGoodReplyFail_WaitForSession)
    {
        currentTime = Tp(2);
        responderToState(ResponderState::WaitForSession, remoteIidI, remotePid, 22);

        currentTime = Tp(4);
        const uint64_t session = 123;
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidI, SomeIPMsgHeader{pid, session, 1u}, 99, 1, iid, remotePid, 0, 0)).WillOnce(Return(false));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session, 1}, 99, 1, remoteIidI, 0, 0, 0);

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(Tp(0), pstate->lastSentTime);
        EXPECT_EQ(Tp(4), pstate->lastReceiveTime);
        EXPECT_EQ(0, pstate->activeSessionId);
        EXPECT_EQ(1, pstate->lastSentMessageId);
        EXPECT_EQ(1, pstate->lastReceivedMessageId);
        EXPECT_EQ(ResponderState::WaitForSession, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Connected_up_Connected_ignoreRedundantUp)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::Connected, remoteIidI, remotePid, session);
        const auto pstateBefore = *connsysIR.getParticipantState(remoteIidI);

        fromStack.handleServiceAvailable(remoteIidI);
        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidI));

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Connected_down_Unavailable)
    {
        responderToState(ResponderState::Connected, remoteIidI, remotePid, 22);

        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        fromStack.handleServiceUnavailable(remoteIidI);

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(ResponderState::Unavailable, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Connected_pinfoGood_Connected_disconnectConnectOnNewSession)
    {
        currentTime = Tp(2);
        responderToState(ResponderState::Connected, remoteIidI, remotePid, 23);

        currentTime = Tp(4);
        const uint64_t session = 123;
        InSequence seq;
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidI, SomeIPMsgHeader{pid, session, 1u}, 99, 1, iid, remotePid, 0, 0)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session, 1}, 99, 1, remoteIidI, remotePid, 0, 0);

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(Tp(4), pstate->lastSentTime);
        EXPECT_EQ(Tp(4), pstate->lastReceiveTime);
        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(1, pstate->lastSentMessageId);
        EXPECT_EQ(1, pstate->lastReceivedMessageId);
        EXPECT_EQ(ResponderState::Connected, pstate->responderState);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Connected_keepAliveOk_Connected_preventTimeoutByKeepAlives)
    {
        currentTime = Tp(2000);
        const uint64_t session = 123;
        responderToState(ResponderState::Connected, remoteIidI, remotePid, session);

        currentTime = Tp(2400);
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 1}, 0, true);

        currentTime = Tp(2600); // would have receive timeout
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 1}, _, true)).WillOnce(Return(true)); // unrelated
        connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500});

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(Tp(2400), pstate->lastReceiveTime);
        EXPECT_EQ(session, pstate->activeSessionId);
        EXPECT_EQ(1, pstate->lastReceivedMessageId);
        EXPECT_EQ(ResponderState::Connected, pstate->responderState);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Connected_keepAliveBadMid_WaitForSession_disconnectAndErrorOnBadKeepaliveMid)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::Connected, remoteIidI, remotePid, session);

        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 0}, _, _)).WillOnce(Return(true)); // error
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 5}, 0, true);

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(0, pstate->activeSessionId);
        EXPECT_EQ(ResponderState::WaitForSession, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Connected_keepAliveBadSid_Connected_sendsErrorForBadSidButStaysConnected)
    {
        responderToState(ResponderState::Connected, remoteIidI, remotePid, 22);

        const auto pstateBefore = *connsysIR.getParticipantState(remoteIidI);

        const uint64_t session = 123;
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 0}, _, _)).WillOnce(Return(true)); // error
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 1}, 0, true);

        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidI));
        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Connected_keepAliveBadSid_Connected_sendErrorForBadSidFailsButStaysConnected)
    {
        responderToState(ResponderState::Connected, remoteIidI, remotePid, 22);

        const auto pstateBefore = *connsysIR.getParticipantState(remoteIidI);

        const uint64_t session = 123;
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 0}, _, _)).WillOnce(Return(false)); // error
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 1}, 0, true);

        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidI));
        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Connected_sendKeepaliveOnInterval)
    {
        const uint64_t session = 234;
        currentTime = Tp(2000);
        responderToState(ResponderState::Connected, remoteIidI, remotePid, session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(Tp(2000), pstate->lastSentTime);

        EXPECT_EQ(Tp(2100), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));

        currentTime = Tp(2099); // not time for keepalive yet
        EXPECT_EQ(Tp(2100), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(Tp(2000), pstate->lastSentTime);

        currentTime = Tp(2100); // exact fit
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 1}, _, true)).WillOnce(Return(true));
        EXPECT_EQ(Tp(2200), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(Tp(2100), pstate->lastSentTime);

        currentTime = Tp(2250); // a bit late
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 1}, _, true)).WillOnce(Return(true));
        EXPECT_EQ(Tp(2350), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(Tp(2250), pstate->lastSentTime);
        EXPECT_EQ(1, pstate->lastSentMessageId);
        EXPECT_EQ(Tp(2000), pstate->lastReceiveTime); // untouched
        EXPECT_EQ(ResponderState::Connected, pstate->responderState);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Connected_ignoresKeepaliveSendFails)
    {
        const uint64_t session = 234;
        currentTime = Tp(2000);
        responderToState(ResponderState::Connected, remoteIidI, remotePid, session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(Tp(2000), pstate->lastSentTime);

        currentTime = Tp(2100);
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 1}, _, true)).WillOnce(Return(false));
        EXPECT_EQ(Tp(2200), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(Tp(2100), pstate->lastSentTime); // still updated to prevent spam
        EXPECT_EQ(ResponderState::Connected, pstate->responderState);

        // will not try to send prematurely again after fail
        currentTime = Tp(2101);
        EXPECT_EQ(Tp(2200), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        currentTime = Tp(2199);
        EXPECT_EQ(Tp(2200), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));

        currentTime = Tp(2200); // regular next time
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 1}, _, true)).WillOnce(Return(true));
        EXPECT_EQ(Tp(2300), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(Tp(2200), pstate->lastSentTime);
        EXPECT_EQ(1, pstate->lastSentMessageId);
        EXPECT_EQ(ResponderState::Connected, pstate->responderState);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Connected_receiveTimeout_WaitForSession_sendErrorOnTimeout)
    {
        const uint64_t session = 123;
        currentTime = Tp(2000);
        responderToState(ResponderState::Connected, remoteIidI, remotePid, session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidI);

        currentTime = Tp(2499); // before timeout
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 1}, _, true)).WillOnce(Return(true)); // unrelated
        EXPECT_EQ(Tp(2500), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));
        EXPECT_EQ(Tp(2000), pstate->lastReceiveTime);

        currentTime = Tp(2500); // exact match
        InSequence seq;
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 0}, _, true)).WillOnce(Return(true)); // error
        EXPECT_EQ(Tp(2600), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));

        EXPECT_EQ(0, pstate->activeSessionId);
        EXPECT_EQ(ResponderState::WaitForSession, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Connected_receiveKeepAliveUpdatesValues)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::Connected, remoteIidI, remotePid, session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidI);

        // incrementing mid
        currentTime = Tp(2000);
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 2u}, 0, false);
        EXPECT_EQ(2u, pstate->lastReceivedMessageId);
        EXPECT_EQ(Tp(2000), pstate->lastReceiveTime);

        // same mid, still update ts
        currentTime = Tp(2100);
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 2u}, 0, true);
        EXPECT_EQ(2u, pstate->lastReceivedMessageId);
        EXPECT_EQ(Tp(2100), pstate->lastReceiveTime);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Connected_sendKeepAliveOnlyWhenNeeded)
    {
        const uint64_t session = 123;
        currentTime = Tp(2000);
        responderToState(ResponderState::Connected, remoteIidI, remotePid, session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidI);

        currentTime = Tp(2050);
        EXPECT_CALL(stack, sendTestMessage(remoteIidI, SomeIPMsgHeader{pid, session, 2u}, 456)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendTestMessage(Guid(remotePid), 456));

        currentTime = Tp(2110); // would send keepalive without normal send
        EXPECT_EQ(Tp(2150), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));

        currentTime = Tp(2200); // now needs keepalive
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 2u}, _, true)).WillOnce(Return(true));
        EXPECT_EQ(Tp(2300), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));

        currentTime = Tp(2290);
        EXPECT_CALL(stack, sendTestMessage(remoteIidI, SomeIPMsgHeader{pid, session, 3u}, 789)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->broadcastTestMessage(789));

        currentTime = Tp(2300); // would send keepalive without normal broadcast
        EXPECT_EQ(Tp(2390), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));

        currentTime = Tp(2400); // now needs keepalive
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 3u}, _, true)).WillOnce(Return(true));
        EXPECT_EQ(Tp(2500), connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500}));

        EXPECT_EQ(Tp(2400), pstate->lastSentTime);
        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Connected_canSendNormal)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::Connected, remoteIidI, remotePid, session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidI);

        currentTime = Tp(2000);
        EXPECT_CALL(stack, sendTestMessage(remoteIidI, SomeIPMsgHeader{pid, session, 2u}, 456)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendTestMessage(Guid(remotePid), 456));
        EXPECT_EQ(2u, pstate->lastSentMessageId);
        EXPECT_EQ(Tp(2000), pstate->lastSentTime);
        EXPECT_EQ(ResponderState::Connected, pstate->responderState);

        currentTime = Tp(2100);
        EXPECT_CALL(stack, sendTestMessage(remoteIidI, SomeIPMsgHeader{pid, session, 3u}, 789)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->broadcastTestMessage(789));
        EXPECT_EQ(3u, pstate->lastSentMessageId);
        EXPECT_EQ(Tp(2100), pstate->lastSentTime);
        EXPECT_EQ(ResponderState::Connected, pstate->responderState);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Connected_unicastFail_WaitForSession_handleFailedUnicast)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::Connected, remoteIidI, remotePid, session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidI);

        currentTime = Tp(2000);
        InSequence seq;
        EXPECT_CALL(stack, sendTestMessage(remoteIidI, SomeIPMsgHeader{pid, session, 2u}, 456)).WillOnce(Return(false));
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 0}, _, true)).WillOnce(Return(true)); // error
        EXPECT_FALSE(connsys->sendTestMessage(Guid(remotePid), 456));

        EXPECT_EQ(0, pstate->activeSessionId);
        EXPECT_EQ(ResponderState::WaitForSession, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Connected_broaadcastFail_WaitForSession_handleFailedBroadcast)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::Connected, remoteIidI, remotePid, session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidI);

        currentTime = Tp(2000);
        InSequence seq;
        EXPECT_CALL(stack, sendTestMessage(remoteIidI, SomeIPMsgHeader{pid, session, 2u}, 789)).WillOnce(Return(false));
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 0}, _, true)).WillOnce(Return(true)); // error
        EXPECT_TRUE(connsys->broadcastTestMessage(789));

        EXPECT_EQ(0, pstate->activeSessionId);
        EXPECT_EQ(ResponderState::WaitForSession, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Connected_receiveNormalUpdatesValues)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::Connected, remoteIidI, remotePid, session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidI);

        currentTime = Tp(2000);
        EXPECT_CALL(connsys->consumer, handleTestMessage(345, Guid(remotePid)));
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session, 2u}, 345);
        EXPECT_EQ(2u, pstate->lastReceivedMessageId);
        EXPECT_EQ(Tp(2000), pstate->lastReceiveTime);

        currentTime = Tp(2100);
        EXPECT_CALL(connsys->consumer, handleTestMessage(678, Guid(remotePid)));
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session, 3u}, 678);
        EXPECT_EQ(3u, pstate->lastReceivedMessageId);
        EXPECT_EQ(Tp(2100), pstate->lastReceiveTime);

        EXPECT_EQ(ResponderState::Connected, pstate->responderState);
        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Connected_receiveBadMid_WaitForSession_errorOnWrongMid)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::Connected, remoteIidI, remotePid, session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidI);

        InSequence seq;
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 0}, _, true)).WillOnce(Return(true)); // error
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session, 3u}, 345);

        EXPECT_EQ(0, pstate->activeSessionId);
        EXPECT_EQ(ResponderState::WaitForSession, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Connected_receiveBadSid_Connected_errorOnWrongSidButStayConnected)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::Connected, remoteIidI, remotePid, session);
        const auto pstateBefore = *connsysIR.getParticipantState(remoteIidI);

        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, 23, 0}, _, true)).WillOnce(Return(true)); // error
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, 23, 1u}, 345); // no callback triggered

        EXPECT_EQ(pstateBefore, *connsysIR.getParticipantState(remoteIidI));
        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Connected_sendFailAndErrorSendFail_WaitForSession_ignoresFailedErrorSending)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::Connected, remoteIidI, remotePid, session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidI);

        currentTime = Tp(2000);
        InSequence seq;
        EXPECT_CALL(stack, sendTestMessage(remoteIidI, SomeIPMsgHeader{pid, session, 2u}, 456)).WillOnce(Return(false)); // trigger error condition
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 0}, _, true)).WillOnce(Return(false)); // send error fails
        EXPECT_FALSE(connsys->sendTestMessage(Guid(remotePid), 456));

        EXPECT_EQ(0, pstate->activeSessionId);
        EXPECT_EQ(ResponderState::WaitForSession, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponder, triggerLogsAtDifferentStates)
    {
        // Make robust in case of loglevel overwrites
        ScopedLogContextLevel scopedLogLevel(CONTEXT_COMMUNICATION, ELogLevel::Info);
        StringOutputStream sos;

        connsysIR.writePeriodicInfo(sos);
        connsysIR.writeConnectionInfo(sos);

        connect();
        connsysIR.writeConnectionInfo(sos);
        connsysIR.writePeriodicInfo(sos);

        responderToState(ResponderState::Connected, remoteIidI, 100, 123);
        responderToState(ResponderState::WaitForUp, TestInstanceId(11), 101, 124);
        uint64_t session = 0;
        initiatorToState(InitiatorState::Connected, remoteIidR, 200, &session);
        connsysIR.writeConnectionInfo(sos);
        connsysIR.writePeriodicInfo(sos);

        expectRemoteDisconnects({100, 200});
        disconnect();
        connsysIR.writeConnectionInfo(sos);
        connsysIR.writePeriodicInfo(sos);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, detectMissingPackageWhenReceivingNextNormal)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::Connected, remoteIidI, remotePid, session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidI);

        EXPECT_CALL(connsys->consumer, handleTestMessage(1, Guid(remotePid)));
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session, 2}, 1);
        EXPECT_CALL(connsys->consumer, handleTestMessage(2, Guid(remotePid)));
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session, 3}, 2);

        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 0u}, _, true)).WillOnce(Return(true)); // error
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session, 5}, 3);

        EXPECT_EQ(ResponderState::WaitForSession, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, detectMissingPackageWhenReceivingNextKeepAlive)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::Connected, remoteIidI, remotePid, session);
        const auto* pstate = connsysIR.getParticipantState(remoteIidI);

        EXPECT_CALL(connsys->consumer, handleTestMessage(1, Guid(remotePid)));
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session, 2}, 1);
        EXPECT_CALL(connsys->consumer, handleTestMessage(2, Guid(remotePid)));
        connsys->handleTestMessage(SomeIPMsgHeader{remotePid, session, 3}, 2);

        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, session, 0u}, _, true)).WillOnce(Return(true)); // error
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, session, 4u}, 0, true);

        EXPECT_EQ(ResponderState::WaitForSession, pstate->responderState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Invalid_pinfoBadMid_Unavailable_ignoresPinfoMid)
    {
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, 123, 2}, 99, 1, remoteIidR, 0, 0, 0);
        const auto* pstate = connsysIR.getParticipantState(remoteIidR);
        EXPECT_EQ(InitiatorState::Unavailable, pstate->initiatorState);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_Invalid_upPinfoBadMid_Unavailable_ignoresPinfoMid)
    {
        uint64_t session = 0;
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        fromStack.handleServiceAvailable(remoteIidR);

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session, 2}, 99, 1, remoteIidR, 0, 0, 0);

        const auto* pstate = connsysIR.getParticipantState(remoteIidR);
        EXPECT_EQ(InitiatorState::Connected, pstate->initiatorState);
        EXPECT_EQ(1u, pstate->lastReceivedMessageId);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, I_WaitForSession_pinfoBadMid_Connected_ignoresPinfoMid)
    {
        uint64_t session = 0;
        initiatorToState(InitiatorState::WaitForSessionReply, remoteIidR, remotePid, &session);

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, session, 5}, 99, 1, remoteIidR, 0, 0, 0);

        const auto* pstate = connsysIR.getParticipantState(remoteIidR);
        EXPECT_EQ(InitiatorState::Connected, pstate->initiatorState);
        EXPECT_EQ(1u, pstate->lastReceivedMessageId);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Invalid_upPinfoBadMid_Connected_ignoresPinfoMid)
    {
        uint64_t session = 0;
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidI, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0)).WillOnce(StoreSessionAndReturn(&session, true));
        fromStack.handleServiceAvailable(remoteIidI);

        EXPECT_CALL(stack, sendParticipantInfo(remoteIidI, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, 123, 2}, 99, 1, remoteIidI, 0, 0, 0);

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(ResponderState::Connected, pstate->responderState);
        EXPECT_EQ(1u, pstate->lastReceivedMessageId);
        EXPECT_EQ(123, pstate->activeSessionId);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForUp_pinfoBadMid_WaitForUp_ignoresPinfoMidAndUpdatesSession)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::WaitForUp, remoteIidI, remotePid, session);

        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, 456, 2}, 99, 1, remoteIidI, 0, 0, 0);

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(ResponderState::WaitForUp, pstate->responderState);
        EXPECT_EQ(1u, pstate->lastReceivedMessageId);
        EXPECT_EQ(456, pstate->activeSessionId);
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_WaitForSession_pinfoBadMid_Connected_ignoresPinfoMid)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::WaitForSession, remoteIidI, remotePid, session);

        EXPECT_CALL(stack, sendParticipantInfo(remoteIidI, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, 456, 2}, 99, 1, remoteIidI, 0, 0, 0);

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(ResponderState::Connected, pstate->responderState);
        EXPECT_EQ(1u, pstate->lastReceivedMessageId);
        EXPECT_EQ(456, pstate->activeSessionId);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, R_Connected_pinfoBadMid_Connected_ignoresPinfoMid)
    {
        const uint64_t session = 123;
        responderToState(ResponderState::Connected, remoteIidI, remotePid, session);

        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidI, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, 456, 2}, 99, 1, remoteIidI, 0, 0, 0);

        const auto* pstate = connsysIR.getParticipantState(remoteIidI);
        EXPECT_EQ(ResponderState::Connected, pstate->responderState);
        EXPECT_EQ(1u, pstate->lastReceivedMessageId);
        EXPECT_EQ(456, pstate->activeSessionId);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, isResponsibleForMinorProtocolVersion)
    {
        EXPECT_FALSE(connsysIR.isResponsibleForMinorProtocolVersion(SomeIPConstants::FallbackMinorProtocolVersion));
        EXPECT_TRUE(connsysIR.isResponsibleForMinorProtocolVersion(connsysIR.getSupportedMinorProtocolVersion()));
        EXPECT_TRUE(connsysIR.isResponsibleForMinorProtocolVersion(connsysIR.getSupportedMinorProtocolVersion() + 1));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, canHandleMinorVersionUpgrade_selfIsResponder)
    {
        currentTime = Tp(2000);

        // connect old
        uint64_t sessionOld = 0;
        connectRemoteBaseOnly(remoteIidI, Guid(remotePid), 123, &sessionOld);

        // verify responsibility fully with old
        EXPECT_TRUE(connsys->isInstanceAvailable(remoteIidI));
        EXPECT_TRUE(connsysIR.isInstanceAvailable(remoteIidI));
        EXPECT_FALSE(connsysIR.isResponsibleForInstance(remoteIidI));
        EXPECT_FALSE(connsysIR.isResponsibleForParticipant(Guid(remotePid)));

        const auto* pstateOld = connsys->getParticipantState(remoteIidI);
        ASSERT_TRUE(pstateOld);
        EXPECT_TRUE(pstateOld->pid.isValid());
        EXPECT_NE(0u, pstateOld->expectedRecvSessionId);

        const auto* pstateIR = connsysIR.getParticipantState(remoteIidI);
        ASSERT_TRUE(pstateIR);
        EXPECT_FALSE(pstateIR->remotePid.isValid());
        EXPECT_EQ(InitiatorState::Invalid, pstateIR->initiatorState);
        EXPECT_EQ(ResponderState::Invalid, pstateIR->responderState);

        // trigger handover old -> new responder
        const uint64_t sessionNew = 123;
        Sequence seq;  // order matters here to ensure strict disconnect -> connect
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid))).InSequence(seq);
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidI, SomeIPMsgHeader{pid, sessionNew, 1u}, 99, 1, iid, remotePid, 0, 0))
            .InSequence(seq).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid))).InSequence(seq);
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, sessionNew, 1}, 99, 1, remoteIidI, 0, 0, 0);

        // verify responsibility fully with new
        EXPECT_TRUE(connsys->isInstanceAvailable(remoteIidI));
        EXPECT_TRUE(connsysIR.isInstanceAvailable(remoteIidI));
        EXPECT_TRUE(connsysIR.isResponsibleForInstance(remoteIidI));
        EXPECT_TRUE(connsysIR.isResponsibleForParticipant(Guid(remotePid)));

        ASSERT_EQ(pstateOld, connsys->getParticipantState(remoteIidI));
        EXPECT_FALSE(pstateOld->pid.isValid());
        EXPECT_EQ(0u, pstateOld->expectedRecvSessionId);

        EXPECT_EQ(pstateIR, connsysIR.getParticipantState(remoteIidI));
        EXPECT_TRUE(pstateIR->remotePid.isValid());
        EXPECT_EQ(InitiatorState::Invalid, pstateIR->initiatorState);
        EXPECT_EQ(ResponderState::Connected, pstateIR->responderState);

        // verify messages end up in new only
        currentTime = Tp(2100);
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, sessionNew, 1u}, 0, true);
        EXPECT_EQ(Tp(2000), pstateOld->lastRecv);
        EXPECT_EQ(currentTime, pstateIR->lastReceiveTime);

        // ensure only new sends keepalives
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, sessionNew, 1u}, _, true)).WillOnce(Return(true));
        connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500});
        EXPECT_EQ(Tp(2000), pstateOld->lastSent);
        EXPECT_EQ(currentTime, pstateIR->lastSentTime);

        // force receive timeout
        currentTime = Tp(3000);
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        EXPECT_CALL(stack, sendKeepAlive(remoteIidI, SomeIPMsgHeader{pid, sessionNew, 0}, _, true)).WillOnce(Return(true)); // error
        connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500});
        EXPECT_EQ(ResponderState::WaitForSession, pstateIR->responderState);

        // unavailability tracked by both
        fromStack.handleServiceUnavailable(remoteIidI);
        EXPECT_FALSE(connsys->isInstanceAvailable(remoteIidI));
        EXPECT_FALSE(connsysIR.isInstanceAvailable(remoteIidI));
        EXPECT_FALSE(connsysIR.isResponsibleForInstance(remoteIidI));
        EXPECT_TRUE(connsysIR.isResponsibleForParticipant(Guid(remotePid)));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, canHandleMinorVersionUpgrade_selfIsInitiator)
    {
        // connect old
        uint64_t sessionOld = 0;
        connectRemoteBaseOnly(remoteIidR, Guid(remotePid), 123, &sessionOld);

        // verify responsibility fully with old
        EXPECT_TRUE(connsys->isInstanceAvailable(remoteIidR));
        EXPECT_TRUE(connsysIR.isInstanceAvailable(remoteIidR));
        EXPECT_FALSE(connsysIR.isResponsibleForInstance(remoteIidR));
        EXPECT_FALSE(connsysIR.isResponsibleForParticipant(Guid(remotePid)));

        const auto* pstateOld = connsys->getParticipantState(remoteIidR);
        ASSERT_TRUE(pstateOld);
        EXPECT_TRUE(pstateOld->pid.isValid());
        EXPECT_NE(0u, pstateOld->expectedRecvSessionId);

        const auto* pstateIR = connsysIR.getParticipantState(remoteIidR);
        ASSERT_TRUE(pstateIR);
        EXPECT_FALSE(pstateIR->remotePid.isValid());
        EXPECT_EQ(InitiatorState::Invalid, pstateIR->initiatorState);
        EXPECT_EQ(ResponderState::Invalid, pstateIR->responderState);

        // trigger handover old -> new initiator
        uint64_t sessionNew = 0;
        Sequence seq;  // order matters here to ensure strict disconnect -> connect
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid))).InSequence(seq);
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)).InSequence(seq).WillOnce(StoreSessionAndReturn(&sessionNew, true)); // init session
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, 123, 1}, 99, 1, remoteIidR, 0, 0, 0);

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid))).InSequence(seq);
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, sessionNew, 1}, 99, 1, remoteIidR, 0, 0, 0); // reply

        // verify responsibility fully with new
        EXPECT_TRUE(connsys->isInstanceAvailable(remoteIidR));
        EXPECT_TRUE(connsysIR.isInstanceAvailable(remoteIidR));
        EXPECT_TRUE(connsysIR.isResponsibleForInstance(remoteIidR));
        EXPECT_TRUE(connsysIR.isResponsibleForParticipant(Guid(remotePid)));

        ASSERT_EQ(pstateOld, connsys->getParticipantState(remoteIidR));
        EXPECT_FALSE(pstateOld->pid.isValid());
        EXPECT_EQ(0u, pstateOld->expectedRecvSessionId);

        EXPECT_EQ(pstateIR, connsysIR.getParticipantState(remoteIidR));
        EXPECT_TRUE(pstateIR->remotePid.isValid());
        EXPECT_EQ(InitiatorState::Connected, pstateIR->initiatorState);
        EXPECT_EQ(ResponderState::Invalid, pstateIR->responderState);

        expectRemoteDisconnects({remotePid});
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, canHandleMinorVersionDowngrade_connectWithFirstPinfoThenUp)
    {
        currentTime = Tp(2000);

        // connect new  (first pinfo, then up)
        uint64_t sessionNew = 0;
        initiatorToState(InitiatorState::Connected, remoteIidR, remotePid, &sessionNew);

        // verify responsibility fully with new
        EXPECT_TRUE(connsys->isInstanceAvailable(remoteIidR));
        EXPECT_TRUE(connsysIR.isInstanceAvailable(remoteIidR));
        EXPECT_TRUE(connsysIR.isResponsibleForInstance(remoteIidR));
        EXPECT_TRUE(connsysIR.isResponsibleForParticipant(Guid(remotePid)));

        const auto* pstateOld = connsys->getParticipantState(remoteIidR);
        ASSERT_TRUE(pstateOld);
        EXPECT_FALSE(pstateOld->pid.isValid());
        EXPECT_EQ(0u, pstateOld->expectedRecvSessionId);

        const auto* pstateIR = connsysIR.getParticipantState(remoteIidR);
        ASSERT_TRUE(pstateIR);
        EXPECT_TRUE(pstateIR->remotePid.isValid());
        EXPECT_EQ(InitiatorState::Connected, pstateIR->initiatorState);
        EXPECT_EQ(ResponderState::Invalid, pstateIR->responderState);

        // trigger handover new -> old
        const uint64_t sessionOldIn = 123;
        uint64_t sessionOldOut = 0;
        Sequence seq;  // order matters here to ensure strict disconnect -> connect
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid))).InSequence(seq);
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0))
            .InSequence(seq).WillOnce(StoreSessionAndReturn(&sessionOldOut, true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid))).InSequence(seq);
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, sessionOldIn, 1}, 99, 0, remoteIidR, 0, 0, 0);

        // verify responsibility fully with old
        EXPECT_TRUE(connsys->isInstanceAvailable(remoteIidR));
        EXPECT_TRUE(connsysIR.isInstanceAvailable(remoteIidR));
        EXPECT_FALSE(connsysIR.isResponsibleForInstance(remoteIidR));
        EXPECT_FALSE(connsysIR.isResponsibleForParticipant(Guid(remotePid)));

        ASSERT_EQ(pstateOld, connsys->getParticipantState(remoteIidR));
        EXPECT_TRUE(pstateOld->pid.isValid());
        EXPECT_NE(0u, pstateOld->expectedRecvSessionId);

        EXPECT_EQ(pstateIR, connsysIR.getParticipantState(remoteIidR));
        EXPECT_FALSE(pstateIR->remotePid.isValid());
        EXPECT_EQ(InitiatorState::Invalid, pstateIR->initiatorState);
        EXPECT_EQ(ResponderState::Invalid, pstateIR->responderState);

        // verify messages end up in old only
        currentTime = Tp(2100);
        fromStack.handleKeepAlive(SomeIPMsgHeader{remotePid, sessionOldIn, 2u}, 0, false);
        EXPECT_EQ(currentTime, pstateOld->lastRecv);
        EXPECT_EQ(std::chrono::steady_clock::time_point{}, pstateIR->lastReceiveTime);

        // ensure only old sends keepalives
        EXPECT_CALL(stack, sendKeepAlive(remoteIidR, SomeIPMsgHeader{pid, sessionOldOut, 2u}, _, false)).WillOnce(Return(true));
        connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500});
        EXPECT_EQ(currentTime, pstateOld->lastSent);
        EXPECT_EQ(std::chrono::steady_clock::time_point{}, pstateIR->lastSentTime);

        // force receive timeout
        currentTime = Tp(3000);
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid)));
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidR, ValidHdr(pid, 1u), 99, 1, iid, remotePid, 0, 0)) // new session
            .InSequence(seq).WillOnce(StoreSessionAndReturn(&sessionOldOut, true));
        connsys->doOneThreadLoop(std::chrono::milliseconds{100}, std::chrono::milliseconds{500});

        // unavailability tracked by both
        fromStack.handleServiceUnavailable(remoteIidI);
        EXPECT_FALSE(connsys->isInstanceAvailable(remoteIidI));
        EXPECT_FALSE(connsysIR.isInstanceAvailable(remoteIidI));
        EXPECT_FALSE(connsysIR.isResponsibleForInstance(remoteIidI));
        EXPECT_FALSE(connsysIR.isResponsibleForParticipant(Guid(remotePid)));
    }

    TEST_F(AConnectionSystemInitiatorResponderConnected, canHandleMinorVersionDowngrade_connectWithFirstUpThenPinfo)
    {
        // connect new  (first up then pinfo
        const uint64_t sessionNew = 123;
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidI, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(remoteIidI);
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidI, SomeIPMsgHeader{pid, sessionNew, 1u}, 99, 1, iid, remotePid, 0, 0)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, sessionNew, 1}, 99, 1, remoteIidI, pid, 0, 0);

        // verify responsibility fully with new
        EXPECT_TRUE(connsys->isInstanceAvailable(remoteIidI));
        EXPECT_TRUE(connsysIR.isInstanceAvailable(remoteIidI));
        EXPECT_TRUE(connsysIR.isResponsibleForInstance(remoteIidI));
        EXPECT_TRUE(connsysIR.isResponsibleForParticipant(Guid(remotePid)));

        const auto* pstateOld = connsys->getParticipantState(remoteIidI);
        ASSERT_TRUE(pstateOld);
        EXPECT_FALSE(pstateOld->pid.isValid());
        EXPECT_EQ(0u, pstateOld->expectedRecvSessionId);

        const auto* pstateIR = connsysIR.getParticipantState(remoteIidI);
        ASSERT_TRUE(pstateIR);
        EXPECT_TRUE(pstateIR->remotePid.isValid());
        EXPECT_EQ(InitiatorState::Invalid, pstateIR->initiatorState);
        EXPECT_EQ(ResponderState::Connected, pstateIR->responderState);

        // trigger handover new -> old
        const uint64_t sessionOldIn = 123;
        uint64_t sessionOldOut = 0;
        Sequence seq;  // order matters here to ensure strict disconnect -> connect
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(remotePid))).InSequence(seq);
        EXPECT_CALL(stack, sendParticipantInfo(remoteIidI, ValidHdr(pid, 1u), 99, 1, iid, 0, 0, 0))
            .InSequence(seq).WillOnce(StoreSessionAndReturn(&sessionOldOut, true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(remotePid))).InSequence(seq);
        fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid, sessionOldIn, 1}, 99, 0, remoteIidI, 0, 0, 0);

        // verify responsibility fully with old
        EXPECT_TRUE(connsys->isInstanceAvailable(remoteIidI));
        EXPECT_TRUE(connsysIR.isInstanceAvailable(remoteIidI));
        EXPECT_FALSE(connsysIR.isResponsibleForInstance(remoteIidI));
        EXPECT_FALSE(connsysIR.isResponsibleForParticipant(Guid(remotePid)));

        ASSERT_EQ(pstateOld, connsys->getParticipantState(remoteIidI));
        EXPECT_TRUE(pstateOld->pid.isValid());
        EXPECT_NE(0u, pstateOld->expectedRecvSessionId);

        EXPECT_EQ(pstateIR, connsysIR.getParticipantState(remoteIidI));
        EXPECT_FALSE(pstateIR->remotePid.isValid());
        EXPECT_EQ(InitiatorState::Invalid, pstateIR->initiatorState);
        EXPECT_EQ(ResponderState::Invalid, pstateIR->responderState);

        expectRemoteDisconnects({remotePid});
    }

/*
  - TODO unavailable -> up send error when had session once?

  untested error cases
  - same pid, different iid (+ mixed old/new? -> detect + log?)
  - same iid, chaning pid
 */
}
