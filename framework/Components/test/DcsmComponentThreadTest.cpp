//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/DcsmComponent.h"
#include "TransportCommon/FakeConnectionStatusUpdateNotifier.h"
#include "CommunicationSystemMock.h"
#include "DcsmEventHandlerMocks.h"
#include "gmock/gmock.h"
#include "DcsmGmockPrinter.h"
#include "Utils/ThreadBarrier.h"
#include <thread>
#include <random>
#include <atomic>


namespace ramses_internal
{
    using namespace ::testing;

    class ADcsmComponentThreadTest : public ::testing::Test
    {
    public:
        ADcsmComponentThreadTest()
            : localId(true)
            , remoteId(true)
            , comp(localId, comm, connNotifier, frameworkLock)
            , startBarrier(4)
            , shouldStop(false)
        {
        }

        void SetUp() override
        {
            EXPECT_CALL(comm, sendDcsmBroadcastRegisterContent(_, _)).Times(AnyNumber());
            EXPECT_CALL(comm, sendDcsmRegisterContent(_, _, _)).Times(AnyNumber());
            EXPECT_CALL(comm, sendDcsmContentAvailable(_, _, _, _)).Times(AnyNumber());
            EXPECT_CALL(comm, sendDcsmCategoryContentSwitchRequest(_, _)).Times(AnyNumber());
            EXPECT_CALL(comm, sendDcsmBroadcastRequestUnregisterContent(_)).Times(AnyNumber());
            EXPECT_CALL(comm, sendDcsmCanvasSizeChange(_, _, _, _)).Times(AnyNumber());
            EXPECT_CALL(comm, sendDcsmContentStatusChange(_, _, _, _)).Times(AnyNumber());

            EXPECT_CALL(provider, canvasSizeChange(_, _, _, _)).Times(AnyNumber());
            EXPECT_CALL(provider, contentStatusChange(_, _, _, _)).Times(AnyNumber());

            EXPECT_CALL(consumer, registerContent(_, _)).Times(AnyNumber());
            EXPECT_CALL(consumer, contentAvailable(_, _, _)).Times(AnyNumber());
            EXPECT_CALL(consumer, categoryContentSwitchRequest(_)).Times(AnyNumber());
            EXPECT_CALL(consumer, requestUnregisterContent(_)).Times(AnyNumber());
            EXPECT_CALL(consumer, forceUnregisterContent(_)).Times(AnyNumber());
        }

        void commThread(unsigned int seed)
        {
            std::mt19937 gen(seed);
            auto rnd = [&]() {
                           std::uniform_int_distribution<uint32_t> dis(0, 100);
                           return dis(gen);
                       };

            startBarrier.wait();

            uint32_t cnt = 1;
            {
                PlatformGuard g(frameworkLock);
                comp.newParticipantHasConnected(remoteId);
            }
            while (!shouldStop)
            {
                {
                    PlatformGuard g(frameworkLock);
                    if (rnd() < 10)
                        comp.participantHasDisconnected(remoteId);
                    if (rnd() < 90)
                        comp.newParticipantHasConnected(remoteId);
                    if (rnd() < 20)
                        comp.newParticipantHasConnected(Guid(true));
                    if (rnd() < 20)
                        comp.sendRegisterContent(ContentID{rnd()%5}, Category{cnt++});
                    if (rnd() < 50)
                        comp.sendContentAvailable(localId, ContentID{rnd()%5}, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{cnt++});
                    if (rnd() < 10)
                        comp.sendContentAvailable(Guid(true), ContentID{rnd()%5}, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{cnt++});
                    if (rnd() < 50)
                        comp.sendCategoryContentSwitchRequest(localId, ContentID{rnd()%5});
                    if (rnd() < 10)
                        comp.sendCategoryContentSwitchRequest(Guid(true), ContentID{rnd()%5});
                    if (rnd() < 50)
                        comp.sendRequestUnregisterContent(ContentID{rnd()%5});
                    if (rnd() < 50)
                        comp.sendCanvasSizeChange(ContentID{rnd()%5}, SizeInfo{1, 1}, AnimationInformation{20, 100});
                    if (rnd() < 50)
                        comp.sendContentStatusChange(ContentID{rnd()%5}, static_cast<EDcsmStatus>(rnd()%8), AnimationInformation{10, 20});
                }
                std::this_thread::sleep_for(std::chrono::milliseconds{rnd() % 10});
            }
            {
                PlatformGuard g(frameworkLock);
                comp.participantHasDisconnected(remoteId);
            }
        }

