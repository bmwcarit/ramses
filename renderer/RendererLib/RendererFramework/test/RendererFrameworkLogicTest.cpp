//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "RendererFramework/RendererFrameworkLogic.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "ComponentMocks.h"
#include "ResourceMock.h"
#include "MockConnectionStatusUpdateNotifier.h"
#include "RendererCommandVisitorMock.h"
#include "MockResourceHash.h"

#include <string>

using namespace testing;

namespace ramses_internal
{
    class ARendererFrameworkLogic : public testing::Test
    {
    public:
        ARendererFrameworkLogic()
            : fixture(sceneGraphConsumerComponent, rendererCommandBuffer, frameworkLock)
            , providerID(20)
            , sceneId(33u)
            , sceneName("scene")
        {
        }

    protected:
        void visitPendingCommands()
        {
            cmdVisitor.visit(rendererCommandBuffer);
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

        StrictMock<SceneGraphConsumerComponentMock> sceneGraphConsumerComponent;
        RendererCommandBuffer rendererCommandBuffer;
        PlatformLock frameworkLock;
        RendererFrameworkLogic fixture;
        StrictMock<RendererCommandVisitorMock> cmdVisitor;

        const Guid providerID;
        const SceneId sceneId;
        const std::string sceneName;
        const SceneSizeInformation sizeInfo;
    };

    TEST_F(ARendererFrameworkLogic, generatesPublishedRendererCommand)
    {
        fixture.handleNewSceneAvailable(SceneInfo(sceneId, sceneName, EScenePublicationMode_LocalAndRemote), providerID);
        EXPECT_CALL(cmdVisitor, handleScenePublished(sceneId, EScenePublicationMode_LocalAndRemote));
        visitPendingCommands();
    }

    TEST_F(ARendererFrameworkLogic, generatesReceiveRendererCommand)
    {
        const SceneInfo sceneInfo{ sceneId, sceneName, EScenePublicationMode_LocalAndRemote };
        fixture.handleNewSceneAvailable(sceneInfo, providerID);
        fixture.handleInitializeScene(sceneInfo, providerID);

        InSequence seq;
        EXPECT_CALL(cmdVisitor, handleScenePublished(sceneId, EScenePublicationMode_LocalAndRemote));
        EXPECT_CALL(cmdVisitor, handleSceneReceived(sceneInfo));
        visitPendingCommands();
    }

    TEST_F(ARendererFrameworkLogic, generatesUnpublishRendererCommand)
    {
        fixture.handleNewSceneAvailable(SceneInfo(sceneId, "", EScenePublicationMode_LocalAndRemote), providerID);
        fixture.handleSceneBecameUnavailable(sceneId, providerID);

        InSequence seq;
        EXPECT_CALL(cmdVisitor, handleScenePublished(sceneId, EScenePublicationMode_LocalAndRemote));
        EXPECT_CALL(cmdVisitor, handleSceneUnpublished(sceneId));
        visitPendingCommands();
    }

    TEST_F(ARendererFrameworkLogic, ignoresSecondPublishFromDifferentProvider)
    {
        fixture.handleNewSceneAvailable(SceneInfo(sceneId, sceneName, EScenePublicationMode_LocalAndRemote), providerID);
        fixture.handleNewSceneAvailable(SceneInfo(sceneId, sceneName, EScenePublicationMode_LocalAndRemote), Guid(30));

        EXPECT_CALL(cmdVisitor, handleScenePublished(sceneId, EScenePublicationMode_LocalAndRemote));
        visitPendingCommands();
    }

