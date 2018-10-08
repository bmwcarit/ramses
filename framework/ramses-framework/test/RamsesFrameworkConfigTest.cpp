//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "RamsesFrameworkConfigImpl.h"

using namespace ramses;
using namespace ramses_internal;

class ARamsesFrameworkConfig : public testing::Test
{
protected:
    RamsesFrameworkConfig frameworkConfig;
};

TEST_F(ARamsesFrameworkConfig, IsInitializedCorrectly)
{
    EXPECT_EQ(ERamsesShellType_Default, frameworkConfig.impl.m_shellType);
}

TEST_F(ARamsesFrameworkConfig, CanSetShellConsoleType)
{
    frameworkConfig.setRequestedRamsesShellType(ERamsesShellType_Console);
    EXPECT_EQ(ERamsesShellType_Console, frameworkConfig.impl.m_shellType);
}

TEST_F(ARamsesFrameworkConfig, CanSetShellTypeNone)
{
    frameworkConfig.setRequestedRamsesShellType(ERamsesShellType_None);
    EXPECT_EQ(ERamsesShellType_None, frameworkConfig.impl.m_shellType);
}

TEST_F(ARamsesFrameworkConfig, CanSetShellTypeDefault)
{
    frameworkConfig.setRequestedRamsesShellType(ERamsesShellType_Default);
    EXPECT_EQ(ERamsesShellType_Default, frameworkConfig.impl.m_shellType);
}

TEST_F(ARamsesFrameworkConfig, CanEnableRamshFromCommandLine)
{
    const char* args[] = { "framework", "-ramsh" };
    RamsesFrameworkConfig config(2, args);
    EXPECT_EQ(ERamsesShellType_Console, config.impl.m_shellType);
}

TEST_F(ARamsesFrameworkConfig, TestSetandGetApplicationInformation)
{
    const char* application_id = "myap";
    const char* application_description = "mydescription";

    frameworkConfig.setDLTApplicationID(application_id);
    frameworkConfig.setDLTApplicationDescription(application_description);

    EXPECT_STREQ(application_id, frameworkConfig.getDLTApplicationID());
    EXPECT_STREQ(application_description, frameworkConfig.getDLTApplicationDescription());
}
