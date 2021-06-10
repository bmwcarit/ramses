//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/DcsmStatusMessage.h"
#include "DcsmStatusMessageImpl.h"
#include "gtest/gtest.h"
#include <memory>

namespace ramses
{
    TEST(DcsmStatusMessageTest, canGetSubclassEasily)
    {
        StreamStatusMessage msg(StreamStatusMessage::Status::Valid);
        DcsmStatusMessage& msgbase = msg;
        auto highptr = msgbase.getAsStreamStatus();
        EXPECT_TRUE(highptr);
    }

    // TODO: Add test that we can't get the wrong kind of derived class - as soon as we have another one

    TEST(DcsmStatusMessageTest, serializesAndMoreThanOnceDeserializesStreamStatusMessageProperly)
    {
        StreamStatusMessage msg(StreamStatusMessage::Status::Halted);
        EXPECT_EQ(StreamStatusMessage::Status::Halted, msg.getStreamStatus());
        EXPECT_EQ(StreamStatusMessage::Status::Halted, msg.getStreamStatus());
    }

    TEST(DcsmStatusMessageTest, createsStreamStatusMessageFromImplFromSerializedData)
    {
        StreamStatusMessage msg(StreamStatusMessage::Status::VideoResolutionChanged);
        auto impl = std::make_unique<DcsmStatusMessageImpl>(static_cast<uint64_t>(msg.impl->getType()), absl::Span<const ramses_internal::Byte>{ msg.impl->getData().data(), msg.impl->getData().size() });
        auto newMsg = DcsmStatusMessageImpl::CreateMessage(std::move(impl));
        ASSERT_TRUE(newMsg);
        auto highptr = newMsg->getAsStreamStatus();
        ASSERT_TRUE(highptr);

        EXPECT_EQ(StreamStatusMessage::Status::VideoResolutionChanged, highptr->getStreamStatus());
    }
}
