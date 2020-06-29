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

using namespace testing;

namespace ramses_internal
{
    class ARendererFrameworkLogic : public testing::Test
    {
    public:
        ARendererFrameworkLogic()
            : testing::Test()
            , fixture(
            connectionStatusUpdateNotifier,
            resourceComponent,
            sceneGraphConsumerComponent,
            rendererCommandBuffer,
            frameworkLock
            )
            , providerID(20)
            , sceneId(33u)
            , sceneName("scene")
        {
        }

    protected:
        void expectSceneCommand(ERendererCommand commandType)
        {
            RendererCommandContainer commands;
            rendererCommandBuffer.swapCommandContainer(commands);
            ASSERT_EQ(1u, commands.getTotalCommandCount());
            EXPECT_EQ(commandType, commands.getCommandType(0u));
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

        RendererCommandContainer dispatchCommands()
        {
            RendererCommandContainer cmds;
            rendererCommandBuffer.swapCommandContainer(cmds);
            return cmds;
        }

        StrictMock<ResourceConsumerComponentMock> resourceComponent;
        StrictMock<SceneGraphConsumerComponentMock> sceneGraphConsumerComponent;

        RendererCommandBuffer rendererCommandBuffer;
        NiceMock<MockConnectionStatusUpdateNotifier> connectionStatusUpdateNotifier;
        PlatformLock frameworkLock;
        RendererFrameworkLogic fixture;

        const Guid providerID;
        const SceneId sceneId;
        const String sceneName;
        const SceneSizeInformation sizeInfo;
    };

    TEST_F(ARendererFrameworkLogic, generatesPublishedRendererCommand)
    {
        fixture.handleNewScenesAvailable(SceneInfoVector(1, SceneInfo(sceneId, sceneName)), providerID, EScenePublicationMode_LocalAndRemote);
        expectSceneCommand(ERendererCommand_PublishedScene);
    }

    TEST_F(ARendererFrameworkLogic, generatesReceiveRendererCommand)
    {
        fixture.handleInitializeScene(SceneInfo(sceneId, sceneName), providerID);
        expectSceneCommand(ERendererCommand_ReceivedScene);
    }

    TEST_F(ARendererFrameworkLogic, generatesUnpublishRendererCommand)
    {
        fixture.handleNewScenesAvailable(SceneInfoVector(1, SceneInfo(sceneId)), providerID, EScenePublicationMode_LocalAndRemote);
        expectSceneCommand(ERendererCommand_PublishedScene);
        fixture.handleScenesBecameUnavailable(SceneInfoVector(1, SceneInfo(sceneId)), providerID);
        expectSceneCommand(ERendererCommand_UnpublishedScene);
    }

    TEST_F(ARendererFrameworkLogic, ignoresSecondPublishFromDifferentProvider)
    {
        fixture.handleNewScenesAvailable(SceneInfoVector(1, SceneInfo(sceneId, sceneName)), providerID, EScenePublicationMode_LocalAndRemote);
        fixture.handleNewScenesAvailable(SceneInfoVector(1, SceneInfo(sceneId, sceneName)), Guid(30), EScenePublicationMode_LocalAndRemote);
        expectSceneCommand(ERendererCommand_PublishedScene);
    }

    TEST_F(ARendererFrameworkLogic, doesAutomaticUnpublisThenPublishhWhenPublishedAgainFromSameProvider)
    {
        fixture.handleNewScenesAvailable(SceneInfoVector(1, SceneInfo(sceneId, sceneName)), providerID, EScenePublicationMode_LocalAndRemote);
        expectSceneCommand(ERendererCommand_PublishedScene);

        fixture.handleNewScenesAvailable(SceneInfoVector(1, SceneInfo(sceneId, sceneName)), providerID, EScenePublicationMode_LocalAndRemote);
        RendererCommandContainer commands;
        rendererCommandBuffer.swapCommandContainer(commands);

        ASSERT_EQ(2u, commands.getTotalCommandCount());

        EXPECT_EQ(ERendererCommand_UnpublishedScene, commands.getCommandType(0u));
        const auto command_0 = commands.getCommandData<SceneInfoCommand>(0u);
        EXPECT_EQ(sceneId, command_0.sceneInformation.sceneID);

        EXPECT_EQ(ERendererCommand_PublishedScene, commands.getCommandType(1u));
        const auto command_1 = commands.getCommandData<SceneInfoCommand>(1u);
        EXPECT_EQ(sceneId, command_1.sceneInformation.sceneID);
        EXPECT_EQ(EScenePublicationMode_LocalAndRemote, command_1.sceneInformation.publicationMode);
    }

