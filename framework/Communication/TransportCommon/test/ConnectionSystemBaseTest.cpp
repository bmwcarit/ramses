//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Collections/StringOutputStream.h"
#include "Utils/StatisticCollection.h"
#include "ConnectionSystemTestCommon.h"
#include "ScopedLogContextLevel.h"

namespace ramses_internal
{
    using namespace TestConnectionSystemBase;

    class AConnectionSystemBase : public Test
    {
    public:
        std::unique_ptr<TestConnectionSystem> construct(uint32_t commUserId, TestInstanceId serviceIid, const ParticipantIdentifier& namedPid, uint32_t protocolVersion,
                                                        const std::shared_ptr<StrictMock<StackMock>>& stack,
                                                        uint64_t keepAliveInterval, uint64_t keepAliveTimeout,
                                                        const std::function<std::chrono::steady_clock::time_point(void)>& steadyClockNow = std::chrono::steady_clock::now)
        {
            if (stack)
                EXPECT_CALL(*stack, getServiceInstanceId()).WillRepeatedly(Return(serviceIid));

            return std::make_unique<TestConnectionSystem>(stack, commUserId, namedPid, protocolVersion, lock,
                                                          std::chrono::milliseconds(keepAliveInterval), std::chrono::milliseconds(keepAliveTimeout),
                                                          steadyClockNow);
        }

        PlatformLock lock;
    };

    TEST_F(AConnectionSystemBase, canBeConstructedAndDestroyed)
    {
        auto ptr = construct(1, TestInstanceId(2), ParticipantIdentifier(Guid(3), "foo"), 1, std::make_shared<StrictMock<StackMock>>(), 0, 0);
        ASSERT_TRUE(ptr != nullptr);
        ptr.reset();
    }

    TEST_F(AConnectionSystemBase, canBeConstructedAndDestroyedWithKeepAliveValues)
    {
        auto ptr = construct(1, TestInstanceId(2), ParticipantIdentifier(Guid(3), "foo"), 1, std::make_shared<StrictMock<StackMock>>(), 10, 100);
        ASSERT_TRUE(ptr != nullptr);
        ptr.reset();
    }

    TEST_F(AConnectionSystemBase, constructorArgumentCheckSucceedsWithValidArguments)
    {
        auto stack = std::make_shared<StrictMock<StackMock>>();
        EXPECT_CALL(*stack, getServiceInstanceId()).WillRepeatedly(Return(TestInstanceId(2)));
        ASSERT_TRUE(TestConnectionSystem::CheckConstructorArguments(stack, 1, ParticipantIdentifier(Guid(3), "foo"), 1, std::chrono::milliseconds(5), std::chrono::milliseconds(10), CONTEXT_COMMUNICATION, "test"));
    }

    TEST_F(AConnectionSystemBase, constructorArgumentCheckFailsWithInvalidArguments)
    {
        auto stack = std::make_shared<StrictMock<StackMock>>();
        EXPECT_CALL(*stack, getServiceInstanceId()).WillRepeatedly(Return(TestInstanceId(2)));
        ASSERT_FALSE(TestConnectionSystem::CheckConstructorArguments(stack, 0, ParticipantIdentifier(Guid(3), "foo"), 1, std::chrono::milliseconds(0), std::chrono::milliseconds(0), CONTEXT_COMMUNICATION, "test"));
        ASSERT_FALSE(TestConnectionSystem::CheckConstructorArguments(stack, 1, ParticipantIdentifier(Guid(), "foo"), 1, std::chrono::milliseconds(0), std::chrono::milliseconds(0), CONTEXT_COMMUNICATION, "test"));
        ASSERT_FALSE(TestConnectionSystem::CheckConstructorArguments(stack, 1, ParticipantIdentifier(Guid(3), "foo"), 0, std::chrono::milliseconds(0), std::chrono::milliseconds(0), CONTEXT_COMMUNICATION, "test"));
        ASSERT_FALSE(TestConnectionSystem::CheckConstructorArguments(nullptr, 1, ParticipantIdentifier(Guid(3), "foo"), 1, std::chrono::milliseconds(0), std::chrono::milliseconds(0), CONTEXT_COMMUNICATION, "test"));
        ASSERT_FALSE(TestConnectionSystem::CheckConstructorArguments(stack, 1, ParticipantIdentifier(Guid(3), "foo"), 1, std::chrono::milliseconds(0), std::chrono::milliseconds(10), CONTEXT_COMMUNICATION, "test"));
        ASSERT_FALSE(TestConnectionSystem::CheckConstructorArguments(stack, 1, ParticipantIdentifier(Guid(3), "foo"), 1, std::chrono::milliseconds(10), std::chrono::milliseconds(5), CONTEXT_COMMUNICATION, "test"));
        ASSERT_FALSE(TestConnectionSystem::CheckConstructorArguments(stack, 1, ParticipantIdentifier(Guid(3), "foo"), 1, std::chrono::milliseconds(10), std::chrono::milliseconds(10), CONTEXT_COMMUNICATION, "test"));
        EXPECT_CALL(*stack, getServiceInstanceId()).WillRepeatedly(Return(TestInstanceId()));
        ASSERT_FALSE(TestConnectionSystem::CheckConstructorArguments(stack, 1, ParticipantIdentifier(Guid(3), "foo"), 1, std::chrono::milliseconds(5), std::chrono::milliseconds(10), CONTEXT_COMMUNICATION, "test"));
    }

    TEST_F(AConnectionSystemBase, canConnectDisconnectMultipleTimesWithKeepaliveThread)
    {
        auto stack = std::make_shared<StrictMock<StackMock>>();
        EXPECT_CALL(*stack, getServiceInstanceId()).WillRepeatedly(Return(TestInstanceId(2)));
        EXPECT_CALL(*stack, connect()).WillRepeatedly(Return(true));
        EXPECT_CALL(*stack, disconnect()).WillRepeatedly(Return(true));
        auto connsys = construct(1, TestInstanceId(2), ParticipantIdentifier(Guid(3), "foo"), 1, stack, 10, 100);
        ASSERT_TRUE(connsys != nullptr);
        {
            PlatformGuard g(lock);
            ASSERT_TRUE(connsys->connect());
        }
        {
            PlatformGuard g(lock);
            ASSERT_TRUE(connsys->disconnect());
            ASSERT_TRUE(connsys->connect());
        }
        {
            PlatformGuard g(lock);
            ASSERT_TRUE(connsys->disconnect());
            ASSERT_TRUE(connsys->connect());
        }
        {
            PlatformGuard g(lock);
            ASSERT_TRUE(connsys->disconnect());
        }
    }


