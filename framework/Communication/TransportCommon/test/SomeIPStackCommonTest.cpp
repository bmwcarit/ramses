//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportCommon/SomeIPStackCommon.h"
#include "gtest/gtest.h"

namespace ramses_internal
{
    TEST(ASomeIPMsgHeader, canCompareEqual)
    {
        EXPECT_TRUE(SomeIPMsgHeader({1, 2, 3}) == SomeIPMsgHeader({1, 2, 3}));
        EXPECT_TRUE(SomeIPMsgHeader({0, 0, 0}) == SomeIPMsgHeader({0, 0, 0}));
        EXPECT_FALSE(SomeIPMsgHeader({5, 2, 3}) == SomeIPMsgHeader({1, 2, 3}));
        EXPECT_FALSE(SomeIPMsgHeader({1, 5, 3}) == SomeIPMsgHeader({1, 2, 3}));
        EXPECT_FALSE(SomeIPMsgHeader({1, 2, 5}) == SomeIPMsgHeader({1, 2, 3}));
    }

    TEST(ASomeIPMsgHeader, conFormat)
    {
        EXPECT_EQ("Hdr(pid:1 sid:2 mid:3)", fmt::to_string(SomeIPMsgHeader{1, 2, 3}));
    }
}
