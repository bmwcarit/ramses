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
#include "SceneRendererHandlerMock.h"
#include "TransportCommon/SceneUpdateSerializer.h"
#include "Components/SceneUpdate.h"
#include "SceneUpdateSerializerTestHelper.h"
#include "Resource/ArrayResource.h"
#include "Resource/TextureResource.h"

using namespace ramses_internal;

class ASceneGraphComponent : public ::testing::Test
{
public:
    ASceneGraphComponent()
        : localParticipantID(11)
        , remoteParticipantID(12)
        , sceneGraphComponent(localParticipantID, communicationSystem, connectionStatusUpdateNotifier, resourceComponent, frameworkLock)
    {
        localSceneIdInfo = SceneInfo(localSceneId, "sceneName", EScenePublicationMode_LocalOnly);
        localSceneIdInfoVector.push_back(localSceneIdInfo);
        localAndRemoteSceneIdInfo = SceneInfo(localSceneId, "sceneName", EScenePublicationMode_LocalAndRemote);
        localAndRemoteSceneIdInfoVector.push_back(localAndRemoteSceneIdInfo);
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

    std::vector<std::vector<Byte>> actionsToChunks(const SceneActionCollection& actions, uint32_t chunkSize = 100000, const ManagedResourceVector& resources = {}, const FlushInformation& flushinfo = {})
    {
        SceneUpdate update{actions.copy(), resources, flushinfo.copy()};
        return TestSerializeSceneUpdateToVectorChunked(SceneUpdateSerializer(update), chunkSize);
    }

    void expectSendSceneActionsToNetwork(Guid remote, SceneId sceneId, const SceneActionCollection& expectedActions)
    {
        EXPECT_CALL(communicationSystem, sendSceneUpdate(remote, sceneId, _)).WillOnce([&](auto, auto, auto& serializer) {
            // grab actions directly out of serializer
            const auto actions = static_cast<const SceneUpdateSerializer&>(serializer).getUpdate().actions.copy();
            EXPECT_EQ(expectedActions, actions);
            return true;
        });
    }

    void publishScene(UInt32 sceneId, const String& name, EScenePublicationMode pubMode)
    {
        SceneInfo info(SceneId(sceneId), name, pubMode);
        EXPECT_CALL(consumer, handleNewSceneAvailable(info, _));
        if (pubMode != EScenePublicationMode_LocalOnly)
            EXPECT_CALL(communicationSystem, broadcastNewScenesAvailable(SceneInfoVector{info}));
        sceneGraphComponent.sendPublishScene(SceneId(sceneId), pubMode, name);
    }

protected:
    PlatformLock frameworkLock;
    SceneId localSceneId;
    SceneInfo localSceneIdInfo;
    SceneInfoVector localSceneIdInfoVector;
    SceneInfo localAndRemoteSceneIdInfo;
    SceneInfoVector localAndRemoteSceneIdInfoVector;
    SceneId remoteSceneId;
    Guid localParticipantID;
    Guid remoteParticipantID;
    StrictMock<CommunicationSystemMock> communicationSystem;
    NiceMock<MockConnectionStatusUpdateNotifier> connectionStatusUpdateNotifier;
    NiceMock<ResourceProviderComponentMock> resourceComponent;
    SceneGraphComponent sceneGraphComponent;
    StrictMock<SceneRendererHandlerMock> consumer;
    StrictMock<SceneProviderEventConsumerMock> eventConsumer;
};


TEST_F(ASceneGraphComponent, sendsSceneToLocalConsumer)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);

    EXPECT_CALL(consumer, handleInitializeScene(_, _)).Times(1);
    sceneGraphComponent.sendCreateScene(localParticipantID, SceneId(666u), EScenePublicationMode_LocalAndRemote);
}

TEST_F(ASceneGraphComponent, doesntSendSceneIfLocalConsumerIsntSet)
{
    EXPECT_CALL(consumer, handleInitializeScene(_, localParticipantID)).Times(0);

    sceneGraphComponent.sendCreateScene(localParticipantID, SceneId(666u), EScenePublicationMode_LocalAndRemote);
}

TEST_F(ASceneGraphComponent, sendsSceneToRemoteProvider)
{
    EXPECT_CALL(communicationSystem, sendInitializeScene(remoteParticipantID, SceneId(666u)));
    sceneGraphComponent.sendCreateScene(remoteParticipantID, SceneId(666u), EScenePublicationMode_LocalAndRemote);
}

TEST_F(ASceneGraphComponent, publishesSceneAtLocalConsumerInLocalAndRemoteMode)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);

    EXPECT_CALL(consumer, handleNewSceneAvailable(localAndRemoteSceneIdInfo, _));
    EXPECT_CALL(communicationSystem, broadcastNewScenesAvailable(localAndRemoteSceneIdInfoVector));
    sceneGraphComponent.sendPublishScene(localSceneId, EScenePublicationMode_LocalAndRemote, "sceneName");
}

TEST_F(ASceneGraphComponent, publishesSceneAtLocalConsumerInLocalOnlyMode)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);

    EXPECT_CALL(consumer, handleNewSceneAvailable(localSceneIdInfo, _));
    sceneGraphComponent.sendPublishScene(localSceneId, EScenePublicationMode_LocalOnly, "sceneName");
}

TEST_F(ASceneGraphComponent, doesntPublishSceneIfLocalConsumerIsntSet)
{
    EXPECT_CALL(consumer, handleNewSceneAvailable(localAndRemoteSceneIdInfo, _)).Times(0);
    EXPECT_CALL(communicationSystem, broadcastNewScenesAvailable(localSceneIdInfoVector));
    sceneGraphComponent.sendPublishScene(localSceneId, EScenePublicationMode_LocalAndRemote, "sceneName");
}

TEST_F(ASceneGraphComponent, doesNotPublishSceneToRemoteProviderInLocalOnlyMode)
{
    sceneGraphComponent.sendPublishScene(remoteSceneId, EScenePublicationMode_LocalOnly, "sceneName");
    // no expect needed, StrictMock on communicationSystem checks it already
}

TEST_F(ASceneGraphComponent, publishesSceneAtRemoteProvider)
{
    sceneGraphComponent.newParticipantHasConnected(Guid(33));

    SceneInfoVector newScenes(1, SceneInfo(remoteSceneId, "sceneName"));
    EXPECT_CALL(communicationSystem, broadcastNewScenesAvailable(newScenes));
    sceneGraphComponent.sendPublishScene(remoteSceneId, EScenePublicationMode_LocalAndRemote, "sceneName");
}

TEST_F(ASceneGraphComponent, alreadyPublishedSceneGetsRepublishedWhenLocalConsumerIsSet)
{
    sceneGraphComponent.sendPublishScene(localSceneId, EScenePublicationMode_LocalOnly, "sceneName");

    EXPECT_CALL(consumer, handleNewSceneAvailable(localSceneIdInfo, _));
    sceneGraphComponent.setSceneRendererHandler(&consumer);
}

TEST_F(ASceneGraphComponent, unpublishesSceneAtLocalConsumer)
{
    const SceneId sceneId(999);
    sceneGraphComponent.setSceneRendererHandler(&consumer);

    SceneInfoVector newScenes(1, SceneInfo(sceneId, "sceneName"));
    EXPECT_CALL(consumer, handleNewSceneAvailable(SceneInfo(sceneId, "sceneName", EScenePublicationMode_LocalOnly), _));
    sceneGraphComponent.sendPublishScene(sceneId, EScenePublicationMode_LocalOnly, "sceneName");

    EXPECT_CALL(consumer, handleSceneBecameUnavailable(sceneId, _));
    sceneGraphComponent.sendUnpublishScene(sceneId, EScenePublicationMode_LocalOnly);
}

TEST_F(ASceneGraphComponent, doesntUnpublishSceneIfLocalConsumerIsntSet)
{
    EXPECT_CALL(consumer, handleSceneBecameUnavailable(_, _)).Times(0);

    const SceneId sceneId(1ull << 63);
    sceneGraphComponent.sendPublishScene(sceneId, EScenePublicationMode_LocalOnly, "sceneName");
    sceneGraphComponent.sendUnpublishScene(sceneId, EScenePublicationMode_LocalOnly);
}

