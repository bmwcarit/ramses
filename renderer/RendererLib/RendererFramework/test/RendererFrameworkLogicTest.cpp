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
            , providerID(true)
            , sceneId(33u)
            , sceneName("scene")
        {
        }

    protected:
        void expectSceneCommand(ERendererCommand commandType)
        {
            const auto& commands = rendererCommandBuffer.getCommands();
            ASSERT_EQ(1u, commands.getTotalCommandCount());
            EXPECT_EQ(commandType, commands.getCommandType(0u));
            rendererCommandBuffer.clear();
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

    TEST_F(ARendererFrameworkLogic, handlesSceneActionListWithFlush)
    {
        fixture.handleNewScenesAvailable(SceneInfoVector(1, SceneInfo(sceneId)), providerID, EScenePublicationMode_LocalAndRemote);
        expectSceneCommand(ERendererCommand_PublishedScene);

        SceneActionCollection actions(createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode , ESceneActionId_Flush }));
        fixture.handleSceneActionList(sceneId, std::move(actions), 0u, providerID);

        const RendererCommandContainer& commands = rendererCommandBuffer.getCommands();
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

        const RendererCommandContainer& commands = rendererCommandBuffer.getCommands();
        EXPECT_EQ(0u, commands.getTotalCommandCount());
    }

    TEST_F(ARendererFrameworkLogic, buffersSceneActionsUntilFlush)
    {
        fixture.handleNewScenesAvailable(SceneInfoVector(1, SceneInfo(sceneId)), providerID, EScenePublicationMode_LocalAndRemote);
        expectSceneCommand(ERendererCommand_PublishedScene);

        const RendererCommandContainer& commands = rendererCommandBuffer.getCommands();
        SceneActionCollection expectedActions(createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode, ESceneActionId_SetStateDepthFunc, ESceneActionId_Flush }));

        fixture.handleSceneActionList(sceneId, createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode }), 0u, providerID);
        EXPECT_EQ(0u, commands.getTotalCommandCount());

        fixture.handleSceneActionList(sceneId, createFakeSceneActionCollectionFromTypes({ ESceneActionId_SetStateDepthFunc }), 0u, providerID);
        EXPECT_EQ(0u, commands.getTotalCommandCount());

        fixture.handleSceneActionList(sceneId, createFakeSceneActionCollectionFromTypes({ ESceneActionId_Flush }), 0u, providerID);
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

        const RendererCommandContainer& commands = rendererCommandBuffer.getCommands();

        SceneActionCollection expectedActionsScene1(createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode, ESceneActionId_AllocateRenderable, ESceneActionId_Flush }));
        SceneActionCollection expectedActionsScene2(createFakeSceneActionCollectionFromTypes({ ESceneActionId_SetCameraFrustum, ESceneActionId_SetStateDepthFunc, ESceneActionId_Flush }));

        fixture.handleSceneActionList(sceneId1, createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode }), 0u, providerID);
        EXPECT_EQ(0u, commands.getTotalCommandCount());

        fixture.handleSceneActionList(sceneId2, createFakeSceneActionCollectionFromTypes({ ESceneActionId_SetCameraFrustum }), 0u, providerID);
        EXPECT_EQ(0u, commands.getTotalCommandCount());

        fixture.handleSceneActionList(sceneId2, createFakeSceneActionCollectionFromTypes({ ESceneActionId_SetStateDepthFunc }), 0u, providerID);
        EXPECT_EQ(0u, commands.getTotalCommandCount());

        fixture.handleSceneActionList(sceneId2, createFakeSceneActionCollectionFromTypes({ ESceneActionId_Flush }), 0u, providerID);
        ASSERT_EQ(1u, commands.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, commands.getCommandType(0u));
        const SceneActionsCommand& cmdScene2 = commands.getCommandData<SceneActionsCommand>(0u);
        EXPECT_EQ(sceneId2, cmdScene2.sceneId);
        EXPECT_EQ(3u, cmdScene2.sceneActions.numberOfActions());
        EXPECT_EQ(expectedActionsScene2, cmdScene2.sceneActions);

        fixture.handleSceneActionList(sceneId1, createFakeSceneActionCollectionFromTypes({ ESceneActionId_AllocateRenderable }), 0u, providerID);
        EXPECT_EQ(1u, commands.getTotalCommandCount());

        fixture.handleSceneActionList(sceneId1, createFakeSceneActionCollectionFromTypes({ ESceneActionId_Flush }), 0u, providerID);
        ASSERT_EQ(2u, commands.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, commands.getCommandType(1u));
        const SceneActionsCommand& cmdScene1 = commands.getCommandData<SceneActionsCommand>(1u);
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
        rendererCommandBuffer.clear();

        fixture.participantHasDisconnected(providerID);

        const auto& commands = rendererCommandBuffer.getCommands();
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
        fixture.handleNewScenesAvailable(SceneInfoVector(1, SceneInfo(sceneId, sceneName)), Guid(true), EScenePublicationMode_LocalAndRemote);
        rendererCommandBuffer.clear();

        fixture.participantHasDisconnected(providerID);

        const auto& commands = rendererCommandBuffer.getCommands();
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

        RequesterID requester(1);

        EXPECT_CALL(resourceComponent, requestResourceAsynchronouslyFromFramework(resources, requester, providerID));
        fixture.requestResourceAsyncronouslyFromFramework(resources, requester, sceneId);
    }

    TEST_F(ARendererFrameworkLogic, cancelsResourceRequestViaResourceComponent)
    {
        const ResourceContentHash resource(44u, 0);

        RequesterID requester(1);

        EXPECT_CALL(resourceComponent, cancelResourceRequest(resource, requester));
        fixture.cancelResourceRequest(resource, requester);
    }

    TEST_F(ARendererFrameworkLogic, generatesUnsubscribeRendererCommandsWhenDetectingSceneActionListCounterMismatch)
    {
        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId, sceneName));
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);
        fixture.handleInitializeScene(SceneInfo(sceneId), providerID);
        rendererCommandBuffer.clear();

        // send one correct/normal list
        SceneActionCollection actions(createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode , ESceneActionId_Flush }));
        fixture.handleSceneActionList(sceneId, actions.copy(), 1u, providerID);

        const RendererCommandContainer& commandsAfterNormalList = rendererCommandBuffer.getCommands();
        ASSERT_EQ(1u, commandsAfterNormalList.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, commandsAfterNormalList.getCommandType(0u));

        // send list with mismatched counter value
        fixture.handleSceneActionList(sceneId, actions.copy(), 3u, providerID);

        const auto& commandsAfterMismatch = rendererCommandBuffer.getCommands();
        EXPECT_EQ(ERendererCommand_UnsubscribeScene, commandsAfterMismatch.getCommandType(commandsAfterMismatch.getTotalCommandCount()-1));
        EXPECT_TRUE(commandsAfterMismatch.getCommandData<ramses_internal::SceneInfoCommand>(commandsAfterMismatch.getTotalCommandCount() - 1).indirect);
    }

    TEST_F(ARendererFrameworkLogic, expectsCountingSceneActionsFromStartAfterInitializeScene)
    {
        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId, sceneName));
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);
        fixture.handleInitializeScene(SceneInfo(sceneId), providerID);

        rendererCommandBuffer.clear();

        // send one correct/normal list
        SceneActionCollection actions(createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode , ESceneActionId_Flush }));
        fixture.handleSceneActionList(sceneId, actions.copy(), 1u, providerID);

        const RendererCommandContainer& commandsAfterFirstList = rendererCommandBuffer.getCommands();
        ASSERT_EQ(1u, commandsAfterFirstList.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, commandsAfterFirstList.getCommandType(0u));

        fixture.handleInitializeScene(SceneInfo(sceneId), providerID);
        rendererCommandBuffer.clear();
        fixture.handleSceneActionList(sceneId, actions.copy(), 1u, providerID);

        const RendererCommandContainer& commandsAfterSecondList = rendererCommandBuffer.getCommands();
        ASSERT_EQ(1u, commandsAfterSecondList.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, commandsAfterSecondList.getCommandType(0u));
    }

    TEST_F(ARendererFrameworkLogic, expectsCountingSceneActionsFromStartAfterRepublish)
    {
        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId, sceneName));
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);
        fixture.handleInitializeScene(SceneInfo(sceneId), providerID);

        rendererCommandBuffer.clear();

        // send one correct/normal list
        SceneActionCollection actions(createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode , ESceneActionId_Flush }));
        fixture.handleSceneActionList(sceneId, actions.copy(), 1u, providerID);

        const RendererCommandContainer& commandsAfterFirstList = rendererCommandBuffer.getCommands();
        ASSERT_EQ(1u, commandsAfterFirstList.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, commandsAfterFirstList.getCommandType(0u));

        fixture.handleScenesBecameUnavailable({ SceneInfo(sceneId) }, providerID);
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);
        fixture.handleInitializeScene(SceneInfo(sceneId), providerID);

        rendererCommandBuffer.clear();
        fixture.handleSceneActionList(sceneId, actions.copy(), 1u, providerID);

        const RendererCommandContainer& commandsAfterSecondList = rendererCommandBuffer.getCommands();
        ASSERT_EQ(1u, commandsAfterSecondList.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, commandsAfterSecondList.getCommandType(0u));
    }


    TEST_F(ARendererFrameworkLogic, generatesUnsubscribeRendererCommandsWhenContinueCountingSceneActionListAfterInitializeScene)
    {
        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId, sceneName));
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);
        fixture.handleInitializeScene(SceneInfo(sceneId), providerID);
        rendererCommandBuffer.clear();

        // send one correct/normal list
        SceneActionCollection actions(createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode , ESceneActionId_Flush }));
        fixture.handleSceneActionList(sceneId, actions.copy(), 1u, providerID);

        const RendererCommandContainer& commandsAfterNormalList = rendererCommandBuffer.getCommands();
        ASSERT_EQ(1u, commandsAfterNormalList.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, commandsAfterNormalList.getCommandType(0u));

        fixture.handleInitializeScene(SceneInfo(sceneId), providerID);
        rendererCommandBuffer.clear();

        // send list with mismatched counter value
        fixture.handleSceneActionList(sceneId, actions.copy(), 2u, providerID);

        const auto& commandsAfterMismatch = rendererCommandBuffer.getCommands();
        EXPECT_EQ(ERendererCommand_UnsubscribeScene, commandsAfterMismatch.getCommandType(commandsAfterMismatch.getTotalCommandCount()-1));
        EXPECT_TRUE(commandsAfterMismatch.getCommandData<ramses_internal::SceneInfoCommand>(commandsAfterMismatch.getTotalCommandCount() - 1).indirect);
    }

    TEST_F(ARendererFrameworkLogic, generatesUnpublishRendererCommandsWhenParticipantDisconnectAndStartsAgainWithNewCounter)
    {
        fixture.newParticipantHasConnected(providerID);

        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId, sceneName));
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);
        fixture.handleInitializeScene(SceneInfo(sceneId), providerID);
        rendererCommandBuffer.clear();

        // send one correct/normal list
        SceneActionCollection actions(createFakeSceneActionCollectionFromTypes({ ESceneActionId_AddChildToNode , ESceneActionId_Flush }));
        fixture.handleSceneActionList(sceneId, actions.copy(), 1u, providerID);

        const RendererCommandContainer& commandsAfterNormalList = rendererCommandBuffer.getCommands();
        ASSERT_EQ(1u, commandsAfterNormalList.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, commandsAfterNormalList.getCommandType(0u));

        rendererCommandBuffer.clear();
        fixture.participantHasDisconnected(providerID);
        const RendererCommandContainer& commandsAfterDisconnect = rendererCommandBuffer.getCommands();
        ASSERT_EQ(1u, commandsAfterDisconnect.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_UnpublishedScene, commandsAfterDisconnect.getCommandType(0u));

        fixture.newParticipantHasConnected(providerID);
        fixture.handleNewScenesAvailable(newScenes, providerID, EScenePublicationMode_LocalAndRemote);
        fixture.handleInitializeScene(SceneInfo(sceneId), providerID);
        rendererCommandBuffer.clear();

        fixture.handleSceneActionList(sceneId, actions.copy(), 1u, providerID);
        const RendererCommandContainer& commandsAfterReInitialize = rendererCommandBuffer.getCommands();
        ASSERT_EQ(1u, commandsAfterReInitialize.getTotalCommandCount());
        EXPECT_EQ(ERendererCommand_SceneActions, commandsAfterReInitialize.getCommandType(0u));
    }
}
