//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportCommon/RamsesConnectionSystem.h"
#include "SomeIPStackMocks.h"
#include "MockConnectionStatusListener.h"
#include "ServiceHandlerMocks.h"
#include "gmock/gmock.h"
#include "TransportCommon/ISomeIPRamsesStack.h"
#include "Utils/LogMacros.h"
#include "ConnectionSystemTestCommon.h"
#include "TransportCommon/ISceneUpdateSerializer.h"
#include "SceneUpdateSerializerTestHelper.h"
#include "ConcreteConnectionSystemCommonTest.h"

namespace ramses_internal
{
    class ARamsesConnectionSystem : public TestWithParam<ConcreteConnectionSystemCommonTest::Config>,
                                    public ConcreteConnectionSystemCommonTest
    {
    public:
        ARamsesConnectionSystem()
            : stackPtr(std::make_shared<StrictMock<SomeIPRamsesStackMock>>())
            , stack(*stackPtr)
            , connsys(construct(stackPtr))
            , fromStack(*connsys)
        {
        }

        std::unique_ptr<RamsesConnectionSystem> construct(const std::shared_ptr<StrictMock<SomeIPRamsesStackMock>>& stackMock)
        {
            EXPECT_CALL(stack, getServiceInstanceId()).WillRepeatedly(Return(RamsesInstanceId{GetParam().serviceIid}));
            auto ptr = RamsesConnectionSystem::Construct(stackMock, 3, ParticipantIdentifier(Guid(pid), "foobar"), 99, lock,
                                                       std::chrono::milliseconds(0), std::chrono::milliseconds(0),
                                                       CountingConnectionSystemClock());
            assert(ptr);

            ptr->setSceneProviderServiceHandler(&sceneProvider);
            ptr->setSceneRendererServiceHandler(&sceneRenderer);
            ptr->getConnectionStatusUpdateNotifier().registerForConnectionUpdates(&connections);
            EXPECT_CALL(stack, getSendDataSizes()).WillRepeatedly(Return(RamsesStackSendDataSizes{1000, 1000, 10, 10}));

            return ptr;
        }

        uint64_t connectRemote(RamsesInstanceId remoteIidArg, const Guid& remotePid)
        {
            return connectRemoteHelper(GetParam(), stack, fromStack, remoteIidArg, remotePid);
        }

        std::shared_ptr<StrictMock<SomeIPRamsesStackMock>> stackPtr;
        StrictMock<SomeIPRamsesStackMock>& stack;
        StrictMock<SceneProviderServiceHandlerMock> sceneProvider;
        StrictMock<SceneRendererServiceHandlerMock> sceneRenderer;
        PlatformLock lock;
        std::unique_ptr<RamsesConnectionSystem> connsys;
        ISomeIPRamsesStackCallbacks& fromStack;
        StrictMock<SceneUpdateSerializerMock> serializerMock;
        RamsesInstanceId remoteIid{GetParam().remoteIid};

        SceneInfoVector sceneInfos{SceneInfo{SceneId(99), "asd"}, SceneInfo{SceneId(1), ""}, SceneInfo{SceneId(0), "boo"}};
        std::vector<SceneAvailabilityUpdate> sceneUpdateAvailable{{SceneId(99), "asd", true}, {SceneId(1), "", true}, {SceneId(0), "boo", true}};
        std::vector<SceneAvailabilityUpdate> sceneUpdateUnavailable{{SceneId(99), "asd", false}, {SceneId(1), "", false}, {SceneId(0), "boo", false}};

        std::vector<Byte> dataBlob{200, 127, 255, 0, 1, 80};
        std::vector<Byte> dataBlob_2{8, 7, 6, 5, 4, 2, 0, 100};
    };

    INSTANTIATE_TEST_SUITE_P(ARamsesConnectionSystemP,
                             ARamsesConnectionSystem,
                             testing::ValuesIn(ConcreteConnectionSystemCommonTest::ConfigValues()),
                             ConcreteConnectionSystemCommonTest::ConfigPrinter());

