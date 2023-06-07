//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ServiceHandlerMocks.h"
#include "CommunicationSystemTest.h"
#include "ConnectionSystemTestHelper.h"
#include "Components/SceneUpdate.h"
#include "TransportCommon/SceneUpdateSerializer.h"

namespace ramses_internal
{
    INSTANTIATE_TEST_SUITE_P(TypedCommunicationTest, ACommunicationSystem, ::testing::ValuesIn(CommunicationSystemTestState::GetAvailableCommunicationSystemTypes()));

    INSTANTIATE_TEST_SUITE_P(TypedCommunicationTestRamses, ACommunicationSystemWithDaemon, TESTING_SERVICETYPE_RAMSES(CommunicationSystemTestState::GetAvailableCommunicationSystemTypes()));

    TEST_P(ACommunicationSystem, canStartStopDiscoveryDaemon)
    {
        auto daemon = std::make_unique<ConnectionSystemTestDaemon>();
        EXPECT_TRUE(daemon->start());
        EXPECT_TRUE(daemon->stop());
    }

    TEST_P(ACommunicationSystem, canRegisterAndUnregisterForConnectionUpdates)
    {
        auto csw = std::make_unique<CommunicationSystemTestWrapper>(*state);
        csw->commSystem->getRamsesConnectionStatusUpdateNotifier().registerForConnectionUpdates(&listener);
        csw->commSystem->getRamsesConnectionStatusUpdateNotifier().unregisterForConnectionUpdates(&listener);
    }

    TEST_P(ACommunicationSystem, sendFunctionsFailWhenNotYetConnected)
    {
        Guid to(5);
        StatisticCollectionScene sceneStatistics;
        auto csw = std::make_unique<CommunicationSystemTestWrapper>(*state);
        EXPECT_FALSE(csw->commSystem->broadcastNewScenesAvailable(SceneInfoVector(), ramses::EFeatureLevel_01));
        EXPECT_FALSE(csw->commSystem->broadcastScenesBecameUnavailable(SceneInfoVector()));
        EXPECT_FALSE(csw->commSystem->sendScenesAvailable(to, SceneInfoVector(), ramses::EFeatureLevel_01));
        EXPECT_FALSE(csw->commSystem->sendSubscribeScene(to, SceneId(123)));
        EXPECT_FALSE(csw->commSystem->sendUnsubscribeScene(to, SceneId(123)));
        EXPECT_FALSE(csw->commSystem->sendInitializeScene(to, SceneId()));
        EXPECT_FALSE(csw->commSystem->sendSceneUpdate(to, SceneId(123), SceneUpdateSerializer(SceneUpdate(), sceneStatistics)));
    }

    TEST_P(ACommunicationSystem, sendFunctionsFailAfterCallingDisconnect)
    {
        Guid to(5);
        StatisticCollectionScene sceneStatistics;
        auto csw = std::make_unique<CommunicationSystemTestWrapper>(*state);
        csw->commSystem->connectServices();
        csw->commSystem->disconnectServices();

        EXPECT_FALSE(csw->commSystem->broadcastNewScenesAvailable(SceneInfoVector(), ramses::EFeatureLevel_01));
        EXPECT_FALSE(csw->commSystem->broadcastScenesBecameUnavailable(SceneInfoVector()));
        EXPECT_FALSE(csw->commSystem->sendScenesAvailable(to, SceneInfoVector(), ramses::EFeatureLevel_01));
        EXPECT_FALSE(csw->commSystem->sendSubscribeScene(to, SceneId(123)));
        EXPECT_FALSE(csw->commSystem->sendUnsubscribeScene(to, SceneId(123)));
        EXPECT_FALSE(csw->commSystem->sendInitializeScene(to, SceneId()));
        EXPECT_FALSE(csw->commSystem->sendSceneUpdate(to, SceneId(123), SceneUpdateSerializer(SceneUpdate(), sceneStatistics)));
    }

    TEST_P(ACommunicationSystemWithDaemon, canConnectAndDisconnectWithoutBlocking)
    {
        auto csw = std::make_unique<CommunicationSystemTestWrapper>(*state);
        EXPECT_TRUE(csw->commSystem->connectServices());
        csw->commSystem->disconnectServices();
    }

    TEST_P(ACommunicationSystemWithDaemon, receivedParticipantConnectedAndDisconnectedNotifications)
    {
        auto csw1 = std::make_unique<CommunicationSystemTestWrapper>(*state, "csw1");
        auto csw2 = std::make_unique<CommunicationSystemTestWrapper>(*state, "csw2");

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
        csw1->commSystem->disconnectServices();
        csw2->commSystem->disconnectServices();
        ASSERT_TRUE(state->event.waitForEvents(2));

        csw1->unregisterForConnectionUpdates();
        csw2->unregisterForConnectionUpdates();
    }

