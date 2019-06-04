//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AbstractSenderAndReceiverTest.h"
#include "ServiceHandlerMocks.h"
#include "SenderAndReceiverTestUtils.h"
#include "Scene/SceneActionCollectionCreator.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "SceneActionCollectionTestHelpers.h"

namespace ramses_internal
{
    using namespace testing;

    class ASceneGraphProtocolSenderAndReceiverTest : public AbstractSenderAndReceiverTest
    {
    public:
        ASceneGraphProtocolSenderAndReceiverTest()
            : AbstractSenderAndReceiverTest(EServiceType::Ramses)
        {
            receiver.setSceneRendererServiceHandler(&consumerHandler);
            receiver.setSceneProviderServiceHandler(&providerHandler);
        }

        StrictMock<SceneRendererServiceHandlerMock> consumerHandler;
        StrictMock<SceneProviderServiceHandlerMock> providerHandler;
    };

    INSTANTIATE_TEST_CASE_P(TypedCommunicationTest, ASceneGraphProtocolSenderAndReceiverTest,
                            ::testing::ValuesIn(CommunicationSystemTestState::GetAvailableCommunicationSystemTypes()));

    TEST_P(ASceneGraphProtocolSenderAndReceiverTest, broadcastNewScenesAvailable)
    {
        uint32_t numberMessagesSent = m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue();
        uint32_t numberMessagesReceived = m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue();

        const SceneId sceneId(55u);
        const String name("sceneName");
        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(sceneId, name));

        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleNewScenesAvailable(newScenes, senderId, EScenePublicationMode_LocalAndRemote)).WillOnce(SendHandlerCalledEvent(this));
        }
        EXPECT_TRUE(sender.broadcastNewScenesAvailable(newScenes));
        ASSERT_TRUE(waitForEvent());

        EXPECT_LE(numberMessagesSent + 1, m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue());
        EXPECT_LE(numberMessagesReceived + 1, m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue());
    }

    TEST_P(ASceneGraphProtocolSenderAndReceiverTest, sendInitializeScene)
    {
        uint32_t numberMessagesSent = m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue();
        uint32_t numberMessagesReceived = m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue();

        const String name("test");
        const SceneId sceneId(1ull << 63);
        SceneInfo sceneInfo(sceneId, name);

        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleInitializeScene(sceneInfo, senderId)).WillOnce(SendHandlerCalledEvent(this));
        }
        EXPECT_TRUE(sender.sendInitializeScene(receiverId, sceneInfo));
        ASSERT_TRUE(waitForEvent());

        EXPECT_LE(numberMessagesSent + 1, m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue());
        EXPECT_LE(numberMessagesReceived + 1, m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue());
    }

    TEST_P(ASceneGraphProtocolSenderAndReceiverTest, DISABLED_broadcastScenesBecameUnavailable)
    {
        uint32_t numberMessagesSent = m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue();
        uint32_t numberMessagesReceived = m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue();

        const SceneId sceneId(1ull << 63);
        SceneInfoVector unavailableScenes;
        unavailableScenes.push_back(SceneInfo(sceneId));

        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleScenesBecameUnavailable(unavailableScenes, senderId)).WillRepeatedly(SendHandlerCalledEvent(this));
        }

        // TODO(tobias) work around gtest versions that don't allow copying of AssertionResult
        int i = 15;
        do {
            EXPECT_TRUE(sender.broadcastScenesBecameUnavailable(unavailableScenes));
            testing::AssertionResult result{waitForEvent(1, 200)};
            if (result)
            {
                break;
            }
            --i;
            if (i == 0)
            {
                ASSERT_TRUE(result);
            }
        } while (i > 0);

        EXPECT_LE(numberMessagesSent + 1, m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue());
        EXPECT_LE(numberMessagesReceived + 1, m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue());
    }

    TEST_P(ASceneGraphProtocolSenderAndReceiverTest, sendSceneActionList)
    {
        uint32_t numberMessagesSent = m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue();
        uint32_t numberMessagesReceived = m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue();

        const SceneId sceneId(1ull << 63);
        SceneActionCollection actions;
        SceneActionCollectionCreator creator(actions);
        creator.allocateNode(0u, NodeHandle(123u));
        creator.allocateRenderable(NodeHandle(123u), RenderableHandle(456u));

        SceneActionCollection receivedActions;
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleSceneActionList_rvr(sceneId, _, _, senderId)).WillOnce(DoAll(WithArgs<1>(INVOKE_SAVE_SCENEACTIONCOLLECTION(receivedActions)), SendHandlerCalledEvent(this)));
        }
        const uint64_t numberOfChunksSent = sender.sendSceneActionList(receiverId, sceneId, actions, 0u);
        EXPECT_EQ(1u, numberOfChunksSent);
        ASSERT_TRUE(waitForEvent());

        ASSERT_EQ(2u, receivedActions.numberOfActions());
        EXPECT_EQ(ESceneActionId_AllocateNode, receivedActions[0].type());
        EXPECT_EQ(ESceneActionId_AllocateRenderable, receivedActions[1].type());

        EXPECT_LE(numberMessagesSent + 1, m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue());
        EXPECT_LE(numberMessagesReceived + 1, m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue());
    }

    TEST_P(ASceneGraphProtocolSenderAndReceiverTest, sendScenesAvailable)
    {
        uint32_t numberMessagesSent = m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue();
        uint32_t numberMessagesReceived = m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue();

        SceneInfoVector newScenes;
        newScenes.push_back(SceneInfo(SceneId(55u), "sceneName"));

        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleNewScenesAvailable(newScenes, senderId, EScenePublicationMode_LocalAndRemote)).WillOnce(SendHandlerCalledEvent(this));
        }
        EXPECT_TRUE(sender.sendScenesAvailable(receiverId, newScenes));
        ASSERT_TRUE(waitForEvent());

        EXPECT_LE(numberMessagesSent + 1, m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue());
        EXPECT_LE(numberMessagesReceived + 1, m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue());
    }

    TEST_P(ASceneGraphProtocolSenderAndReceiverTest, sendSubscribeScene)
    {
        uint32_t numberMessagesSent = m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue();
        uint32_t numberMessagesReceived = m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue();

        SceneId sceneId;
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(providerHandler, handleSubscribeScene(sceneId, senderId)).WillOnce(SendHandlerCalledEvent(this));
        }
        EXPECT_TRUE(sender.sendSubscribeScene(receiverId, sceneId));
        ASSERT_TRUE(waitForEvent());

        EXPECT_LE(numberMessagesSent + 1, m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue());
        EXPECT_LE(numberMessagesReceived + 1, m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue());
    }

    TEST_P(ASceneGraphProtocolSenderAndReceiverTest, sendUnsubscribeScene)
    {
        uint32_t numberMessagesSent = m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue();
        uint32_t numberMessagesReceived = m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue();

        SceneId sceneId;
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(providerHandler, handleUnsubscribeScene(sceneId, senderId)).WillOnce(SendHandlerCalledEvent(this));
        }
        EXPECT_TRUE(sender.sendUnsubscribeScene(receiverId, sceneId));
        ASSERT_TRUE(waitForEvent());

        EXPECT_LE(numberMessagesSent + 1, m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue());
        EXPECT_LE(numberMessagesReceived + 1, m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue());
    }

    TEST_P(ASceneGraphProtocolSenderAndReceiverTest, sendSceneNotAvailable)
    {
        uint32_t numberMessagesSent = m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue();
        uint32_t numberMessagesReceived = m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue();

        SceneId sceneId;
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleSceneNotAvailable(sceneId, senderId)).WillOnce(SendHandlerCalledEvent(this));
        }
        EXPECT_TRUE(sender.sendSceneNotAvailable(receiverId, sceneId));
        ASSERT_TRUE(waitForEvent());

        EXPECT_LE(numberMessagesSent + 1, m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue());
        EXPECT_LE(numberMessagesReceived + 1, m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue());
    }

    TEST_P(ASceneGraphProtocolSenderAndReceiverTest, sendSceneActionListIsSplitUpIfMaxNumberIsExceeded)
    {
        const CommunicationSendDataSizes sendDataSizes = sender.getSendDataSizes();
        const UInt32 expectedNumberOfMessages = sendDataSizes.sceneActionNumber == std::numeric_limits<UInt32>::max() ? 1u : 2u;
        const UInt32 numberOfSceneActions     = sendDataSizes.sceneActionNumber == std::numeric_limits<UInt32>::max() ? 5u : sendDataSizes.sceneActionNumber + 1u;
        const SceneId sceneId(1ull << 63);

        SceneActionCollection actions;
        SceneActionCollectionCreator creator(actions);

        for(UInt32 i = 0u; i < numberOfSceneActions; ++i)
        {
            creator.allocateNode(0u, NodeHandle(123u + i));
        }

        std::vector<SceneActionCollection> receivedActionVectors;
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleSceneActionList_rvr(sceneId, _, _, senderId)).Times(expectedNumberOfMessages).WillRepeatedly(DoAll(WithArgs<1>(INVOKE_APPEND_SCENEACTIONCOLLECTION(receivedActionVectors)), SendHandlerCalledEvent(this)));
        }
        const uint64_t numberOfChunksSent = sender.sendSceneActionList(receiverId, sceneId, actions, 0u);
        EXPECT_EQ(expectedNumberOfMessages, numberOfChunksSent);
        ASSERT_TRUE(waitForEvent(expectedNumberOfMessages));

        SceneActionCollection receivedActions;
        for(const auto& recvSceneActions : receivedActionVectors)
        {
            EXPECT_LE(recvSceneActions.numberOfActions(), sendDataSizes.sceneActionNumber);
            receivedActions.append(recvSceneActions);
        }
        ASSERT_EQ(numberOfSceneActions, receivedActions.numberOfActions());
        EXPECT_EQ(actions, receivedActions);
    }

    TEST_P(ASceneGraphProtocolSenderAndReceiverTest, sendSceneActionListIsSplitUpIfMaxSizeIsExceeded)
    {
        CommunicationSendDataSizes sendDataSizes = sender.getSendDataSizes();

        if (sendDataSizes.sceneActionDataArray == std::numeric_limits<UInt32>::max())
        {
            sendDataSizes.sceneActionDataArray = 512u;
            sender.setSendDataSizes(sendDataSizes);
        }

        const UInt32 expectedNumberOfMessages = 2u;
        const UInt32 sizeOfSceneActionData    = sendDataSizes.sceneActionDataArray /4;
        const UInt32 numberOfSceneActions     = 5u;
        const SceneId sceneId(1ull << 63);

        ASSERT_LE(numberOfSceneActions, sendDataSizes.sceneActionNumber);

        std::unique_ptr<Byte[]> data(new Byte[sizeOfSceneActionData]);

        SceneActionCollection actions;
        SceneActionCollectionCreator creator(actions);
        for(UInt32 i = 0u; i < numberOfSceneActions; ++i)
        {
            creator.allocateNode(0u, NodeHandle(123u + i));
            PlatformMemory::Set(data.get(), static_cast<UInt8>(i), sizeOfSceneActionData);
            actions.write(data.get(), sizeOfSceneActionData);
        }

        std::vector<SceneActionCollection> receivedActionVectors;
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleSceneActionList_rvr(sceneId, _, _, senderId)).Times(expectedNumberOfMessages).WillRepeatedly(DoAll(WithArgs<1>(INVOKE_APPEND_SCENEACTIONCOLLECTION(receivedActionVectors)), SendHandlerCalledEvent(this)));
        }
        const uint64_t numberOfChunksSent = sender.sendSceneActionList(receiverId, sceneId, actions, 0u);
        EXPECT_EQ(expectedNumberOfMessages, numberOfChunksSent);
        ASSERT_TRUE(waitForEvent(expectedNumberOfMessages));

        SceneActionCollection receivedActions;
        for (const auto& recvSceneActions : receivedActionVectors)
        {
            EXPECT_LE(recvSceneActions.collectionData().size(), sendDataSizes.sceneActionDataArray);
            EXPECT_LE(recvSceneActions.numberOfActions(), sendDataSizes.sceneActionNumber);
            receivedActions.append(recvSceneActions);
        }
        ASSERT_EQ(numberOfSceneActions, receivedActions.numberOfActions());
        EXPECT_EQ(actions, receivedActions);
    }

    TEST_P(ASceneGraphProtocolSenderAndReceiverTest, SendCorrectCounterValuesViaAllCommunicationSystems)
    {
        const SceneId sceneId(1ull << 63);
        SceneActionCollection actions;
        SceneActionCollectionCreator creator(actions);
        creator.allocateNode(0u, NodeHandle(123u));

        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleSceneActionList_rvr(sceneId, _, 15u, senderId)).WillOnce(SendHandlerCalledEvent(this));
        }
        EXPECT_TRUE(sender.sendSceneActionList(receiverId, sceneId, actions, 15u) > 0);
        ASSERT_TRUE(waitForEvent());


        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleSceneActionList_rvr(sceneId, _, 59u, senderId)).WillOnce(SendHandlerCalledEvent(this));
        }
        sender.sendSceneActionList(receiverId, sceneId, actions, 59);
        ASSERT_TRUE(waitForEvent());
    }
}
