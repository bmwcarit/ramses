//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Communication/TransportTCP/TCPConnectionSystem.h"
#include "internal/Core/Utils/StatisticCollection.h"
#include "internal/Core/Utils/ThreadBarrier.h"
#include "ScopedConsoleLogDisable.h"
#include "CommunicationSystemTest.h"
#include "gtest/gtest.h"
#include <thread>

namespace ramses::internal
{
    class ATCPConnectionSystem : public ::testing::Test
    {
    public:
        ATCPConnectionSystem()
            : addr(Guid(111), "foo", "127.0.0.1", 0)
            , daemonAddr(TCPConnectionSystem::GetDaemonId(), "SM", "127.0.0.1", 5999)
            , connsys(addr, 0, daemonAddr, false, lock, statistics, std::chrono::milliseconds{1000}, std::chrono::milliseconds{10000})
            , startBarrier(5)
        {}

        NetworkParticipantAddress addr;
        NetworkParticipantAddress daemonAddr;
        PlatformLock lock;
        StatisticCollectionFramework statistics;
        TCPConnectionSystem connsys;
        ThreadBarrier startBarrier;
        Guid other{333};
        std::atomic<bool> shouldStop{false};
    };


    TEST_F(ATCPConnectionSystem, threadStressTest)
    {
        ScopedConsoleLogDisable consoleDisabler;

        std::thread log_per {
            [&]() {
                startBarrier.wait();
                while (!shouldStop)
                {
                    {
                        PlatformGuard guard(lock);
                        connsys.triggerLogMessageForPeriodicLog();
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds{1});
                }
            }
        };
        std::thread log_cinfo {
            [&]() {
                startBarrier.wait();
                while (!shouldStop)
                {
                    connsys.logConnectionInfo();
                    std::this_thread::sleep_for(std::chrono::milliseconds{1});
                }
            }
        };
        std::thread sender {
            [&]() {
                startBarrier.wait();
                while (!shouldStop)
                {
                    {
                        PlatformGuard guard(lock);
                        connsys.sendSubscribeScene(other, SceneId{123});
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds{1});
                }
            }
        };
        std::thread connecter {
            [&]() {
                startBarrier.wait();
                while (!shouldStop)
                {
                    connsys.connectServices();
                    std::this_thread::sleep_for(std::chrono::milliseconds{50});
                    connsys.disconnectServices();
                    std::this_thread::sleep_for(std::chrono::milliseconds{50});
                }
            }
        };

        startBarrier.wait();
        std::this_thread::sleep_for(std::chrono::seconds{1});
        shouldStop = true;
        log_per.join();
        log_cinfo.join();
        connecter.join();
        sender.join();
    }

    class ACommunicationSystemWithDaemon_TCP : public ACommunicationSystemWithDaemon
    {
    };

    INSTANTIATE_TEST_SUITE_P(TypedCommunicationTest, ACommunicationSystemWithDaemon_TCP, ::testing::Combine(::testing::Values(ECommunicationSystemType::Tcp), ::testing::Values(EServiceType::Ramses)));

    TEST_P(ACommunicationSystemWithDaemon_TCP, canEstablishConnectionToNewParticipantWithSameGuid)
    {
        const Guid csw1Id(2);
        const Guid csw2Id(1);   // csw2 id MUST be smaller => forces csw1 to initiate connections

        auto csw1 = std::make_unique<CommunicationSystemTestWrapper>(*state, "csw1", csw1Id);
        auto csw2 = std::make_unique<CommunicationSystemTestWrapper>(*state, "csw2", csw2Id);

        EXPECT_TRUE(csw1->commSystem->connectServices());
        {
            PlatformGuard g(csw1->frameworkLock);
            EXPECT_CALL(csw1->statusUpdateListener, newParticipantHasConnected(csw2->id));
        }
        csw1->registerForConnectionUpdates();

        EXPECT_TRUE(csw2->commSystem->connectServices());
        {
            PlatformGuard g(csw2->frameworkLock);
            EXPECT_CALL(csw2->statusUpdateListener, newParticipantHasConnected(csw1->id));
        }
        csw2->registerForConnectionUpdates();


        ASSERT_TRUE(state->event.waitForEvents(2));

        {
            PlatformGuard g(csw1->frameworkLock);
            EXPECT_CALL(csw1->statusUpdateListener, participantHasDisconnected(csw2->id));
        }
        {
            PlatformGuard g(csw2->frameworkLock);
            EXPECT_CALL(csw2->statusUpdateListener, participantHasDisconnected(csw1->id));
        }
        csw2->commSystem->disconnectServices();
        ASSERT_TRUE(state->event.waitForEvents(2));

        Mock::VerifyAndClearExpectations(&csw1->statusUpdateListener);

        // construct new csw2 with same guid but most likely other port
        csw2 = std::make_unique<CommunicationSystemTestWrapper>(*state, "csw2", csw2Id);

        {
            PlatformGuard g(csw1->frameworkLock);
            EXPECT_CALL(csw1->statusUpdateListener, newParticipantHasConnected(csw2->id));
        }
        {
            PlatformGuard g(csw2->frameworkLock);
            EXPECT_CALL(csw2->statusUpdateListener, newParticipantHasConnected(csw1->id));
        }
        EXPECT_TRUE(csw2->commSystem->connectServices());
        csw2->registerForConnectionUpdates();

        ASSERT_TRUE(state->event.waitForEvents(2));

        {
            PlatformGuard g(csw1->frameworkLock);
            EXPECT_CALL(csw1->statusUpdateListener, participantHasDisconnected(csw2->id));
        }
        {
            PlatformGuard g(csw2->frameworkLock);
            EXPECT_CALL(csw2->statusUpdateListener, participantHasDisconnected(csw1->id));
        }
        csw1->commSystem->disconnectServices();
        csw2->commSystem->disconnectServices();
        ASSERT_TRUE(state->event.waitForEvents(2));
    }
}