    TEST_F(ARendererFrameworkLogic, handlesSceneActionListWithFlush)
    {
        fixture.handleNewScenesAvailable(SceneInfoVector(1, SceneInfo(sceneId)), providerID, EScenePublicationMode_LocalAndRemote);
        expectSceneCommand(ERendererCommand_PublishedScene);

        SceneActionCollection actions(createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode , ESceneActionId_Flush }));
        fixture.handleSceneActionList(sceneId, std::move(actions), 0u, providerID);

        RendererCommandContainer commands;
        rendererCommandBuffer.swapCommandContainer(commands);
        ASSERT_EQ(1u, commands.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, commands.getCommandType(0u));
        const SceneActionsCommand& cmd = commands.getCommandData<SceneActionsCommand>(0u);
        EXPECT_EQ(sceneId, cmd.sceneId);
        EXPECT_EQ(2u, cmd.sceneActions.numberOfActions());
        EXPECT_EQ(ESceneActionId_AddChildToNode, cmd.sceneActions[0].type());
    }

    TEST_F(ARendererFrameworkLogic, doesntHandleSceneActionsWithoutFlush)
    {
        SceneActionCollection actions(createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode }));
        fixture.handleSceneActionList(sceneId, std::move(actions), 0u, providerID);
        EXPECT_EQ(0u, dispatchCommands().getTotalCommandCount());
    }

    TEST_F(ARendererFrameworkLogic, buffersSceneActionsUntilFlush)
    {
        fixture.handleNewScenesAvailable(SceneInfoVector(1, SceneInfo(sceneId)), providerID, EScenePublicationMode_LocalAndRemote);
        expectSceneCommand(ERendererCommand_PublishedScene);

        SceneActionCollection expectedActions(createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode, ESceneActionId_SetStateDepthFunc, ESceneActionId_Flush }));

        fixture.handleSceneActionList(sceneId, createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode }), 0u, providerID);
        EXPECT_EQ(0u, dispatchCommands().getTotalCommandCount());

        fixture.handleSceneActionList(sceneId, createFakeSceneActionCollectionFromTypes({ ESceneActionId_SetStateDepthFunc }), 0u, providerID);
        EXPECT_EQ(0u, dispatchCommands().getTotalCommandCount());

        fixture.handleSceneActionList(sceneId, createFakeSceneActionCollectionFromTypes({ ESceneActionId_Flush }), 0u, providerID);
        const auto commands = dispatchCommands();
        ASSERT_EQ(1u, commands.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, commands.getCommandType(0u));
        const SceneActionsCommand& cmd = commands.getCommandData<SceneActionsCommand>(0u);
        EXPECT_EQ(sceneId, cmd.sceneId);
        EXPECT_EQ(3u, cmd.sceneActions.numberOfActions());
        EXPECT_EQ(expectedActions, cmd.sceneActions);
    }

    TEST_F(ARendererFrameworkLogic, buffersSceneActionsUntilFlushForDifferentScenes)
    {
        const SceneId sceneId1(1u);
        const SceneId sceneId2(2u);

        fixture.handleNewScenesAvailable({ SceneInfo(sceneId1) }, providerID, EScenePublicationMode_LocalAndRemote);
        expectSceneCommand(ERendererCommand_PublishedScene);
        fixture.handleNewScenesAvailable({ SceneInfo(sceneId2) }, providerID, EScenePublicationMode_LocalAndRemote);
        expectSceneCommand(ERendererCommand_PublishedScene);

        SceneActionCollection expectedActionsScene1(createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode, ESceneActionId_AllocateRenderable, ESceneActionId_Flush }));
        SceneActionCollection expectedActionsScene2(createFakeSceneActionCollectionFromTypes({ ESceneActionId_SetCameraFrustum, ESceneActionId_SetStateDepthFunc, ESceneActionId_Flush }));

        fixture.handleSceneActionList(sceneId1, createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode }), 0u, providerID);
        EXPECT_EQ(0u, dispatchCommands().getTotalCommandCount());

        fixture.handleSceneActionList(sceneId2, createFakeSceneActionCollectionFromTypes({ ESceneActionId_SetCameraFrustum }), 0u, providerID);
        EXPECT_EQ(0u, dispatchCommands().getTotalCommandCount());

        fixture.handleSceneActionList(sceneId2, createFakeSceneActionCollectionFromTypes({ ESceneActionId_SetStateDepthFunc }), 0u, providerID);
        EXPECT_EQ(0u, dispatchCommands().getTotalCommandCount());

        fixture.handleSceneActionList(sceneId2, createFakeSceneActionCollectionFromTypes({ ESceneActionId_Flush }), 0u, providerID);
        auto commands = dispatchCommands();
        ASSERT_EQ(1u, commands.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, commands.getCommandType(0u));
        const SceneActionsCommand& cmdScene2 = commands.getCommandData<SceneActionsCommand>(0u);
        EXPECT_EQ(sceneId2, cmdScene2.sceneId);
        EXPECT_EQ(3u, cmdScene2.sceneActions.numberOfActions());
        EXPECT_EQ(expectedActionsScene2, cmdScene2.sceneActions);

        fixture.handleSceneActionList(sceneId1, createFakeSceneActionCollectionFromTypes({ ESceneActionId_AllocateRenderable }), 0u, providerID);
        EXPECT_EQ(0u, dispatchCommands().getTotalCommandCount());

        fixture.handleSceneActionList(sceneId1, createFakeSceneActionCollectionFromTypes({ ESceneActionId_Flush }), 0u, providerID);
        commands = dispatchCommands();
        ASSERT_EQ(1u, commands.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, commands.getCommandType(0u));
        const SceneActionsCommand& cmdScene1 = commands.getCommandData<SceneActionsCommand>(0u);
        EXPECT_EQ(sceneId1, cmdScene1.sceneId);
        EXPECT_EQ(3u, cmdScene1.sceneActions.numberOfActions());
        EXPECT_EQ(expectedActionsScene1, cmdScene1.sceneActions);
    }

    TEST_F(ARendererFrameworkLogic, generatesUnpublishRendererCommandsForScenesFromDisconnectedClient)
    {
        const SceneId sceneId1(1u);
        const SceneId sceneId2(2u);

        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId1, sceneName));
        newScenes.push_back(SceneInfo(sceneId2, sceneName));
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);
        dispatchCommands();

        fixture.participantHasDisconnected(providerID);

        const auto commands = dispatchCommands();
        ASSERT_EQ(2u, commands.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_UnpublishedScene, commands.getCommandType(0u));
        EXPECT_EQ(ERendererCommand_UnpublishedScene, commands.getCommandType(1u));

        const SceneInfoCommand& command1 = commands.getCommandData<SceneInfoCommand>(0u);
        EXPECT_EQ(sceneId2, command1.sceneInformation.sceneID);

        const SceneInfoCommand& command2 = commands.getCommandData<SceneInfoCommand>(1u);
        EXPECT_EQ(sceneId1, command2.sceneInformation.sceneID);
    }

    TEST_F(ARendererFrameworkLogic, generatesUnpublishRendererCommandsForScenesFromDisconnectedClient_DoesNotModifyOtherClientsScene)
    {
        const SceneId sceneId1(1u);
        const SceneId sceneId2(2u);

        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId1, sceneName));
        newScenes.push_back(SceneInfo(sceneId2, sceneName));
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);
        fixture.handleNewScenesAvailable(SceneInfoVector(1, SceneInfo(sceneId, sceneName)), Guid(999), EScenePublicationMode_LocalAndRemote);
        dispatchCommands();

        fixture.participantHasDisconnected(providerID);

        const auto commands = dispatchCommands();
        ASSERT_EQ(2u, commands.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_UnpublishedScene, commands.getCommandType(0u));
        EXPECT_EQ(ERendererCommand_UnpublishedScene, commands.getCommandType(1u));

        const SceneInfoCommand& command1 = commands.getCommandData<SceneInfoCommand>(0u);
        EXPECT_EQ(sceneId2, command1.sceneInformation.sceneID);

        const SceneInfoCommand& command2 = commands.getCommandData<SceneInfoCommand>(1u);
        EXPECT_EQ(sceneId1, command2.sceneInformation.sceneID);
    }

    TEST_F(ARendererFrameworkLogic, requestsResourcesViaResourceComponentWithCorrectProviderID)
    {
        fixture.handleNewScenesAvailable(SceneInfoVector(1, SceneInfo(sceneId, sceneName)), providerID, EScenePublicationMode_LocalAndRemote);

        const ResourceContentHash resource(44u, 0);
        ResourceContentHashVector resources;
        resources.push_back(resource);

        ResourceRequesterID requester(1);

        EXPECT_CALL(resourceComponent, requestResourceAsynchronouslyFromFramework(resources, requester, providerID));
        fixture.requestResourceAsyncronouslyFromFramework(resources, requester, sceneId);
    }

    TEST_F(ARendererFrameworkLogic, cancelsResourceRequestViaResourceComponent)
    {
        const ResourceContentHash resource(44u, 0);

        ResourceRequesterID requester(1);

        EXPECT_CALL(resourceComponent, cancelResourceRequest(resource, requester));
        fixture.cancelResourceRequest(resource, requester);
    }

    TEST_F(ARendererFrameworkLogic, generatesUnsubscribeRendererCommandsWhenDetectingSceneActionListCounterMismatch)
    {
        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId, sceneName));
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);
        fixture.handleInitializeScene(SceneInfo(sceneId), providerID);
        dispatchCommands();

        // send one correct/normal list
        SceneActionCollection actions(createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode , ESceneActionId_Flush }));
        fixture.handleSceneActionList(sceneId, actions.copy(), 1u, providerID);

        auto commands = dispatchCommands();
        ASSERT_EQ(1u, commands.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, commands.getCommandType(0u));

        // send list with mismatched counter value
        fixture.handleSceneActionList(sceneId, actions.copy(), 3u, providerID);

        commands = dispatchCommands();
        ASSERT_EQ(1u, commands.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_UnsubscribeScene, commands.getCommandType(0));
        EXPECT_TRUE(commands.getCommandData<ramses_internal::SceneInfoCommand>(0).indirect);
    }

    TEST_F(ARendererFrameworkLogic, expectsCountingSceneActionsFromStartAfterInitializeScene)
    {
        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId, sceneName));
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);
        fixture.handleInitializeScene(SceneInfo(sceneId), providerID);
        dispatchCommands();

        // send one correct/normal list
        SceneActionCollection actions(createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode , ESceneActionId_Flush }));
        fixture.handleSceneActionList(sceneId, actions.copy(), 1u, providerID);

        auto cmds = dispatchCommands();
        ASSERT_EQ(1u, cmds.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, cmds.getCommandType(0u));

        fixture.handleInitializeScene(SceneInfo(sceneId), providerID);
        dispatchCommands();
        fixture.handleSceneActionList(sceneId, actions.copy(), 1u, providerID);

        cmds = dispatchCommands();
        ASSERT_EQ(1u, cmds.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, cmds.getCommandType(0u));
    }

    TEST_F(ARendererFrameworkLogic, expectsCountingSceneActionsFromStartAfterRepublish)
    {
        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId, sceneName));
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);
        fixture.handleInitializeScene(SceneInfo(sceneId), providerID);
        dispatchCommands();

        // send one correct/normal list
        SceneActionCollection actions(createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode , ESceneActionId_Flush }));
        fixture.handleSceneActionList(sceneId, actions.copy(), 1u, providerID);

        auto cmds = dispatchCommands();
        ASSERT_EQ(1u, cmds.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, cmds.getCommandType(0u));

        fixture.handleScenesBecameUnavailable({ SceneInfo(sceneId) }, providerID);
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);
        fixture.handleInitializeScene(SceneInfo(sceneId), providerID);

        dispatchCommands();
        fixture.handleSceneActionList(sceneId, actions.copy(), 1u, providerID);

        cmds = dispatchCommands();
        ASSERT_EQ(1u, cmds.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, cmds.getCommandType(0u));
    }


    TEST_F(ARendererFrameworkLogic, generatesUnsubscribeRendererCommandsWhenContinueCountingSceneActionListAfterInitializeScene)
    {
        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId, sceneName));
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);
        fixture.handleInitializeScene(SceneInfo(sceneId), providerID);
        dispatchCommands();

        // send one correct/normal list
        SceneActionCollection actions(createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode , ESceneActionId_Flush }));
        fixture.handleSceneActionList(sceneId, actions.copy(), 1u, providerID);

        auto cmds = dispatchCommands();
        ASSERT_EQ(1u, cmds.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, cmds.getCommandType(0u));

        fixture.handleInitializeScene(SceneInfo(sceneId), providerID);
        dispatchCommands();

        // send list with mismatched counter value
        fixture.handleSceneActionList(sceneId, actions.copy(), 2u, providerID);

        cmds = dispatchCommands();
        ASSERT_EQ(1u, cmds.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_UnsubscribeScene, cmds.getCommandType(0));
        EXPECT_TRUE(cmds.getCommandData<ramses_internal::SceneInfoCommand>(0).indirect);
    }

    TEST_F(ARendererFrameworkLogic, generatesUnpublishRendererCommandsWhenParticipantDisconnectAndStartsAgainWithNewCounter)
    {
        fixture.newParticipantHasConnected(providerID);

        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId, sceneName));
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);
        fixture.handleInitializeScene(SceneInfo(sceneId), providerID);
        dispatchCommands();

        // send one correct/normal list
        SceneActionCollection actions(createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode , ESceneActionId_Flush }));
        fixture.handleSceneActionList(sceneId, actions.copy(), 1u, providerID);

        auto cmds = dispatchCommands();
        ASSERT_EQ(1u, cmds.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, cmds.getCommandType(0u));

        dispatchCommands();
        fixture.participantHasDisconnected(providerID);
        cmds = dispatchCommands();
        ASSERT_EQ(1u, cmds.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_UnpublishedScene, cmds.getCommandType(0u));

        fixture.newParticipantHasConnected(providerID);
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);
        fixture.handleInitializeScene(SceneInfo(sceneId), providerID);
        dispatchCommands();

        fixture.handleSceneActionList(sceneId, actions.copy(), 1u, providerID);
        cmds = dispatchCommands();
        ASSERT_EQ(1u, cmds.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, cmds.getCommandType(0u));
    }

    TEST_F(ARendererFrameworkLogic, willNotSendSubscribeMessageWhenProviderUnknown)
    {
        fixture.sendSubscribeScene(SceneId{ 123 });
    }

    TEST_F(ARendererFrameworkLogic, willSendSubscribeMessageToCorrectProvider)
    {
        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId, sceneName));
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);

        newScenes.clear();
        newScenes.push_back(SceneInfo(SceneId{ 123 }, "foo"));
        fixture.handleNewScenesAvailable(newScenes, Guid{ 456 }, EScenePublicationMode_LocalAndRemote);

        EXPECT_CALL(sceneGraphConsumerComponent, subscribeScene(providerID, sceneId));
        fixture.sendSubscribeScene(sceneId);
    }

    TEST_F(ARendererFrameworkLogic, willNotSendUnsubscribeMessageWhenProviderUnknown)
    {
        fixture.sendUnsubscribeScene(SceneId{ 123 });
    }

    TEST_F(ARendererFrameworkLogic, willSendUnsubscribeMessageToCorrectProvider)
    {
        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId, sceneName));
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);

        newScenes.clear();
        newScenes.push_back(SceneInfo(SceneId{ 123 }, "foo"));
        fixture.handleNewScenesAvailable(newScenes, Guid{ 456 }, EScenePublicationMode_LocalAndRemote);

        EXPECT_CALL(sceneGraphConsumerComponent, unsubscribeScene(providerID, sceneId));
        fixture.sendUnsubscribeScene(sceneId);
    }

    TEST_F(ARendererFrameworkLogic, willNotSendSceneStateChangedMessageWhenProviderUnknown)
    {
        fixture.sendSceneStateChanged(SceneId{ 123 }, SceneId{ 456 }, RendererSceneState::Available);
    }

    TEST_F(ARendererFrameworkLogic, willSendCorrectSceneStateChangedMessageToCorrectProvider)
    {
        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId, sceneName));
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);

        newScenes.clear();
        newScenes.push_back(SceneInfo(SceneId{ 123 }, "foo"));
        fixture.handleNewScenesAvailable(newScenes, Guid{ 456 }, EScenePublicationMode_LocalAndRemote);

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
        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId, sceneName));
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);

        newScenes.clear();
        newScenes.push_back(SceneInfo(SceneId{ 123 }, "foo"));
        fixture.handleNewScenesAvailable(newScenes, Guid{ 456 }, EScenePublicationMode_LocalAndRemote);

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
        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId, sceneName));
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);

        newScenes.clear();
        newScenes.push_back(SceneInfo(SceneId{ 123 }, "foo"));
        fixture.handleNewScenesAvailable(newScenes, Guid{ 456 }, EScenePublicationMode_LocalAndRemote);

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
        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId, sceneName));
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);

        newScenes.clear();
        newScenes.push_back(SceneInfo(SceneId{ 123 }, "foo"));
        fixture.handleNewScenesAvailable(newScenes, Guid{ 456 }, EScenePublicationMode_LocalAndRemote);

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
