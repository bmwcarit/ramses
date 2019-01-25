//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestWithWaylandEnvironment.h"
#include "WaylandUtilities/UnixDomainSocket.h"
#include "gmock/gmock.h"
#include "WaylandUtilities/WaylandEnvironmentUtils.h"
#include "PlatformAbstraction/PlatformEnvironmentVariables.h"
#include "WaylandUtilities/WaylandEnvironmentUtils.h"
#include "Collections/StringOutputStream.h"

namespace ramses_internal
{
    using namespace testing;

    class WaylandEnvironmentUtilsTest : public TestWithWaylandEnvironment
    {
    protected:
        UnixDomainSocket m_socket = UnixDomainSocket("wayland-0", m_initialValueOfXdgRuntimeDir);
    };


    TEST_F(WaylandEnvironmentUtilsTest, IsEnvironmentInProperStateFailsIfNoXdgRuntimeDirIsSet)
    {
        //neither WAYLAND_DISPLAY is set nor WAYLAND_SOCKET is set
        EXPECT_FALSE(WaylandEnvironmentUtils::IsEnvironmentInProperState());

        //WAYLAND_DISPLAY is set
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandDisplay, m_initialValueOfWaylandDisplay);
        EXPECT_FALSE(WaylandEnvironmentUtils::IsEnvironmentInProperState());

        //WAYLAND_SOCKET is set
        WaylandEnvironmentUtils::UnsetVariable(WaylandEnvironmentVariable::WaylandDisplay);

        StringOutputStream fileDescriptor;
        fileDescriptor << m_socket.createBoundFileDescriptor();
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandSocket, fileDescriptor.c_str());
    }

    TEST_F(WaylandEnvironmentUtilsTest, IsEnvironmentInProperStateFailsIfXdgRuntimeDirPathIsNotExistant)
    {
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, "/this/should/lead/nowhere");
        EXPECT_FALSE(WaylandEnvironmentUtils::IsEnvironmentInProperState());
    }

    TEST_F(WaylandEnvironmentUtilsTest, IsEnvironmentInProperStateSucceedsIfXdgRuntimeDirIsAProperPath)
    {
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, m_initialValueOfXdgRuntimeDir);
        EXPECT_TRUE(WaylandEnvironmentUtils::IsEnvironmentInProperState());
    }

    TEST_F(WaylandEnvironmentUtilsTest, IsEnvironmentInProperStateFailsIfXdgRuntimeDirIsAProperPathAndWaylandDisplayIsSetToNonExistingFile)
    {
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, m_initialValueOfXdgRuntimeDir);
        ASSERT_TRUE(WaylandEnvironmentUtils::IsEnvironmentInProperState());

        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandDisplay, "someSocketFileName");
        EXPECT_FALSE(WaylandEnvironmentUtils::IsEnvironmentInProperState());
    }

    TEST_F(WaylandEnvironmentUtilsTest, IsEnvironmentInProperStateSucceedsIfXdgRuntimeDirIsAProperPathAndWaylandDisplayIsSetToProperFile)
    {
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, m_initialValueOfXdgRuntimeDir);
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandDisplay, m_initialValueOfWaylandDisplay);
        EXPECT_TRUE(WaylandEnvironmentUtils::IsEnvironmentInProperState());
    }

    TEST_F(WaylandEnvironmentUtilsTest,  IsEnvironmentInProperStateFailsIfWaylandSocketIsSetToNonExistingFileDescriptor)
    {
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, m_initialValueOfXdgRuntimeDir);
        ASSERT_TRUE(WaylandEnvironmentUtils::IsEnvironmentInProperState());

        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandSocket, "734");
        EXPECT_FALSE(WaylandEnvironmentUtils::IsEnvironmentInProperState());
    }

    TEST_F(WaylandEnvironmentUtilsTest, IsEnvironmentInProperStateSucceedsIfWaylandSocketIsSetToProperFileDescriptor)
    {
        StringOutputStream fileDescriptor;
        fileDescriptor << m_socket.createConnectedFileDescriptor(true);
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, m_initialValueOfXdgRuntimeDir);
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandSocket, fileDescriptor.c_str());
        EXPECT_TRUE(WaylandEnvironmentUtils::IsEnvironmentInProperState());
    }

    TEST_F(WaylandEnvironmentUtilsTest,  IsEnvironmentInProperStateFailsIfWaylandSocketAndWaylandDisplayIsSet)
    {
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, m_initialValueOfXdgRuntimeDir);
        ASSERT_TRUE(WaylandEnvironmentUtils::IsEnvironmentInProperState());

        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandSocket, "734");
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandDisplay, m_initialValueOfWaylandDisplay);
        EXPECT_FALSE(WaylandEnvironmentUtils::IsEnvironmentInProperState());
    }
}
