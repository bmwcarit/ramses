//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/SceneGraphComponent.h"
#include "ComponentMocks.h"
#include "CommunicationSystemMock.h"
#include "MockConnectionStatusUpdateNotifier.h"
#include "ServiceHandlerMocks.h"
#include "Components/FlushTimeInformation.h"

using namespace ramses_internal;

class ASceneGraphComponent : public ::testing::Test
{
public:
    ASceneGraphComponent()
        : localParticipantID(true)
        , remoteParticipantID(true)
        , sceneGraphComponent(localParticipantID, communicationSystem, connectionStatusUpdateNotifier, frameworkLock)
    {
        localSceneIdInfoVector.push_back(SceneInfo(localSceneId, "sceneName"));
    }

    SceneActionCollection createFakeSceneActionCollectionFromTypes(const std::vector<ESceneActionId>& types)
    {
        SceneActionCollection collection;
        for (auto t : types)
        {
            collection.addRawSceneActionInformation(t, 0);
        }
        return collection;
    }

    void publishScene(UInt32 sceneId, const String& name, EScenePublicationMode pubMode)
    {
        SceneInfoVector sceneInfoVec{ SceneInfo(SceneId(sceneId), name) };
        EXPECT_CALL(consumer, handleNewScenesAvailable(sceneInfoVec, _, pubMode));
        if (pubMode != EScenePublicationMode_LocalOnly)
            EXPECT_CALL(communicationSystem, broadcastNewScenesAvailable(sceneInfoVec));
        sceneGraphComponent.sendPublishScene(SceneId(sceneId), pubMode, name);
    }

protected:
    PlatformLock frameworkLock;
    SceneId localSceneId;
    SceneInfoVector localSceneIdInfoVector;
    SceneId remoteSceneId;
    Guid localParticipantID;
    Guid remoteParticipantID;
    StrictMock<CommunicationSystemMock> communicationSystem;
    NiceMock<MockConnectionStatusUpdateNotifier> connectionStatusUpdateNotifier;
    SceneGraphComponent sceneGraphComponent;
    SceneRendererServiceHandlerMock consumer;
    SceneProviderServiceHandlerMock provider;
};


TEST_F(ASceneGraphComponent, sendsSceneToLocalConsumer)
{
    sceneGraphComponent.setSceneRendererServiceHandler(&consumer);

    EXPECT_CALL(consumer, handleInitializeScene(_, _)).Times(1);
    sceneGraphComponent.sendCreateScene(localParticipantID, SceneInfo(SceneId(666u), "test scene"), EScenePublicationMode_LocalAndRemote);
}

TEST_F(ASceneGraphComponent, doesntSendSceneIfLocalConsumerIsntSet)
{
    EXPECT_CALL(consumer, handleInitializeScene(_, localParticipantID)).Times(0);

    sceneGraphComponent.sendCreateScene(localParticipantID, SceneInfo(SceneId(666u), "test scene"), EScenePublicationMode_LocalAndRemote);
}

TEST_F(ASceneGraphComponent, sendsSceneToRemoteProvider)
{
    sceneGraphComponent.setSceneProviderServiceHandler(&provider);

    SceneInfo sceneInfo(SceneId(666u), "test scene");
    EXPECT_CALL(communicationSystem, sendInitializeScene(remoteParticipantID, sceneInfo));
    sceneGraphComponent.sendCreateScene(remoteParticipantID, sceneInfo, EScenePublicationMode_LocalAndRemote);
}

TEST_F(ASceneGraphComponent, publishesSceneAtLocalConsumerInLocalAndRemoteMode)
{
    sceneGraphComponent.setSceneRendererServiceHandler(&consumer);

    Guid providerID(true);
    EXPECT_CALL(consumer, handleNewScenesAvailable(localSceneIdInfoVector, _, EScenePublicationMode_LocalAndRemote)).Times(1);
    EXPECT_CALL(communicationSystem, broadcastNewScenesAvailable(localSceneIdInfoVector));
    sceneGraphComponent.sendPublishScene(localSceneId, EScenePublicationMode_LocalAndRemote, "sceneName");
}