    class ARamsesConnectionSystemConnected : public ARamsesConnectionSystem
    {
    public:
        void SetUp() override
        {
            lock.lock();
            EXPECT_CALL(stack, connect()).WillOnce(Return(true));
            ASSERT_TRUE(connsys->connect());
        }

        void TearDown() override
        {
            EXPECT_CALL(stack, disconnect()).WillOnce(Return(true));
            ASSERT_TRUE(connsys->disconnect());
            lock.unlock();
        }
    };

    INSTANTIATE_TEST_SUITE_P(ARamsesConnectionSystemConnectedP,
                             ARamsesConnectionSystemConnected,
                             testing::ValuesIn(ConcreteConnectionSystemCommonTest::ConfigValues()),
                             ConcreteConnectionSystemCommonTest::ConfigPrinter());

    TEST_P(ARamsesConnectionSystem, sendMethodsFailWhenNotConnected)
    {
        std::lock_guard<std::recursive_mutex> g(lock);
        EXPECT_FALSE(connsys->sendScenesAvailable(Guid(10), sceneInfos));
        EXPECT_FALSE(connsys->sendSubscribeScene(Guid(10), SceneId(321)));
        EXPECT_FALSE(connsys->sendUnsubscribeScene(Guid(10), SceneId(678)));
        EXPECT_FALSE(connsys->sendInitializeScene(Guid(10), SceneId(65)));
        EXPECT_FALSE(connsys->sendRendererEvent(Guid(10), SceneId(2), dataBlob));
        EXPECT_FALSE(connsys->sendSceneUpdate(Guid(10), SceneId(876), FakseSceneUpdateSerializer({dataBlob}, 1000)));
    }

    TEST_P(ARamsesConnectionSystemConnected, canSendUnicastMessagesToConnectedParticipant)
    {
        connectRemote(RamsesInstanceId(1), Guid(2));
        connectRemote(remoteIid, Guid(10));

        SomeIPMsgHeader firstHdr;
        EXPECT_CALL(stack, sendSceneAvailabilityChange(remoteIid, ValidHdr(pid, 2u), sceneUpdateAvailable)).WillOnce(DoAll(SaveArg<1>(&firstHdr), Return(true)));
        EXPECT_TRUE(connsys->sendScenesAvailable(Guid(10), sceneInfos));

        EXPECT_CALL(stack, sendSceneSubscriptionChange(remoteIid, SomeIPMsgHeader{pid, firstHdr.sessionId, 3u}, std::vector<SceneSubscriptionUpdate>({{SceneId(321), true}}))).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendSubscribeScene(Guid(10), SceneId(321)));

