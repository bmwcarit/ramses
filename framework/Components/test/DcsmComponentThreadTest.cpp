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
#include "ScopedConsoleLogDisable.h"
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
            : localId(111)
            , remoteId(222)
            , comp(localId, comm, connNotifier, frameworkLock)
            , startBarrier(4)
            , shouldStop(false)
        {
        }

        void SetUp() override
        {
            EXPECT_CALL(comm, sendDcsmBroadcastOfferContent(_, _)).Times(AnyNumber());
            EXPECT_CALL(comm, sendDcsmOfferContent(_, _, _)).Times(AnyNumber());
            EXPECT_CALL(comm, sendDcsmContentDescription(_, _, _, _)).Times(AnyNumber());
            EXPECT_CALL(comm, sendDcsmContentReady(_, _)).Times(AnyNumber());
            EXPECT_CALL(comm, sendDcsmContentEnableFocusRequest(_, _, _)).Times(AnyNumber());
            EXPECT_CALL(comm, sendDcsmContentDisableFocusRequest(_, _, _)).Times(AnyNumber());
            EXPECT_CALL(comm, sendDcsmBroadcastRequestStopOfferContent(_)).Times(AnyNumber());
            EXPECT_CALL(comm, sendDcsmBroadcastForceStopOfferContent(_)).Times(AnyNumber());
            EXPECT_CALL(comm, sendDcsmCanvasSizeChange(_, _, _, _)).Times(AnyNumber());
            EXPECT_CALL(comm, sendDcsmContentStateChange(_, _, _, _, _)).Times(AnyNumber());
            EXPECT_CALL(comm, sendDcsmUpdateContentMetadata(_, _, _)).Times(AnyNumber());

            EXPECT_CALL(provider, contentSizeChange(_, _, _)).Times(AnyNumber());
            EXPECT_CALL(provider, contentStateChange(_, _, _, _)).Times(AnyNumber());

            EXPECT_CALL(consumer, contentOffered(_, _)).Times(AnyNumber());
            EXPECT_CALL(consumer, contentReady(_)).Times(AnyNumber());
            EXPECT_CALL(consumer, contentEnableFocusRequest(_, _)).Times(AnyNumber());
            EXPECT_CALL(consumer, contentDisableFocusRequest(_, _)).Times(AnyNumber());
            EXPECT_CALL(consumer, contentStopOfferRequest(_)).Times(AnyNumber());
            EXPECT_CALL(consumer, forceContentOfferStopped(_)).Times(AnyNumber());
            EXPECT_CALL(consumer, contentMetadataUpdated(_, _)).Times(AnyNumber());
        }

        template <typename T>
        DcsmMetadata randomMetadata(T& rnd)
        {
            std::u32string desc;
            desc.reserve(10);
            for (int i = 0; i < 10; ++i)
                desc.push_back(rnd());
            DcsmMetadata dm;
            dm.setPreviewDescription(desc);
            return dm;
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
                        comp.newParticipantHasConnected(Guid(rnd()));
                    if (rnd() < 40)
                        comp.handleOfferContent(ContentID{rnd()%5}, Category{cnt++}, localId);
                    if (rnd() < 10)
                        comp.handleOfferContent(ContentID{rnd()%5}, Category{cnt++}, Guid(rnd()));
                    if (rnd() < 50)
                        comp.handleContentDescription(ContentID{rnd()%5}, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{cnt++}, localId);
                    if (rnd() < 10)
                        comp.handleContentDescription(ContentID{rnd()%5}, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{cnt++}, Guid(rnd()));
                    if (rnd() < 50)
                        comp.handleContentReady(ContentID{rnd()%5}, localId);
                    if (rnd() < 10)
                        comp.handleContentReady(ContentID{rnd()%5}, Guid(rnd()));
                    if (rnd() < 50)
                        comp.handleContentEnableFocusRequest(ContentID{rnd()%5}, rnd()%2, localId);
                    if (rnd() < 50)
                        comp.handleContentDisableFocusRequest(ContentID{ rnd() % 5 }, rnd()%2, localId);
                    if (rnd() < 10)
                        comp.handleContentEnableFocusRequest(ContentID{rnd()%5}, rnd()%2, Guid(rnd()));
                    if (rnd() < 10)
                        comp.handleContentDisableFocusRequest(ContentID{ rnd() % 5 }, rnd()%2, Guid(rnd()));
                    if (rnd() < 30)
                        comp.handleRequestStopOfferContent(ContentID{rnd()%5}, localId);
                    if (rnd() < 10)
                        comp.handleRequestStopOfferContent(ContentID{rnd()%5}, Guid(rnd()));
                    if (rnd() < 40)
                        comp.handleUpdateContentMetadata(ContentID{rnd()%5}, randomMetadata(rnd), localId);
                    if (rnd() < 40)
                        comp.handleUpdateContentMetadata(ContentID{rnd()%5}, randomMetadata(rnd), Guid(rnd()));
                    if (rnd() < 50)
                        comp.handleCanvasSizeChange(ContentID{rnd()%5}, CategoryInfo{1, 1}, AnimationInformation{20, 100}, localId);
                    if (rnd() < 10)
                        comp.handleCanvasSizeChange(ContentID{rnd()%5}, CategoryInfo{1, 1}, AnimationInformation{20, 100}, Guid(rnd()));
                    if (rnd() < 70)
                        comp.handleContentStateChange(ContentID{rnd()%5}, static_cast<EDcsmState>(rnd()%8), CategoryInfo{}, AnimationInformation{10, 20}, localId);
                    if (rnd() < 50)
                        comp.handleContentStateChange(ContentID{rnd()%5}, static_cast<EDcsmState>(rnd()%8), CategoryInfo{1, 1}, AnimationInformation{10, 20}, localId);
                    if (rnd() < 30)
                        comp.handleContentStateChange(ContentID{rnd()%5}, static_cast<EDcsmState>(rnd()%8), CategoryInfo{}, AnimationInformation{10, 20}, localId);
                    if (rnd() < 20)
                        comp.handleContentStateChange(ContentID{rnd()%5}, static_cast<EDcsmState>(rnd()%8), CategoryInfo{1, 1}, AnimationInformation{10, 20}, localId);
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

            startBarrier.wait();

            comp.setLocalProviderAvailability(true);
            while (!shouldStop)
            {
                if (rnd() < 50)
                    comp.sendOfferContent(ContentID{rnd()%5}, Category{1}, false);
                if (rnd() < 50)
                    comp.sendContentDescription(ContentID{rnd()%5}, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{2});
                if (rnd() < 50)
                    comp.sendContentReady(ContentID{rnd()%5});
                if (rnd() < 50)
                    comp.sendContentEnableFocusRequest(ContentID{rnd()%5}, rnd()%2);
                if (rnd() < 50)
                    comp.sendContentDisableFocusRequest(ContentID{ rnd() % 5 }, rnd()% 2);
                if (rnd() < 30)
                    comp.sendRequestStopOfferContent(ContentID{rnd()%5});
                if (rnd() < 40)
                    comp.sendUpdateContentMetadata(ContentID{rnd()%5}, randomMetadata(rnd));
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
                    comp.sendCanvasSizeChange(ContentID{rnd()%5}, CategoryInfo{1, 2}, AnimationInformation{10, 20});
                if (rnd() < 70)
                    comp.sendContentStateChange(ContentID{rnd()%5}, static_cast<EDcsmState>(rnd()%8), CategoryInfo{}, AnimationInformation{10, 20});
                if (rnd() < 30)
                    comp.sendContentStateChange(ContentID{rnd()%5}, static_cast<EDcsmState>(rnd()%8), CategoryInfo{1, 1}, AnimationInformation{10, 20});
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
        ScopedConsoleLogDisable consoleDisabler;

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