    class AConnectionSystem : public AConnectionSystemBase
    {
    public:
        AConnectionSystem()
            : stackPtr(std::make_shared<StrictMock<StackMock>>())
            , stack(*stackPtr)
            , connsys(construct(3, TestInstanceId(5), ParticipantIdentifier(Guid(pid), "foobar"), 99, stackPtr, 0, 0, [this]() { return steadyClockNow(); }))
            , fromStack(*connsys)
        {
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

        void connectRemote(TestInstanceId remoteIid, const Guid& remotePid, uint64_t sessionId = 123)
        {
            EXPECT_CALL(stack, sendParticipantInfo(remoteIid, ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
            fromStack.handleServiceAvailable(remoteIid);
            EXPECT_CALL(connsys->connections, newParticipantHasConnected(remotePid));
            const auto tsBefore = connsys->getParticipantState(TestInstanceId(remoteIid))->lastRecv;
            fromStack.handleParticipantInfo(SomeIPMsgHeader{remotePid.get(), sessionId, 1}, 99, remoteIid, 0, 0, 0);
            if (hasCounterTime)
            {
                EXPECT_LT(tsBefore, connsys->getParticipantState(TestInstanceId(remoteIid))->lastRecv);
            }
        }

        void expectRemoteDisconnects(std::initializer_list<uint64_t> guids)
        {
            for (auto g : guids)
                EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(g)));
        }

        virtual std::chrono::steady_clock::time_point steadyClockNow()
        {
            timeCounter += std::chrono::seconds(1);
            return timeCounter;
        }

        std::shared_ptr<StrictMock<StackMock>> stackPtr;
        StrictMock<StackMock>& stack;
        uint64_t pid{4};
        TestInstanceId iid{5};
        std::unique_ptr<TestConnectionSystem> connsys;
        Callbacks& fromStack;
        std::chrono::steady_clock::time_point timeCounter{std::chrono::seconds(0)};
        bool hasCounterTime = true;
    };

    class AConnectionSystemConnected : public AConnectionSystem
    {
    public:
        AConnectionSystemConnected()
        {
            connect();
        }

        ~AConnectionSystemConnected()
        {
            disconnect();
        }
    };

    TEST_F(AConnectionSystem, hasCorrectConnectDisconnectBehavior)
    {
        std::lock_guard<std::recursive_mutex> g(lock);
        EXPECT_FALSE(connsys->disconnect());

        EXPECT_CALL(stack, connect()).WillOnce(Return(true));
        EXPECT_TRUE(connsys->connect());
        EXPECT_FALSE(connsys->connect());

        EXPECT_CALL(stack, disconnect()).WillOnce(Return(true));
        EXPECT_TRUE(connsys->disconnect());
        EXPECT_FALSE(connsys->disconnect());
    }

    TEST_F(AConnectionSystem, connectDisconnectForwardsStackFailures)
    {
        std::lock_guard<std::recursive_mutex> g(lock);
        EXPECT_CALL(stack, connect()).WillOnce(Return(false));
        EXPECT_FALSE(connsys->connect());

        EXPECT_CALL(stack, connect()).WillOnce(Return(true));
        EXPECT_TRUE(connsys->connect());

        EXPECT_CALL(stack, disconnect()).WillOnce(Return(false));
        EXPECT_FALSE(connsys->disconnect());
        EXPECT_CALL(stack, disconnect()).WillOnce(Return(true));
        EXPECT_TRUE(connsys->disconnect());
    }

    TEST_F(AConnectionSystem, disconnectsOnDestruction)
    {
        std::lock_guard<std::recursive_mutex> g(lock);
        EXPECT_CALL(stack, connect()).WillOnce(Return(true));
        EXPECT_TRUE(connsys->connect());
        EXPECT_CALL(stack, disconnect()).WillOnce(Return(false));
        connsys.reset();
    }

    TEST_F(AConnectionSystemConnected, sendParticipantInfoOnserviceAvailable)
    {
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(3), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(3));

        fromStack.handleServiceUnavailable(TestInstanceId(1));
    }