TEST_F(ASceneGraphComponent, doesNotUnpublishesSceneAtRemoteProviderIfSceneWasPublishedLocalOnly)
{
    const SceneId sceneId(1ull << 63);
    sceneGraphComponent.sendPublishScene(sceneId, EScenePublicationMode_LocalOnly, "sceneName");
    sceneGraphComponent.sendUnpublishScene(sceneId, EScenePublicationMode_LocalOnly);
    // expect noting on remote, checked by strict mock
}

TEST_F(ASceneGraphComponent, unpublishesSceneAtRemoteProvider)
{
    const SceneId sceneId(1ull << 63);
    sceneGraphComponent.newParticipantHasConnected(Guid(33));
    EXPECT_CALL(communicationSystem, broadcastNewScenesAvailable(_));
    sceneGraphComponent.sendPublishScene(sceneId, EScenePublicationMode_LocalAndRemote, "sceneName");

    SceneInfoVector unavailableScenes(1, SceneInfo(sceneId, "sceneName"));
    EXPECT_CALL(communicationSystem, broadcastScenesBecameUnavailable(unavailableScenes));
    sceneGraphComponent.sendUnpublishScene(sceneId, EScenePublicationMode_LocalAndRemote);
}

TEST_F(ASceneGraphComponent, sendsAvailableScenesToNewParticipant)
{
    SceneInfoVector newScenes(1, SceneInfo(remoteSceneId, "sceneName"));
    EXPECT_CALL(communicationSystem, broadcastNewScenesAvailable(newScenes));
    sceneGraphComponent.sendPublishScene(remoteSceneId, EScenePublicationMode_LocalAndRemote, "sceneName");

    Guid id(33);
    EXPECT_CALL(communicationSystem, sendScenesAvailable(id, newScenes));
    sceneGraphComponent.newParticipantHasConnected(id);
}

TEST_F(ASceneGraphComponent, doesNotSendAvailableSceneToNewParticipantIfLocalOnlyScene)
{
    SceneInfoVector newScenes(1, SceneInfo(remoteSceneId, "sceneName"));
    sceneGraphComponent.sendPublishScene(remoteSceneId, EScenePublicationMode_LocalOnly, "sceneName");

    Guid id(33);
    EXPECT_CALL(communicationSystem, sendScenesAvailable(id, newScenes)).Times(0);
    sceneGraphComponent.newParticipantHasConnected(id);
}

// TODO(Carsten): (un)subscribeScene with local provider currently untested, think about how to test

TEST_F(ASceneGraphComponent, subscribeSceneAtRemoteConsumer)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    SceneId sceneId;
    EXPECT_CALL(communicationSystem, sendSubscribeScene(remoteParticipantID, sceneId));
    sceneGraphComponent.subscribeScene(remoteParticipantID, sceneId);
}

TEST_F(ASceneGraphComponent, unsubscribesSceneAtRemoteConsumer)
{
    const SceneId sceneId(1ull << 63);
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    EXPECT_CALL(communicationSystem, sendSubscribeScene(_, _));
    sceneGraphComponent.subscribeScene(remoteParticipantID, sceneId);
    EXPECT_CALL(communicationSystem, sendUnsubscribeScene(remoteParticipantID, sceneId));
    sceneGraphComponent.unsubscribeScene(remoteParticipantID, sceneId);
}

TEST_F(ASceneGraphComponent, sendsSceneActionToLocalConsumer)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);

    SceneActionCollection list(createFakeSceneActionCollectionFromTypes({ ESceneActionId::TestAction }));
    EXPECT_CALL(consumer, handleSceneUpdate_rvr(SceneId(666u), _, localParticipantID)).WillOnce([&](auto, const auto& update, auto) {
        EXPECT_EQ(list, update.actions);
    });
    SceneUpdate update;
    update.actions = list.copy();
    sceneGraphComponent.sendSceneUpdate({ localParticipantID }, std::move(update), SceneId(666u), EScenePublicationMode_LocalOnly);
}

TEST_F(ASceneGraphComponent, sendsResourcesUnCompressedToLocalConsumer)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);

    ResourceBlob blob(1024 * EnumToSize(EDataType::Float));
    std::iota(blob.data(), blob.data() + blob.size(), static_cast<uint8_t>(10));
    ManagedResourceVector resources{
        std::make_shared<const ArrayResource>(EResourceType_VertexArray, 1024u, EDataType::Float, blob.data(), ResourceCacheFlag_DoNotCache, "fl")
    };
    // not compressed after creating
    EXPECT_FALSE(resources[0]->isCompressedAvailable());

    EXPECT_CALL(consumer, handleSceneUpdate_rvr(SceneId(666u), _, localParticipantID)).WillOnce([&](auto, const auto& update, auto) {
        EXPECT_EQ(resources, update.resources);
        // compressed for sending
        EXPECT_FALSE(update.resources[0]->isCompressedAvailable());
        });
    SceneUpdate update;
    update.resources = resources;
    sceneGraphComponent.sendSceneUpdate({ localParticipantID }, std::move(update), SceneId(666u), EScenePublicationMode_LocalOnly);
}

TEST_F(ASceneGraphComponent, sendsResourcesCompressedToRemoteProvider)
{
    SceneId sceneId;
    EXPECT_CALL(communicationSystem, sendInitializeScene(_, _)).Times(1);
    sceneGraphComponent.sendCreateScene(remoteParticipantID, sceneId, EScenePublicationMode_LocalAndRemote);

    ResourceBlob blob(1024 * EnumToSize(EDataType::Float));
    std::iota(blob.data(), blob.data() + blob.size(), static_cast<uint8_t>(10));
    ManagedResourceVector resourcesToSend{
        std::make_shared<const ArrayResource>(EResourceType_VertexArray, 1024u, EDataType::Float, blob.data(), ResourceCacheFlag_DoNotCache, "fl")
    };

    EXPECT_CALL(communicationSystem, sendSceneUpdate(remoteParticipantID, sceneId, _)).WillOnce([&](auto, auto, auto& serializer) {
        // grab resources directly out of serializer
        const auto resources = static_cast<const SceneUpdateSerializer&>(serializer).getUpdate().resources;
        EXPECT_EQ(resourcesToSend, resources);
        EXPECT_TRUE(resources[0]->isCompressedAvailable());
        return true;
        });
    SceneUpdate update;
    update.resources= resourcesToSend;
    sceneGraphComponent.sendSceneUpdate({ remoteParticipantID }, std::move(update), sceneId, EScenePublicationMode_LocalAndRemote);
}


TEST_F(ASceneGraphComponent, doesntSendSceneActionIfLocalConsumerIsntSet)
{
    SceneActionCollection list(createFakeSceneActionCollectionFromTypes({ ESceneActionId::TestAction }));
    EXPECT_CALL(consumer, handleSceneUpdate_rvr(_, _, _)).Times(0);
    SceneUpdate update;
    update.actions = list.copy();
    sceneGraphComponent.sendSceneUpdate({ localParticipantID }, std::move(update), SceneId(666u), EScenePublicationMode_LocalOnly);
}

TEST_F(ASceneGraphComponent, sendsSceneActionToRemoteProvider)
{
    SceneId sceneId;
    EXPECT_CALL(communicationSystem, sendInitializeScene(_, _)).Times(1);
    sceneGraphComponent.sendCreateScene(remoteParticipantID, sceneId, EScenePublicationMode_LocalAndRemote);

    SceneActionCollection list(createFakeSceneActionCollectionFromTypes({ ESceneActionId::TestAction }));
    expectSendSceneActionsToNetwork(remoteParticipantID, sceneId, list);
    SceneUpdate update;
    update.actions = list.copy();
    sceneGraphComponent.sendSceneUpdate({ remoteParticipantID }, std::move(update), sceneId, EScenePublicationMode_LocalAndRemote);
}