TEST_F(ASceneGraphComponent, publishesSceneAtLocalConsumerInLocalOnlyMode)
{
    sceneGraphComponent.setSceneRendererServiceHandler(&consumer);

    EXPECT_CALL(consumer, handleNewScenesAvailable(_, _, EScenePublicationMode_LocalOnly)).Times(1);
    sceneGraphComponent.sendPublishScene(localSceneId, EScenePublicationMode_LocalOnly, "sceneName");
}

TEST_F(ASceneGraphComponent, doesntPublishSceneIfLocalConsumerIsntSet)
{
    Guid providerID(true);
    EXPECT_CALL(consumer, handleNewScenesAvailable(_, _, EScenePublicationMode_LocalAndRemote)).Times(0);
    EXPECT_CALL(communicationSystem, broadcastNewScenesAvailable(localSceneIdInfoVector));
    sceneGraphComponent.sendPublishScene(localSceneId, EScenePublicationMode_LocalAndRemote, "sceneName");
}

TEST_F(ASceneGraphComponent, doesNotPublishSceneToRemoteProviderInLocalOnlyMode)
{
    sceneGraphComponent.setSceneProviderServiceHandler(&provider);
    sceneGraphComponent.sendPublishScene(remoteSceneId, EScenePublicationMode_LocalOnly, "sceneName");
    // no expect needed, StrictMock on communicationSystem checks it already
}

TEST_F(ASceneGraphComponent, publishesSceneAtRemoteProvider)
{
    sceneGraphComponent.setSceneProviderServiceHandler(&provider);
    sceneGraphComponent.newParticipantHasConnected(Guid(true));

    SceneInfoVector newScenes(1, SceneInfo(remoteSceneId, "sceneName"));
    EXPECT_CALL(communicationSystem, broadcastNewScenesAvailable(newScenes));
    sceneGraphComponent.sendPublishScene(remoteSceneId, EScenePublicationMode_LocalAndRemote, "sceneName");
}

TEST_F(ASceneGraphComponent, alreadyPublishedSceneGetsRepublishedWhenLocalConsumerIsSet)
{
    sceneGraphComponent.sendPublishScene(localSceneId, EScenePublicationMode_LocalOnly, "sceneName");

    EXPECT_CALL(consumer, handleNewScenesAvailable(localSceneIdInfoVector, _, EScenePublicationMode_LocalOnly));
    sceneGraphComponent.setSceneRendererServiceHandler(&consumer);
}

TEST_F(ASceneGraphComponent, unpublishesSceneAtLocalConsumer)
{
    const SceneId sceneId(1ull << 63);
    sceneGraphComponent.setSceneRendererServiceHandler(&consumer);

    SceneInfoVector newScenes(1, SceneInfo(sceneId, "sceneName"));
    EXPECT_CALL(consumer, handleNewScenesAvailable(newScenes, _, EScenePublicationMode_LocalOnly));
    sceneGraphComponent.sendPublishScene(sceneId, EScenePublicationMode_LocalOnly, "sceneName");

    SceneInfoVector unavailableScenes(1, SceneInfo(sceneId, "sceneName"));
    EXPECT_CALL(consumer, handleScenesBecameUnavailable(unavailableScenes, _));
    sceneGraphComponent.sendUnpublishScene(sceneId, EScenePublicationMode_LocalOnly);
}

TEST_F(ASceneGraphComponent, doesntUnpublishSceneIfLocalConsumerIsntSet)
{
    EXPECT_CALL(consumer, handleScenesBecameUnavailable(_, _)).Times(0);

    const SceneId sceneId(1ull << 63);
    sceneGraphComponent.sendPublishScene(sceneId, EScenePublicationMode_LocalOnly, "sceneName");
    sceneGraphComponent.sendUnpublishScene(sceneId, EScenePublicationMode_LocalOnly);
}