        EXPECT_CALL(stack, sendSceneSubscriptionChange(remoteIid, SomeIPMsgHeader{pid, firstHdr.sessionId, 4u}, std::vector<SceneSubscriptionUpdate>({{SceneId(678), false}}))).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendUnsubscribeScene(Guid(10), SceneId(678)));

        EXPECT_CALL(stack, sendInitializeScene(remoteIid, SomeIPMsgHeader{pid, firstHdr.sessionId, 5u}, SceneId(65))).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendInitializeScene(Guid(10), SceneId(65)));

        EXPECT_CALL(stack, sendRendererEvent(remoteIid, SomeIPMsgHeader{pid, firstHdr.sessionId, 6u}, SceneId(2), dataBlob)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendRendererEvent(Guid(10), SceneId(2), dataBlob));

        EXPECT_CALL(stack, sendSceneUpdate(remoteIid, SomeIPMsgHeader{pid, firstHdr.sessionId, 7u}, SceneId(999), dataBlob)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendSceneUpdate(Guid(10), SceneId(999), FakseSceneUpdateSerializer({dataBlob}, 1000)));

        expectRemoteDisconnects(stack, {2, 10});
    }

    TEST_P(ARamsesConnectionSystemConnected, canSendBroadcastMessagesToConnectedParticipants)
    {
        connectRemote(RamsesInstanceId(1), Guid(2));
        connectRemote(remoteIid, Guid(10));

        SomeIPMsgHeader firstHdrIid_1;
        SomeIPMsgHeader firstHdrIid_3;

        EXPECT_CALL(stack, sendSceneAvailabilityChange(RamsesInstanceId(1), ValidHdr(pid, 2u), sceneUpdateAvailable)).WillOnce(DoAll(SaveArg<1>(&firstHdrIid_1), Return(true)));
        EXPECT_CALL(stack, sendSceneAvailabilityChange(remoteIid, ValidHdr(pid, 2u), sceneUpdateAvailable)).WillOnce(DoAll(SaveArg<1>(&firstHdrIid_3), Return(true)));
        EXPECT_TRUE(connsys->broadcastNewScenesAvailable(sceneInfos));

        EXPECT_CALL(stack, sendSceneAvailabilityChange(RamsesInstanceId(1), SomeIPMsgHeader{pid, firstHdrIid_1.sessionId, 3u}, sceneUpdateUnavailable)).WillOnce(Return(true));
        EXPECT_CALL(stack, sendSceneAvailabilityChange(remoteIid, SomeIPMsgHeader{pid, firstHdrIid_3.sessionId, 3u}, sceneUpdateUnavailable)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->broadcastScenesBecameUnavailable(sceneInfos));

        expectRemoteDisconnects(stack, {2, 10});
    }

    TEST_P(ARamsesConnectionSystemConnected, cannotSendUnicastMessageToUnconnectedParticipant)
    {
        connectRemote(remoteIid, Guid(2));
        expectRemoteDisconnects(stack, {2});
        fromStack.handleServiceUnavailable(remoteIid);
        Mock::VerifyAndClearExpectations(&connections);

        EXPECT_FALSE(connsys->sendScenesAvailable(Guid(10), sceneInfos));
        EXPECT_FALSE(connsys->sendSubscribeScene(Guid(10), SceneId(321)));
        EXPECT_FALSE(connsys->sendUnsubscribeScene(Guid(10), SceneId(678)));
        EXPECT_FALSE(connsys->sendInitializeScene(Guid(10), SceneId(65)));
        EXPECT_FALSE(connsys->sendRendererEvent(Guid(10), SceneId(2), dataBlob));
        EXPECT_FALSE(connsys->sendSceneUpdate(Guid(10), SceneId(876), FakseSceneUpdateSerializer({dataBlob}, 1000)));

        EXPECT_FALSE(connsys->sendScenesAvailable(Guid(2), sceneInfos));
        EXPECT_FALSE(connsys->sendSubscribeScene(Guid(2), SceneId(321)));
        EXPECT_FALSE(connsys->sendUnsubscribeScene(Guid(2), SceneId(678)));
        EXPECT_FALSE(connsys->sendInitializeScene(Guid(2), SceneId(65)));
        EXPECT_FALSE(connsys->sendRendererEvent(Guid(2), SceneId(2), dataBlob));
        EXPECT_FALSE(connsys->sendSceneUpdate(Guid(2), SceneId(876), FakseSceneUpdateSerializer({dataBlob}, 1000)));
    }

    TEST_P(ARamsesConnectionSystemConnected, passesThroughMessagesFromConnectedParticipant)
    {
        const uint64_t session = connectRemote(remoteIid, Guid(10));

        EXPECT_CALL(sceneProvider, handleSubscribeScene(SceneId(67), Guid(10)));
        EXPECT_CALL(sceneProvider, handleUnsubscribeScene(SceneId(68), Guid(10)));
        EXPECT_CALL(sceneProvider, handleSubscribeScene(SceneId(1), Guid(10)));
        fromStack.handleSceneSubscriptionChange(SomeIPMsgHeader{10, session, 2}, {{SceneId(67), true}, {SceneId(68), false}, {SceneId(1), true}});
        fromStack.handleSceneSubscriptionChange(SomeIPMsgHeader{10, session, 3}, {});  // empty list triggers nothing

        EXPECT_CALL(sceneProvider, handleRendererEvent(SceneId(69), dataBlob, Guid(10)));
        fromStack.handleRendererEvent(SomeIPMsgHeader{10, session, 4}, SceneId(69), dataBlob);

        EXPECT_CALL(sceneRenderer, handleNewScenesAvailable(SceneInfoVector{SceneInfo(SceneId(99), "asd", EScenePublicationMode_LocalAndRemote), SceneInfo(SceneId(0), "boo", EScenePublicationMode_LocalAndRemote)}, Guid(10)));
        EXPECT_CALL(sceneRenderer, handleScenesBecameUnavailable(SceneInfoVector{SceneInfo(SceneId(400001), "xxxxxxuzuuu"), SceneInfo(SceneId(1), "")}, Guid(10)));
        fromStack.handleSceneAvailabilityChange(SomeIPMsgHeader{10, session, 5}, {{SceneId(99), "asd", true}, {SceneId(400001), "xxxxxxuzuuu", false},
                                                                              {SceneId(1), "", false}, {SceneId(0), "boo", true}});
        fromStack.handleSceneAvailabilityChange(SomeIPMsgHeader{10, session, 6}, {});  // empty list triggers nothing

        EXPECT_CALL(sceneRenderer, handleInitializeScene(SceneId(9999954), Guid(10)));
        fromStack.handleInitializeScene(SomeIPMsgHeader{10, session, 7}, SceneId(9999954));

        EXPECT_CALL(sceneRenderer, handleSceneUpdate(SceneId(34534), absl::Span<const Byte>(dataBlob), Guid(10)));
        fromStack.handleSceneUpdate(SomeIPMsgHeader{10, session, 8}, SceneId(34534), dataBlob);

        expectRemoteDisconnects(stack, {10});
    }

    TEST_P(ARamsesConnectionSystemConnected, sendMethodsForwardStackFailures_sendScenesAvailable)
    {
        connectRemote(remoteIid, Guid(10));
        expectRemoteDisconnects(stack, {10});

        EXPECT_CALL(stack, sendParticipantInfo(remoteIid, ValidHdr(pid, 1u), 99, _, RamsesInstanceId{GetParam().serviceIid}, _, _, _)).Times(AtMost(1)); // if initiator will try to send new session
        EXPECT_CALL(stack, sendSceneAvailabilityChange(remoteIid, ValidHdr(pid, 2u), sceneUpdateAvailable)).WillOnce(Return(false));
        EXPECT_FALSE(connsys->sendScenesAvailable(Guid(10), sceneInfos));
    }

    TEST_P(ARamsesConnectionSystemConnected, sendMethodsForwardStackFailures_sendSubscribeScene)
    {
        connectRemote(remoteIid, Guid(10));
        expectRemoteDisconnects(stack, {10});

        EXPECT_CALL(stack, sendSceneSubscriptionChange(remoteIid, ValidHdr(pid, 2u), std::vector<SceneSubscriptionUpdate>({{SceneId(321), true}})))
            .WillOnce(Return(false));
        EXPECT_CALL(stack, sendParticipantInfo(remoteIid, ValidHdr(pid, 1u), 99, _, RamsesInstanceId{GetParam().serviceIid}, _, _, _)).Times(AtMost(1)); // if initiator will try to send new session
        EXPECT_FALSE(connsys->sendSubscribeScene(Guid(10), SceneId(321)));
    }

    TEST_P(ARamsesConnectionSystemConnected, sendMethodsForwardStackFailures_sendUnsubscribeScene)
    {
        connectRemote(remoteIid, Guid(10));
        expectRemoteDisconnects(stack, {10});

        EXPECT_CALL(stack, sendSceneSubscriptionChange(remoteIid, ValidHdr(pid, 2u), std::vector<SceneSubscriptionUpdate>({{SceneId(678), false}})))
            .WillOnce(Return(false));
        EXPECT_CALL(stack, sendParticipantInfo(remoteIid, ValidHdr(pid, 1u), 99, _, RamsesInstanceId{GetParam().serviceIid}, _, _, _)).Times(AtMost(1)); // if initiator will try to send new session
        EXPECT_FALSE(connsys->sendUnsubscribeScene(Guid(10), SceneId(678)));
    }

    TEST_P(ARamsesConnectionSystemConnected, sendMethodsForwardStackFailures_sendInitializeScene)
    {
        connectRemote(remoteIid, Guid(10));
        expectRemoteDisconnects(stack, {10});

        EXPECT_CALL(stack, sendInitializeScene(remoteIid, ValidHdr(pid, 2u), SceneId(65))).WillOnce(Return(false));
        EXPECT_CALL(stack, sendParticipantInfo(remoteIid, ValidHdr(pid, 1u), 99, _, RamsesInstanceId{GetParam().serviceIid}, _, _, _)).Times(AtMost(1)); // if initiator will try to send new session
        EXPECT_FALSE(connsys->sendInitializeScene(Guid(10), SceneId(65)));
    }

    TEST_P(ARamsesConnectionSystemConnected, sendMethodsForwardStackFailures_sendRendererEvent)
    {
        connectRemote(remoteIid, Guid(10));
        expectRemoteDisconnects(stack, {10});

        EXPECT_CALL(stack, sendRendererEvent(remoteIid, ValidHdr(pid, 2u), SceneId(2), dataBlob)).WillOnce(Return(false));
        EXPECT_CALL(stack, sendParticipantInfo(remoteIid, ValidHdr(pid, 1u), 99, _, RamsesInstanceId{GetParam().serviceIid}, _, _, _)).Times(AtMost(1)); // if initiator will try to send new session
        EXPECT_FALSE(connsys->sendRendererEvent(Guid(10), SceneId(2), dataBlob));
    }

    TEST_P(ARamsesConnectionSystemConnected, sendMethodsForwardStackFailures_sendSceneUpdate)
    {
        connectRemote(remoteIid, Guid(10));
        expectRemoteDisconnects(stack, {10});

        EXPECT_CALL(stack, sendSceneUpdate(remoteIid, ValidHdr(pid, 2u), SceneId(999), dataBlob)).WillOnce(Return(false));
        EXPECT_CALL(stack, sendParticipantInfo(remoteIid, ValidHdr(pid, 1u), 99, _, RamsesInstanceId{GetParam().serviceIid}, _, _, _)).Times(AtMost(1)); // if initiator will try to send new session
        EXPECT_FALSE(connsys->sendSceneUpdate(Guid(10), SceneId(999), FakseSceneUpdateSerializer({dataBlob, dataBlob_2}, 1000)));
    }

    TEST_P(ARamsesConnectionSystemConnected, blobSendMethodsChunkData)
    {
        connectRemote(remoteIid, Guid(10));

        EXPECT_CALL(stack, sendSceneUpdate(remoteIid, ValidHdr(pid, 2u), SceneId(999), dataBlob)).WillOnce(Return(true));
        EXPECT_CALL(stack, sendSceneUpdate(remoteIid, ValidHdr(pid, 3u), SceneId(999), dataBlob_2)).WillOnce(Return(true));
        EXPECT_TRUE(connsys->sendSceneUpdate(Guid(10), SceneId(999), FakseSceneUpdateSerializer({ dataBlob, dataBlob_2 }, 1000)));

        expectRemoteDisconnects(stack, { 10 });
    }
}