TEST_F(ASceneGraphComponent, sendsAllSceneActionsAtOnceToRemoteProvider)
{
    SceneId sceneId(1);
    EXPECT_CALL(communicationSystem, sendInitializeScene(_, _)).Times(1);
    sceneGraphComponent.sendCreateScene(remoteParticipantID, sceneId, EScenePublicationMode_LocalAndRemote);

    SceneActionCollection list(createFakeSceneActionCollectionFromTypes({ ESceneActionId::TestAction, ESceneActionId::SetDataVector2fArray, ESceneActionId::AllocateNode }));
    expectSendSceneActionsToNetwork(remoteParticipantID, sceneId, list);
    SceneUpdate update;
    update.actions = list.copy();
    sceneGraphComponent.sendSceneUpdate({ remoteParticipantID }, std::move(update), sceneId, EScenePublicationMode_LocalAndRemote);
}

TEST_F(ASceneGraphComponent, doesNotsendSceneUpdateToRemoteIfSceneWasPublishedLocalOnly)
{
    const SceneId sceneId(111);
    sceneGraphComponent.sendPublishScene(sceneId, EScenePublicationMode_LocalOnly, "sceneName");
    SceneActionCollection list(createFakeSceneActionCollectionFromTypes({ ESceneActionId::TestAction }));

    SceneUpdate update;
    update.actions = list.copy();
    sceneGraphComponent.sendSceneUpdate({ localParticipantID }, std::move(update), sceneId, EScenePublicationMode_LocalOnly);

    // expect noting for remote, check by strict mock
}

TEST_F(ASceneGraphComponent, canSendSceneActionsToLocalAndRemote)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);

    const SceneId sceneId(456);
    EXPECT_CALL(communicationSystem, sendInitializeScene(_, _)).Times(1);
    sceneGraphComponent.sendCreateScene(remoteParticipantID, sceneId, EScenePublicationMode_LocalAndRemote);

    SceneActionCollection list(createFakeSceneActionCollectionFromTypes({ ESceneActionId::TestAction }));
    InSequence seq;
    expectSendSceneActionsToNetwork(remoteParticipantID, sceneId, list);
    EXPECT_CALL(consumer, handleSceneUpdate_rvr(sceneId, _, localParticipantID)).WillOnce([&](auto, const auto& update, auto) {
        EXPECT_EQ(list, update.actions);
    });
    SceneUpdate update;
    update.actions = list.copy();
    sceneGraphComponent.sendSceneUpdate({ remoteParticipantID, localParticipantID }, std::move(update), sceneId, EScenePublicationMode_LocalAndRemote);
}

TEST_F(ASceneGraphComponent, canRepublishALocalOnlySceneToBeDistributedRemotely)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(33));

    // publish LocalOnly
    EXPECT_CALL(consumer, handleNewSceneAvailable(localSceneIdInfo, _));
    sceneGraphComponent.sendPublishScene(localSceneId, EScenePublicationMode_LocalOnly, "sceneName");
    Mock::VerifyAndClearExpectations(&communicationSystem);

    // unpublish
    EXPECT_CALL(consumer, handleSceneBecameUnavailable(_, _));
    sceneGraphComponent.sendUnpublishScene(localSceneId, EScenePublicationMode_LocalOnly);
    Mock::VerifyAndClearExpectations(&communicationSystem);
    Mock::VerifyAndClearExpectations(&consumer);

    // publish LocalAndRemote
    SceneInfoVector newScenes(1, SceneInfo(localSceneId, "sceneName"));
    EXPECT_CALL(consumer, handleNewSceneAvailable(localAndRemoteSceneIdInfo, localParticipantID));
    EXPECT_CALL(communicationSystem, broadcastNewScenesAvailable(newScenes));
    sceneGraphComponent.sendPublishScene(localSceneId, EScenePublicationMode_LocalAndRemote, "sceneName");
}

TEST_F(ASceneGraphComponent, canRepublishARemoteSceneToBeLocalOnly)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(33));

    // publish LocalAndRemote
    EXPECT_CALL(consumer, handleNewSceneAvailable(localAndRemoteSceneIdInfo, localParticipantID));
    EXPECT_CALL(communicationSystem, broadcastNewScenesAvailable(localAndRemoteSceneIdInfoVector));
    sceneGraphComponent.sendPublishScene(localSceneId, EScenePublicationMode_LocalAndRemote, "sceneName");
    Mock::VerifyAndClearExpectations(&communicationSystem);
    Mock::VerifyAndClearExpectations(&consumer);

    // unpublish
    EXPECT_CALL(consumer, handleSceneBecameUnavailable(localSceneId, localParticipantID));
    EXPECT_CALL(communicationSystem, broadcastScenesBecameUnavailable(localAndRemoteSceneIdInfoVector));
    sceneGraphComponent.sendUnpublishScene(localSceneId, EScenePublicationMode_LocalAndRemote);
    Mock::VerifyAndClearExpectations(&communicationSystem);
    Mock::VerifyAndClearExpectations(&consumer);

    // publish LocalOnly
    EXPECT_CALL(consumer, handleNewSceneAvailable(_, _));
    sceneGraphComponent.sendPublishScene(localSceneId, EScenePublicationMode_LocalOnly, "sceneName");
    Mock::VerifyAndClearExpectations(&communicationSystem);
}

TEST_F(ASceneGraphComponent, disconnectFromNetworkBroadcastsScenesUnavailableOnNetworkOnly)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    publishScene(1, "name", EScenePublicationMode_LocalAndRemote);
    EXPECT_CALL(communicationSystem, broadcastScenesBecameUnavailable(SceneInfoVector{ SceneInfo(SceneId(1), "name") }));
    sceneGraphComponent.disconnectFromNetwork();
}

TEST_F(ASceneGraphComponent, disconnectFromNetworkBroadcastsScenesUnavailableOnNetworkForAllRemoteScenes)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
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
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    publishScene(1, "name", EScenePublicationMode_LocalAndRemote);
    EXPECT_CALL(communicationSystem, broadcastScenesBecameUnavailable(SceneInfoVector{ SceneInfo(SceneId(1), "name") }));
    sceneGraphComponent.disconnectFromNetwork();

    EXPECT_CALL(communicationSystem, sendScenesAvailable(remoteParticipantID, SceneInfoVector{ SceneInfo(SceneId(1), "name") }));
    sceneGraphComponent.newParticipantHasConnected(remoteParticipantID);
}

