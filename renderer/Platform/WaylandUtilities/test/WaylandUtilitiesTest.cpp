//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "WaylandUtilities/WaylandUtilities.h"
#include "PlatformAbstraction/PlatformEnvironmentVariables.h"
#include "UnixUtilities/EnvironmentVariableHelper.h"
#include "UnixUtilities/UnixDomainSocketHelper.h"
#include "Collections/StringOutputStream.h"

namespace ramses_internal
{
    using namespace testing;

    class WaylandUtilitiesTest : public ::testing::Test
    {
    protected:
        UnixDomainSocketHelper    m_socketHelper = UnixDomainSocketHelper("wayland-0");
        EnvironmentVariableHelper m_environment;
    };


    TEST_F(WaylandUtilitiesTest, IsEnvironmentInProperStateFailsIfNoXdgRuntimeDirIsSet)
    {
        m_environment.unsetVariable(EnvironmentVariableName::XDGRuntimeDir);
        EXPECT_FALSE(WaylandUtilities::IsEnvironmentInProperState());
    }

    TEST_F(WaylandUtilitiesTest, IsEnvironmentInProperStateFailsIfNoXdgRuntimeDirPathIsNotExistant)
    {
        m_environment.setVariable(EnvironmentVariableName::XDGRuntimeDir, "/this/should/lead/nowhere");
        EXPECT_FALSE(WaylandUtilities::IsEnvironmentInProperState());
    }

    TEST_F(WaylandUtilitiesTest, IsEnvironmentInProperStateSucceedsIfXdgRuntimeDirIsAProperPath)
    {
        m_environment.setVariable(EnvironmentVariableName::XDGRuntimeDir, "/tmp");
        EXPECT_TRUE(WaylandUtilities::IsEnvironmentInProperState());
    }

    TEST_F(WaylandUtilitiesTest, IsEnvironmentInProperStateFailsIfXdgRuntimeDirIsAProperPathAndWaylandDisplayIsSetToNonExistantFile)
    {
        m_environment.setVariable(EnvironmentVariableName::WaylandDisplay, "someSocketFileName");
        EXPECT_FALSE(WaylandUtilities::IsEnvironmentInProperState());
    }

    TEST_F(WaylandUtilitiesTest, IsEnvironmentInProperStateSucceedsIfXdgRuntimeDirIsAProperPathAndWaylandDisplayIsSetToProperFile)
    {
        m_environment.setVariable(EnvironmentVariableName::WaylandDisplay, "wayland-0");
        EXPECT_TRUE(WaylandUtilities::IsEnvironmentInProperState());
    }

    TEST_F(WaylandUtilitiesTest,  IsEnvironmentInProperStateFailsIfWaylandSocketIsSetToNonExistantFileDescriptor)
    {
        m_environment.setVariable(EnvironmentVariableName::WaylandSocket, "734");
        EXPECT_FALSE(WaylandUtilities::IsEnvironmentInProperState());
    }

    TEST_F(WaylandUtilitiesTest, IsEnvironmentInProperStateSucceedsIfWaylandSocketIsSetToProperFileDescriptor)
    {
        StringOutputStream fileDescriptor;
        fileDescriptor << m_socketHelper.createConnectedFileDescriptor(true);
        m_environment.setVariable(EnvironmentVariableName::WaylandSocket, fileDescriptor.c_str());
        EXPECT_TRUE(WaylandUtilities::IsEnvironmentInProperState());
    }

    TEST_F(WaylandUtilitiesTest,  IsEnvironmentInProperStateFailsIfWaylandSocketAndWaylandDisplayIsSet)
    {
        m_environment.setVariable(EnvironmentVariableName::WaylandSocket, "734");
        m_environment.setVariable(EnvironmentVariableName::WaylandDisplay, "wayland-0");
        EXPECT_FALSE(WaylandUtilities::IsEnvironmentInProperState());
    }
}