    TEST_F(ARendererFrameworkLogic, handlesSceneUpdateWithFlush)
    {
        fixture.handleNewSceneAvailable(SceneInfo(sceneId, "", EScenePublicationMode_LocalAndRemote), providerID);
        EXPECT_CALL(cmdVisitor, handleScenePublished(sceneId, EScenePublicationMode_LocalAndRemote));
        visitPendingCommands();

        SceneUpdate sceneUpdate;
        sceneUpdate.actions = createFakeSceneActionCollectionFromTypes({ ESceneActionId::AddChildToNode });
        sceneUpdate.resources.push_back(MockResourceHash::GetManagedResource(MockResourceHash::EffectHash));
        // copy before move
        SceneUpdate expectedData;
        expectedData.actions = sceneUpdate.actions.copy();
        expectedData.resources = sceneUpdate.resources;
        expectedData.flushInfos = sceneUpdate.flushInfos.copy();

        fixture.handleSceneUpdate(sceneId, std::move(sceneUpdate), providerID);

        EXPECT_CALL(cmdVisitor, handleSceneUpdate(sceneId, _)).WillOnce(Invoke([&](auto, const SceneUpdate& data)
        {
            EXPECT_EQ(expectedData.actions, data.actions);
            EXPECT_EQ(expectedData.resources, data.resources);
            EXPECT_EQ(expectedData.flushInfos, data.flushInfos);
        }));
        visitPendingCommands();
    }

    TEST_F(ARendererFrameworkLogic, willNotSendSubscribeMessageWhenProviderUnknown)
    {
        fixture.sendSubscribeScene(SceneId{ 123 });
    }

    TEST_F(ARendererFrameworkLogic, willSendSubscribeMessageToCorrectProvider)
    {
        fixture.handleNewSceneAvailable(SceneInfo(sceneId, sceneName, EScenePublicationMode_LocalAndRemote), providerID);
        fixture.handleNewSceneAvailable(SceneInfo(SceneId{123}, "foo", EScenePublicationMode_LocalAndRemote), Guid{456});

        EXPECT_CALL(sceneGraphConsumerComponent, subscribeScene(providerID, sceneId));
        fixture.sendSubscribeScene(sceneId);
    }

    TEST_F(ARendererFrameworkLogic, willNotSendUnsubscribeMessageWhenProviderUnknown)
    {
        fixture.sendUnsubscribeScene(SceneId{ 123 });
    }

    TEST_F(ARendererFrameworkLogic, willSendUnsubscribeMessageToCorrectProvider)
    {
        fixture.handleNewSceneAvailable(SceneInfo(sceneId, sceneName, EScenePublicationMode_LocalAndRemote), providerID);
        fixture.handleNewSceneAvailable(SceneInfo(SceneId{ 123 }, "foo", EScenePublicationMode_LocalAndRemote), Guid{ 456 });

        EXPECT_CALL(sceneGraphConsumerComponent, unsubscribeScene(providerID, sceneId));
        fixture.sendUnsubscribeScene(sceneId);
    }

    TEST_F(ARendererFrameworkLogic, willNotSendSceneStateChangedMessageWhenProviderUnknown)
    {
        fixture.sendSceneStateChanged(SceneId{ 123 }, SceneId{ 456 }, RendererSceneState::Available);
    }

    TEST_F(ARendererFrameworkLogic, willSendCorrectSceneStateChangedMessageToCorrectProvider)
    {
        fixture.handleNewSceneAvailable(SceneInfo(sceneId, sceneName, EScenePublicationMode_LocalAndRemote), providerID);
        fixture.handleNewSceneAvailable(SceneInfo(SceneId{ 123 }, "foo", EScenePublicationMode_LocalAndRemote), Guid{ 456 });

        EXPECT_CALL(sceneGraphConsumerComponent, sendSceneReferenceEvent(providerID, _)).WillOnce([this](Guid const&, SceneReferenceEvent const& event)
            {
                EXPECT_EQ(event.masterSceneId, sceneId);
                EXPECT_EQ(event.type, SceneReferenceEventType::SceneStateChanged);
                EXPECT_EQ(event.referencedScene, SceneId { 456 });
                EXPECT_EQ(event.sceneState, RendererSceneState::Available);
            });
        fixture.sendSceneStateChanged(sceneId, SceneId{ 456 }, RendererSceneState::Available);
    }

    TEST_F(ARendererFrameworkLogic, willNotSendSceneFlushedMessageWhenProviderUnknown)
    {
        fixture.sendSceneFlushed(SceneId{ 123 }, SceneId{ 456 }, SceneVersionTag{ 789 });
    }