TEST_F(ASceneGraphComponent, disconnectDoesNotAffectLocalScenesAtAllButUnpublishesRemoteScenes)
{
    SceneInfo sceneInfo(SceneInfo(SceneId(1), "foo"));
    ClientScene scene(sceneInfo);

    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.handleCreateScene(scene, false, eventConsumer);

    // subscribe local and remote
    EXPECT_CALL(consumer, handleNewSceneAvailable(sceneInfo, _));
    EXPECT_CALL(communicationSystem, broadcastNewScenesAvailable(SceneInfoVector{ sceneInfo }));
    sceneGraphComponent.handlePublishScene(SceneId(1), EScenePublicationMode_LocalAndRemote);

    EXPECT_CALL(communicationSystem, sendScenesAvailable(remoteParticipantID, SceneInfoVector{ sceneInfo }));
    sceneGraphComponent.newParticipantHasConnected(remoteParticipantID);
    sceneGraphComponent.handleSubscribeScene(SceneId(1), remoteParticipantID);

    sceneGraphComponent.handleSubscribeScene(SceneId(1), localParticipantID);

    EXPECT_CALL(communicationSystem, sendInitializeScene(_, _));
    EXPECT_CALL(communicationSystem, sendSceneUpdate(remoteParticipantID, SceneId(1), _));

    EXPECT_CALL(consumer, handleInitializeScene(sceneInfo, _));
    EXPECT_CALL(consumer, handleSceneUpdate_rvr(SceneId(1), _,  _));

    const FlushTimeInformation flushTimesWithExpirationToPreventFlushOptimizazion {FlushTime::Clock::time_point {std::chrono::milliseconds{1}}, FlushTime::Clock::time_point {}, FlushTime::Clock::getClockType() };
    EXPECT_TRUE(sceneGraphComponent.handleFlush(SceneId(1), flushTimesWithExpirationToPreventFlushOptimizazion, {}));

    // disconnect
    EXPECT_CALL(communicationSystem, broadcastScenesBecameUnavailable(SceneInfoVector{ sceneInfo }));
    sceneGraphComponent.disconnectFromNetwork();

    // local flushing unaffected, nothing sent to network
    EXPECT_CALL(consumer, handleSceneUpdate_rvr(SceneId(1), _, _));
    EXPECT_TRUE(sceneGraphComponent.handleFlush(SceneId(1), flushTimesWithExpirationToPreventFlushOptimizazion, {}));

    // reconnect remote participant
    EXPECT_CALL(communicationSystem, sendScenesAvailable(remoteParticipantID, SceneInfoVector{ sceneInfo }));
    sceneGraphComponent.newParticipantHasConnected(remoteParticipantID);

    EXPECT_CALL(communicationSystem, sendInitializeScene(_, _));
    EXPECT_CALL(communicationSystem, sendSceneUpdate(remoteParticipantID, SceneId(1), _)).WillOnce(Return(1));
    sceneGraphComponent.handleSubscribeScene(SceneId(1), remoteParticipantID);

    // flush again
    EXPECT_CALL(communicationSystem, sendSceneUpdate(remoteParticipantID, SceneId(1), _)).WillOnce(Return(1));
    EXPECT_CALL(consumer, handleSceneUpdate_rvr(SceneId(1), _, _));
    EXPECT_TRUE(sceneGraphComponent.handleFlush(SceneId(1), flushTimesWithExpirationToPreventFlushOptimizazion, {}));

    // cleanup
    EXPECT_CALL(communicationSystem, broadcastScenesBecameUnavailable(SceneInfoVector{ sceneInfo }));
    EXPECT_CALL(consumer, handleSceneBecameUnavailable(sceneInfo.sceneID, _));
    sceneGraphComponent.handleRemoveScene(SceneId(1));
}


    TEST_F(ASceneGraphComponent, forwardsResourceAvailabilityEventsToCorrectHandler)
    {
        StrictMock<SceneProviderEventConsumerMock> scene2Consumer;

        ResourceAvailabilityEvent event1;
        event1.sceneid = SceneId{ 123 };
        ResourceAvailabilityEvent event2;
        event2.sceneid = SceneId{ 10000 };
        SceneInfo sceneInfo1(SceneInfo(SceneId{ 123 }, "foo"));
        ClientScene scene1(sceneInfo1);
        sceneGraphComponent.handleCreateScene(scene1, false, eventConsumer);
        SceneInfo sceneInfo2(SceneInfo(SceneId{ 10000 }, "bar"));
        ClientScene scene2(sceneInfo2);
        sceneGraphComponent.handleCreateScene(scene2, false, scene2Consumer);

        InSequence seq;
        EXPECT_CALL(eventConsumer, handleResourceAvailabilityEvent(_, _)).Times(1);
        sceneGraphComponent.sendResourceAvailabilityEvent(localParticipantID, event1);
        EXPECT_CALL(scene2Consumer, handleResourceAvailabilityEvent(_, _)).Times(1);
        sceneGraphComponent.sendResourceAvailabilityEvent(localParticipantID, event2);

        EXPECT_CALL(eventConsumer, handleResourceAvailabilityEvent(_, _)).Times(1);
        std::vector<Byte> data1;
        event1.writeToBlob(data1);
        sceneGraphComponent.handleRendererEvent(event1.sceneid, data1, remoteParticipantID);

        EXPECT_CALL(scene2Consumer, handleResourceAvailabilityEvent(_, _)).Times(1);
        std::vector<Byte> data2;
        event2.writeToBlob(data2);
        sceneGraphComponent.handleRendererEvent(event2.sceneid, data2, remoteParticipantID);
    }
TEST_F(ASceneGraphComponent, sendSceneReferenceEventViaCommSystemForRemoteParticipant)
{
    SceneReferenceEvent event(SceneId{ 123 });
    event.type = SceneReferenceEventType::SceneFlushed;
    event.referencedScene = SceneId{ 456 };
    event.tag = SceneVersionTag{ 1000 };
    EXPECT_CALL(communicationSystem, sendRendererEvent(remoteParticipantID, SceneId{ 123 }, _)).WillOnce([&event](const Guid&, const SceneId& sceneId , const std::vector<Byte>& data) -> bool
    {
        SceneReferenceEvent e{ sceneId };
        e.readFromBlob(data);
        EXPECT_EQ(e.masterSceneId, event.masterSceneId);
        EXPECT_EQ(e.type, event.type);
        EXPECT_EQ(e.referencedScene, event.referencedScene);
        EXPECT_EQ(e.tag, event.tag);

        return true;
    });
    sceneGraphComponent.sendSceneReferenceEvent(remoteParticipantID, event);
}

TEST_F(ASceneGraphComponent, sendResourceAvailabilityEventViaCommSystemForRemoteParticipant)
{
    ResourceAvailabilityEvent event;
    event.sceneid = SceneId { 123 };
    EXPECT_CALL(communicationSystem, sendRendererEvent(remoteParticipantID, SceneId{ 123 },_)).WillOnce([&event](const Guid&, const SceneId& sceneId, const std::vector<Byte>& data) -> bool
    {
        ResourceAvailabilityEvent e;
        e.readFromBlob(data);
        EXPECT_EQ(e.sceneid, event.sceneid);
        EXPECT_EQ(e.sceneid, sceneId);

        return true;
    });
    sceneGraphComponent.sendResourceAvailabilityEvent(remoteParticipantID, event);
}

TEST_F(ASceneGraphComponent, sendSceneReferenceEventForLocalParticipantCallsHandlerDirectly)
{
    SceneReferenceEvent event(SceneId{ 123 });
    SceneInfo sceneInfo(SceneInfo(SceneId{ 123 }, "foo"));
    ClientScene scene(sceneInfo);
    sceneGraphComponent.handleCreateScene(scene, false, eventConsumer);
    event.type = SceneReferenceEventType::SceneFlushed;
    event.referencedScene = SceneId{ 456 };
    event.tag = SceneVersionTag{ 1000 };
    EXPECT_CALL(eventConsumer, handleSceneReferenceEvent(_, localParticipantID)).WillOnce([&event](SceneReferenceEvent const& e, const Guid&)
    {
        EXPECT_EQ(e.masterSceneId, event.masterSceneId);
        EXPECT_EQ(e.type, event.type);
        EXPECT_EQ(e.referencedScene, event.referencedScene);
        EXPECT_EQ(e.tag, event.tag);
    });
    sceneGraphComponent.sendSceneReferenceEvent(localParticipantID, event);
}

TEST_F(ASceneGraphComponent, sendResourceAvailabilityEventForLocalParticipantCallsHandlerDirectly)
{
    SceneReferenceEvent event(SceneId { 123 });
    SceneInfo sceneInfo(SceneInfo(SceneId{ 123 }, "foo"));
    ClientScene scene(sceneInfo);
    sceneGraphComponent.handleCreateScene(scene, false, eventConsumer);
    event.type = SceneReferenceEventType::SceneFlushed;
    event.referencedScene = SceneId{ 456 };
    event.tag = SceneVersionTag{ 1000 };
    EXPECT_CALL(eventConsumer, handleSceneReferenceEvent(_, localParticipantID)).WillOnce([&event](SceneReferenceEvent const& e, const Guid&)
    {
        EXPECT_EQ(e.masterSceneId, event.masterSceneId);
        EXPECT_EQ(e.type, event.type);
        EXPECT_EQ(e.referencedScene, event.referencedScene);
        EXPECT_EQ(e.tag, event.tag);
    });
    sceneGraphComponent.sendSceneReferenceEvent(localParticipantID, event);
}

