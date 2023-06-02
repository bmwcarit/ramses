//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "TestWithWaylandEnvironment.h"
#include "SystemCompositorController_Wayland_IVI/SystemCompositorController_Wayland_IVI.h"
#include "WaylandUtilities/WaylandEnvironmentUtils.h"
#include "WaylandUtilities/UnixDomainSocket.h"
#include "Collections/StringOutputStream.h"

namespace ramses_internal
{
    using namespace testing;

    class ASystemCompositorController_Wayland_IVI : public TestWithWaylandEnvironment
    {
    protected:
        SystemCompositorController_Wayland_IVI m_scc;
    };


    TEST_F(ASystemCompositorController_Wayland_IVI, CanBeInited)
    {
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, m_initialValueOfXdgRuntimeDir);
        EXPECT_TRUE(m_scc.init());
    }

    TEST_F(ASystemCompositorController_Wayland_IVI, IfXdgRuntimeDirIsNotSetInitWillFail)
    {
        ASSERT_STREQ("", WaylandEnvironmentUtils::GetVariable(WaylandEnvironmentVariable::XDGRuntimeDir).c_str());

        EXPECT_FALSE(m_scc.init());
    }

    TEST_F(ASystemCompositorController_Wayland_IVI, IfXdgRuntimeDirIsNotCorrectInitWillFail)
    {
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, "/this/should/lead/nowhere");
        EXPECT_FALSE(m_scc.init());
    }

    TEST_F(ASystemCompositorController_Wayland_IVI, IfXdgRuntimeDirIsNotSetButWaylandSocketInitWillSucceed)
    {
        ASSERT_STREQ("", WaylandEnvironmentUtils::GetVariable(WaylandEnvironmentVariable::XDGRuntimeDir).c_str());

        UnixDomainSocket socket = UnixDomainSocket("wayland-0", m_initialValueOfXdgRuntimeDir);
        StringOutputStream fileDescriptor;
        fileDescriptor << socket.createConnectedFileDescriptor(true);
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandSocket, fileDescriptor.data());

        EXPECT_TRUE(m_scc.init());
    }

    TEST_F(ASystemCompositorController_Wayland_IVI, IfWaylandDisplayIsNotCorrectInitWillFail)
    {
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, m_initialValueOfXdgRuntimeDir);
        WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandDisplay, "SomeFilenameThatDoesNotExist");
        EXPECT_FALSE(m_scc.init());
    }
}