        void providerThread(unsigned int seed)
        {
            std::mt19937 gen(seed);
            auto rnd = [&]() {
                           std::uniform_int_distribution<uint32_t> dis(0, 100);
                           return dis(gen);
                       };
            auto target = [&]() {
                              const auto rval = rnd();
                              if (rval < 40)
                                  return localId;
                              else if (rval < 70)
                                  return remoteId;
                              else if (rval < 95)
                                  return Guid(true);
                              return Guid(false);
                          };

            startBarrier.wait();

            comp.setLocalProviderAvailability(true);
            while (!shouldStop)
            {
                if (rnd() < 50)
                    comp.sendRegisterContent(ContentID{rnd()%5}, Category{1});
                if (rnd() < 50)
                    comp.sendContentAvailable(target(), ContentID{rnd()%5}, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{2});
                if (rnd() < 50)
                    comp.sendCategoryContentSwitchRequest(target(), ContentID{rnd()%5});
                if (rnd() < 50)
                    comp.sendRequestUnregisterContent(ContentID{rnd()%5});
                if (rnd() < 10)
                    comp.setLocalProviderAvailability(false);
                if (rnd() < 90)
                    comp.setLocalProviderAvailability(true);
                if (rnd() < 50)
                    comp.dispatchProviderEvents(provider);
                std::this_thread::sleep_for(std::chrono::milliseconds{rnd() % 10});
            }
            comp.setLocalProviderAvailability(false);
        }

        void consumerThread(unsigned int seed)
        {
            std::mt19937 gen(seed);
            auto rnd = [&]() {
                           std::uniform_int_distribution<uint32_t> dis(0, 100);
                           return dis(gen);
                       };
            startBarrier.wait();

            comp.setLocalConsumerAvailability(true);
            while (!shouldStop)
            {
                if (rnd() < 50)
                    comp.sendCanvasSizeChange(ContentID{rnd()%5}, SizeInfo{1, 2}, AnimationInformation{10, 20});
                if (rnd() < 50)
                    comp.sendContentStatusChange(ContentID{rnd()%5}, static_cast<EDcsmStatus>(rnd()%8), AnimationInformation{10, 20});
                if (rnd() < 10)
                    comp.setLocalConsumerAvailability(false);
                if (rnd() < 90)
                    comp.setLocalConsumerAvailability(true);
                if (rnd() < 50)
                    comp.dispatchConsumerEvents(consumer);
                std::this_thread::sleep_for(std::chrono::milliseconds{rnd() % 10});
            }
            comp.setLocalConsumerAvailability(false);
        }

        PlatformLock frameworkLock;
        Guid localId;
        Guid remoteId;
        FakeConnectionStatusUpdateNotifier connNotifier;
        StrictMock<CommunicationSystemMock> comm;
        NiceMock<DcsmProviderEventHandlerMock> provider;
        NiceMock<DcsmConsumerEventHandlerMock> consumer;
        DcsmComponent comp;
        ThreadBarrier startBarrier;
        std::atomic<bool> shouldStop;
        std::random_device randomSource;
    };

    TEST_F(ADcsmComponentThreadTest, run)
    {
        unsigned int seedBase = randomSource();
        SCOPED_TRACE(seedBase);

        std::thread tComm([&](){ commThread(seedBase+0); });
        std::thread tProvider([&](){ providerThread(seedBase+1); });
        std::thread tConsumer([&](){ consumerThread(seedBase+2); });

        startBarrier.wait();
        std::this_thread::sleep_for(std::chrono::milliseconds{600});
        shouldStop = true;

        tComm.join();
        tProvider.join();
        tConsumer.join();
    }
}
