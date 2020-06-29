//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportTCP/TCPConnectionSystem.h"
#include "Utils/StatisticCollection.h"
#include "Utils/ThreadBarrier.h"
#include "ScopedConsoleLogDisable.h"
#include "gtest/gtest.h"
#include <thread>

namespace ramses_internal
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
}
