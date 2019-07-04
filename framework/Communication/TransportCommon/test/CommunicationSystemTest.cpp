//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportCommon/IConnectionStatusUpdateNotifier.h"
#include "Resource/ArrayResource.h"
#include "ServiceHandlerMocks.h"
#include "Components/ManagedResource.h"
#include "ResourceMock.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "Utils/BinaryOutputStream.h"
#include "Scene/SceneActionCollectionCreator.h"
#include "ResourceSerializationTestHelper.h"
#include "CommunicationSystemTest.h"

namespace ramses_internal
{
    INSTANTIATE_TEST_CASE_P(TypedCommunicationTest, ACommunicationSystem, ::testing::ValuesIn(CommunicationSystemTestState::GetAvailableCommunicationSystemTypes()));

    TEST_P(ACommunicationSystem, canStartStopDiscoveryDaemon)
    {
        std::unique_ptr<CommunicationSystemDiscoveryDaemonTestWrapper> daemon{CommunicationSystemTestFactory::ConstructDiscoveryDaemonTestWrapper(*state)};
        EXPECT_TRUE(daemon->start());
        EXPECT_TRUE(daemon->stop());
    }

    TEST_P(ACommunicationSystem, canRegisterAndUnregisterForConnectionUpdates)
    {
        std::unique_ptr<CommunicationSystemTestWrapper> csw{CommunicationSystemTestFactory::ConstructTestWrapper(*state)};
        csw->commSystem->getRamsesConnectionStatusUpdateNotifier().registerForConnectionUpdates(&listener);
        csw->commSystem->getRamsesConnectionStatusUpdateNotifier().unregisterForConnectionUpdates(&listener);
        csw->commSystem->getDcsmConnectionStatusUpdateNotifier().registerForConnectionUpdates(&listener);
        csw->commSystem->getDcsmConnectionStatusUpdateNotifier().unregisterForConnectionUpdates(&listener);
    }

    TEST_P(ACommunicationSystem, sendFunctionsFailWhenNotYetConnected)
    {
        Guid to(true);
        std::unique_ptr<CommunicationSystemTestWrapper> csw{CommunicationSystemTestFactory::ConstructTestWrapper(*state)};
        EXPECT_FALSE(csw->commSystem->sendRequestResources(to, ResourceContentHashVector()));
        EXPECT_FALSE(csw->commSystem->sendResourcesNotAvailable(to, ResourceContentHashVector()));
        EXPECT_FALSE(csw->commSystem->sendResources(to, ManagedResourceVector()));
        EXPECT_FALSE(csw->commSystem->broadcastNewScenesAvailable(SceneInfoVector()));
        EXPECT_FALSE(csw->commSystem->broadcastScenesBecameUnavailable(SceneInfoVector()));
        EXPECT_FALSE(csw->commSystem->sendScenesAvailable(to, SceneInfoVector()));
        EXPECT_FALSE(csw->commSystem->sendSubscribeScene(to, SceneId(123)));
        EXPECT_FALSE(csw->commSystem->sendUnsubscribeScene(to, SceneId(123)));
        EXPECT_FALSE(csw->commSystem->sendSceneNotAvailable(to, SceneId(123)));
        EXPECT_FALSE(csw->commSystem->sendInitializeScene(to, SceneInfo()));
        EXPECT_EQ(0u, csw->commSystem->sendSceneActionList(to, SceneId(123), SceneActionCollection(), 1));
        EXPECT_FALSE(csw->commSystem->sendDcsmBroadcastOfferContent(ContentID{}, Category{}));
        EXPECT_FALSE(csw->commSystem->sendDcsmOfferContent(to, ContentID{}, Category{}));
        EXPECT_FALSE(csw->commSystem->sendDcsmContentReady(to, ContentID{}, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{}));
        EXPECT_FALSE(csw->commSystem->sendDcsmContentFocusRequest(to, ContentID{}));
        EXPECT_FALSE(csw->commSystem->sendDcsmBroadcastRequestStopOfferContent(ContentID{}));
        EXPECT_FALSE(csw->commSystem->sendDcsmBroadcastForceStopOfferContent(ContentID{}));
        EXPECT_FALSE(csw->commSystem->sendDcsmCanvasSizeChange(to, ContentID{}, SizeInfo{}, AnimationInformation{}));
        EXPECT_FALSE(csw->commSystem->sendDcsmContentStateChange(to, ContentID{}, EDcsmState::Offered, SizeInfo{}, AnimationInformation{}));
    }

