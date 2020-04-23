//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AbstractSenderAndReceiverTest.h"
#include "ServiceHandlerMocks.h"
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

    INSTANTIATE_TEST_SUITE_P(TypedCommunicationTest, ADcsmSenderAndReceiverTest,
                            ::testing::ValuesIn(CommunicationSystemTestState::GetAvailableCommunicationSystemTypes()));

    TEST_P(ADcsmSenderAndReceiverTest, broadcastOfferContent)
    {
        ContentID contentID(987);
        Category category(567);
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleOfferContent(contentID, category, senderId)).WillOnce(InvokeWithoutArgs([&]{ sendEvent(); }));
        }
        EXPECT_TRUE(sender.sendDcsmBroadcastOfferContent(contentID, category));
        ASSERT_TRUE(waitForEvent());
    }

    TEST_P(ADcsmSenderAndReceiverTest, sendOfferContent)
    {
        ContentID contentID(987);
        Category category(567);
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleOfferContent(contentID, category, senderId)).WillOnce(InvokeWithoutArgs([&]{ sendEvent(); }));
        }
        EXPECT_TRUE(sender.sendDcsmOfferContent(receiverId, contentID, category));
        ASSERT_TRUE(waitForEvent());
    }

    TEST_P(ADcsmSenderAndReceiverTest, sendContentDescription)
    {
        ContentID contentID(987);
        ETechnicalContentType techtype = ETechnicalContentType::RamsesSceneID;
        TechnicalContentDescriptor descriptor(123);
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleContentDescription(contentID, techtype, descriptor, senderId)).WillOnce(InvokeWithoutArgs([&]{ sendEvent(); }));
        }
        EXPECT_TRUE(sender.sendDcsmContentDescription(receiverId, contentID, techtype, descriptor));
        ASSERT_TRUE(waitForEvent());
    }

    TEST_P(ADcsmSenderAndReceiverTest, sendContentReady)
    {
        ContentID contentID(987);
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleContentReady(contentID, senderId)).WillOnce(InvokeWithoutArgs([&]{ sendEvent(); }));
        }
        EXPECT_TRUE(sender.sendDcsmContentReady(receiverId, contentID));
        ASSERT_TRUE(waitForEvent());
    }

    TEST_P(ADcsmSenderAndReceiverTest, sendBroadcastRequestStopOfferContent)
    {
        ContentID contentID(987);
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleRequestStopOfferContent(contentID, senderId)).WillOnce(InvokeWithoutArgs([&]{ sendEvent(); }));
        }
        EXPECT_TRUE(sender.sendDcsmBroadcastRequestStopOfferContent(contentID));
        ASSERT_TRUE(waitForEvent());
    }

    TEST_P(ADcsmSenderAndReceiverTest, sendBroadcastForceStopOfferContent)
    {
        ContentID contentID(987);
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleForceStopOfferContent(contentID, senderId)).WillOnce(InvokeWithoutArgs([&]{ sendEvent(); }));
        }
        EXPECT_TRUE(sender.sendDcsmBroadcastForceStopOfferContent(contentID));
        ASSERT_TRUE(waitForEvent());
    }

    TEST_P(ADcsmSenderAndReceiverTest, sendContentFocusRequest)
    {
        ContentID contentID(987);
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleContentFocusRequest(contentID, senderId)).WillOnce(InvokeWithoutArgs([&]{ sendEvent(); }));
        }
        EXPECT_TRUE(sender.sendDcsmContentFocusRequest(receiverId, contentID));
        ASSERT_TRUE(waitForEvent());
    }

    TEST_P(ADcsmSenderAndReceiverTest, sendUpdateContentMetadata)
    {
        ContentID contentID(987);
        DcsmMetadata dm;
        EXPECT_TRUE(dm.setPreviewDescription(U"hello world"));
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(consumerHandler, handleUpdateContentMetadata(contentID, dm, senderId)).WillOnce(InvokeWithoutArgs([&] { sendEvent(); }));
        }
        EXPECT_TRUE(sender.sendDcsmUpdateContentMetadata(receiverId, contentID, dm));
        ASSERT_TRUE(waitForEvent());
    }

    TEST_P(ADcsmSenderAndReceiverTest, sendCanvasSizeChange)
    {
        ContentID contentID(987);
        SizeInfo si{ 780, 480 };
        AnimationInformation ai{ 678,789 };
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(providerHandler, handleCanvasSizeChange(contentID, si, ai, senderId)).WillOnce(InvokeWithoutArgs([&]{ sendEvent(); }));
        }
        EXPECT_TRUE(sender.sendDcsmCanvasSizeChange(receiverId, contentID, si, ai));
        ASSERT_TRUE(waitForEvent());
    }

    TEST_P(ADcsmSenderAndReceiverTest, sendContentStateChange)
    {
        ContentID contentID(987);
        EDcsmState status(EDcsmState::Shown);
        SizeInfo si{123, 432};
        AnimationInformation ai{ 678,789 };
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(providerHandler, handleContentStateChange(contentID, status, si, ai, senderId)).WillOnce(InvokeWithoutArgs([&] { sendEvent(); }));
        }
        EXPECT_TRUE(sender.sendDcsmContentStateChange(receiverId, contentID, status, si, ai));
        ASSERT_TRUE(waitForEvent());
    }
}