TEST_F(ASceneGraphComponent, doesNotSendSceneReferenceEventForLocalParticipantCallsHandlerDirectlyIfSceneUnknown)
{
    SceneReferenceEvent event(SceneId{ 123 });
    sceneGraphComponent.sendSceneReferenceEvent(localParticipantID, event);
}

TEST_F(ASceneGraphComponent, doesNotSendResourceAvailabilityEventForLocalParticipantCallsHandlerDirectlyIfSceneUnknown)
{
    SceneReferenceEvent event(SceneId { 123 });
    sceneGraphComponent.sendSceneReferenceEvent(localParticipantID, event);
}

TEST_F(ASceneGraphComponent, unpacksRendererEventToSceneReferenceEventAndForwardsToHandler)
{
    SceneReferenceEvent event(SceneId{ 123 });
    SceneInfo sceneInfo(SceneInfo(SceneId{ 123 }, "foo"));
    ClientScene scene(sceneInfo);
    sceneGraphComponent.handleCreateScene(scene, false, eventConsumer);
    event.type = SceneReferenceEventType::SceneFlushed;
    event.referencedScene = SceneId{ 456 };
    event.tag = SceneVersionTag{ 1000 };
    EXPECT_CALL(eventConsumer, handleSceneReferenceEvent(_, localParticipantID)).WillOnce([&event](SceneReferenceEvent const& e, const Guid&)
    {
        EXPECT_EQ(e.masterSceneId, event.masterSceneId);
        EXPECT_EQ(e.type, event.type);
        EXPECT_EQ(e.referencedScene, event.referencedScene);
        EXPECT_EQ(e.tag, event.tag);
    });
    std::vector<Byte> data;
    event.writeToBlob(data);
    sceneGraphComponent.handleRendererEvent(event.masterSceneId, data, remoteParticipantID);
}

TEST_F(ASceneGraphComponent, unpacksRendererEventToResourceAvailabilityEventAndForwardsToHandler)
{
    ResourceAvailabilityEvent event;
    event.sceneid = SceneId { 123 };
    SceneInfo sceneInfo(SceneInfo(SceneId{ 123 }, "foo"));
    ClientScene scene(sceneInfo);
    sceneGraphComponent.handleCreateScene(scene, false, eventConsumer);
    event.availableResources.push_back(ResourceContentHash(1,2));
    EXPECT_CALL(eventConsumer, handleResourceAvailabilityEvent(_, localParticipantID)).WillOnce([&event](ResourceAvailabilityEvent const& e, const Guid&)
    {
        EXPECT_EQ(e.sceneid, event.sceneid);
        EXPECT_EQ(e.availableResources, event.availableResources);
    });
    std::vector<Byte> data;
    event.writeToBlob(data);
    sceneGraphComponent.handleRendererEvent(event.sceneid, data, remoteParticipantID);
}

TEST_F(ASceneGraphComponent, doesNotForwardRendererEventIfSceneUnknown)
{
    SceneReferenceEvent event(SceneId{ 123 });
    std::vector<Byte> data;
    event.writeToBlob(data);
    sceneGraphComponent.handleRendererEvent(event.masterSceneId, data, remoteParticipantID);
}

TEST_F(ASceneGraphComponent, doesNotForwardResourceAvailabilityRendererEventIfSceneUnknown)
{
    ResourceAvailabilityEvent event;
    event.sceneid = SceneId { 123 };
    std::vector<Byte> data;
    event.writeToBlob(data);
    sceneGraphComponent.handleRendererEvent(event.sceneid, data, remoteParticipantID);
}

TEST_F(ASceneGraphComponent, forwardsEventsToCorrectHandler)
{
    StrictMock<SceneProviderEventConsumerMock> scene2Consumer;

    SceneReferenceEvent event1(SceneId{ 123 });
    SceneReferenceEvent event2(SceneId{ 10000 });
    SceneInfo sceneInfo1(SceneInfo(SceneId{ 123 }, "foo"));
    ClientScene scene1(sceneInfo1);
    sceneGraphComponent.handleCreateScene(scene1, false, eventConsumer);
    SceneInfo sceneInfo2(SceneInfo(SceneId{ 10000 }, "bar"));
    ClientScene scene2(sceneInfo2);
    sceneGraphComponent.handleCreateScene(scene2, false, scene2Consumer);

    InSequence seq;
    EXPECT_CALL(eventConsumer, handleSceneReferenceEvent(_, _)).Times(1);
    sceneGraphComponent.sendSceneReferenceEvent(localParticipantID, event1);
    EXPECT_CALL(scene2Consumer, handleSceneReferenceEvent(_, _)).Times(1);
    sceneGraphComponent.sendSceneReferenceEvent(localParticipantID, event2);

    EXPECT_CALL(eventConsumer, handleSceneReferenceEvent(_, _)).Times(1);
    std::vector<Byte> data1;
    event1.writeToBlob(data1);
    sceneGraphComponent.handleRendererEvent(event1.masterSceneId, data1, remoteParticipantID);

    EXPECT_CALL(scene2Consumer, handleSceneReferenceEvent(_, _)).Times(1);
    std::vector<Byte> data2;
    event1.writeToBlob(data2);
    sceneGraphComponent.handleRendererEvent(event2.masterSceneId, data2, remoteParticipantID);
}

TEST(AERendererToClientEventType, canBePrinted)
{
    EXPECT_EQ("ERendererToClientEventType::SceneReferencingEvent", fmt::to_string(ERendererToClientEventType::SceneReferencingEvent));
    EXPECT_EQ("ERendererToClientEventType::ResourcesAvailableAtRendererEvent", fmt::to_string(ERendererToClientEventType::ResourcesAvailableAtRendererEvent));
}

TEST_F(ASceneGraphComponent, forwardsPublishFromRemoteToConsumer)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(33));
    sceneGraphComponent.newParticipantHasConnected(Guid(22));

    SceneInfo info_33(SceneId(1), "", EScenePublicationMode_LocalAndRemote);
    SceneInfo info_22(SceneId(2), "", EScenePublicationMode_LocalAndRemote);

    EXPECT_CALL(consumer, handleNewSceneAvailable(info_33, Guid(33)));
    sceneGraphComponent.handleNewScenesAvailable({info_33}, Guid(33));

    EXPECT_CALL(consumer, handleNewSceneAvailable(info_22, Guid(22)));
    sceneGraphComponent.handleNewScenesAvailable({info_22}, Guid(22));
}

TEST_F(ASceneGraphComponent, forwardsUnpublishFromRemoteToConsumer)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(33));
    sceneGraphComponent.newParticipantHasConnected(Guid(22));

    SceneInfo info_33(SceneId(1), "", EScenePublicationMode_LocalAndRemote);
    SceneInfo info_22(SceneId(2), "", EScenePublicationMode_LocalAndRemote);

    EXPECT_CALL(consumer, handleNewSceneAvailable(info_33, Guid(33)));
    sceneGraphComponent.handleNewScenesAvailable({info_33}, Guid(33));

    EXPECT_CALL(consumer, handleNewSceneAvailable(info_22, Guid(22)));
    sceneGraphComponent.handleNewScenesAvailable({info_22}, Guid(22));

    EXPECT_CALL(consumer, handleSceneBecameUnavailable(info_22.sceneID, Guid(22)));
    sceneGraphComponent.handleScenesBecameUnavailable({info_22}, Guid(22));
}

TEST_F(ASceneGraphComponent, unpublishesRemoteScenesToConsumerOnParticipantDisconnect)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(33));
    sceneGraphComponent.newParticipantHasConnected(Guid(22));

    SceneInfo info_33(SceneId(1), "", EScenePublicationMode_LocalAndRemote);
    SceneInfo info_22(SceneId(2), "", EScenePublicationMode_LocalAndRemote);

    EXPECT_CALL(consumer, handleNewSceneAvailable(info_33, _));
    sceneGraphComponent.handleNewScenesAvailable({info_33}, Guid(33));

    EXPECT_CALL(consumer, handleNewSceneAvailable(info_22, _));
    sceneGraphComponent.handleNewScenesAvailable({info_22}, Guid(22));

    EXPECT_CALL(consumer, handleSceneBecameUnavailable(info_22.sceneID, Guid(22)));
    sceneGraphComponent.participantHasDisconnected(Guid(22));
}