TEST_F(ASceneGraphComponent, doesNotUnpublishesSceneAtRemoteProviderIfSceneWasPublishedLocalOnly)
{
    const SceneId sceneId(1ull << 63);
    sceneGraphComponent.setSceneProviderServiceHandler(&provider);
    sceneGraphComponent.sendPublishScene(sceneId, EScenePublicationMode_LocalOnly, "sceneName");
    sceneGraphComponent.sendUnpublishScene(sceneId, EScenePublicationMode_LocalOnly);
    // expect noting on remote, checked by strict mock
}

TEST_F(ASceneGraphComponent, unpublishesSceneAtRemoteProvider)
{
    const SceneId sceneId(1ull << 63);
    sceneGraphComponent.setSceneProviderServiceHandler(&provider);
    sceneGraphComponent.newParticipantHasConnected(Guid(true));
    EXPECT_CALL(communicationSystem, broadcastNewScenesAvailable(_));
    sceneGraphComponent.sendPublishScene(sceneId, EScenePublicationMode_LocalAndRemote, "sceneName");

    SceneInfoVector unavailableScenes(1, SceneInfo(sceneId, "sceneName"));
    EXPECT_CALL(communicationSystem, broadcastScenesBecameUnavailable(unavailableScenes));
    sceneGraphComponent.sendUnpublishScene(sceneId, EScenePublicationMode_LocalAndRemote);
}

TEST_F(ASceneGraphComponent, sendsAvailableScenesToNewParticipant)
{
    sceneGraphComponent.setSceneProviderServiceHandler(&provider);
    SceneInfoVector newScenes(1, SceneInfo(remoteSceneId, "sceneName"));
    EXPECT_CALL(communicationSystem, broadcastNewScenesAvailable(newScenes));
    sceneGraphComponent.sendPublishScene(remoteSceneId, EScenePublicationMode_LocalAndRemote, "sceneName");

    Guid id(true);
    EXPECT_CALL(communicationSystem, sendScenesAvailable(id, newScenes));
    sceneGraphComponent.newParticipantHasConnected(id);
}

TEST_F(ASceneGraphComponent, doesNotSendAvailableSceneToNewParticipantIfLocalOnlyScene)
{
    sceneGraphComponent.setSceneProviderServiceHandler(&provider);
    SceneInfoVector newScenes(1, SceneInfo(remoteSceneId, "sceneName"));
    sceneGraphComponent.sendPublishScene(remoteSceneId, EScenePublicationMode_LocalOnly, "sceneName");

    Guid id(true);
    EXPECT_CALL(communicationSystem, sendScenesAvailable(id, newScenes)).Times(0);
    sceneGraphComponent.newParticipantHasConnected(id);
}

TEST_F(ASceneGraphComponent, subscribeSceneAtLocalProvider)
{
    sceneGraphComponent.setSceneProviderServiceHandler(&provider);

    EXPECT_CALL(provider, handleSubscribeScene(_, _)).Times(1);
    sceneGraphComponent.subscribeScene(localParticipantID, SceneId(0u));
}

TEST_F(ASceneGraphComponent, doesntSubscribeSceneIfLocalProviderIsntSet)
{
    EXPECT_CALL(provider, handleSubscribeScene(_, _)).Times(0);
    sceneGraphComponent.subscribeScene(localParticipantID, SceneId(0u));
}

TEST_F(ASceneGraphComponent, subscribeSceneAtRemoteConsumer)
{
    sceneGraphComponent.setSceneRendererServiceHandler(&consumer);
    SceneId sceneId;
    EXPECT_CALL(communicationSystem, sendSubscribeScene(remoteParticipantID, sceneId));
    sceneGraphComponent.subscribeScene(remoteParticipantID, sceneId);
}

TEST_F(ASceneGraphComponent, unsubscribesSceneAtLocalProvider)
{
    sceneGraphComponent.setSceneProviderServiceHandler(&provider);

    EXPECT_CALL(provider, handleUnsubscribeScene(_, _)).Times(1);
    sceneGraphComponent.unsubscribeScene(localParticipantID, SceneId(0u));
}

