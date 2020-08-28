//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportCommon/TransportUtilities.h"
#include "gmock/gmock.h"

namespace ramses_internal
{
    using namespace testing;

    class ATransportUtilities : public Test
    {
    public:
        ATransportUtilities()
            : callback([&](uint32_t start, uint32_t end) { return mock.sendFun(start, end); })
        {
        }

        struct TestMock
        {
            MOCK_METHOD(bool, sendFun, (uint32_t start, uint32_t end), ());
        };

        StrictMock<TestMock> mock;
        std::function<bool(uint32_t, uint32_t)> callback;
    };

    TEST_F(ATransportUtilities, canHandleZero)
    {
        EXPECT_TRUE(TransportUtilities::SplitToChunks(10, 0, callback));
    }

    TEST_F(ATransportUtilities, canHandleNonCHunking)
    {
        EXPECT_CALL(mock, sendFun(0, 1)).WillOnce(Return(true));
        EXPECT_TRUE(TransportUtilities::SplitToChunks(10, 1, callback));

        EXPECT_CALL(mock, sendFun(0, 5)).WillOnce(Return(true));
        EXPECT_TRUE(TransportUtilities::SplitToChunks(10, 5, callback));

        EXPECT_CALL(mock, sendFun(0, 10)).WillOnce(Return(true));
        EXPECT_TRUE(TransportUtilities::SplitToChunks(10, 10, callback));
    }

    TEST_F(ATransportUtilities, canDoChunking)
    {
        EXPECT_CALL(mock, sendFun(0, 4)).WillOnce(Return(true));
        EXPECT_CALL(mock, sendFun(4, 5)).WillOnce(Return(true));
        EXPECT_TRUE(TransportUtilities::SplitToChunks(4, 5, callback));

        EXPECT_CALL(mock, sendFun(0, 4)).WillOnce(Return(true));
        EXPECT_CALL(mock, sendFun(4, 8)).WillOnce(Return(true));
        EXPECT_TRUE(TransportUtilities::SplitToChunks(4, 8, callback));

        EXPECT_CALL(mock, sendFun(0, 4)).WillOnce(Return(true));
        EXPECT_CALL(mock, sendFun(4, 8)).WillOnce(Return(true));
        EXPECT_CALL(mock, sendFun(8, 9)).WillOnce(Return(true));
        EXPECT_TRUE(TransportUtilities::SplitToChunks(4, 9, callback));
    }

    TEST_F(ATransportUtilities, passesThroughErrorOnFirstCall)
    {
        EXPECT_CALL(mock, sendFun(0, 4)).WillOnce(Return(false));
        EXPECT_FALSE(TransportUtilities::SplitToChunks(4, 5, callback));
    }

    TEST_F(ATransportUtilities, passesThroughErrorOnLaterCall)
    {
        EXPECT_CALL(mock, sendFun(0, 4)).WillOnce(Return(true));
        EXPECT_CALL(mock, sendFun(4, 8)).WillOnce(Return(false));
        EXPECT_FALSE(TransportUtilities::SplitToChunks(4, 9, callback));
    }
}