TEST_F(ASceneGraphComponent, ignoresDuplicatePublishFromOtherRemote)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(22));
    sceneGraphComponent.newParticipantHasConnected(Guid(33));

    SceneInfo info_22(SceneId(2), "", EScenePublicationMode_LocalAndRemote);

    EXPECT_CALL(consumer, handleNewSceneAvailable(info_22, Guid(22)));
    sceneGraphComponent.handleNewScenesAvailable({info_22}, Guid(22));
    sceneGraphComponent.handleNewScenesAvailable({info_22}, Guid(33));
}

TEST_F(ASceneGraphComponent, unpublishesFirstOnDuplicatePublishFromSameRemote)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(22));

    SceneInfo info_22(SceneId(2), "", EScenePublicationMode_LocalAndRemote);

    EXPECT_CALL(consumer, handleNewSceneAvailable(info_22, Guid(22)));
    sceneGraphComponent.handleNewScenesAvailable({info_22}, Guid(22));

    EXPECT_CALL(consumer, handleSceneBecameUnavailable(info_22.sceneID, Guid(22)));
    EXPECT_CALL(consumer, handleNewSceneAvailable(info_22, Guid(22)));
    sceneGraphComponent.handleNewScenesAvailable({info_22}, Guid(22));
}

TEST_F(ASceneGraphComponent, ignoresDuplcateUnpublishFromRemote)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(22));

    SceneInfo info_22(SceneId(2), "", EScenePublicationMode_LocalAndRemote);
    EXPECT_CALL(consumer, handleNewSceneAvailable(info_22, Guid(22)));
    sceneGraphComponent.handleNewScenesAvailable({info_22}, Guid(22));

    EXPECT_CALL(consumer, handleSceneBecameUnavailable(info_22.sceneID, Guid(22)));
    sceneGraphComponent.handleScenesBecameUnavailable({info_22}, Guid(22));
    sceneGraphComponent.handleScenesBecameUnavailable({info_22}, Guid(22));
}

TEST_F(ASceneGraphComponent, forwardsInitializeSceneToConsumer)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(22));

    SceneInfo info_22(SceneId(2), "", EScenePublicationMode_LocalAndRemote);
    EXPECT_CALL(consumer, handleNewSceneAvailable(info_22, Guid(22)));
    sceneGraphComponent.handleNewScenesAvailable({info_22}, Guid(22));

    EXPECT_CALL(consumer, handleInitializeScene(info_22, Guid(22)));
    sceneGraphComponent.handleInitializeScene(SceneId(2), Guid(22));
}

TEST_F(ASceneGraphComponent, ignoresInitializeSceneForUnknownScene)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(22));

    sceneGraphComponent.handleInitializeScene(SceneId(2), Guid(22));
}

TEST_F(ASceneGraphComponent, ignoresInitializeSceneFromWrongProvider)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(22));
    sceneGraphComponent.newParticipantHasConnected(Guid(33));

    SceneInfo info_22(SceneId(2), "", EScenePublicationMode_LocalAndRemote);
    EXPECT_CALL(consumer, handleNewSceneAvailable(info_22, Guid(22)));
    sceneGraphComponent.handleNewScenesAvailable({info_22}, Guid(22));

    sceneGraphComponent.handleInitializeScene(SceneId(2), Guid(33));
}

TEST_F(ASceneGraphComponent, forwardsSceneActionListFromRemote)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(22));

    SceneInfo info_22(SceneId(2), "", EScenePublicationMode_LocalAndRemote);
    EXPECT_CALL(consumer, handleNewSceneAvailable(info_22, Guid(22)));
    sceneGraphComponent.handleNewScenesAvailable({info_22}, Guid(22));

    EXPECT_CALL(consumer, handleInitializeScene(info_22, Guid(22)));
    sceneGraphComponent.handleInitializeScene(SceneId(2), Guid(22));

    SceneActionCollection actions_1(createFakeSceneActionCollectionFromTypes({ ESceneActionId::TestAction }));
    EXPECT_CALL(consumer, handleSceneUpdate_rvr(SceneId(2), _, Guid(22))).WillOnce([&](const auto&, const auto& update, const auto&) {
        EXPECT_EQ(actions_1, update.actions);
    });
    const auto actionBlobs_1 = actionsToChunks(actions_1);
    sceneGraphComponent.handleSceneUpdate(SceneId(2), actionBlobs_1[0], Guid(22));

    SceneActionCollection actions_2(createFakeSceneActionCollectionFromTypes({ ESceneActionId::CompoundRenderable }));
    EXPECT_CALL(consumer, handleSceneUpdate_rvr(SceneId(2), _, Guid(22))).WillOnce([&](const auto&, const auto& update, const auto&) {
        EXPECT_EQ(actions_2, update.actions);
    });
    const auto actionBlobs_2 = actionsToChunks(actions_2);
    sceneGraphComponent.handleSceneUpdate(SceneId(2), actionBlobs_2[0], Guid(22));
}

TEST_F(ASceneGraphComponent, ignoreSceneActionsFromRemoteForUnpublishedScene)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(22));

    SceneActionCollection actions_1(createFakeSceneActionCollectionFromTypes({ ESceneActionId::AllocateNode }));
    sceneGraphComponent.handleSceneUpdate(SceneId(2), actionsToChunks(actions_1)[0], Guid(22));
}

TEST_F(ASceneGraphComponent, ignoreSceneActionsFromRemoteWithoutInitializeScene)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(22));

    SceneInfo info_22(SceneId(2), "", EScenePublicationMode_LocalAndRemote);
    EXPECT_CALL(consumer, handleNewSceneAvailable(info_22, Guid(22)));
    sceneGraphComponent.handleNewScenesAvailable({info_22}, Guid(22));

    SceneActionCollection actions_1(createFakeSceneActionCollectionFromTypes({ ESceneActionId::AllocateNode }));
    sceneGraphComponent.handleSceneUpdate(SceneId(2), actionsToChunks(actions_1)[0], Guid(22));
}

TEST_F(ASceneGraphComponent, ignoreSceneActionsFromRemoteFromWrongProvider)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(22));
    sceneGraphComponent.newParticipantHasConnected(Guid(33));

    SceneInfo info_22(SceneId(2), "", EScenePublicationMode_LocalAndRemote);
    EXPECT_CALL(consumer, handleNewSceneAvailable(info_22, Guid(22)));
    sceneGraphComponent.handleNewScenesAvailable({info_22}, Guid(22));

    EXPECT_CALL(consumer, handleInitializeScene(info_22, Guid(22)));
    sceneGraphComponent.handleInitializeScene(SceneId(2), Guid(22));

    SceneActionCollection actions_1(createFakeSceneActionCollectionFromTypes({ ESceneActionId::AllocateNode }));
    sceneGraphComponent.handleSceneUpdate(SceneId(2), actionsToChunks(actions_1)[0], Guid(33));
}

TEST_F(ASceneGraphComponent, ignoreSceneActionsFromRemoteWithEmptyData)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(22));

    SceneInfo info_22(SceneId(2), "", EScenePublicationMode_LocalAndRemote);
    EXPECT_CALL(consumer, handleNewSceneAvailable(info_22, Guid(22)));
    sceneGraphComponent.handleNewScenesAvailable({info_22}, Guid(22));

    EXPECT_CALL(consumer, handleInitializeScene(info_22, Guid(22)));
    sceneGraphComponent.handleInitializeScene(SceneId(2), Guid(22));

    sceneGraphComponent.handleSceneUpdate(SceneId(2), {}, Guid(22));
}