TEST_F(ASceneGraphComponent, doesntUnsubscribeSceneIfLocalProviderIsntSet)
{
    EXPECT_CALL(provider, handleUnsubscribeScene(_, _)).Times(0);
    sceneGraphComponent.unsubscribeScene(localParticipantID, SceneId(0u));
}

TEST_F(ASceneGraphComponent, unsubscribesSceneAtRemoteConsumer)
{
    const SceneId sceneId(1ull << 63);
    sceneGraphComponent.setSceneRendererServiceHandler(&consumer);
    EXPECT_CALL(communicationSystem, sendSubscribeScene(_, _));
    sceneGraphComponent.subscribeScene(remoteParticipantID, sceneId);
    EXPECT_CALL(communicationSystem, sendUnsubscribeScene(remoteParticipantID, sceneId));
    sceneGraphComponent.unsubscribeScene(remoteParticipantID, sceneId);
}

TEST_F(ASceneGraphComponent, sendsSceneActionToLocalConsumer)
{
    sceneGraphComponent.setSceneRendererServiceHandler(&consumer);

    SceneActionCollection list(createFakeSceneActionCollectionFromTypes({ ESceneActionId_TestAction }));
    EXPECT_CALL(consumer, handleSceneActionList_rvr(_, _, _, _)).Times(1);
    sceneGraphComponent.sendSceneActionList({ localParticipantID }, std::move(list), SceneId(666u), EScenePublicationMode_LocalOnly);
}

TEST_F(ASceneGraphComponent, doesntSendSceneActionIfLocalConsumerIsntSet)
{
    SceneActionCollection list(createFakeSceneActionCollectionFromTypes({ ESceneActionId_TestAction }));
    EXPECT_CALL(consumer, handleSceneActionList_rvr(_, _, _, _)).Times(0);
    sceneGraphComponent.sendSceneActionList({ localParticipantID }, std::move(list), SceneId(666u), EScenePublicationMode_LocalOnly);
}

TEST_F(ASceneGraphComponent, sendsSceneActionToRemoteProvider)
{
    sceneGraphComponent.setSceneProviderServiceHandler(&provider);
    SceneId sceneId;
    EXPECT_CALL(communicationSystem, sendInitializeScene(_, _)).Times(1);
    sceneGraphComponent.sendCreateScene(remoteParticipantID, SceneInfo{ sceneId }, EScenePublicationMode_LocalAndRemote);

    SceneActionCollection list(createFakeSceneActionCollectionFromTypes({ ESceneActionId_TestAction }));
    EXPECT_CALL(communicationSystem, sendSceneActionList(remoteParticipantID, sceneId, _, _));
    sceneGraphComponent.sendSceneActionList({ remoteParticipantID }, std::move(list), sceneId, EScenePublicationMode_LocalAndRemote);
}

TEST_F(ASceneGraphComponent, sendsAllSceneActionsAtOnceToRemoteProvider)
{
    sceneGraphComponent.setSceneProviderServiceHandler(&provider);
    SceneId sceneId;
    EXPECT_CALL(communicationSystem, sendInitializeScene(_, _)).Times(1);
    sceneGraphComponent.sendCreateScene(remoteParticipantID, SceneInfo{ sceneId }, EScenePublicationMode_LocalAndRemote);

    SceneActionCollection list(createFakeSceneActionCollectionFromTypes({ ESceneActionId_TestAction, ESceneActionId_SetDataVector2fArray, ESceneActionId_AllocateNode }));
    EXPECT_CALL(communicationSystem, sendSceneActionList(remoteParticipantID, sceneId, _, _)).Times(1);
    sceneGraphComponent.sendSceneActionList({ remoteParticipantID }, std::move(list), sceneId, EScenePublicationMode_LocalAndRemote);
}

