//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMUDPSOCKETTEST_H
#define RAMSES_PLATFORMUDPSOCKETTEST_H

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "PlatformAbstraction/PlatformUdpSocket.h"

namespace ramses_internal
{
    class PlatformUdpSocketTest : public testing::Test
    {
    public:
        PlatformUdpSocketTest();
        ~PlatformUdpSocketTest();
        void SetUp();
        void TearDown();
    protected:
    };
}

#endif // RAMSES_PLATFORMUDPSOCKETTEST_H
