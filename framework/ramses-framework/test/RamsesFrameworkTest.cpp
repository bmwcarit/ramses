//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "RamsesFrameworkImpl.h"

using namespace ramses;

TEST(ARamsesFramework, canDefaultConstruct)
{
    RamsesFramework fw;
    EXPECT_GT(fw.impl.getParticipantAddress().getParticipantId().get(), 0xFF);
}

TEST(ARamsesFramework, canConstructFromConfig)
{
    const char* argv[] = {"", "-guid", "0000-000000000123"};
    RamsesFrameworkConfig config(3, argv);
    RamsesFramework fw(config);
    EXPECT_EQ(fw.impl.getParticipantAddress().getParticipantId().get(), 0x123);
}


TEST(ARamsesFramework, canConstructWithArgcArgv)
{
    const char* argv[] = {"", "-guid", "0000-000000000124"};
    RamsesFramework fw(3, argv);
    EXPECT_EQ(fw.impl.getParticipantAddress().getParticipantId().get(), 0x124);
}

TEST(ARamsesFramework, isNotConnectedInitially)
{
    RamsesFramework fw;
    EXPECT_FALSE(fw.isConnected());
}

TEST(ARamsesFramework, connectLifeCycleOK)
{
    RamsesFramework fw;
    EXPECT_EQ(ramses::StatusOK, fw.connect());
    EXPECT_TRUE(fw.isConnected());
    EXPECT_EQ(ramses::StatusOK, fw.disconnect());
}

TEST(ARamsesFramework, reportsErrorWhenConnectingSecondTime)
{
    RamsesFramework fw;
    fw.connect();
    EXPECT_NE(ramses::StatusOK, fw.connect());
    EXPECT_TRUE(fw.isConnected());
}

TEST(ARamsesFramework, reportsErrorWhenDisconnectingSecondTime)
{
    RamsesFramework fw;
    fw.connect();
    EXPECT_EQ(ramses::StatusOK, fw.disconnect());
    EXPECT_NE(ramses::StatusOK, fw.disconnect());
}