TEST_F(ASceneGraphComponent, doesNotsendSceneActionListToRemoteIfSceneWasPublishedLocalOnly)
{
    const SceneId sceneId(1ull << 63);
    sceneGraphComponent.sendPublishScene(sceneId, EScenePublicationMode_LocalOnly, "sceneName");
    SceneActionCollection list(createFakeSceneActionCollectionFromTypes({ ESceneActionId_TestAction }));

    sceneGraphComponent.setSceneProviderServiceHandler(&provider);
    sceneGraphComponent.sendSceneActionList({ localParticipantID }, std::move(list), sceneId, EScenePublicationMode_LocalOnly);

    // expect noting for remote, check by strict mock
}

TEST_F(ASceneGraphComponent, canRepublishALocalOnlySceneToBeDistributedRemotely)
{
    Guid remoteID(true);
    const SceneId sceneId(1ull << 63);
    sceneGraphComponent.setSceneRendererServiceHandler(&consumer);
    sceneGraphComponent.setSceneProviderServiceHandler(&provider);
    sceneGraphComponent.newParticipantHasConnected(Guid(true));

    // publish LocalOnly
    EXPECT_CALL(consumer, handleNewScenesAvailable(_, _, EScenePublicationMode_LocalOnly));
    sceneGraphComponent.sendPublishScene(sceneId, EScenePublicationMode_LocalOnly, "sceneName");
    Mock::VerifyAndClearExpectations(&communicationSystem);

    // unpublish
    EXPECT_CALL(consumer, handleScenesBecameUnavailable(_, _));
    sceneGraphComponent.sendUnpublishScene(sceneId, EScenePublicationMode_LocalOnly);
    Mock::VerifyAndClearExpectations(&communicationSystem);
    Mock::VerifyAndClearExpectations(&consumer);

    // publish LocalAndRemote
    SceneInfoVector newScenes(1, SceneInfo(sceneId, "sceneName"));
    EXPECT_CALL(consumer, handleNewScenesAvailable(newScenes, localParticipantID, EScenePublicationMode_LocalAndRemote));
    EXPECT_CALL(communicationSystem, broadcastNewScenesAvailable(newScenes));
    sceneGraphComponent.sendPublishScene(sceneId, EScenePublicationMode_LocalAndRemote, "sceneName");
}

TEST_F(ASceneGraphComponent, canRepublishARemoteSceneToBeLocalOnly)
{
    const SceneId sceneId(1ull << 63);
    sceneGraphComponent.setSceneRendererServiceHandler(&consumer);
    sceneGraphComponent.setSceneProviderServiceHandler(&provider);
    sceneGraphComponent.newParticipantHasConnected(Guid(true));

    // publish LocalAndRemote
    SceneInfoVector knownScenes(1, SceneInfo(sceneId, "sceneName"));
    EXPECT_CALL(consumer, handleNewScenesAvailable(knownScenes, localParticipantID, _));
    EXPECT_CALL(communicationSystem, broadcastNewScenesAvailable(knownScenes));
    sceneGraphComponent.sendPublishScene(sceneId, EScenePublicationMode_LocalAndRemote, "sceneName");
    Mock::VerifyAndClearExpectations(&communicationSystem);
    Mock::VerifyAndClearExpectations(&consumer);

    // unpublish
    EXPECT_CALL(consumer, handleScenesBecameUnavailable(knownScenes, localParticipantID));
    EXPECT_CALL(communicationSystem, broadcastScenesBecameUnavailable(knownScenes));
    sceneGraphComponent.sendUnpublishScene(sceneId, EScenePublicationMode_LocalAndRemote);
    Mock::VerifyAndClearExpectations(&communicationSystem);
    Mock::VerifyAndClearExpectations(&consumer);

    // publish LocalOnly
    EXPECT_CALL(consumer, handleNewScenesAvailable(_, _, _));
    sceneGraphComponent.sendPublishScene(sceneId, EScenePublicationMode_LocalOnly, "sceneName");
    Mock::VerifyAndClearExpectations(&communicationSystem);
}