    TEST_P(ACommunicationSystemWithDaemon, doesNotReceiveNotificationsAfterUnregistering)
    {
        auto csw1 = std::make_unique<CommunicationSystemTestWrapper>(*state, "csw1");
        auto csw2 = std::make_unique<CommunicationSystemTestWrapper>(*state, "csw2");

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

        csw1->unregisterForConnectionUpdates();

        {
            PlatformGuard g(csw2->frameworkLock);
            EXPECT_CALL(csw2->statusUpdateListener, participantHasDisconnected(csw1->id));
        }
        csw1->commSystem->disconnectServices();
        csw2->commSystem->disconnectServices();

        ASSERT_TRUE(state->event.waitForEvents(1));

        csw2->unregisterForConnectionUpdates();
    }

    TEST_P(ACommunicationSystemWithDaemon, canSendMessageToConnectedParticipant)
    {
        auto sender = std::make_unique<CommunicationSystemTestWrapper>(*state, "sender");
        auto receiver = std::make_unique<CommunicationSystemTestWrapper>(*state, "receiver");

        state->connectAll();
        ASSERT_TRUE(state->blockOnAllConnected());

        StrictMock<SceneProviderServiceHandlerMock> handler;
        receiver->commSystem->setSceneProviderServiceHandler(&handler);

        SceneId sceneId;
        {
            PlatformGuard g(receiver->frameworkLock);
            EXPECT_CALL(handler, handleSubscribeScene(sceneId, sender->id)).WillOnce(InvokeWithoutArgs([&](){ state->sendEvent(); }));
        }
        sender->commSystem->sendSubscribeScene(receiver->id, sceneId);
        ASSERT_TRUE(state->event.waitForEvents(1));

        state->disconnectAll();
    }

    TEST_P(ACommunicationSystemWithDaemon, canConnectAndDisconnectMultipleTimes)
    {
        auto csw = std::make_unique<CommunicationSystemTestWrapper>(*state);
        EXPECT_TRUE(csw->commSystem->connectServices());
        csw->commSystem->disconnectServices();
        EXPECT_TRUE(csw->commSystem->connectServices());
        csw->commSystem->disconnectServices();
        EXPECT_TRUE(csw->commSystem->connectServices());
        csw->commSystem->disconnectServices();
    }

    TEST_P(ACommunicationSystemWithDaemon, canSendMessageInBothDirectionBetweenTwoParticipants)
    {
        auto csw1 = std::make_unique<CommunicationSystemTestWrapper>(*state, "csw1");
        auto csw2 = std::make_unique<CommunicationSystemTestWrapper>(*state, "csw2");
        state->connectAll();
        ASSERT_TRUE(state->blockOnAllConnected());

        SceneId sceneId_1;
        StrictMock<SceneProviderServiceHandlerMock> handler_1;
        csw1->commSystem->setSceneProviderServiceHandler(&handler_1);

        SceneId sceneId_2;
        StrictMock<SceneProviderServiceHandlerMock> handler_2;
        csw2->commSystem->setSceneProviderServiceHandler(&handler_2);

        EXPECT_CALL(handler_1, handleSubscribeScene(sceneId_2, csw2->id)).WillOnce(InvokeWithoutArgs([&](){ state->sendEvent(); }));
        EXPECT_CALL(handler_2, handleSubscribeScene(sceneId_1, csw1->id)).WillOnce(InvokeWithoutArgs([&](){ state->sendEvent(); }));

        csw1->commSystem->sendSubscribeScene(csw2->id, sceneId_1);
        csw2->commSystem->sendSubscribeScene(csw1->id, sceneId_2);
        ASSERT_TRUE(state->event.waitForEvents(2));

        state->disconnectAll();
    }

