//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "WaylandUtilities/WaylandUtilities.h"
#include "TestWithWaylandEnvironment.h"
#include "WaylandUtilities/UnixDomainSocketHelper.h"
#include "gmock/gmock.h"
#include "WaylandUtilities/WaylandUtilities.h"
#include "PlatformAbstraction/PlatformEnvironmentVariables.h"
#include "WaylandUtilities/WaylandEnvironmentUtils.h"
#include "Collections/StringOutputStream.h"

namespace ramses_internal
{
    using namespace testing;

    class WaylandUtilitiesTest : public TestWithWaylandEnvironment
    {
    protected:
        UnixDomainSocketHelper    m_socketHelper = UnixDomainSocketHelper("wayland-0", m_initialValueOfXdgRuntimeDir);
    };


    TEST_F(WaylandUtilitiesTest, IsEnvironmentInProperStateFailsIfNoXdgRuntimeDirIsSet)
    {
        //neither WAYLAND_DISPLAY is set nor WAYLAND_SOCKET is set
        EXPECT_FALSE(WaylandUtilities::IsEnvironmentInProperState());

        //WAYLAND_DISPLAY is set
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandDisplay, m_initialValueOfWaylandDisplay);
        EXPECT_FALSE(WaylandUtilities::IsEnvironmentInProperState());

        //WAYLAND_SOCKET is set
        WaylandEnvironmentUtils::UnsetVariable(WaylandEnvironmentVariable::WaylandDisplay);

        StringOutputStream fileDescriptor;
        fileDescriptor << m_socketHelper.createBoundFileDescriptor();
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandSocket, fileDescriptor.c_str());
        EXPECT_FALSE(WaylandUtilities::IsEnvironmentInProperState());
    }

    TEST_F(WaylandUtilitiesTest, IsEnvironmentInProperStateFailsIfXdgRuntimeDirPathIsNotExisting)
    {
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, "/this/should/lead/nowhere");
        EXPECT_FALSE(WaylandUtilities::IsEnvironmentInProperState());
    }

    TEST_F(WaylandUtilitiesTest, IsEnvironmentInProperStateSucceedsIfXdgRuntimeDirIsAProperPath)
    {
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, m_initialValueOfXdgRuntimeDir);
        EXPECT_TRUE(WaylandUtilities::IsEnvironmentInProperState());
    }

    TEST_F(WaylandUtilitiesTest, IsEnvironmentInProperStateFailsIfXdgRuntimeDirIsAProperPathAndWaylandDisplayIsSetToNonExistingFile)
    {
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, m_initialValueOfXdgRuntimeDir);
        ASSERT_TRUE(WaylandUtilities::IsEnvironmentInProperState());

        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandDisplay, "someSocketFileName");
        EXPECT_FALSE(WaylandUtilities::IsEnvironmentInProperState());
    }

    TEST_F(WaylandUtilitiesTest, IsEnvironmentInProperStateSucceedsIfXdgRuntimeDirIsAProperPathAndWaylandDisplayIsSetToProperFile)
    {
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, m_initialValueOfXdgRuntimeDir);
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandDisplay, m_initialValueOfWaylandDisplay);
        EXPECT_TRUE(WaylandUtilities::IsEnvironmentInProperState());
    }

    TEST_F(WaylandUtilitiesTest,  IsEnvironmentInProperStateFailsIfWaylandSocketIsSetToNonExistingFileDescriptor)
    {
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, m_initialValueOfXdgRuntimeDir);
        ASSERT_TRUE(WaylandUtilities::IsEnvironmentInProperState());

        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandSocket, "734");
        EXPECT_FALSE(WaylandUtilities::IsEnvironmentInProperState());
    }

    TEST_F(WaylandUtilitiesTest, IsEnvironmentInProperStateSucceedsIfWaylandSocketIsSetToProperFileDescriptor)
    {
        StringOutputStream fileDescriptor;
        fileDescriptor << m_socketHelper.createConnectedFileDescriptor(true);
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, m_initialValueOfXdgRuntimeDir);
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandSocket, fileDescriptor.c_str());
        EXPECT_TRUE(WaylandUtilities::IsEnvironmentInProperState());
    }

    TEST_F(WaylandUtilitiesTest,  IsEnvironmentInProperStateFailsIfWaylandSocketAndWaylandDisplayIsSet)
    {
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, m_initialValueOfXdgRuntimeDir);
        ASSERT_TRUE(WaylandUtilities::IsEnvironmentInProperState());

        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandSocket, "734");
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandDisplay, m_initialValueOfWaylandDisplay);
        EXPECT_FALSE(WaylandUtilities::IsEnvironmentInProperState());
    }
}