TEST_F(ASceneGraphComponent, sceneactionCounterIsAlwaysZeroWithLocalScenes)
{
    sceneGraphComponent.setSceneRendererServiceHandler(&consumer);

    SceneActionCollection list(createFakeSceneActionCollectionFromTypes({ ESceneActionId_TestAction }));
    EXPECT_CALL(consumer, handleSceneActionList_rvr(_, _, 0u, _)).Times(5);
    sceneGraphComponent.sendSceneActionList({ localParticipantID }, list.copy(), SceneId(666u), EScenePublicationMode_LocalOnly);
    sceneGraphComponent.sendSceneActionList({ localParticipantID }, list.copy(), SceneId(666u), EScenePublicationMode_LocalOnly);
    sceneGraphComponent.sendSceneActionList({ localParticipantID }, list.copy(), SceneId(666u), EScenePublicationMode_LocalOnly);
    sceneGraphComponent.sendSceneActionList({ localParticipantID }, list.copy(), SceneId(666u), EScenePublicationMode_LocalOnly);
    sceneGraphComponent.sendSceneActionList({ localParticipantID }, list.copy(), SceneId(666u), EScenePublicationMode_LocalOnly);
}

TEST_F(ASceneGraphComponent, sceneactionCounterIsCountingUpForNOnLocalScenes)
{
    sceneGraphComponent.setSceneRendererServiceHandler(&consumer);
    EXPECT_CALL(communicationSystem, sendInitializeScene(_, _)).Times(1);
    sceneGraphComponent.sendCreateScene(remoteParticipantID, SceneInfo{SceneId(666u)}, EScenePublicationMode_LocalAndRemote);

    SceneActionCollection list(createFakeSceneActionCollectionFromTypes({ ESceneActionId_TestAction }));
    testing::InSequence sequence;
    EXPECT_CALL(communicationSystem, sendSceneActionList(remoteParticipantID, SceneId(666u), _, 1u)).WillOnce(Return (1u));
    sceneGraphComponent.sendSceneActionList({ remoteParticipantID }, list.copy(), SceneId(666u), EScenePublicationMode_LocalAndRemote);

    EXPECT_CALL(communicationSystem, sendSceneActionList(remoteParticipantID, SceneId(666u), _, 2u)).WillOnce(Return(1u));
    sceneGraphComponent.sendSceneActionList({ remoteParticipantID }, list.copy(), SceneId(666u), EScenePublicationMode_LocalAndRemote);

    EXPECT_CALL(communicationSystem, sendSceneActionList(remoteParticipantID, SceneId(666u), _, 3u)).WillOnce(Return(1u));
    sceneGraphComponent.sendSceneActionList({ remoteParticipantID }, list.copy(), SceneId(666u), EScenePublicationMode_LocalAndRemote);
}

TEST_F(ASceneGraphComponent, sceneactionCounterIsCountingAccordingToNumberOfSentChunks)
{
    sceneGraphComponent.setSceneRendererServiceHandler(&consumer);
    EXPECT_CALL(communicationSystem, sendInitializeScene(_, _)).Times(1);
    sceneGraphComponent.sendCreateScene(remoteParticipantID, SceneInfo{ SceneId(666u) }, EScenePublicationMode_LocalAndRemote);

    SceneActionCollection list(createFakeSceneActionCollectionFromTypes({ ESceneActionId_TestAction }));
    testing::InSequence sequence;
    EXPECT_CALL(communicationSystem, sendSceneActionList(remoteParticipantID, SceneId(666u), _, 1u)).WillOnce(Return(5u));
    sceneGraphComponent.sendSceneActionList({ remoteParticipantID }, list.copy(), SceneId(666u), EScenePublicationMode_LocalAndRemote);

    EXPECT_CALL(communicationSystem, sendSceneActionList(remoteParticipantID, SceneId(666u), _, 6u)).WillOnce(Return(99u));
    sceneGraphComponent.sendSceneActionList({ remoteParticipantID }, list.copy(), SceneId(666u), EScenePublicationMode_LocalAndRemote);

    EXPECT_CALL(communicationSystem, sendSceneActionList(remoteParticipantID, SceneId(666u), _, 105u));
    sceneGraphComponent.sendSceneActionList({ remoteParticipantID }, list.copy(), SceneId(666u), EScenePublicationMode_LocalAndRemote);
}

