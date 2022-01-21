//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ConnectionSystemTestCommon.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "ScopedConsoleLogDisable.h"
#include <chrono>

namespace ramses_internal
{
    using namespace TestConnectionSystemBase;

    namespace {
        enum class EMinorProtocolVersion
        {
            FallbackOnly,
            FallbackAndNew
        };
    }

    class AConnectionSystemSynchronousStressTest : public TestWithParam<EMinorProtocolVersion>
    {
    public:
        virtual void SetUp() override
        {
            auto stack = std::make_shared<StrictMock<StackMock>>();

            EXPECT_CALL(*stack, getServiceInstanceId()).WillRepeatedly(Return(TestInstanceId(3)));

            connsys = std::make_unique<TestConnectionSystem>(stack,
                                                             123, ParticipantIdentifier(Guid(2), "foobar"), 4,
                                                             lock,
                                                             std::chrono::milliseconds(0), std::chrono::milliseconds(0),
                                                             [&](){ return currentTime; },
                                                             true);

            ASSERT_TRUE(connsys);

            EXPECT_CALL(connsys->connections, newParticipantHasConnected(_)).Times(AnyNumber());
            EXPECT_CALL(connsys->connections, participantHasDisconnected(_)).Times(AnyNumber());

            fromStack = connsys.get();
            EXPECT_CALL(*stack, connect()).WillOnce(Return(true));
            ASSERT_TRUE(connsys->connect());

        }

        PlatformLock lock;
        std::unique_ptr<TestConnectionSystem> connsys;
        std::random_device randomSource;
        Callbacks* fromStack;
        std::chrono::steady_clock::time_point currentTime{std::chrono::milliseconds{1}};
        ScopedConsoleLogDisable consoleDisabler;
    };

    INSTANTIATE_TEST_SUITE_P(AConnectionSystemSynchronousStressTestP,
                             AConnectionSystemSynchronousStressTest,
                             ::testing::Values(EMinorProtocolVersion::FallbackOnly, EMinorProtocolVersion::FallbackAndNew));

    TEST_P(AConnectionSystemSynchronousStressTest, synchronousStressTest)
    {
        unsigned int seed = randomSource();
        SCOPED_TRACE(seed);
        std::mt19937 gen(seed);
        auto rnd = [&]() {
            std::uniform_int_distribution<uint32_t> dis(0, 100);
            return dis(gen);
        };
        auto num = [&]() {
            std::uniform_int_distribution<uint32_t> dis(0, 6);
            return dis(gen);
        };

        for (int i = 0; i < 20000; ++i)
        {
            currentTime += std::chrono::milliseconds{num()};
            EXPECT_CALL(*connsys->stack, connect()).WillRepeatedly(Return(rnd() > 20));
            EXPECT_CALL(*connsys->stack, disconnect()).WillRepeatedly(Return(rnd() > 20));
            EXPECT_CALL(*connsys->stack, logConnectionState(_)).Times(AnyNumber());
            ;

            EXPECT_CALL(*connsys->stack, sendParticipantInfo(_, _, _, _, _, _, _, _)).WillRepeatedly(Return(rnd() > 20));
            EXPECT_CALL(*connsys->stack, sendKeepAlive(_, _, _, _)).WillRepeatedly(Return(rnd() > 20));
            EXPECT_CALL(*connsys->stack, sendTestMessage(_, _, _)).WillRepeatedly(Return(rnd() > 20));

            EXPECT_CALL(connsys->consumer, handleTestMessage(_, _)).WillRepeatedly(Return(rnd() > 20));

            if (rnd() < 10)
            {
                connsys->disconnect();
                connsys->connect();
            }
            if (rnd() < 25)
            {
                const auto interval = num()+1;
                connsys->doOneThreadLoop(std::chrono::milliseconds(interval),std::chrono::milliseconds(interval + 1 + num()));
            }
            if (rnd() < 75)
                connsys->broadcastTestMessage(num());
            if (rnd() < 75)
                connsys->sendTestMessage(Guid(num()), num());
            if (rnd() < 30)
            {
                if (GetParam() == EMinorProtocolVersion::FallbackOnly)
                {
                    fromStack->handleParticipantInfo(SomeIPMsgHeader{num(), num(), num()}, static_cast<uint16_t>(num()), 0, TestInstanceId(static_cast<uint16_t>(num())), num(), static_cast<uint8_t>(num()), num());
                }
                else
                {
                    // use single random value as base for remotePid, remoteIid and minorProtocolVersion. Then unsupported cases
                    // where iid jumps versions or changes pid cannot be generated.
                    const uint32_t val = num();
                    fromStack->handleParticipantInfo(SomeIPMsgHeader{val, num(), num()}, static_cast<uint16_t>(num()), val % 2, TestInstanceId(static_cast<uint16_t>(val)), num(), static_cast<uint8_t>(num()), num());
                }
            }
            if (rnd() < 40)
                fromStack->handleKeepAlive(SomeIPMsgHeader{num(), num(), num()}, num(), false);
            if (rnd() < 40)
                connsys->handleTestMessage(SomeIPMsgHeader{num(), num(), num()}, num());
            if (rnd() < 50)
                fromStack->handleServiceAvailable(TestInstanceId(static_cast<uint16_t>(num())));
            if (rnd() < 50)
                fromStack->handleServiceUnavailable(TestInstanceId(static_cast<uint16_t>(num())));
            if (rnd() < 5)
            {
                connsys->logPeriodicInfo();
                connsys->logConnectionInfo();
            }
        }
    }
}
