//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

namespace ramses_internal
{
    TEST(Endianess, isLittleEndianOnAllRamsesSystems)
    {
        // Test using network order
        ASSERT_NE(42u, htonl(42));

        // Test using bytes of one integer
        union
        {
            UInt32 anInt;
            char theIntAsByteArray[4];
        } integerUnion = { 0x01000000 };

        const bool isBigEndian = (integerUnion.theIntAsByteArray[0] == 1);
        const bool isLittleEndian = (integerUnion.theIntAsByteArray[3] == 1);

        ASSERT_FALSE(isBigEndian);
        ASSERT_TRUE(isLittleEndian);
    }
}