TEST_F(ASceneGraphComponent, sceneactionCounterIsWrappedAround)
{
    sceneGraphComponent.setSceneRendererServiceHandler(&consumer);
    EXPECT_CALL(communicationSystem, sendInitializeScene(_, _)).Times(1);
    sceneGraphComponent.sendCreateScene(remoteParticipantID, SceneInfo{ SceneId(666u) }, EScenePublicationMode_LocalAndRemote);

    SceneActionCollection list(createFakeSceneActionCollectionFromTypes({ ESceneActionId_TestAction }));

    testing::InSequence sequence;
    // expected times to call 1...(wrap-1), so wrap-1 times
    EXPECT_CALL(communicationSystem, sendSceneActionList(remoteParticipantID, SceneId(666u), _, _)).Times(SceneActionList_CounterWrapAround - 1).WillRepeatedly(Return(1u));
    for (uint32_t i = 1; i < (SceneActionList_CounterWrapAround); ++i)
    {
        sceneGraphComponent.sendSceneActionList({ remoteParticipantID }, list.copy(), SceneId(666u), EScenePublicationMode_LocalAndRemote);
    }

    // wrap around starts again at '1'
    EXPECT_CALL(communicationSystem, sendSceneActionList(remoteParticipantID, SceneId(666u), _, 1u)).WillOnce(Return(1u));
    sceneGraphComponent.sendSceneActionList({ remoteParticipantID }, list.copy(), SceneId(666u), EScenePublicationMode_LocalAndRemote);

    // normal counting again
    EXPECT_CALL(communicationSystem, sendSceneActionList(remoteParticipantID, SceneId(666u), _, 2u)).WillOnce(Return(1u));
    sceneGraphComponent.sendSceneActionList({ remoteParticipantID }, list.copy(), SceneId(666u), EScenePublicationMode_LocalAndRemote);
}

TEST_F(ASceneGraphComponent, disconnectFromNetworkBroadcastsScenesUnavailableOnNetworkOnly)
{
    sceneGraphComponent.setSceneRendererServiceHandler(&consumer);
    publishScene(1, "name", EScenePublicationMode_LocalAndRemote);
    EXPECT_CALL(communicationSystem, broadcastScenesBecameUnavailable(SceneInfoVector{ SceneInfo(SceneId(1), "name") }));
    sceneGraphComponent.disconnectFromNetwork();
}

TEST_F(ASceneGraphComponent, disconnectFromNetworkBroadcastsScenesUnavailableOnNetworkForAllRemoteScenes)
{
    sceneGraphComponent.setSceneRendererServiceHandler(&consumer);
    publishScene(1, "name1", EScenePublicationMode_LocalAndRemote);
    publishScene(5, "name2", EScenePublicationMode_LocalOnly);
    publishScene(3, "name3", EScenePublicationMode_LocalAndRemote);

    SceneInfoVector unpubScenes;
    EXPECT_CALL(communicationSystem, broadcastScenesBecameUnavailable(_)).WillOnce(DoAll(SaveArg<0>(&unpubScenes), Return(true)));
    sceneGraphComponent.disconnectFromNetwork();

    ASSERT_EQ(2u, unpubScenes.size());
    EXPECT_TRUE(contains_c(unpubScenes, SceneInfo(SceneId(1), "name1")));
    EXPECT_TRUE(contains_c(unpubScenes, SceneInfo(SceneId(3), "name3")));
}

