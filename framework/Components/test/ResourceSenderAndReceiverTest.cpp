//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AbstractSenderAndReceiverTest.h"
#include "ServiceHandlerMocks.h"
#include "SceneUpdateSerializerTestHelper.h"

namespace ramses_internal
{
    using namespace testing;

    class AResourceSenderAndReceiverTest : public AbstractSenderAndReceiverTest
    {
    public:
        AResourceSenderAndReceiverTest()
            : AbstractSenderAndReceiverTest(EServiceType::Ramses)
        {
            receiver.setResourceConsumerServiceHandler(&handler);
            receiver.setResourceProviderServiceHandler(&providerHandler);
        }

    protected:
        StrictMock<ResourceConsumerServiceHandlerMock> handler;
        StrictMock<ResourceProviderServiceHandlerMock> providerHandler;
    };

    INSTANTIATE_TEST_SUITE_P(TypedCommunicationTest, AResourceSenderAndReceiverTest,
                            ::testing::ValuesIn(CommunicationSystemTestState::GetAvailableCommunicationSystemTypes()));

    TEST_P(AResourceSenderAndReceiverTest, SendResourceNotAvailable)
    {
        ResourceContentHashVector resourceHashes;
        resourceHashes.push_back(ResourceContentHash(123u, 0));

        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(handler, handleResourcesNotAvailable(_, senderId))
                .WillOnce([&](const auto& receivedResourceHashes, auto) {
                              EXPECT_EQ(resourceHashes, receivedResourceHashes);
                              sendEvent();
                          });
        }

        EXPECT_TRUE(sender.sendResourcesNotAvailable(receiverId, resourceHashes));
        ASSERT_TRUE(waitForEvent());
    }

    TEST_P(AResourceSenderAndReceiverTest, sendResourceData)
    {
        skipStatisticsTest();

        const std::vector<Byte> blob_1({1, 2, 3, 4, 5, 6});
        const std::vector<Byte> blob_2({255, 254, 11, 3});

        {
            PlatformGuard g(receiverExpectCallLock);
            InSequence    seq;
            EXPECT_CALL(handler, handleSendResource(_, senderId)).WillOnce([&](const auto& data, auto) {
                EXPECT_EQ(blob_1, data);
                sendEvent();
            });
            EXPECT_CALL(handler, handleSendResource(_, senderId)).WillOnce([&](const auto& data, auto) {
                EXPECT_EQ(blob_2, data);
                sendEvent();
            });
        }

        FakseSceneUpdateSerializer serializer({blob_1, blob_2}, 300000);
        EXPECT_TRUE(sender.sendResources(receiverId, serializer));
        ASSERT_TRUE(waitForEvent(2));
    }
}