TEST_F(ASceneGraphComponent, stopsDeserilizationFromRemotAfterInvalidPacket)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(22));

    SceneInfo info_22(SceneId(2), "", EScenePublicationMode_LocalAndRemote);
    EXPECT_CALL(consumer, handleNewSceneAvailable(info_22, Guid(22)));
    sceneGraphComponent.handleNewScenesAvailable({info_22}, Guid(22));

    EXPECT_CALL(consumer, handleInitializeScene(info_22, Guid(22)));
    sceneGraphComponent.handleInitializeScene(SceneId(2), Guid(22));

    SceneActionCollection actions_1(createFakeSceneActionCollectionFromTypes({ ESceneActionId::TestAction }));
    EXPECT_CALL(consumer, handleSceneUpdate_rvr(SceneId(2), _, Guid(22))).WillOnce([&](const auto&, const auto& update, const auto&) {
        EXPECT_EQ(actions_1, update.actions);
    });
    const auto actionBlobs_1 = actionsToChunks(actions_1);
    sceneGraphComponent.handleSceneUpdate(SceneId(2), actionBlobs_1[0], Guid(22));

    // break it
    sceneGraphComponent.handleSceneUpdate(SceneId(2), {0}, Guid(22));

    // expect nothing
    SceneActionCollection actions_2(createFakeSceneActionCollectionFromTypes({ ESceneActionId::AllocateNode }));
    sceneGraphComponent.handleSceneUpdate(SceneId(2), actionsToChunks(actions_2)[0], Guid(22));
}

TEST_F(ASceneGraphComponent, brokenActionDeserilizeFromRemoteIsClearOnNextInitialize)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(22));

    SceneInfo info_22(SceneId(2), "", EScenePublicationMode_LocalAndRemote);
    EXPECT_CALL(consumer, handleNewSceneAvailable(info_22, Guid(22)));
    sceneGraphComponent.handleNewScenesAvailable({info_22}, Guid(22));

    EXPECT_CALL(consumer, handleInitializeScene(info_22, Guid(22)));
    sceneGraphComponent.handleInitializeScene(SceneId(2), Guid(22));

    // break it
    sceneGraphComponent.handleSceneUpdate(SceneId(2), {0}, Guid(22));

    EXPECT_CALL(consumer, handleInitializeScene(info_22, Guid(22)));
    sceneGraphComponent.handleInitializeScene(SceneId(2), Guid(22));

    SceneActionCollection actions_1(createFakeSceneActionCollectionFromTypes({ ESceneActionId::TestAction }));
    EXPECT_CALL(consumer, handleSceneUpdate_rvr(SceneId(2), _, Guid(22))).WillOnce([&](const auto&, const auto& update, const auto&) {
        EXPECT_EQ(actions_1, update.actions);
    });
    const auto actionBlobs_1 = actionsToChunks(actions_1);
    sceneGraphComponent.handleSceneUpdate(SceneId(2), actionBlobs_1[0], Guid(22));
}

TEST_F(ASceneGraphComponent, canHandleSceneActionFromRemoteSplitInMultipleChunks)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(22));

    SceneInfo info_22(SceneId(2), "", EScenePublicationMode_LocalAndRemote);
    EXPECT_CALL(consumer, handleNewSceneAvailable(info_22, Guid(22)));
    sceneGraphComponent.handleNewScenesAvailable({info_22}, Guid(22));

    EXPECT_CALL(consumer, handleInitializeScene(info_22, Guid(22)));
    sceneGraphComponent.handleInitializeScene(SceneId(2), Guid(22));

    SceneActionCollection actions;
    actions.getRawDataForDirectWriting().resize(100);
    actions.beginWriteSceneAction(ESceneActionId::AllocateNode);

    const auto actionBlobs = actionsToChunks(actions, 80);
    EXPECT_EQ(2u, actionBlobs.size());

    EXPECT_CALL(consumer, handleSceneUpdate_rvr(SceneId(2), _, Guid(22))).WillOnce([&](const auto&, const auto& update, const auto&) {
        EXPECT_EQ(actions, update.actions);
    });
    for (const auto& b : actionBlobs)
        sceneGraphComponent.handleSceneUpdate(SceneId(2), b, Guid(22));
}

TEST_F(ASceneGraphComponent, deserializesAndForwardsFlushInformationFromRemote)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(22));

    SceneInfo info_22(SceneId(2), "", EScenePublicationMode_LocalAndRemote);
    EXPECT_CALL(consumer, handleNewSceneAvailable(info_22, Guid(22)));
    sceneGraphComponent.handleNewScenesAvailable({ info_22 }, Guid(22));

    EXPECT_CALL(consumer, handleInitializeScene(info_22, Guid(22)));
    sceneGraphComponent.handleInitializeScene(SceneId(2), Guid(22));

    FlushInformation fi;
    fi.containsValidInformation = true;
    fi.flushCounter = 666;
    fi.flushTimeInfo.clock_type = synchronized_clock_type::PTP;
    fi.flushTimeInfo.expirationTimestamp = FlushTime::Clock::time_point(std::chrono::milliseconds(12345));
    fi.flushTimeInfo.internalTimestamp = FlushTime::Clock::time_point(std::chrono::milliseconds(54321));
    fi.hasSizeInfo = true;
    fi.resourceChanges.m_resourcesAdded = { {11, 11}, {22, 22} };
    fi.resourceChanges.m_resourcesRemoved = { {33, 33}, {44, 44} };
    fi.resourceChanges.m_sceneResourceActions = { {} };
    fi.sceneReferences = { {} };
    fi.sizeInfo.nodeCount = 555;
    fi.versionTag = SceneVersionTag{ 9876 };

    SceneActionCollection actions(createFakeSceneActionCollectionFromTypes({ ESceneActionId::TestAction }));
    EXPECT_CALL(consumer, handleSceneUpdate_rvr(SceneId(2), _, Guid(22))).WillOnce([&](const auto&, const auto& update, const auto&) {
        EXPECT_EQ(actions, update.actions);
        EXPECT_EQ(update.flushInfos.containsValidInformation, true);
        EXPECT_EQ(update.flushInfos.flushCounter, 666);
        EXPECT_EQ(update.flushInfos.flushTimeInfo.clock_type, synchronized_clock_type::PTP);
        EXPECT_EQ(update.flushInfos.flushTimeInfo.expirationTimestamp, FlushTime::Clock::time_point(std::chrono::milliseconds(12345)));
        EXPECT_EQ(update.flushInfos.flushTimeInfo.internalTimestamp, FlushTime::Clock::time_point(std::chrono::milliseconds(54321)));
        EXPECT_EQ(update.flushInfos.hasSizeInfo, true);
        ResourceContentHashVector resvec1{ {11, 11}, {22, 22} };
        ResourceContentHashVector resvec2{ {33, 33}, {44, 44} };
        SceneResourceActionVector savec{ SceneResourceAction() };
        SceneReferenceActionVector sravec{ {} };
        EXPECT_EQ(update.flushInfos.resourceChanges.m_resourcesAdded, resvec1);
        EXPECT_EQ(update.flushInfos.resourceChanges.m_resourcesRemoved, resvec2);
        EXPECT_EQ(update.flushInfos.resourceChanges.m_sceneResourceActions, savec);
        EXPECT_EQ(update.flushInfos.sceneReferences, sravec);
        EXPECT_EQ(update.flushInfos.sizeInfo.nodeCount, 555u);
        EXPECT_EQ(update.flushInfos.versionTag, SceneVersionTag{ 9876 });
        });
    const auto actionBlobs = actionsToChunks(actions, 100000, {}, fi);
    sceneGraphComponent.handleSceneUpdate(SceneId(2), actionBlobs[0], Guid(22));
}