TEST_F(ASceneGraphComponent, sendsPublishForNewParticipantsAfterDisconnect)
{
    sceneGraphComponent.setSceneRendererServiceHandler(&consumer);
    publishScene(1, "name", EScenePublicationMode_LocalAndRemote);
    EXPECT_CALL(communicationSystem, broadcastScenesBecameUnavailable(SceneInfoVector{ SceneInfo(SceneId(1), "name") }));
    sceneGraphComponent.disconnectFromNetwork();

    EXPECT_CALL(communicationSystem, sendScenesAvailable(remoteParticipantID, SceneInfoVector{ SceneInfo(SceneId(1), "name") }));
    sceneGraphComponent.newParticipantHasConnected(remoteParticipantID);
}

TEST_F(ASceneGraphComponent, disconnectDoesNotAffectLocalScenesAtAllButUnpublishesAndResetsCountersForRemoteScenes)
{
    SceneInfo sceneInfo(SceneInfo(SceneId(1), "foo"));
    ClientScene scene(sceneInfo);

    sceneGraphComponent.setSceneRendererServiceHandler(&consumer);
    sceneGraphComponent.handleCreateScene(scene, false);

    // subscribe local and remote
    EXPECT_CALL(consumer, handleNewScenesAvailable(SceneInfoVector{ sceneInfo }, _, _));
    EXPECT_CALL(communicationSystem, broadcastNewScenesAvailable(SceneInfoVector{ sceneInfo }));
    sceneGraphComponent.handlePublishScene(SceneId(1), EScenePublicationMode_LocalAndRemote);

    EXPECT_CALL(communicationSystem, sendScenesAvailable(remoteParticipantID, SceneInfoVector{ sceneInfo }));
    sceneGraphComponent.newParticipantHasConnected(remoteParticipantID);
    sceneGraphComponent.handleSceneSubscription(SceneId(1), remoteParticipantID);

    sceneGraphComponent.handleSceneSubscription(SceneId(1), localParticipantID);

    EXPECT_CALL(communicationSystem, sendInitializeScene(_, _));
    EXPECT_CALL(communicationSystem, sendSceneActionList(remoteParticipantID, SceneId(1), _, 1));

    EXPECT_CALL(consumer, handleInitializeScene(sceneInfo, _));
    EXPECT_CALL(consumer, handleSceneActionList_rvr(SceneId(1), _, 0, _));

    sceneGraphComponent.handleFlush(SceneId(1), ESceneFlushMode_Synchronous, {}, {});

    // disconnect
    EXPECT_CALL(communicationSystem, broadcastScenesBecameUnavailable(SceneInfoVector{ sceneInfo }));
    sceneGraphComponent.disconnectFromNetwork();

    // local flushing unaffected, nothing sent to network
    EXPECT_CALL(consumer, handleSceneActionList_rvr(SceneId(1), _, 0, _));
    sceneGraphComponent.handleFlush(SceneId(1), ESceneFlushMode_Synchronous, {}, {});

    // reconnect remote participant
    EXPECT_CALL(communicationSystem, sendScenesAvailable(remoteParticipantID, SceneInfoVector{ sceneInfo }));
    sceneGraphComponent.newParticipantHasConnected(remoteParticipantID);

    EXPECT_CALL(communicationSystem, sendInitializeScene(_, _));
    EXPECT_CALL(communicationSystem, sendSceneActionList(remoteParticipantID, SceneId(1), _, 1)).WillOnce(Return(1));
    sceneGraphComponent.handleSceneSubscription(SceneId(1), remoteParticipantID);

    // flush again, remote now at flushCounter 2, local always 0
    EXPECT_CALL(communicationSystem, sendSceneActionList(remoteParticipantID, SceneId(1), _, 2)).WillOnce(Return(1));
    EXPECT_CALL(consumer, handleSceneActionList_rvr(SceneId(1), _, 0, _));
    sceneGraphComponent.handleFlush(SceneId(1), ESceneFlushMode_Synchronous, {}, {});

    // cleanup
    EXPECT_CALL(communicationSystem, broadcastScenesBecameUnavailable(SceneInfoVector{ sceneInfo }));
    EXPECT_CALL(consumer, handleScenesBecameUnavailable(SceneInfoVector{ sceneInfo }, _));
    sceneGraphComponent.handleRemoveScene(SceneId(1));
}
