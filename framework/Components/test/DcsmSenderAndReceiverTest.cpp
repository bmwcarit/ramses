//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AbstractSenderAndReceiverTest.h"
#include "ServiceHandlerMocks.h"
#include "SenderAndReceiverTestUtils.h"
#include "PlatformAbstraction/PlatformThread.h"

namespace ramses_internal
{
    using namespace testing;

    class ADcsmSenderAndReceiverTest : public AbstractSenderAndReceiverTest
    {
    public:
        ADcsmSenderAndReceiverTest()
            : AbstractSenderAndReceiverTest(EServiceType::Dcsm)
        {
            receiver.setDcsmConsumerServiceHandler(&consumerHandler);
            receiver.setDcsmProviderServiceHandler(&providerHandler);
        }
        StrictMock<DcsmConsumerServiceHandlerMock> consumerHandler;
        StrictMock<DcsmProviderServiceHandlerMock> providerHandler;
    };

    //TODO(tobias) change to run on all communication system once someip stubs for dcsm are in
    INSTANTIATE_TEST_CASE_P(TypedCommunicationTest, ADcsmSenderAndReceiverTest,
                            ::testing::ValuesIn(CommunicationSystemTestState::GetAvailableCommunicationSystemTypes(ECommunicationSystemType_Tcp)));

    TEST_P(ADcsmSenderAndReceiverTest, broadcastRegisterContent)
    {
        uint32_t numberMessagesSent = m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue();
        uint32_t numberMessagesReceived = m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue();

        ContentID contentID(987);
        Category category(567);
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleRegisterContent(contentID, category, senderId)).WillOnce(SendHandlerCalledEvent(this));
        }
        EXPECT_TRUE(sender.sendDcsmBroadcastRegisterContent(contentID, category));
        ASSERT_TRUE(waitForEvent());

        EXPECT_LE(numberMessagesSent + 1, m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue());
        EXPECT_LE(numberMessagesReceived + 1, m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue());
    }

    TEST_P(ADcsmSenderAndReceiverTest, sendRegisterContent)
    {
        uint32_t numberMessagesSent = m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue();
        uint32_t numberMessagesReceived = m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue();

        ContentID contentID(987);
        Category category(567);
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleRegisterContent(contentID, category, senderId)).WillOnce(SendHandlerCalledEvent(this));
        }
        EXPECT_TRUE(sender.sendDcsmRegisterContent(receiverId, contentID, category));
        ASSERT_TRUE(waitForEvent());

        EXPECT_LE(numberMessagesSent + 1, m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue());
        EXPECT_LE(numberMessagesReceived + 1, m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue());
    }

    TEST_P(ADcsmSenderAndReceiverTest, sendContentAvailable)
    {
        uint32_t numberMessagesSent = m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue();
        uint32_t numberMessagesReceived = m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue();

        ContentID contentID(987);
        ETechnicalContentType techtype = ETechnicalContentType::RamsesSceneID;
        TechnicalContentDescriptor descriptor(123);
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleContentAvailable(contentID, techtype, descriptor, senderId)).WillOnce(SendHandlerCalledEvent(this));
        }
        EXPECT_TRUE(sender.sendDcsmContentAvailable(receiverId, contentID, techtype, descriptor));
        ASSERT_TRUE(waitForEvent());

        EXPECT_LE(numberMessagesSent + 1, m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue());
        EXPECT_LE(numberMessagesReceived + 1, m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue());
    }

    TEST_P(ADcsmSenderAndReceiverTest, sendRequestUnregister)
    {
        uint32_t numberMessagesSent = m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue();
        uint32_t numberMessagesReceived = m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue();

        ContentID contentID(987);
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleRequestUnregisterContent(contentID, senderId)).WillOnce(SendHandlerCalledEvent(this));
        }
        EXPECT_TRUE(sender.sendDcsmBroadcastRequestUnregisterContent(contentID));
        ASSERT_TRUE(waitForEvent());

        EXPECT_LE(numberMessagesSent + 1, m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue());
        EXPECT_LE(numberMessagesReceived + 1, m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue());
    }

    TEST_P(ADcsmSenderAndReceiverTest, sendCategoryContentSwitchRequest)
    {
        uint32_t numberMessagesSent = m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue();
        uint32_t numberMessagesReceived = m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue();

        ContentID contentID(987);
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleCategoryContentSwitchRequest(contentID, senderId)).WillOnce(SendHandlerCalledEvent(this));
        }
        EXPECT_TRUE(sender.sendDcsmCategoryContentSwitchRequest(receiverId, contentID));
        ASSERT_TRUE(waitForEvent());

        EXPECT_LE(numberMessagesSent + 1, m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue());
        EXPECT_LE(numberMessagesReceived + 1, m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue());
    }

    TEST_P(ADcsmSenderAndReceiverTest, sendCanvasSizeChange)
    {
        uint32_t numberMessagesSent = m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue();
        uint32_t numberMessagesReceived = m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue();

        ContentID contentID(987);
        SizeInfo si{ 780, 480 };
        AnimationInformation ai{ 678,789 };
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(providerHandler, handleCanvasSizeChange(contentID, si, ai, senderId)).WillOnce(SendHandlerCalledEvent(this));
        }
        EXPECT_TRUE(sender.sendDcsmCanvasSizeChange(receiverId, contentID, si, ai));
        ASSERT_TRUE(waitForEvent());

        EXPECT_LE(numberMessagesSent + 1, m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue());
        EXPECT_LE(numberMessagesReceived + 1, m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue());
    }

    TEST_P(ADcsmSenderAndReceiverTest, sendContentStatusChange)
    {
        uint32_t numberMessagesSent = m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue();
        uint32_t numberMessagesReceived = m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue();

        ContentID contentID(987);
        EDcsmStatus status(EDcsmStatus::Shown);
        AnimationInformation ai{ 678,789 };
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(providerHandler, handleContentStatusChange(contentID, status, ai, senderId)).WillOnce(SendHandlerCalledEvent(this));
        }
        EXPECT_TRUE(sender.sendDcsmContentStatusChange(receiverId, contentID, status, ai));
        ASSERT_TRUE(waitForEvent());

        EXPECT_LE(numberMessagesSent + 1, m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue());
        EXPECT_LE(numberMessagesReceived + 1, m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue());
    }
}