    TEST_P(ACommunicationSystem, sendFunctionsFailAfterCallingDisconnect)
    {
        Guid to(true);
        std::unique_ptr<CommunicationSystemTestWrapper> csw{CommunicationSystemTestFactory::ConstructTestWrapper(*state)};
        csw->commSystem->connectServices();
        csw->commSystem->disconnectServices();

        EXPECT_FALSE(csw->commSystem->sendRequestResources(to, ResourceContentHashVector()));
        EXPECT_FALSE(csw->commSystem->sendResourcesNotAvailable(to, ResourceContentHashVector()));
        EXPECT_FALSE(csw->commSystem->sendResources(to, ManagedResourceVector()));
        EXPECT_FALSE(csw->commSystem->broadcastNewScenesAvailable(SceneInfoVector()));
        EXPECT_FALSE(csw->commSystem->broadcastScenesBecameUnavailable(SceneInfoVector()));
        EXPECT_FALSE(csw->commSystem->sendScenesAvailable(to, SceneInfoVector()));
        EXPECT_FALSE(csw->commSystem->sendSubscribeScene(to, SceneId(123)));
        EXPECT_FALSE(csw->commSystem->sendUnsubscribeScene(to, SceneId(123)));
        EXPECT_FALSE(csw->commSystem->sendSceneNotAvailable(to, SceneId(123)));
        EXPECT_FALSE(csw->commSystem->sendInitializeScene(to, SceneInfo()));
        EXPECT_EQ(0u, csw->commSystem->sendSceneActionList(to, SceneId(123), SceneActionCollection(), 1));
        EXPECT_FALSE(csw->commSystem->sendDcsmBroadcastOfferContent(ContentID{}, Category{}));
        EXPECT_FALSE(csw->commSystem->sendDcsmOfferContent(to, ContentID{}, Category{}));
        EXPECT_FALSE(csw->commSystem->sendDcsmContentReady(to, ContentID{}, ETechnicalContentType::RamsesSceneID, TechnicalContentDescriptor{}));
        EXPECT_FALSE(csw->commSystem->sendDcsmContentFocusRequest(to, ContentID{}));
        EXPECT_FALSE(csw->commSystem->sendDcsmBroadcastRequestStopOfferContent(ContentID{}));
        EXPECT_FALSE(csw->commSystem->sendDcsmBroadcastForceStopOfferContent(ContentID{}));
        EXPECT_FALSE(csw->commSystem->sendDcsmCanvasSizeChange(to, ContentID{}, SizeInfo{}, AnimationInformation{}));
        EXPECT_FALSE(csw->commSystem->sendDcsmContentStateChange(to, ContentID{}, EDcsmState::Offered, SizeInfo{}, AnimationInformation{}));
    }

    class ACommunicationSystemWithDaemonConnectionSetup : public ACommunicationSystemWithDaemon
    {
    };

    INSTANTIATE_TEST_CASE_P(TypedCommunicationTest, ACommunicationSystemWithDaemon, TESTING_SERVICETYPE_RAMSES(CommunicationSystemTestState::GetAvailableCommunicationSystemTypes()));
    INSTANTIATE_TEST_CASE_P(TypedCommunicationTest_ramses, ACommunicationSystemWithDaemonConnectionSetup, TESTING_SERVICETYPE_RAMSES(CommunicationSystemTestState::GetAvailableCommunicationSystemTypes()));
    INSTANTIATE_TEST_CASE_P(TypedCommunicationTest_dcsm, ACommunicationSystemWithDaemonConnectionSetup, TESTING_SERVICETYPE_DCSM(CommunicationSystemTestState::GetAvailableCommunicationSystemTypes(ECommunicationSystemType_Tcp)));