    TEST_F(ARendererFrameworkLogic, willSendCorrectSceneFlushedMessageToCorrectProvider)
    {
        fixture.handleNewSceneAvailable(SceneInfo(sceneId, sceneName, EScenePublicationMode_LocalAndRemote), providerID);
        fixture.handleNewSceneAvailable(SceneInfo(SceneId{ 123 }, "foo", EScenePublicationMode_LocalAndRemote), Guid{ 456 });

        EXPECT_CALL(sceneGraphConsumerComponent, sendSceneReferenceEvent(providerID, _)).WillOnce([this](Guid const&, SceneReferenceEvent const& event)
            {
                EXPECT_EQ(event.masterSceneId, sceneId);
                EXPECT_EQ(event.type, SceneReferenceEventType::SceneFlushed);
                EXPECT_EQ(event.referencedScene, SceneId { 456 });
                EXPECT_EQ(event.tag, SceneVersionTag { 789 });
            });
        fixture.sendSceneFlushed(sceneId, SceneId{ 456 }, SceneVersionTag{ 789 });
    }

    TEST_F(ARendererFrameworkLogic, willNotSendDataLinkedMessageWhenProviderUnknown)
    {
        fixture.sendDataLinked(SceneId{ 123 }, SceneId{ 456 }, DataSlotId{ 12 }, SceneId{ 789 }, DataSlotId{ 34 }, false);
    }

    TEST_F(ARendererFrameworkLogic, willSendCorrectDataLinkedMessageToCorrectProvider)
    {
        fixture.handleNewSceneAvailable(SceneInfo(sceneId, sceneName, EScenePublicationMode_LocalAndRemote), providerID);
        fixture.handleNewSceneAvailable(SceneInfo(SceneId{ 123 }, "foo", EScenePublicationMode_LocalAndRemote), Guid{ 456 });

        EXPECT_CALL(sceneGraphConsumerComponent, sendSceneReferenceEvent(providerID, _)).WillOnce([this](Guid const&, SceneReferenceEvent const& event)
            {
                EXPECT_EQ(event.masterSceneId, sceneId);
                EXPECT_EQ(event.type, SceneReferenceEventType::DataLinked);
                EXPECT_EQ(event.providerScene, SceneId { 456 });
                EXPECT_EQ(event.dataProvider, DataSlotId{ 12 });
                EXPECT_EQ(event.consumerScene, SceneId { 789 });
                EXPECT_EQ(event.dataConsumer, DataSlotId { 34 });
                EXPECT_EQ(event.status, false);
            });
        fixture.sendDataLinked(sceneId, SceneId{ 456 }, DataSlotId{ 12 }, SceneId{ 789 }, DataSlotId{ 34 }, false);
    }

    TEST_F(ARendererFrameworkLogic, willNotSendDataUnlinkedMessageWhenProviderUnknown)
    {
        fixture.sendDataUnlinked(SceneId{ 123 }, SceneId{ 456 }, DataSlotId{ 12 }, true);
    }

    TEST_F(ARendererFrameworkLogic, willSendCorrectDataUnlinkedMessageToCorrectProvider)
    {
        fixture.handleNewSceneAvailable(SceneInfo(sceneId, sceneName, EScenePublicationMode_LocalAndRemote), providerID);
        fixture.handleNewSceneAvailable(SceneInfo(SceneId{ 123 }, "foo", EScenePublicationMode_LocalAndRemote), Guid{ 456 });

        EXPECT_CALL(sceneGraphConsumerComponent, sendSceneReferenceEvent(providerID, _)).WillOnce([this](Guid const&, SceneReferenceEvent const& event)
            {
                EXPECT_EQ(event.masterSceneId, sceneId);
                EXPECT_EQ(event.type, SceneReferenceEventType::DataUnlinked);
                EXPECT_EQ(event.consumerScene, SceneId { 456 });
                EXPECT_EQ(event.dataConsumer, DataSlotId { 12 });
                EXPECT_EQ(event.status, true);
            });
        fixture.sendDataUnlinked(sceneId, SceneId{ 456 }, DataSlotId{ 12 }, true);
    }
}
