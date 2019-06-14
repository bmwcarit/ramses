//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PlatformAbstraction/PlatformEnvironmentVariables.h"
#include "gtest/gtest.h"

namespace ramses_internal
{
    TEST(EnvironmentVariables, getTest)
    {
        String tmp;
        EXPECT_TRUE(PlatformEnvironmentVariables::get("PATH", tmp));
    }

    TEST(EnvironmentVariables, hasTest)
    {
        String tmp;
        EXPECT_TRUE(PlatformEnvironmentVariables::HasEnvVar("PATH"));
        EXPECT_FALSE(PlatformEnvironmentVariables::HasEnvVar("RAMSES_THIS_VAR_WILL_NOT_EXIST"));
    }

    TEST(EnvironmentVariables, setTest)
    {
        PlatformEnvironmentVariables::SetEnvVar("RAMSES_SOME_ENVVAR", "foo");
        String tmp;
        EXPECT_TRUE(PlatformEnvironmentVariables::get("RAMSES_SOME_ENVVAR", tmp));
        EXPECT_EQ("foo", tmp);
    }

    TEST(EnvironmentVariables, unsetTest)
    {
        PlatformEnvironmentVariables::SetEnvVar("RAMSES_SOME_OTHER_ENVVAR", "foo");
        PlatformEnvironmentVariables::UnsetEnvVar("RAMSES_SOME_OTHER_ENVVAR");
        String tmp;
        EXPECT_FALSE(PlatformEnvironmentVariables::get("RAMSES_SOME_OTHER_ENVVAR", tmp));
    }

}
