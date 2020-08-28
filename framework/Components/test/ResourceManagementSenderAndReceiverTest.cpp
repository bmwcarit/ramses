//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AbstractSenderAndReceiverTest.h"
#include "ServiceHandlerMocks.h"

namespace ramses_internal
{
    using namespace testing;

    class AResourceManagementSenderAndReceiverTest : public AbstractSenderAndReceiverTest
    {
    public:
        AResourceManagementSenderAndReceiverTest()
            : AbstractSenderAndReceiverTest(EServiceType::Ramses)
        {
            receiver.setResourceProviderServiceHandler(&handler);
        }

        StrictMock<ResourceProviderServiceHandlerMock> handler;
    };

    INSTANTIATE_TEST_SUITE_P(TypedCommunicationTest, AResourceManagementSenderAndReceiverTest,
                            ::testing::ValuesIn(CommunicationSystemTestState::GetAvailableCommunicationSystemTypes()));

    TEST_P(AResourceManagementSenderAndReceiverTest, RequestResources)
    {
        const ResourceContentHash hash1(1u, 0);
        const ResourceContentHash hash2(2u, 0);
        const ResourceContentHash hash3(3u, 0);
        ResourceContentHashVector requestedHashes;
        requestedHashes.push_back(hash1);
        requestedHashes.push_back(hash2);
        requestedHashes.push_back(hash3);

        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(handler, handleRequestResources(_, senderId)).WillOnce([&](const auto& ids, auto)
            {
                ASSERT_EQ(3u, ids.size());
                EXPECT_EQ(hash1, ids[0]);
                EXPECT_EQ(hash2, ids[1]);
                EXPECT_EQ(hash3, ids[2]);
                sendEvent();
            });
        }

        EXPECT_TRUE(sender.sendRequestResources(receiverId, requestedHashes));
        ASSERT_TRUE(waitForEvent());
    }
}
