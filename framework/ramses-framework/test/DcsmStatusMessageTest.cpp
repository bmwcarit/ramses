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
    TEST(DcsmStatusMessageTest, getAsStreamStatusFromBase)
    {
        StreamStatusMessage msg(StreamStatusMessage::Status::Valid);
        DcsmStatusMessage& msgbase = msg;
        EXPECT_EQ(&msg, msgbase.getAsStreamStatus());
        EXPECT_EQ(nullptr, msgbase.getAsActiveLayout());
        EXPECT_EQ(nullptr, msgbase.getAsWidgetFocusStatus());
    }

    TEST(DcsmStatusMessageTest, getAsActiveLayoutFromBase)
    {
        ActiveLayoutMessage msg(ActiveLayoutMessage::Layout::Sport_Road);
        DcsmStatusMessage& msgbase = msg;
        EXPECT_EQ(nullptr, msgbase.getAsStreamStatus());
        EXPECT_EQ(&msg, msgbase.getAsActiveLayout());
        EXPECT_EQ(nullptr, msgbase.getAsWidgetFocusStatus());
    }

    TEST(DcsmStatusMessageTest, getAsWidgetFocusStatusFromBase)
    {
        WidgetFocusStatusMessage msg(WidgetFocusStatusMessage::Status::NotFocused);
        DcsmStatusMessage& msgbase = msg;
        EXPECT_EQ(nullptr, msgbase.getAsStreamStatus());
        EXPECT_EQ(nullptr, msgbase.getAsActiveLayout());
        EXPECT_EQ(&msg, msgbase.getAsWidgetFocusStatus());
    }

    TEST(DcsmStatusMessageTest, serializesAndMoreThanOnceDeserializesStreamStatusMessageProperly)
    {
        StreamStatusMessage msg(StreamStatusMessage::Status::Halted);
        EXPECT_EQ(StreamStatusMessage::Status::Halted, msg.getStreamStatus());
        EXPECT_EQ(StreamStatusMessage::Status::Halted, msg.getStreamStatus());
    }

    TEST(DcsmStatusMessageTest, serializesAndMoreThanOnceDeserializesActiveLayoutMessageProperly)
    {
        ActiveLayoutMessage msg(ActiveLayoutMessage::Layout::Focus);
        EXPECT_EQ(ActiveLayoutMessage::Layout::Focus, msg.getLayout());
        EXPECT_EQ(ActiveLayoutMessage::Layout::Focus, msg.getLayout());
    }

    TEST(DcsmStatusMessageTest, serializesAndMoreThanOnceDeserializesWidgetFocusStatusMessageProperly)
    {
        WidgetFocusStatusMessage msg(WidgetFocusStatusMessage::Status::Focused);
        EXPECT_EQ(WidgetFocusStatusMessage::Status::Focused, msg.getWidgetFocusStatus());
        EXPECT_EQ(WidgetFocusStatusMessage::Status::Focused, msg.getWidgetFocusStatus());
    }

    TEST(DcsmStatusMessageTest, createsStreamStatusMessageFromImplFromSerializedData)
    {
        StreamStatusMessage msg(StreamStatusMessage::Status::VideoResolutionChanged);
        auto impl = std::make_unique<DcsmStatusMessageImpl>(msg.impl->getType(), absl::Span<const ramses_internal::Byte>{ msg.impl->getData().data(), msg.impl->getData().size() });
        auto newMsg = DcsmStatusMessageImpl::CreateMessage(std::move(impl));
        ASSERT_TRUE(newMsg);
        auto highptr = newMsg->getAsStreamStatus();
        ASSERT_TRUE(highptr);

        EXPECT_EQ(StreamStatusMessage::Status::VideoResolutionChanged, highptr->getStreamStatus());
    }

    TEST(DcsmStatusMessageTest, createsActiveLayoutMessageFromImplFromSerializedData)
    {
        ActiveLayoutMessage msg(ActiveLayoutMessage::Layout::Sport_Track);
        auto impl = std::make_unique<DcsmStatusMessageImpl>(msg.impl->getType(), absl::Span<const ramses_internal::Byte>{ msg.impl->getData().data(), msg.impl->getData().size() });
        auto newMsg = DcsmStatusMessageImpl::CreateMessage(std::move(impl));
        ASSERT_TRUE(newMsg);
        auto highptr = newMsg->getAsActiveLayout();
        ASSERT_TRUE(highptr);

        EXPECT_EQ(ActiveLayoutMessage::Layout::Sport_Track, highptr->getLayout());
    }

    TEST(DcsmStatusMessageTest, createsWidgetFocusStatusMessageFromImplFromSerializedData)
    {
        WidgetFocusStatusMessage msg(WidgetFocusStatusMessage::Status::NotFocused);
        auto impl = std::make_unique<DcsmStatusMessageImpl>(msg.impl->getType(), absl::Span<const ramses_internal::Byte>{ msg.impl->getData().data(), msg.impl->getData().size() });
        auto newMsg = DcsmStatusMessageImpl::CreateMessage(std::move(impl));
        ASSERT_TRUE(newMsg);
        auto highptr = newMsg->getAsWidgetFocusStatus();
        ASSERT_TRUE(highptr);

        EXPECT_EQ(WidgetFocusStatusMessage::Status::NotFocused, highptr->getWidgetFocusStatus());
    }

    TEST(DcsmStatusMessageTest, formatStreamStatusMessage)
    {
        StreamStatusMessage msg(StreamStatusMessage::Status::VideoResolutionChanged);
        EXPECT_EQ("[StreamStatus: 7,0,0,0]", fmt::to_string(*msg.impl));
    }

    TEST(DcsmStatusMessageTest, formatActiveLayoutMessage)
    {
        ActiveLayoutMessage msg(ActiveLayoutMessage::Layout::Focus);
        EXPECT_EQ("[ActiveLayout: 1,0,0,0]", fmt::to_string(*msg.impl));
    }

    TEST(DcsmStatusMessageTest, formatWidgetFocusStatusMessage)
    {
        WidgetFocusStatusMessage msg(WidgetFocusStatusMessage::Status::NotFocused);
        EXPECT_EQ("[WidgetFocusStatus: 2,0,0,0]", fmt::to_string(*msg.impl));
    }

    TEST(DcsmStatusMessageTest, formatLargeActiveLayoutMessage)
    {
        std::vector<ramses_internal::Byte> vec(32, 7);
        DcsmStatusMessageImpl msg(DcsmStatusMessageImpl::Type::ActiveLayout, vec);
        EXPECT_EQ("[ActiveLayout: <size:32>]", fmt::to_string(msg));
    }
}