    TEST_F(AConnectionSystemConnected, connectedAfterUpThenPinfo)
    {
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(1)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 1}, 99, TestInstanceId(1), 0, 0, 0);

        expectRemoteDisconnects({1});
    }

    TEST_F(AConnectionSystemConnected, notConnectedAfterUpThenPinfoWhenPinfoSendFails)
    {
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(false));
        fromStack.handleServiceAvailable(TestInstanceId(1));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 1}, 99, TestInstanceId(1), 0, 0, 0);
    }

    TEST_F(AConnectionSystemConnected, connectedAfterPinfoThenUp)
    {
        const auto recvTsBefore = timeCounter;
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 1}, 99, TestInstanceId(1), 0, 0, 0);
        EXPECT_LT(recvTsBefore, connsys->getParticipantState(TestInstanceId(1))->lastRecv);

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 1, _, _)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(1)));
        const auto sentTsBefore = connsys->getParticipantState(TestInstanceId(1))->lastSent;
        fromStack.handleServiceAvailable(TestInstanceId(1));
        EXPECT_LT(sentTsBefore, connsys->getParticipantState(TestInstanceId(1))->lastSent);

        expectRemoteDisconnects({1});
    }

    TEST_F(AConnectionSystemConnected, notConnectedAfterPinfoThenUpWhenPinfoSendFails)
    {
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 1}, 99, TestInstanceId(1), 0, 0, 0);

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 1, _, _)).WillOnce(Return(false));
        const auto sentTsBefore = connsys->getParticipantState(TestInstanceId(1))->lastSent;
        fromStack.handleServiceAvailable(TestInstanceId(1));
        // muste update lastSent anyway
        EXPECT_LT(sentTsBefore, connsys->getParticipantState(TestInstanceId(1))->lastSent);
    }

    TEST_F(AConnectionSystemConnected, ignoreInvalidProtocol)
    {
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 1}, 98, TestInstanceId(1), 0, 0, 0);
    }

    TEST_F(AConnectionSystemConnected, ignoreRemoteIidIsSelf)
    {
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 1}, 99, iid, 0, 0, 0);
    }

    TEST_F(AConnectionSystemConnected, ignoreRemoteIidInvalid)
    {
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 1}, 99, TestInstanceId(), 0, 0, 0);
    }

    TEST_F(AConnectionSystemConnected, ignoreRemotePidIsSelf)
    {
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{pid, 123, 1}, 99, TestInstanceId(1), 0, 0, 0);
    }

    TEST_F(AConnectionSystemConnected, ignoreRemotePidInvalid)
    {
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{0, 123, 1}, 99, TestInstanceId(1), 0, 0, 0);
    }

    TEST_F(AConnectionSystemConnected, ignorePinfoSessionZero)
    {
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 0, 1}, 99, TestInstanceId(1), 0, 0, 0);
        fromStack.handleServiceAvailable(TestInstanceId(1));
    }

    TEST_F(AConnectionSystemConnected, ignorePinfoMessageZero)
    {
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 0}, 99, TestInstanceId(1), 0, 0, 0);
        fromStack.handleServiceAvailable(TestInstanceId(1));
    }

    TEST_F(AConnectionSystemConnected, connectedAfterUpDownPinfoThenUp)
    {
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));
        fromStack.handleServiceUnavailable(TestInstanceId(1));

        const auto tsBefore = connsys->getParticipantState(TestInstanceId(1))->lastRecv;
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 1}, 99, TestInstanceId(1), 0, 0, 0);
        EXPECT_LT(tsBefore, connsys->getParticipantState(TestInstanceId(1))->lastRecv);

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 1, _, _)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(1)));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        expectRemoteDisconnects({1});
    }

    TEST_F(AConnectionSystemConnected, handlesValidPinfoAfterConnected)
    {
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(1)));
        auto tsBefore = connsys->getParticipantState(TestInstanceId(1))->lastRecv;
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 1}, 99, TestInstanceId(1), 0, 0, 0);
        EXPECT_LT(tsBefore, connsys->getParticipantState(TestInstanceId(1))->lastRecv);

        tsBefore = connsys->getParticipantState(TestInstanceId(1))->lastRecv;
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 2}, 99, TestInstanceId(1), 0, 0, 0);
        EXPECT_LT(tsBefore, connsys->getParticipantState(TestInstanceId(1))->lastRecv);

        expectRemoteDisconnects({1});
    }

    TEST_F(AConnectionSystemConnected, unconnectedInstanceCanChangePid)
    {
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(false));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 1}, 99, TestInstanceId(1), pid, 0, 0);

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 2, _, _)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(2)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{2, 123, 1}, 99, TestInstanceId(1), pid, 0, 0);

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemConnected, connectedInstanceCanChangePid)
    {
        connectRemote(TestInstanceId(1), Guid(1));

        expectRemoteDisconnects({1});
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 2, _, _)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(2)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{2, 123, 1}, 99, TestInstanceId(1), pid, 0, 0);

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemConnected, connectedInstanceCanChangePidAfterRegularMessages)
    {
        connectRemote(TestInstanceId(1), Guid(1));
        EXPECT_CALL(stack, sendTestMessage(TestInstanceId(1), ValidHdr(pid, 2u), 44)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendTestMessage(Guid(1), 44));

        EXPECT_CALL(connsys->consumer, handleTestMessage(44, Guid(1)));
        connsys->handleTestMessage(SomeIPMsgHeader{1, 123, 2}, 44);

        expectRemoteDisconnects({1});
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 2, _, _)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(2)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{2, 123, 1}, 99, TestInstanceId(1), pid, 0, 0);

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemConnected, getsConnectedWhenInitialPinfoFailedAndAlreadyHasReceivedValidPinfo)
    {
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 1}, 99, TestInstanceId(1), 0, 0, 0);

        SomeIPMsgHeader firstHdr;
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 1, _, _)).WillOnce(DoAll(SaveArg<1>(&firstHdr), Return(false)));
        fromStack.handleServiceAvailable(TestInstanceId(1));
        fromStack.handleServiceUnavailable(TestInstanceId(1));

        SomeIPMsgHeader secondHdr;
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 1, _, _)).WillOnce(DoAll(SaveArg<1>(&firstHdr), Return(true)));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(1)));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        EXPECT_NE(firstHdr.sessionId, secondHdr.sessionId);

        expectRemoteDisconnects({1});
    }

    TEST_F(AConnectionSystemConnected, getsConnectedWhenInitialPinfoFailedAndAlreadyHasReceivedValidPinfoAndKeepalives)
    {
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 1}, 99, TestInstanceId(1), 0, 0, 0);
        fromStack.handleKeepAlive(SomeIPMsgHeader{1, 123, 2}, 0);
        fromStack.handleKeepAlive(SomeIPMsgHeader{1, 123, 3}, 0);
        fromStack.handleKeepAlive(SomeIPMsgHeader{1, 123, 4}, 0);

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 1, _, _)).WillOnce(Return(false));
        fromStack.handleServiceAvailable(TestInstanceId(1));
        fromStack.handleServiceUnavailable(TestInstanceId(1));

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 1, _, _)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(1)));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        expectRemoteDisconnects({1});
    }

    TEST_F(AConnectionSystemConnected, canConnectAfterPinfoAndKeepAlives)
    {
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 1}, 99, TestInstanceId(1), 0, 0, 0);
        fromStack.handleKeepAlive(SomeIPMsgHeader{1, 123, 2}, 0);
        fromStack.handleKeepAlive(SomeIPMsgHeader{1, 123, 3}, 0);
        fromStack.handleKeepAlive(SomeIPMsgHeader{1, 123, 4}, 0);

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 1, _, _)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(1)));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        expectRemoteDisconnects({1});
    }

    TEST_F(AConnectionSystem, disconnectEmitsDisconnectForAllParticipants)
    {
        connect();
        connectRemote(TestInstanceId(1), Guid(2));
        connectRemote(TestInstanceId(3), Guid(10));

        expectRemoteDisconnects({2, 10});
        disconnect();
        Mock::VerifyAndClearExpectations(&connsys->connections);
    }

    TEST_F(AConnectionSystem, triggerLogsAtDifferentStates)
    {
        // Make robust in case of loglevel overwrites
        ScopedLogContextLevel scopedLogLevel(CONTEXT_COMMUNICATION, ELogLevel::Info);

        connsys->logPeriodicInfo();
        EXPECT_CALL(stack, logConnectionState(_));
        connsys->logConnectionInfo();
        connect();
        EXPECT_CALL(stack, logConnectionState(_));
        connsys->logConnectionInfo();
        connsys->logPeriodicInfo();

        connectRemote(TestInstanceId(1), Guid(2));
        connectRemote(TestInstanceId(3), Guid(10));
        EXPECT_CALL(stack, logConnectionState(_));
        connsys->logConnectionInfo();
        connsys->logPeriodicInfo();

        expectRemoteDisconnects({2, 10});
        disconnect();

        EXPECT_CALL(stack, logConnectionState(_));
        connsys->logConnectionInfo();
        connsys->logPeriodicInfo();
    }

    TEST_F(AConnectionSystem, canWriteStateForLog)
    {
        StringOutputStream sos;
        EXPECT_CALL(stack, logConnectionState(_));
        connsys->writeStateForLog(sos);
        EXPECT_FALSE(sos.release().empty());
    }

    TEST_F(AConnectionSystemConnected, disconnectsOnUnicastSendFailure)
    {
        connectRemote(TestInstanceId(1), Guid(2));
        connectRemote(TestInstanceId(3), Guid(10));

        EXPECT_CALL(stack, sendTestMessage(TestInstanceId(3), ValidHdr(pid, 2u), 44)).WillOnce(Return(false));
        expectRemoteDisconnects({10});
        EXPECT_FALSE(connsys->sendTestMessage(Guid(10), 44));

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemConnected, disconnectsOnMulticastSendFailure)
    {
        connectRemote(TestInstanceId(1), Guid(2));
        connectRemote(TestInstanceId(3), Guid(10));
        connectRemote(TestInstanceId(2), Guid(3));

        EXPECT_CALL(stack, sendTestMessage(TestInstanceId(1), ValidHdr(pid, 2u), 44)).WillOnce(Return(false));
        EXPECT_CALL(stack, sendTestMessage(TestInstanceId(3), ValidHdr(pid, 2u), 44)).WillOnce(Return(true));
        EXPECT_CALL(stack, sendTestMessage(TestInstanceId(2), ValidHdr(pid, 2u), 44)).WillOnce(Return(false));
        expectRemoteDisconnects({2, 3});
        EXPECT_TRUE(connsys->broadcastTestMessage(44));

        expectRemoteDisconnects({10});
    }

    TEST_F(AConnectionSystemConnected, ignoresMessagesFromInvalidParticipant)
    {
        connectRemote(TestInstanceId(1), Guid(2));
        expectRemoteDisconnects({2});
        fromStack.handleServiceUnavailable(TestInstanceId(1));
        Mock::VerifyAndClearExpectations(&connsys->connections);

        connsys->handleTestMessage(SomeIPMsgHeader{1, 123, 2}, 44);
        connsys->handleTestMessage(SomeIPMsgHeader{2, 123, 2}, 44);
    }

    TEST_F(AConnectionSystemConnected, ignoresImpossibleMessagesFromConnectedParticipant)
    {
        connectRemote(TestInstanceId(1), Guid(2));

        connsys->handleTestMessage(SomeIPMsgHeader{2, 0, 2}, 44);
        connsys->handleTestMessage(SomeIPMsgHeader{2, 123, 0}, 44);
        connsys->handleTestMessage(SomeIPMsgHeader{2, 123, 1}, 44);

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemConnected, disconnectsOnSessionIdMismatchInRegularMessage)
    {
        connectRemote(TestInstanceId(1), Guid(2));
        connectRemote(TestInstanceId(2), Guid(10));

        expectRemoteDisconnects({10});
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(2), ValidHdr(pid, 1u), 99, iid, 10, _, _)).WillOnce(Return(true));
        connsys->handleTestMessage(SomeIPMsgHeader{10, 124, 2}, 44);

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemConnected, disconnectsOnMessageIdMismatchInRegularMessage)
    {
        connectRemote(TestInstanceId(1), Guid(2));
        connectRemote(TestInstanceId(2), Guid(10));

        expectRemoteDisconnects({10});
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(2), ValidHdr(pid, 1u), 99, iid, 10, _, _)).WillOnce(Return(true));
        connsys->handleTestMessage(SomeIPMsgHeader{10, 123, 3}, 44);

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemConnected, canReconnectAfterServiceDownUpAndNewParticipantInfo)
    {
        connectRemote(TestInstanceId(1), Guid(2));
        expectRemoteDisconnects({2});
        fromStack.handleServiceUnavailable(TestInstanceId(1));

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 2, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(2)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{2, 125, 1}, 99, TestInstanceId(1), 0, 0, 0);

        EXPECT_CALL(connsys->consumer, handleTestMessage(543, Guid(2)));
        connsys->handleTestMessage(SomeIPMsgHeader{2, 125, 2}, 543);

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemConnected, canReconnectAfterServiceDownResetUp)
    {
        connectRemote(TestInstanceId(1), Guid(2));
        expectRemoteDisconnects({2});
        fromStack.handleServiceUnavailable(TestInstanceId(1));

        fromStack.handleParticipantInfo(SomeIPMsgHeader{2, 125, 1}, 99, TestInstanceId(1), 0, 0, 0);

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 2, _, _)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(2)));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        EXPECT_CALL(connsys->consumer, handleTestMessage(543, Guid(2)));
        connsys->handleTestMessage(SomeIPMsgHeader{2, 125, 2}, 543);

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemConnected, canReconnectAfterNewSessionFromRemote)
    {
        connectRemote(TestInstanceId(1), Guid(2));

        expectRemoteDisconnects({2});
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 2, _, _)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(2)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{2, 125, 1}, 99, TestInstanceId(1), 0, 0, 0);

        EXPECT_CALL(connsys->consumer, handleTestMessage(543, Guid(2)));
        connsys->handleTestMessage(SomeIPMsgHeader{2, 125, 2}, 543);

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemConnected, canReconnectAfterRegularMessageWithWrongMessageId)
    {
        connectRemote(TestInstanceId(1), Guid(2));

        EXPECT_CALL(connsys->consumer, handleTestMessage(543, Guid(2)));
        connsys->handleTestMessage(SomeIPMsgHeader{2, 123, 2}, 543);

        expectRemoteDisconnects({2});
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 2, _, _)).WillOnce(Return(true));
        connsys->handleTestMessage(SomeIPMsgHeader{2, 123, 4}, 543);

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(2)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{2, 126, 1}, 99, TestInstanceId(1), 0, 0, 0);

        EXPECT_CALL(connsys->consumer, handleTestMessage(543, Guid(2)));
        connsys->handleTestMessage(SomeIPMsgHeader{2, 126, 2}, 543);

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemConnected, canReconnectAfterSameSessionFromRemoteButMessageIdReset)
    {
        connectRemote(TestInstanceId(1), Guid(2));

        expectRemoteDisconnects({2});
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 2, _, _)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(2)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{2, 123, 1}, 99, TestInstanceId(1), 0, 0, 0);

        EXPECT_CALL(connsys->consumer, handleTestMessage(543, Guid(2)));
        connsys->handleTestMessage(SomeIPMsgHeader{2, 123, 2}, 543);

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystem, canReconnectAfterFullyDisconnectConnect)
    {
        connect();
        connectRemote(TestInstanceId(1), Guid(2));
        EXPECT_CALL(connsys->consumer, handleTestMessage(543, Guid(2)));
        connsys->handleTestMessage(SomeIPMsgHeader{2, 123, 2}, 543);
        expectRemoteDisconnects({2});
        disconnect();

        connect();
        connectRemote(TestInstanceId(1), Guid(2));
        connectRemote(TestInstanceId(2), Guid(3));
        EXPECT_CALL(connsys->consumer, handleTestMessage(543, Guid(2)));
        connsys->handleTestMessage(SomeIPMsgHeader{2, 123, 2}, 543);
        expectRemoteDisconnects({2, 3});
        disconnect();
        Mock::VerifyAndClearExpectations(&connsys->connections);
    }

    TEST_F(AConnectionSystemConnected, canConnectAndHandleMoreThanOneRemote)
    {
        connectRemote(TestInstanceId(1), Guid(2));
        connectRemote(TestInstanceId(2), Guid(3), 432);

        EXPECT_CALL(connsys->consumer, handleTestMessage(333, Guid(2)));
        connsys->handleTestMessage(SomeIPMsgHeader{2, 123, 2}, 333);

        EXPECT_CALL(connsys->consumer, handleTestMessage(444, Guid(3)));
        connsys->handleTestMessage(SomeIPMsgHeader{3, 432, 2}, 444);

        EXPECT_CALL(connsys->consumer, handleTestMessage(555, Guid(2)));
        connsys->handleTestMessage(SomeIPMsgHeader{2, 123, 3}, 555);

        EXPECT_CALL(stack, sendTestMessage(TestInstanceId(2), ValidHdr(pid, 2u), 44)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendTestMessage(Guid(3), 44));

        EXPECT_CALL(stack, sendTestMessage(TestInstanceId(1), ValidHdr(pid, 2u), 33)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendTestMessage(Guid(2), 33));

        EXPECT_CALL(stack, sendTestMessage(TestInstanceId(1), ValidHdr(pid, 3u), 22)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendTestMessage(Guid(2), 22));

        expectRemoteDisconnects({2, 3});
    }

    TEST_F(AConnectionSystemConnected, canConnectAfterPinfoCounterMismatch)
    {
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));
        fromStack.handleServiceUnavailable(TestInstanceId(1));

        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 2}, 99, TestInstanceId(1), 0, 0, 0);

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 1, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(1)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 1}, 99, TestInstanceId(1), 0, 0, 0);

        expectRemoteDisconnects({1});
    }

    TEST_F(AConnectionSystemConnected, notConnectedAfterHandlePinfoMismatchAndSendPinfoFails)
    {
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 1, _, _)).WillOnce(Return(false));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 2}, 99, TestInstanceId(1), 0, 0, 0);
    }

    TEST_F(AConnectionSystemConnected, keepAliveMessageUpdatesLastReceivedForConnected)
    {
        connectRemote(TestInstanceId(1), Guid(2));
        auto tsBefore = connsys->getParticipantState(TestInstanceId(1))->lastRecv;
        fromStack.handleKeepAlive(SomeIPMsgHeader{2, 123, 2}, 0u);
        EXPECT_LT(tsBefore, connsys->getParticipantState(TestInstanceId(1))->lastRecv);
        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemConnected, keepAliveMessageUpdatesLastReceivedForNotConnected)
    {
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 1}, 99, TestInstanceId(1), 0, 0, 0);
        auto tsBefore = connsys->getParticipantState(TestInstanceId(1))->lastRecv;
        fromStack.handleKeepAlive(SomeIPMsgHeader{1, 123, 2}, 0u);
        EXPECT_LT(tsBefore, connsys->getParticipantState(TestInstanceId(1))->lastRecv);
    }

    TEST_F(AConnectionSystemConnected, failedUnicastSendDoesNotUpdateSentTimestamp)
    {
        connectRemote(TestInstanceId(3), Guid(10));

        EXPECT_CALL(stack, sendTestMessage(TestInstanceId(3), _, 44)).WillOnce(Return(false));
        expectRemoteDisconnects({10});

        auto prevTs = connsys->getParticipantState(TestInstanceId(3))->lastSent;
        EXPECT_FALSE(connsys->sendTestMessage(Guid(10), 44));
        EXPECT_EQ(prevTs, connsys->getParticipantState(TestInstanceId(3))->lastSent);
        Mock::VerifyAndClearExpectations(&connsys->connections);
    }

    TEST_F(AConnectionSystemConnected, broadcastSendUpdatesAllTimestamps)
    {
        connectRemote(TestInstanceId(1), Guid(2));
        connectRemote(TestInstanceId(3), Guid(10));

        EXPECT_CALL(stack, sendTestMessage(TestInstanceId(1), _, 44)).WillOnce(Return(true));
        EXPECT_CALL(stack, sendTestMessage(TestInstanceId(3), _, 44)).WillOnce(Return(true));

        const auto prevTs_1 = connsys->getParticipantState(TestInstanceId(1))->lastSent;
        const auto prevTs_3 = connsys->getParticipantState(TestInstanceId(3))->lastSent;
        EXPECT_TRUE(connsys->broadcastTestMessage(44));
        EXPECT_LT(prevTs_1, connsys->getParticipantState(TestInstanceId(1))->lastSent);
        EXPECT_LT(prevTs_3, connsys->getParticipantState(TestInstanceId(3))->lastSent);

        expectRemoteDisconnects({2, 10});
    }

    TEST_F(AConnectionSystemConnected, broadcastSendUpdatesOnlySuccessfulTimestamps)
    {
        connectRemote(TestInstanceId(1), Guid(2));
        connectRemote(TestInstanceId(3), Guid(10));

        EXPECT_CALL(stack, sendTestMessage(TestInstanceId(1), _, 44)).WillOnce(Return(true));
        EXPECT_CALL(stack, sendTestMessage(TestInstanceId(3), _, 44)).WillOnce(Return(false));
        expectRemoteDisconnects({10});

        const auto prevTs_1 = connsys->getParticipantState(TestInstanceId(1))->lastSent;
        const auto prevTs_3 = connsys->getParticipantState(TestInstanceId(3))->lastSent;
        EXPECT_TRUE(connsys->broadcastTestMessage(44));
        EXPECT_LT(prevTs_1, connsys->getParticipantState(TestInstanceId(1))->lastSent);
        EXPECT_EQ(prevTs_3, connsys->getParticipantState(TestInstanceId(3))->lastSent);

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemConnected, sendParticipantInfoUpdatesSendTimestampOnSuccess)
    {
        const auto tsBefore = timeCounter;
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 1}, 99, TestInstanceId(1), 0, 0, 0);
        EXPECT_LT(tsBefore, connsys->getParticipantState(TestInstanceId(1))->lastRecv);

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 1, _, _)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(1)));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        expectRemoteDisconnects({1});
    }

    TEST_F(AConnectionSystemConnected, receivedRegularMessagesUpdateLastReceived)
    {
        // test with not connected because no difference
        auto prevTs = timeCounter;

        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 123, 1}, 99, TestInstanceId(3), 0, 0, 0);
        EXPECT_LT(prevTs, connsys->getParticipantState(TestInstanceId(3))->lastRecv);
        prevTs = connsys->getParticipantState(TestInstanceId(3))->lastRecv;

        const auto initialSentTs = connsys->getParticipantState(TestInstanceId(3))->lastSent;

        connsys->handleTestMessage(SomeIPMsgHeader{10, 123, 2}, 44);
        EXPECT_LT(prevTs, connsys->getParticipantState(TestInstanceId(3))->lastRecv);
        prevTs = connsys->getParticipantState(TestInstanceId(3))->lastRecv;

        connsys->handleTestMessage(SomeIPMsgHeader{10, 123, 3}, 4324);
        EXPECT_LT(prevTs, connsys->getParticipantState(TestInstanceId(3))->lastRecv);
        prevTs = connsys->getParticipantState(TestInstanceId(3))->lastRecv;

        connsys->handleTestMessage(SomeIPMsgHeader{10, 123, 4}, 4324);
        EXPECT_LT(prevTs, connsys->getParticipantState(TestInstanceId(3))->lastRecv);
        prevTs = connsys->getParticipantState(TestInstanceId(3))->lastRecv;

        EXPECT_EQ(initialSentTs, connsys->getParticipantState(TestInstanceId(3))->lastSent);
    }

    TEST_F(AConnectionSystemConnected, unicastSendUpdatesSendTimestamp)
    {
        connectRemote(TestInstanceId(3), Guid(10));

        const auto initialRecvTs = connsys->getParticipantState(TestInstanceId(3))->lastRecv;

        EXPECT_CALL(stack, sendTestMessage(TestInstanceId(3), _, 44)).WillOnce(Return(true));
        auto prevTs = connsys->getParticipantState(TestInstanceId(3))->lastSent;
        EXPECT_TRUE(connsys->sendTestMessage(Guid(10), 44));
        EXPECT_LT(prevTs, connsys->getParticipantState(TestInstanceId(3))->lastSent);

        EXPECT_CALL(stack, sendTestMessage(TestInstanceId(3), _, 33)).WillOnce(Return(true));
        prevTs = connsys->getParticipantState(TestInstanceId(3))->lastSent;
        EXPECT_TRUE(connsys->sendTestMessage(Guid(10), 33));
        EXPECT_LT(prevTs, connsys->getParticipantState(TestInstanceId(3))->lastSent);

        EXPECT_CALL(stack, sendTestMessage(TestInstanceId(3), _, 2)).WillOnce(Return(true));
        prevTs = connsys->getParticipantState(TestInstanceId(3))->lastSent;
        EXPECT_TRUE(connsys->sendTestMessage(Guid(10), 2));
        EXPECT_LT(prevTs, connsys->getParticipantState(TestInstanceId(3))->lastSent);

        EXPECT_EQ(initialRecvTs, connsys->getParticipantState(TestInstanceId(3))->lastRecv);

        expectRemoteDisconnects({10});
    }

    TEST_F(AConnectionSystem, canHandleKeepAliveForUnknownParticipant)
    {
        fromStack.handleKeepAlive(SomeIPMsgHeader{10, 123, 1}, 0);
        //expect nothing
    }

    TEST_F(AConnectionSystemConnected, canConnectWhenRemoteIsUpFirst)
    {
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 123, 1}, 99, TestInstanceId(1), 10, 0, 0);

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 10, _, _)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(10)));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        expectRemoteDisconnects({10});
    }

    TEST_F(AConnectionSystemConnected, canHandleKeepaliveBeforeConnectToRemote)
    {
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 123, 1}, 99, TestInstanceId(1), 10, 0, 0);
        fromStack.handleKeepAlive(SomeIPMsgHeader{10, 123, 2}, 0);

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 10, _, _)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(10)));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        fromStack.handleKeepAlive(SomeIPMsgHeader{10, 123, 3}, 0);

        expectRemoteDisconnects({10});
    }

    TEST_F(AConnectionSystemConnected, canHandleKeepaliveMismatchOnUnconnected)
    {
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 123, 1}, 99, TestInstanceId(1), 10, 0, 0);
        fromStack.handleKeepAlive(SomeIPMsgHeader{10, 123, 3}, 0);

        InSequence seq;
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 10, _, _)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(10)));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        // can react on earlier mismatch only on next received message
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(10)));
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 10, _, _)).WillOnce(Return(true));
        fromStack.handleKeepAlive(SomeIPMsgHeader{10, 123, 4}, 0);

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(10)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 124, 1}, 99, TestInstanceId(1), 10, 0, 0);

        expectRemoteDisconnects({10});
    }

    TEST_F(AConnectionSystemConnected, canHandlePinfoNewSessionOnUnconnected)
    {
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 123, 1}, 99, TestInstanceId(1), 10, 0, 0);
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 124, 1}, 99, TestInstanceId(1), 10, 0, 0);  // new session

        InSequence seq;
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 10, _, _)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(10)));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(10)));
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 10, _, _)).WillOnce(Return(true));
        fromStack.handleKeepAlive(SomeIPMsgHeader{10, 124, 2}, 0);

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(10)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 125, 1}, 99, TestInstanceId(1), 10, 0, 0);

        expectRemoteDisconnects({10});
    }

    TEST_F(AConnectionSystemConnected, suppressSendingPinfoEverySecondTimeWhenContinuallyGetsNewSessions)
    {
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(10)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 123, 1}, 99, TestInstanceId(1), 10, 0, 0);
        // now fully connected

        // disconnect+connect by counter mismatch due to new session -> sending pinfo
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(10)));
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 10, _, _)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(10)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 124, 1}, 99, TestInstanceId(1), 10, 0, 0);

        // disconnect+connect but suppress sending pinfo again to avoid reconnect ping-pong
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(10)));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(10)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 128, 1}, 99, TestInstanceId(1), 10, 0, 0);

        // disconnect+connect by counter mismatch due to new session -> sending pinfo
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(10)));
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 10, _, _)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(10)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 124, 1}, 99, TestInstanceId(1), 10, 0, 0);

        // disconnect+connect but suppress sending pinfo again to avoid reconnect ping-pong
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(10)));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(10)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 128, 1}, 99, TestInstanceId(1), 10, 0, 0);

        expectRemoteDisconnects({10});
    }

    TEST_F(AConnectionSystemConnected, continuesSendingWithPreviousSessionWhenSkipSendingPinfo)
    {
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(10)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 123, 1}, 99, TestInstanceId(1), 10, 0, 0);
        // now fully connected

        // disconnect+connect by counter mismatch due to new session -> sending pinfo
        SomeIPMsgHeader pinfoHdr;
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(10)));
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 10, _, _)).WillOnce(DoAll(SaveArg<1>(&pinfoHdr), Return(true)));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(10)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 124, 1}, 99, TestInstanceId(1), 10, 0, 0);

        // disconnect+connect but suppress sending pinfo again to avoid reconnect ping-pong
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(10)));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(10)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 128, 1}, 99, TestInstanceId(1), 10, 0, 0);

        // continues sending with previously announced session
        EXPECT_CALL(stack, sendTestMessage(TestInstanceId(1), SomeIPMsgHeader{pinfoHdr.participantId, pinfoHdr.sessionId, 2u}, 44)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendTestMessage(Guid(10), 44));

        expectRemoteDisconnects({10});
    }

    TEST_F(AConnectionSystemConnected, canHandleIncomingMessagesAfterSkippedSendingPinfo)
    {
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(10)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 123, 1}, 99, TestInstanceId(1), 10, 0, 0);
        // now fully connected

        // disconnect+connect by counter mismatch due to new session -> sending pinfo
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(10)));
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 10, _, _)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(10)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 124, 1}, 99, TestInstanceId(1), 10, 0, 0);

        // disconnect+connect but suppress sending pinfo again to avoid reconnect ping-pong
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(10)));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(10)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 128, 1}, 99, TestInstanceId(1), 10, 0, 0);

        // can handle incoming message following prev pinfo
        fromStack.handleKeepAlive(SomeIPMsgHeader{10, 128, 2}, 0);

        expectRemoteDisconnects({10});
    }

    TEST_F(AConnectionSystemConnected, canHandleKeepaliveAsFirstMesageFromUnknown)
    {
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));
        fromStack.handleKeepAlive(SomeIPMsgHeader{10, 125, 2}, 0);
    }


    class AConnectionSystemTimeMockConnected : public AConnectionSystemConnected
    {
    public:
        AConnectionSystemTimeMockConnected()
        {
            hasCounterTime = false;
        }

        MOCK_METHOD(std::chrono::steady_clock::time_point, steadyClockNow, (), (override));

        static std::chrono::steady_clock::time_point Tp(uint64_t tpMilliseconds)
        {
            return std::chrono::steady_clock::time_point{std::chrono::milliseconds(tpMilliseconds)};
        }

        static std::chrono::milliseconds Ms(uint64_t milliseconds)
        {
            return std::chrono::milliseconds(milliseconds);
        }

        std::chrono::steady_clock::time_point doLoop(std::chrono::milliseconds interval, std::chrono::milliseconds timeout, std::chrono::steady_clock::time_point now)
        {
            EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(now));
            return connsys->doOneThreadLoop(interval, timeout);
        }
    };

    TEST_F(AConnectionSystemTimeMockConnected, repeatedlyTriesPinfoWithBackoffWhenFails)
    {
        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(1)));
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(false));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        doLoop(Ms(2), Ms(3), Tp(1));
        doLoop(Ms(2), Ms(3), Tp(2));

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(false));
        doLoop(Ms(2), Ms(3), Tp(3));
        doLoop(Ms(2), Ms(3), Tp(4));

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(false));
        doLoop(Ms(2), Ms(3), Tp(5));
    }

    TEST_F(AConnectionSystemTimeMockConnected, canGetConnectedAfterPinfoRetryAndAnswer)
    {
        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(1)));
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(false));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        doLoop(Ms(2), Ms(3), Tp(3));

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(1)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 1}, 99, TestInstanceId(1), 0, 0, 0);

        expectRemoteDisconnects({1});
    }

    TEST_F(AConnectionSystemTimeMockConnected, canGetConnectedAfterPinfoRetryWhenAlreadyHasReceivedPinfo)
    {
        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(1)));
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(false));
        fromStack.handleServiceAvailable(TestInstanceId(1));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 1}, 99, TestInstanceId(1), 0, 0, 0);

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 1, _, _)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(1)));
        doLoop(Ms(2), Ms(3), Tp(3));

        expectRemoteDisconnects({1});
    }

    TEST_F(AConnectionSystemTimeMockConnected, canGetConnectedAfterPinfoRetryWhenHasPinfoAndServiceUpSendPinfoFails)
    {
        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(1)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 1}, 99, TestInstanceId(1), 0, 0, 0);

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 1, _, _)).WillOnce(Return(false));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 1, _, _)).WillOnce(Return(true));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(1)));
        doLoop(Ms(2), Ms(3), Tp(3));

        expectRemoteDisconnects({1});
    }

    TEST_F(AConnectionSystemTimeMockConnected, sendsPinfoInThreadAfterRegularMessageFails)
    {
        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(1)));
        connectRemote(TestInstanceId(1), Guid(2));

        EXPECT_CALL(stack, sendTestMessage(TestInstanceId(1), ValidHdr(pid, 2u), 44)).WillOnce(Return(false));
        expectRemoteDisconnects({2});
        EXPECT_FALSE(connsys->sendTestMessage(Guid(2), 44));

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 2, _, _)).WillOnce(Return(true));
        doLoop(Ms(2), Ms(3), Tp(3));

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(2)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{2, 123, 1}, 99, TestInstanceId(1), pid, 0, 0);

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemTimeMockConnected, canConnectAfterConnectedDownUpDroppedPinfo)
    {
        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(1)));
        connectRemote(TestInstanceId(1), Guid(2));

        expectRemoteDisconnects({2});
        fromStack.handleServiceUnavailable(TestInstanceId(1));

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 2, _, _)).WillOnce(Return(false));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 2, _, _)).WillOnce(Return(true));
        doLoop(Ms(2), Ms(3), Tp(3));

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(2)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{2, 123, 1}, 99, TestInstanceId(1), pid, 0, 0);

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemTimeMockConnected, canConnectAfterReceivedPinfoMismatchAndSendPinfoFail)
    {
        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(1)));
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 1, _, _)).WillOnce(Return(false));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 2}, 99, TestInstanceId(1), 0, 0, 0);

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 1, _, _)).WillOnce(Return(true));
        doLoop(Ms(2), Ms(3), Tp(3));

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(1)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{1, 123, 1}, 99, TestInstanceId(1), pid, 0, 0);

        expectRemoteDisconnects({1});
    }

    TEST_F(AConnectionSystemTimeMockConnected, threadDoesNothingWhenServiceDown)
    {
        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(1)));
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(false));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(false));
        doLoop(Ms(2), Ms(3), Tp(3));

        fromStack.handleServiceUnavailable(TestInstanceId(1));
        doLoop(Ms(2), Ms(3), Tp(5));
        doLoop(Ms(2), Ms(3), Tp(10));
    }

    TEST_F(AConnectionSystemTimeMockConnected, threadSendsKeepAliveAfterSuccessfulPinfo)
    {
        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(1)));
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        doLoop(Ms(2), Ms(3), Tp(2));
        EXPECT_CALL(stack, sendKeepAlive(TestInstanceId(1), ValidHdr(pid, 2u), _)).WillOnce(Return(true));
        doLoop(Ms(2), Ms(3), Tp(3));
        doLoop(Ms(2), Ms(3), Tp(4));
        EXPECT_CALL(stack, sendKeepAlive(TestInstanceId(1), ValidHdr(pid, 3u), _)).WillOnce(Return(true));
        doLoop(Ms(2), Ms(3), Tp(5));
        EXPECT_CALL(stack, sendKeepAlive(TestInstanceId(1), ValidHdr(pid, 4u), _)).WillOnce(Return(true));
        doLoop(Ms(2), Ms(3), Tp(20));
        doLoop(Ms(2), Ms(3), Tp(21));
        EXPECT_CALL(stack, sendKeepAlive(TestInstanceId(1), ValidHdr(pid, 5u), _)).WillOnce(Return(true));
        doLoop(Ms(2), Ms(3), Tp(22));
    }

    TEST_F(AConnectionSystemTimeMockConnected, threadSendsKeepAliveAfterConnected)
    {
        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(5)));
        connectRemote(TestInstanceId(1), Guid(2));

        fromStack.handleKeepAlive(SomeIPMsgHeader{2, 123, 2}, 0);
        doLoop(Ms(2), Ms(3), Tp(6));
        EXPECT_CALL(stack, sendKeepAlive(TestInstanceId(1), ValidHdr(pid, 2u), _)).WillOnce(Return(true));
        doLoop(Ms(2), Ms(3), Tp(7));
        fromStack.handleKeepAlive(SomeIPMsgHeader{2, 123, 3}, 0);
        doLoop(Ms(2), Ms(3), Tp(8));
        EXPECT_CALL(stack, sendKeepAlive(TestInstanceId(1), ValidHdr(pid, 3u), _)).WillOnce(Return(true));
        doLoop(Ms(2), Ms(3), Tp(9));

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemTimeMockConnected, threadStopsKeepAliveWhenServiceDown)
    {
        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(1)));
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        EXPECT_CALL(stack, sendKeepAlive(TestInstanceId(1), ValidHdr(pid, 2u), _)).WillOnce(Return(true));
        doLoop(Ms(2), Ms(3), Tp(3));

        fromStack.handleServiceUnavailable(TestInstanceId(1));
        doLoop(Ms(2), Ms(3), Tp(5));
        doLoop(Ms(2), Ms(3), Tp(10));
    }

    TEST_F(AConnectionSystemTimeMockConnected, threadDoesNotSendKeepAliveWhenSendingRegularMessages)
    {
        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(5)));
        connectRemote(TestInstanceId(1), Guid(2));

        doLoop(Ms(2), Ms(3), Tp(6));
        EXPECT_CALL(stack, sendTestMessage(TestInstanceId(1), ValidHdr(pid, 2u), 44)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendTestMessage(Guid(2), 44));
        fromStack.handleKeepAlive(SomeIPMsgHeader{2, 123, 2}, 0);

        doLoop(Ms(2), Ms(3), Tp(7));
        EXPECT_CALL(stack, sendTestMessage(TestInstanceId(1), ValidHdr(pid, 3u), 44)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendTestMessage(Guid(2), 44));
        fromStack.handleKeepAlive(SomeIPMsgHeader{2, 123, 3}, 0);

        doLoop(Ms(2), Ms(3), Tp(8));

        EXPECT_CALL(stack, sendKeepAlive(TestInstanceId(1), ValidHdr(pid, 4u), _)).WillOnce(Return(true));
        doLoop(Ms(2), Ms(3), Tp(9));

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemTimeMockConnected, threadDisconnectsWhenLastReceivedMessageTooOld)
    {
        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(5)));
        EXPECT_CALL(stack, sendKeepAlive(TestInstanceId(1), _, _)).WillRepeatedly(Return(true));
        connectRemote(TestInstanceId(1), Guid(2));

        doLoop(Ms(2), Ms(3), Tp(6));
        fromStack.handleKeepAlive(SomeIPMsgHeader{2, 123, 2}, 0);

        doLoop(Ms(2), Ms(3), Tp(7));
        doLoop(Ms(2), Ms(3), Tp(8));

        expectRemoteDisconnects({2});
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, _, _, _)).WillOnce(Return(true));
        doLoop(Ms(2), Ms(3), Tp(9)); // Tp >= 6+3
        Mock::VerifyAndClearExpectations(&connsys->connections);
    }

    TEST_F(AConnectionSystemTimeMockConnected, threadDoesNothingWhenRegularlyReceivingMessages)
    {
        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(6)));
        EXPECT_CALL(stack, sendKeepAlive(TestInstanceId(1), _, _)).WillRepeatedly(Return(true));
        connectRemote(TestInstanceId(1), Guid(2));

        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(9)));
        EXPECT_CALL(connsys->consumer, handleTestMessage(_, Guid(2)));
        connsys->handleTestMessage(SomeIPMsgHeader{2, 123, 2}, 44);
        doLoop(Ms(2), Ms(3), Tp(9));

        doLoop(Ms(2), Ms(3), Tp(10));

        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(12)));
        EXPECT_CALL(connsys->consumer, handleTestMessage(_, Guid(2)));
        connsys->handleTestMessage(SomeIPMsgHeader{2, 123, 3}, 44);
        doLoop(Ms(2), Ms(3), Tp(12));

        doLoop(Ms(2), Ms(3), Tp(13));

        expectRemoteDisconnects({2});
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 2, _, _)).WillOnce(Return(true));
        doLoop(Ms(2), Ms(3), Tp(15));
        Mock::VerifyAndClearExpectations(&connsys->connections);
    }

    TEST_F(AConnectionSystemTimeMockConnected, threadDoesNothingWhenNoMessagesAndNotConnected)
    {
        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(6)));
        EXPECT_CALL(stack, sendKeepAlive(TestInstanceId(1), _, _)).WillRepeatedly(Return(true));
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        doLoop(Ms(2), Ms(3), Tp(9));
        doLoop(Ms(2), Ms(3), Tp(10));
        doLoop(Ms(2), Ms(3), Tp(20));
        doLoop(Ms(2), Ms(3), Tp(30));
        doLoop(Ms(2), Ms(3), Tp(40));
    }

    TEST_F(AConnectionSystemTimeMockConnected, canConnectWhenSendKeepaliveFailsInThread)
    {
        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(5)));
        connectRemote(TestInstanceId(1), Guid(2));

        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(8)));
        fromStack.handleKeepAlive(SomeIPMsgHeader{2, 123, 2}, 0);

        EXPECT_CALL(stack, sendKeepAlive(TestInstanceId(1), ValidHdr(pid, 2u), _)).WillOnce(Return(false));
        expectRemoteDisconnects({2});
        doLoop(Ms(2), Ms(3), Tp(10));

        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 2, _, _)).WillOnce(Return(true));
        doLoop(Ms(2), Ms(3), Tp(11));

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(2)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{2, 123, 1}, 99, TestInstanceId(1), pid, 0, 0);

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemTimeMockConnected, canReconnectWhenDisconnectedByRecvTimeout)
    {
        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(5)));
        EXPECT_CALL(stack, sendKeepAlive(TestInstanceId(1), _, _)).WillRepeatedly(Return(true));
        connectRemote(TestInstanceId(1), Guid(2));

        doLoop(Ms(2), Ms(3), Tp(7));

        expectRemoteDisconnects({2});
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 2, _, _)).WillOnce(Return(true));
        doLoop(Ms(2), Ms(3), Tp(9));

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(2)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{2, 123, 1}, 99, TestInstanceId(1), pid, 0, 0);

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemTimeMockConnected, nextWakeupHasFixedDefaultMaximum)
    {
        EXPECT_EQ(Tp(8), doLoop(Ms(3), Ms(4), Tp(5)));
        EXPECT_EQ(Tp(9), doLoop(Ms(3), Ms(4), Tp(6)));
        EXPECT_EQ(Tp(9), doLoop(Ms(1), Ms(4), Tp(8)));
    }

    TEST_F(AConnectionSystemTimeMockConnected, nextWakeupUsesNextSentForAvailable)
    {
        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(5)));
        EXPECT_CALL(stack, sendKeepAlive(TestInstanceId(1), _, _)).WillRepeatedly(Return(true));
        connectRemote(TestInstanceId(1), Guid(2));

        EXPECT_EQ(Tp(7), doLoop(Ms(2), Ms(3), Tp(5)));
        EXPECT_EQ(Tp(7), doLoop(Ms(2), Ms(3), Tp(6)));
        fromStack.handleKeepAlive(SomeIPMsgHeader{2, 123, 2}, 0);

        EXPECT_EQ(Tp(9), doLoop(Ms(2), Ms(3), Tp(7)));
        EXPECT_EQ(Tp(9), doLoop(Ms(2), Ms(3), Tp(8)));

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemTimeMockConnected, nextWakeupUsesTimeoutForConnected)
    {
        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(5)));
        EXPECT_CALL(stack, sendKeepAlive(TestInstanceId(1), _, _)).WillRepeatedly(Return(true));
        connectRemote(TestInstanceId(1), Guid(2));

        EXPECT_EQ(Tp(7), doLoop(Ms(2), Ms(3), Tp(5)));
        EXPECT_EQ(Tp(7), doLoop(Ms(2), Ms(3), Tp(6)));

        EXPECT_EQ(Tp(8), doLoop(Ms(2), Ms(3), Tp(7))); // timeout 5+3
        // keepalive sent
        fromStack.handleKeepAlive(SomeIPMsgHeader{2, 123, 2}, 0);

        EXPECT_EQ(Tp(9), doLoop(Ms(2), Ms(3), Tp(8)));
        EXPECT_EQ(Tp(10), doLoop(Ms(2), Ms(3), Tp(9)));
        fromStack.handleKeepAlive(SomeIPMsgHeader{2, 123, 3}, 0);

        EXPECT_EQ(Tp(11), doLoop(Ms(2), Ms(3), Tp(10)));

        expectRemoteDisconnects({2});
    }

    TEST_F(AConnectionSystemTimeMockConnected, sendsKeepaliveAfterSkippedPinfoWithLastAnnouncedSession)
    {
        EXPECT_CALL(*this, steadyClockNow()).WillRepeatedly(Return(Tp(1)));
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 0, _, _)).WillOnce(Return(true));
        fromStack.handleServiceAvailable(TestInstanceId(1));

        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(10)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 123, 1}, 99, TestInstanceId(1), 10, 0, 0);
        // now fully connected

        // disconnect+connect by counter mismatch due to new session -> sending pinfo
        SomeIPMsgHeader pinfoHdr;
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(10)));
        EXPECT_CALL(stack, sendParticipantInfo(TestInstanceId(1), ValidHdr(pid, 1u), 99, iid, 10, _, _)).WillOnce(DoAll(SaveArg<1>(&pinfoHdr), Return(true)));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(10)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 124, 1}, 99, TestInstanceId(1), 10, 0, 0);

        // disconnect+connect but suppress sending pinfo again to avoid reconnect ping-pong
        EXPECT_CALL(connsys->connections, participantHasDisconnected(Guid(10)));
        EXPECT_CALL(connsys->connections, newParticipantHasConnected(Guid(10)));
        fromStack.handleParticipantInfo(SomeIPMsgHeader{10, 128, 1}, 99, TestInstanceId(1), 10, 0, 0);

        // expect regular keepalive sending on last sent session (this will trigger reconnect when pinfo sending was falsely skipped)
        EXPECT_CALL(stack, sendKeepAlive(TestInstanceId(1), SomeIPMsgHeader{pinfoHdr.participantId, pinfoHdr.sessionId, 2u}, _)).WillOnce(Return(true));
        doLoop(Ms(2), Ms(3), Tp(3));

        expectRemoteDisconnects({10});
    }


    // TODO: test expectedReceiverPid 0, ok, wrong
    // TODO: pid change, old pid was connected
    // TODO: stresstest -> must connect in the end
}