TEST_F(ASceneGraphComponent, deserializesResourcesFromRemote)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(22));

    SceneInfo info_22(SceneId(2), "", EScenePublicationMode_LocalAndRemote);
    EXPECT_CALL(consumer, handleNewSceneAvailable(info_22, Guid(22)));
    sceneGraphComponent.handleNewScenesAvailable({ info_22 }, Guid(22));

    EXPECT_CALL(consumer, handleInitializeScene(info_22, Guid(22)));
    sceneGraphComponent.handleInitializeScene(SceneId(2), Guid(22));

    const uint16_t data1 = 16u;
    const uint32_t data2 = 1234u;
    const float data3 = 0.1f;

    ManagedResourceVector resources{
        std::make_shared<const ArrayResource>(EResourceType_IndexArray, 1u, EDataType::UInt16, &data1, ResourceCacheFlag_DoNotCache, "ui16"),
        std::make_shared<const ArrayResource>(EResourceType_IndexArray, 1u, EDataType::UInt32, &data2, ResourceCacheFlag_DoNotCache, "ui32"),
        std::make_shared<const ArrayResource>(EResourceType_VertexArray, 1u, EDataType::Float, &data3, ResourceCacheFlag_DoNotCache, "fl")
    };

    SceneActionCollection actions(createFakeSceneActionCollectionFromTypes({ ESceneActionId::TestAction }));

    EXPECT_CALL(resourceComponent, manageResource(_, false)).WillRepeatedly([&](const IResource& res, bool) { return ManagedResource(&res); });
    EXPECT_CALL(consumer, handleSceneUpdate_rvr(SceneId(2), _, Guid(22))).WillOnce([&](const auto&, const auto& update, const auto&) {
        ASSERT_EQ(update.resources.size(), 3u);
        EXPECT_EQ(update.resources[0]->getTypeID(), resources[0]->getTypeID());
        EXPECT_EQ(update.resources[0]->getHash(), resources[0]->getHash());
        EXPECT_EQ(update.resources[1]->getTypeID(), resources[1]->getTypeID());
        EXPECT_EQ(update.resources[1]->getHash(), resources[1]->getHash());
        EXPECT_EQ(update.resources[2]->getTypeID(), resources[2]->getTypeID());
        EXPECT_EQ(update.resources[2]->getHash(), resources[2]->getHash());
        });

    const auto actionBlobs = actionsToChunks(actions, 100000, resources);
    sceneGraphComponent.handleSceneUpdate(SceneId(2), actionBlobs[0], Guid(22));
}

TEST_F(ASceneGraphComponent, deserializesResourcesFromRemote_SmallChunks)
{
    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.newParticipantHasConnected(Guid(22));

    SceneInfo info_22(SceneId(2), "", EScenePublicationMode_LocalAndRemote);
    EXPECT_CALL(consumer, handleNewSceneAvailable(info_22, Guid(22)));
    sceneGraphComponent.handleNewScenesAvailable({ info_22 }, Guid(22));

    EXPECT_CALL(consumer, handleInitializeScene(info_22, Guid(22)));
    sceneGraphComponent.handleInitializeScene(SceneId(2), Guid(22));

    const uint16_t data1[] = { 16u, 17u, 18u, 19u };
    const uint32_t data2[] = { 1234u, 1235u, 1236u, 1237u };
    const float data3[] = { 0.1f, 0.2f, 0.3f, 0.4f };

    ManagedResourceVector resources{
        std::make_shared<const ArrayResource>(EResourceType_IndexArray, 4u, EDataType::UInt16, &data1, ResourceCacheFlag_DoNotCache, "ui16"),
        std::make_shared<const ArrayResource>(EResourceType_IndexArray, 4u, EDataType::UInt32, &data2, ResourceCacheFlag_DoNotCache, "ui32"),
        std::make_shared<const ArrayResource>(EResourceType_VertexArray, 4u, EDataType::Float, &data3, ResourceCacheFlag_DoNotCache, "fl")
    };

    SceneActionCollection actions(createFakeSceneActionCollectionFromTypes({ ESceneActionId::TestAction }));

    EXPECT_CALL(resourceComponent, manageResource(_, false)).WillRepeatedly([&](const IResource& res, bool) { return ManagedResource(&res); });
    EXPECT_CALL(consumer, handleSceneUpdate_rvr(SceneId(2), _, Guid(22))).WillOnce([&](const auto&, const auto& update, const auto&) {
        ASSERT_EQ(update.resources.size(), 3u);
        EXPECT_EQ(update.resources[0]->getTypeID(), resources[0]->getTypeID());
        EXPECT_EQ(update.resources[0]->getHash(), resources[0]->getHash());
        EXPECT_EQ(update.resources[1]->getTypeID(), resources[1]->getTypeID());
        EXPECT_EQ(update.resources[1]->getHash(), resources[1]->getHash());
        EXPECT_EQ(update.resources[2]->getTypeID(), resources[2]->getTypeID());
        EXPECT_EQ(update.resources[2]->getHash(), resources[2]->getHash());
        });

    const auto actionBlobs = actionsToChunks(actions, 100, resources);
    EXPECT_GT(actionBlobs.size(), 1u);
    for (const auto& b : actionBlobs)
        sceneGraphComponent.handleSceneUpdate(SceneId(2), b, Guid(22));
}

TEST_F(ASceneGraphComponent, returnsFalseForFlushOnWrongResolvedResourceNumber_Shadow)
{
    const SceneId sceneId(1u);
    SceneInfo sceneInfo(SceneInfo(sceneId, "foo"));
    ClientScene scene(sceneInfo);

    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.handleCreateScene(scene, false, eventConsumer);

    auto res = new TextureResource(EResourceType_Texture2D, TextureMetaInfo(1u, 1u, 1u, ETextureFormat::R8, false, {}, { 1u }), ResourceCacheFlag_DoNotCache, String());
    res->setResourceData(ResourceBlob{ 1 }, { 1u, 1u });
    ManagedResource manRes(res);

    scene.allocateStreamTexture(WaylandIviSurfaceId(123u), { 111, 111 }, StreamTextureHandle{ 0 });
    EXPECT_CALL(resourceComponent, resolveResources(_)).Times(3).WillRepeatedly(Return(ManagedResourceVector{ manRes }));
    EXPECT_TRUE(sceneGraphComponent.handleFlush(sceneId, {}, {}));

    scene.allocateStreamTexture(WaylandIviSurfaceId(124u), { 222, 222 }, StreamTextureHandle{ 1 });
    EXPECT_CALL(resourceComponent, resolveResources(_)).WillOnce(Return(ManagedResourceVector{}));
    EXPECT_FALSE(sceneGraphComponent.handleFlush(sceneId, {}, {}));
}

TEST_F(ASceneGraphComponent, returnsFalseForFlushOnWrongResolvedResourceNumber_Direct)
{
    const SceneId sceneId(1u);
    SceneInfo sceneInfo(SceneInfo(sceneId, "foo"));
    ClientScene scene(sceneInfo);

    sceneGraphComponent.setSceneRendererHandler(&consumer);
    sceneGraphComponent.handleCreateScene(scene, true, eventConsumer);

    auto res = new TextureResource(EResourceType_Texture2D, TextureMetaInfo(1u, 1u, 1u, ETextureFormat::R8, false, {}, { 1u }), ResourceCacheFlag_DoNotCache, String());
    res->setResourceData(ResourceBlob{ 1 }, { 1u, 1u });
    ManagedResource manRes(res);

    scene.allocateStreamTexture(WaylandIviSurfaceId(123u), { 111, 111 }, StreamTextureHandle{ 0 });
    EXPECT_CALL(resourceComponent, resolveResources(_)).Times(2).WillRepeatedly(Return(ManagedResourceVector{ manRes }));
    EXPECT_TRUE(sceneGraphComponent.handleFlush(sceneId, {}, {}));

    scene.allocateStreamTexture(WaylandIviSurfaceId(124u), { 222, 222 }, StreamTextureHandle{ 1 });
    EXPECT_CALL(resourceComponent, resolveResources(_)).WillOnce(Return(ManagedResourceVector{}));
    EXPECT_FALSE(sceneGraphComponent.handleFlush(sceneId, {}, {}));
}
