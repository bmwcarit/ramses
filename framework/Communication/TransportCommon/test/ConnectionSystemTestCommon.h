//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CONNECTIONSYSTEMTESTCOMMON_H
#define RAMSES_CONNECTIONSYSTEMTESTCOMMON_H

#include "TransportCommon/ConnectionSystemBase.h"
#include "MockConnectionStatusListener.h"
#include "gmock/gmock.h"

namespace ramses_internal
{
    using namespace ::testing;

    MATCHER_P2(ValidHdr, pid, mid, "")
    {
        (void)result_listener;
        return pid == arg.participantId &&
            0 != arg.sessionId &&
            mid == arg.messageId;
    }

    namespace TestConnectionSystemBase
    {
        using TestInstanceId = StronglyTypedValue<uint16_t, 0xFFFF, struct TestInstanceIdTag>;

        using Callbacks = ISomeIPStackCallbacksCommon<TestInstanceId>;

        class StackMock : public ISomeIPStackCommon<TestInstanceId>
        {
        public:
            MOCK_METHOD(bool, connect, (), (override));
            MOCK_METHOD(bool, disconnect, (), (override));
            MOCK_METHOD(void, logConnectionState, (StringOutputStream& sos), (override));

            MOCK_METHOD(InstanceIdType, getServiceInstanceId, (), (const, override));

            MOCK_METHOD(bool, sendParticipantInfo, (InstanceIdType to, const SomeIPMsgHeader& header, uint16_t protocolVersion, InstanceIdType senderInstanceId, uint64_t expectedReceiverPid, uint8_t clockType, uint64_t timestampNow), (override));
            MOCK_METHOD(bool, sendKeepAlive, (InstanceIdType to, const SomeIPMsgHeader& header, uint64_t timestampNow), (override));

            MOCK_METHOD(bool, sendTestMessage, (InstanceIdType to, const SomeIPMsgHeader& header, uint32_t arg), ());  // not derived from interface
        };

        class MessageConsumerMock
        {
        public:
            MOCK_METHOD(bool, handleTestMessage, (uint32_t arg, const Guid& pid), ());
        };

        class TestConnectionSystem : public ConnectionSystemBase<Callbacks>
        {
        public:
            TestConnectionSystem(const std::shared_ptr<StrictMock<StackMock>>& _stack, UInt32 communicationUserID, const ParticipantIdentifier& namedPid,
                                 UInt32 protocolVersion, PlatformLock& lock,
                                 std::chrono::milliseconds keepAliveInterval, std::chrono::milliseconds keepAliveTimeout,
                                 std::function<std::chrono::steady_clock::time_point(void)> steadyClockNow)
                : ConnectionSystemBase(_stack, communicationUserID, namedPid, protocolVersion, lock, stats,
                                       keepAliveInterval, keepAliveTimeout,
                                       std::move(steadyClockNow),
                                       CONTEXT_COMMUNICATION, "TEST")
                , stack(_stack)
            {
                getConnectionStatusUpdateNotifier().registerForConnectionUpdates(&connections);
            }

            virtual ~TestConnectionSystem() override
            {
                getConnectionStatusUpdateNotifier().unregisterForConnectionUpdates(&connections);
            }

            using ConnectionSystemBase::CheckConstructorArguments;

            bool sendTestMessage(const Guid& to, uint32_t arg)
            {
                return sendUnicast("sendTestMessage", to, [&](TestInstanceId iid, SomeIPMsgHeader hdr) {
                    return stack->sendTestMessage(iid, hdr, arg);
                });
            }

            bool broadcastTestMessage(uint32_t arg)
            {
                return sendBroadcast("sendTestMessage", [&](TestInstanceId iid, SomeIPMsgHeader hdr) {
                    return stack->sendTestMessage(iid, hdr, arg);
                });
            }

            void handleTestMessage(const SomeIPMsgHeader& header, uint32_t arg)
            {
                if (const Guid* pid = processReceivedMessageHeader(header, "handleTestMessage"))
                    consumer.handleTestMessage(arg, *pid);
            }

            std::shared_ptr<StrictMock<StackMock>> stack;
            StatisticCollectionFramework stats;
            StrictMock<MockConnectionStatusListener> connections;
            StrictMock<MessageConsumerMock> consumer;
        };
    }

    class CountingConnectionSystemClock
    {
    public:
        std::chrono::steady_clock::time_point operator()()
        {
            tp += std::chrono::seconds{1};
            return tp;
        }
    private:
        std::chrono::steady_clock::time_point tp{std::chrono::seconds(0)};
    };
}

MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses_internal::TestConnectionSystemBase::TestInstanceId)

#endif