    TEST_P(ACommunicationSystemWithDaemon, canSendMessageToOnlyOneOutOfTwoParticipants)
    {
        auto sender = std::make_unique<CommunicationSystemTestWrapper>(*state, "sender");
        auto receiverOk = std::make_unique<CommunicationSystemTestWrapper>(*state, "receiverOk");
        auto receiverWrong = std::make_unique<CommunicationSystemTestWrapper>(*state, "receiverWrong");

        state->connectAll();
        ASSERT_TRUE(state->blockOnAllConnected());

        StrictMock<SceneProviderServiceHandlerMock> handlerOk;
        receiverOk->commSystem->setSceneProviderServiceHandler(&handlerOk);

        StrictMock<SceneProviderServiceHandlerMock> handlerWrong;
        receiverWrong->commSystem->setSceneProviderServiceHandler(&handlerWrong);

        SceneId sceneId;

        EXPECT_CALL(handlerOk, handleSubscribeScene(sceneId, sender->id)).WillOnce(InvokeWithoutArgs([&](){ state->sendEvent(); }));
        sender->commSystem->sendSubscribeScene(receiverOk->id, sceneId);
        ASSERT_TRUE(state->event.waitForEvents(1));

        // As it's usually hard to prove the nonexistence of something try to dispatch again
        // with timeout and hope that this would catch message delivery to the other receiver.
        ASSERT_FALSE(state->event.waitForEvents(1, 100));

        state->disconnectAll();
    }

    TEST_P(ACommunicationSystemWithDaemon, canBroadcastMessageToTwoOthers)
    {
        auto sender = std::make_unique<CommunicationSystemTestWrapper>(*state, "sender");
        auto receiver_1 = std::make_unique<CommunicationSystemTestWrapper>(*state, "receiver_1");
        auto receiver_2 = std::make_unique<CommunicationSystemTestWrapper>(*state, "receiver_2");

        state->connectAll();
        ASSERT_TRUE(state->blockOnAllConnected());

        StrictMock<SceneRendererServiceHandlerMock> handler_1;
        receiver_1->commSystem->setSceneRendererServiceHandler(&handler_1);

        StrictMock<SceneRendererServiceHandlerMock> handler_2;
        receiver_2->commSystem->setSceneRendererServiceHandler(&handler_2);

        SceneId sceneId;
        SceneInfoVector unavailableScenes;
        unavailableScenes.push_back(SceneInfo(sceneId));
        EXPECT_CALL(handler_1, handleScenesBecameUnavailable(unavailableScenes, sender->id)).WillOnce(InvokeWithoutArgs([&](){ state->sendEvent(); }));
        EXPECT_CALL(handler_2, handleScenesBecameUnavailable(unavailableScenes, sender->id)).WillOnce(InvokeWithoutArgs([&](){ state->sendEvent(); }));
        sender->commSystem->broadcastScenesBecameUnavailable(unavailableScenes);

        ASSERT_TRUE(state->event.waitForEvents(2));

        state->disconnectAll();
    }

    TEST_P(ACommunicationSystemWithDaemon, getsParticipantConnectedNotificationsAfterConnectAndDisconnectMultipleTimes)
    {
        auto receiver = std::make_unique<CommunicationSystemTestWrapper>(*state, "receiver");
        receiver->commSystem->connectServices();

        auto sender = std::make_unique<CommunicationSystemTestWrapper>(*state, "sender");
        EXPECT_TRUE(sender->commSystem->connectServices());
        sender->commSystem->disconnectServices();
        EXPECT_TRUE(sender->commSystem->connectServices());
        sender->commSystem->disconnectServices();
        EXPECT_TRUE(sender->commSystem->connectServices());
        sender->commSystem->disconnectServices();
        EXPECT_TRUE(sender->commSystem->connectServices());

        ASSERT_TRUE(state->blockOnAllConnected());

        state->disconnectAll();
    }

    TEST_P(ACommunicationSystemWithDaemon, canSendMessageAfterConnectAndDisconnectMultipleTimes)
    {
        auto receiver = std::make_unique<CommunicationSystemTestWrapper>(*state, "receiver");
        receiver->commSystem->connectServices();

        auto sender = std::make_unique<CommunicationSystemTestWrapper>(*state, "sender");
        EXPECT_TRUE(sender->commSystem->connectServices());
        sender->commSystem->disconnectServices();
        EXPECT_TRUE(sender->commSystem->connectServices());
        sender->commSystem->disconnectServices();
        EXPECT_TRUE(sender->commSystem->connectServices());
        sender->commSystem->disconnectServices();
        EXPECT_TRUE(sender->commSystem->connectServices());

        ASSERT_TRUE(state->blockOnAllConnected());

        StrictMock<SceneProviderServiceHandlerMock> handler;
        receiver->commSystem->setSceneProviderServiceHandler(&handler);

        SceneId sceneId;

        EXPECT_CALL(handler, handleSubscribeScene(sceneId, sender->id)).WillOnce(InvokeWithoutArgs([&]() { state->sendEvent(); }));
        sender->commSystem->sendSubscribeScene(receiver->id, sceneId);
        ASSERT_TRUE(state->event.waitForEvents(1));

        state->disconnectAll();
    }
}