    TEST_P(ACommunicationSystemWithDaemon, canConnectAndDisconnectWithoutBlocking)
    {
        std::unique_ptr<CommunicationSystemTestWrapper> csw{CommunicationSystemTestFactory::ConstructTestWrapper(*state)};
        EXPECT_TRUE(csw->commSystem->connectServices());
        csw->commSystem->disconnectServices();
    }

    TEST_P(ACommunicationSystemWithDaemonConnectionSetup, receivedParticipantConnectedAndDisconnectedNotifications)
    {
        std::unique_ptr<CommunicationSystemTestWrapper> csw1{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "csw1")};
        std::unique_ptr<CommunicationSystemTestWrapper> csw2{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "csw2")};

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

        state->startUpHook();

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

        state->shutdownHook();
    }

    TEST_P(ACommunicationSystemWithDaemonConnectionSetup, doesNotReceiveNotificationsAfterUnregistering)
    {
        std::unique_ptr<CommunicationSystemTestWrapper> csw1{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "csw1")};
        std::unique_ptr<CommunicationSystemTestWrapper> csw2{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "csw2")};

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

        state->startUpHook();

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

        state->shutdownHook();
    }

    TEST_P(ACommunicationSystemWithDaemon, canSendMessageToConnectedParticipant)
    {
        std::unique_ptr<CommunicationSystemTestWrapper> sender{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "sender")};
        std::unique_ptr<CommunicationSystemTestWrapper> receiver{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "receiver")};
        state->connectAll();
        ASSERT_TRUE(state->blockOnAllConnected());

        StrictMock<SceneProviderServiceHandlerMock> handler;
        receiver->commSystem->setSceneProviderServiceHandler(&handler);

        SceneId sceneId;
        {
            PlatformGuard g(receiver->frameworkLock);
            EXPECT_CALL(handler, handleSubscribeScene(sceneId, sender->id)).WillOnce(SendHandlerCalledEvent(state.get()));
        }
        sender->commSystem->sendSubscribeScene(receiver->id, sceneId);
        ASSERT_TRUE(state->event.waitForEvents(1));

        state->disconnectAll();
    }

    TEST_P(ACommunicationSystemWithDaemon, canConnectAndDisconnectMultipleTimes)
    {
        std::unique_ptr<CommunicationSystemTestWrapper> csw{CommunicationSystemTestFactory::ConstructTestWrapper(*state)};
        EXPECT_TRUE(csw->commSystem->connectServices());
        csw->commSystem->disconnectServices();
        EXPECT_TRUE(csw->commSystem->connectServices());
        csw->commSystem->disconnectServices();
        EXPECT_TRUE(csw->commSystem->connectServices());
        csw->commSystem->disconnectServices();
    }

    class ACommunicationSystemWithDaemonMultiParticipant : public ACommunicationSystemWithDaemon
    {
    public:
    };

    INSTANTIATE_TEST_CASE_P(TypedCommunicationTest, ACommunicationSystemWithDaemonMultiParticipant,
                            TESTING_SERVICETYPE_RAMSES(CommunicationSystemTestState::GetAvailableCommunicationSystemTypes(ECommunicationSystemType_Tcp)));

    TEST_P(ACommunicationSystemWithDaemonMultiParticipant, canSendMessageInBothDirectionBetweenTwoParticipants)
    {
        std::unique_ptr<CommunicationSystemTestWrapper> csw_1{CommunicationSystemTestFactory::ConstructTestWrapper(*state)};
        std::unique_ptr<CommunicationSystemTestWrapper> csw_2{CommunicationSystemTestFactory::ConstructTestWrapper(*state)};
        state->connectAll();
        ASSERT_TRUE(state->blockOnAllConnected());

        SceneId sceneId_1;
        StrictMock<SceneProviderServiceHandlerMock> handler_1;
        csw_1->commSystem->setSceneProviderServiceHandler(&handler_1);

        SceneId sceneId_2;
        StrictMock<SceneProviderServiceHandlerMock> handler_2;
        csw_2->commSystem->setSceneProviderServiceHandler(&handler_2);

        EXPECT_CALL(handler_1, handleSubscribeScene(sceneId_2, csw_2->id)).WillOnce(SendHandlerCalledEvent(state.get()));
        EXPECT_CALL(handler_2, handleSubscribeScene(sceneId_1, csw_1->id)).WillOnce(SendHandlerCalledEvent(state.get()));

        csw_1->commSystem->sendSubscribeScene(csw_2->id, sceneId_1);
        csw_2->commSystem->sendSubscribeScene(csw_1->id, sceneId_2);
        ASSERT_TRUE(state->event.waitForEvents(2));

        state->disconnectAll();
    }

    TEST_P(ACommunicationSystemWithDaemonMultiParticipant, canSendMessageToOnlyOneOutOfTwoParticipants)
    {
        std::unique_ptr<CommunicationSystemTestWrapper> sender{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "sender")};
        std::unique_ptr<CommunicationSystemTestWrapper> receiverOk{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "receiverOk")};
        std::unique_ptr<CommunicationSystemTestWrapper> receiverWrong{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "receiverWrong")};
        state->connectAll();
        ASSERT_TRUE(state->blockOnAllConnected());

        StrictMock<SceneProviderServiceHandlerMock> handlerOk;
        receiverOk->commSystem->setSceneProviderServiceHandler(&handlerOk);

        StrictMock<SceneProviderServiceHandlerMock> handlerWrong;
        receiverWrong->commSystem->setSceneProviderServiceHandler(&handlerWrong);

        SceneId sceneId;

        EXPECT_CALL(handlerOk, handleSubscribeScene(sceneId, sender->id)).WillOnce(SendHandlerCalledEvent(state.get()));
        sender->commSystem->sendSubscribeScene(receiverOk->id, sceneId);
        ASSERT_TRUE(state->event.waitForEvents(1));

        // As it's usually hard to prove the nonexistence of something try to dispatch again
        // with timeout and hope that this would catch message delivery to the other receiver.
        ASSERT_FALSE(state->event.waitForEvents(1, 100));

        state->disconnectAll();
    }

    TEST_P(ACommunicationSystemWithDaemonMultiParticipant, canSendAndReceiveLargeMessageInOneChunk)
    {
        std::unique_ptr<CommunicationSystemTestWrapper> sender{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "sender")};

        std::unique_ptr<CommunicationSystemTestWrapper> receiver{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "receiver")};
        state->connectAll();
        ASSERT_TRUE(state->blockOnAllConnected());

        StrictMock<ResourceConsumerServiceHandlerMock> handler;
        receiver->commSystem->setResourceConsumerServiceHandler(&handler);

        std::vector<Byte> receivedResourceData;
        std::vector<Byte> sentResourceData;
        EXPECT_CALL(handler, handleSendResource(_, sender->id)).WillOnce(Invoke([this, &receivedResourceData](const ByteArrayView& resourceData, const Guid&)
        {
            receivedResourceData.insert(receivedResourceData.begin(), resourceData.begin(), resourceData.end());
            state->event.signal();
        }));

        {
            std::vector<Float> data(60000);  // data size ~250k
            for (UInt i = 0; i < data.size(); ++i)
            {
                data[i] = 1.1f * (i + 1);
            }

            ScopedPointer<IResource> resource(new ArrayResource(EResourceType_VertexArray, static_cast<UInt32>(data.size()), EDataType_Float, reinterpret_cast<const Byte*>(&data[0]), ResourceCacheFlag(0u), String("resName")));
            resource->compress(IResource::CompressionLevel::REALTIME);

            StrictMock<ManagedResourceDeleterCallbackMock> callback;
            EXPECT_CALL(callback, managedResourceDeleted(_));

            ResourceDeleterCallingCallback callbackWrapper(callback);
            ManagedResourceVector managedResources = { ManagedResource(*resource, callbackWrapper) };

            auto resData = ResourceSerializationTestHelper::ConvertResourcesToResourceDataVector(managedResources, sender->commSystem->getSendDataSizes().resourceDataArray);
            ASSERT_EQ(1u, resData.size());
            sentResourceData = resData[0];

            sender->commSystem->sendResources(receiver->id, managedResources);
        }

        ASSERT_TRUE(state->event.waitForEvents(1));

        ASSERT_TRUE(receivedResourceData.data() != nullptr);
        EXPECT_EQ(sentResourceData, receivedResourceData);

        state->disconnectAll();
    }

    TEST_P(ACommunicationSystemWithDaemonMultiParticipant, canSendAndReceiveLargeMessageInMultipleChunks)
    {
        const UInt32 numFloatElements  = 512 * 1024;
        const UInt32 floatDataSize     = numFloatElements * sizeof(Float);
        const UInt32 maxMessageSize    = floatDataSize * 2 / 5; // we want to have 3 chunks

        std::unique_ptr<CommunicationSystemTestWrapper> sender{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "sender")};
        CommunicationSendDataSizes sendDataSizes = sender->commSystem->getSendDataSizes();
        sendDataSizes.resourceDataArray = maxMessageSize;
        sender->commSystem->setSendDataSizes(sendDataSizes);

        std::unique_ptr<CommunicationSystemTestWrapper> receiver{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "receiver")};
        state->connectAll();
        ASSERT_TRUE(state->blockOnAllConnected());

        StrictMock<ResourceConsumerServiceHandlerMock> handler;
        receiver->commSystem->setResourceConsumerServiceHandler(&handler);

        std::vector<std::vector<Byte>> receivedResourceData;
        std::vector<std::vector<Byte>> sentResourceData;
        EXPECT_CALL(handler, handleSendResource(_, sender->id)).WillRepeatedly(Invoke([this, &receivedResourceData](const ByteArrayView& resourceData, const Guid&)
        {
            std::vector<Byte> data;
            data.insert(data.begin(), resourceData.begin(), resourceData.end());
            receivedResourceData.push_back(data);
            state->event.signal();
        }));

        BinaryOutputStream sentResourceDump;
        {
            std::vector<Float> data(numFloatElements);
            for (UInt i = 0; i < data.size(); ++i)
            {
                data[i] = 1.1f * (i + 1);
            }
            ScopedPointer<IResource> resource(new ArrayResource(EResourceType_VertexArray, static_cast<UInt32>(data.size()), EDataType_Float, reinterpret_cast<const Byte*>(&data[0]), ResourceCacheFlag(0u), String("resName")));
            resource->compress(IResource::CompressionLevel::REALTIME);

            StrictMock<ManagedResourceDeleterCallbackMock> callback;
            EXPECT_CALL(callback, managedResourceDeleted(_));

            ResourceDeleterCallingCallback callbackWrapper(callback);
            ManagedResourceVector managedResources = { ManagedResource(*resource, callbackWrapper) };

            sentResourceData = ResourceSerializationTestHelper::ConvertResourcesToResourceDataVector(managedResources, sender->commSystem->getSendDataSizes().resourceDataArray);

            sender->commSystem->sendResources(receiver->id, managedResources);
        }

        ASSERT_TRUE(state->event.waitForEvents(3));
        EXPECT_EQ(sentResourceData.size(), receivedResourceData.size());
        EXPECT_EQ(sentResourceData, receivedResourceData);

        state->disconnectAll();
    }

    TEST_P(ACommunicationSystemWithDaemonMultiParticipant, DISABLED_canBroadcastMessageToTwoOthers)
    {
        std::unique_ptr<CommunicationSystemTestWrapper> sender{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "sender")};
        std::unique_ptr<CommunicationSystemTestWrapper> receiver_1{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "receiver1")};
        std::unique_ptr<CommunicationSystemTestWrapper> receiver_2{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "receiver2")};
        state->connectAll();
        ASSERT_TRUE(state->blockOnAllConnected());

        StrictMock<SceneRendererServiceHandlerMock> handler_1;
        receiver_1->commSystem->setSceneRendererServiceHandler(&handler_1);

        StrictMock<SceneRendererServiceHandlerMock> handler_2;
        receiver_2->commSystem->setSceneRendererServiceHandler(&handler_2);

        SceneId sceneId;
        SceneInfoVector unavailableScenes;
        unavailableScenes.push_back(SceneInfo(sceneId));
        EXPECT_CALL(handler_1, handleScenesBecameUnavailable(unavailableScenes, sender->id)).WillOnce(SendHandlerCalledEvent(state.get()));
        EXPECT_CALL(handler_2, handleScenesBecameUnavailable(unavailableScenes, sender->id)).WillOnce(SendHandlerCalledEvent(state.get()));
        sender->commSystem->broadcastScenesBecameUnavailable(unavailableScenes);

        ASSERT_TRUE(state->event.waitForEvents(2));

        state->disconnectAll();
    }

    TEST_P(ACommunicationSystemWithDaemonMultiParticipant, DISABLED_getsParticipantConnectedNotificationsAfterConnectAndDisconnectMultipleTimes)
    {
        std::unique_ptr<CommunicationSystemTestWrapper> receiver{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "receiver")};
        receiver->commSystem->connectServices();

        std::unique_ptr<CommunicationSystemTestWrapper> sender{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "sender")};
        EXPECT_TRUE(sender->commSystem->connectServices());
        sender->commSystem->disconnectServices();
        EXPECT_TRUE(sender->commSystem->connectServices());
        sender->commSystem->disconnectServices();
        EXPECT_TRUE(sender->commSystem->connectServices());
        sender->commSystem->disconnectServices();
        EXPECT_TRUE(sender->commSystem->connectServices());

        state->startUpHook();
        ASSERT_TRUE(state->blockOnAllConnected());

        state->disconnectAll();
    }

    TEST_P(ACommunicationSystemWithDaemonMultiParticipant, DISABLED_canSendMessageAfterConnectAndDisconnectMultipleTimes)
    {
        std::unique_ptr<CommunicationSystemTestWrapper> receiver{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "receiver")};
        receiver->commSystem->connectServices();

        std::unique_ptr<CommunicationSystemTestWrapper> sender{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "sender")};
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

        EXPECT_CALL(handler, handleSubscribeScene(sceneId, sender->id)).WillOnce(SendHandlerCalledEvent(state.get()));
        sender->commSystem->sendSubscribeScene(receiver->id, sceneId);
        ASSERT_TRUE(state->event.waitForEvents(1));

        state->disconnectAll();
    }

    TEST_P(ACommunicationSystemWithDaemonMultiParticipant, canEstablishConnectionToNewParticipantWithSameGuid)
    {
        const Guid csw1Id("00000000-0000-0000-0000-000000000002");
        const Guid csw2Id("00000000-0000-0000-0000-000000000001");   // csw2 id MUST be smaller => forces csw1 to initiate connections

        std::unique_ptr<CommunicationSystemTestWrapper> csw1{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "csw1", csw1Id)};
        std::unique_ptr<CommunicationSystemTestWrapper> csw2{CommunicationSystemTestFactory::ConstructTestWrapper(*state, "csw2", csw2Id)};

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

        state->startUpHook();

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

        // construct new csw2 with same guid but mot likely other port
        csw2 = CommunicationSystemTestFactory::ConstructTestWrapper(*state, "csw2", csw2Id);

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

        state->shutdownHook();
    }
}
