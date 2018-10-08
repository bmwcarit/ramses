//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "SystemCompositorController_Wayland_IVI/SystemCompositorController_Wayland_IVI.h"
#include "UnixUtilities/EnvironmentVariableHelper.h"
#include "UnixUtilities/UnixDomainSocketHelper.h"
#include "Collections/StringOutputStream.h"

namespace ramses_internal
{
    using namespace testing;

    class ASystemCompositorController_Wayland_IVI : public ::testing::Test
    {
    protected:
        EnvironmentVariableHelper              m_environment;
        SystemCompositorController_Wayland_IVI m_scc;
    };


    TEST_F(ASystemCompositorController_Wayland_IVI, CanBeInited)
    {
        EXPECT_TRUE(m_scc.init());
    }

    TEST_F(ASystemCompositorController_Wayland_IVI, IfXdgRuntimeDirIsNotSetInitWillFail)
    {
        m_environment.unsetVariable(EnvironmentVariableName::XDGRuntimeDir);
        EXPECT_FALSE(m_scc.init());
    }

    TEST_F(ASystemCompositorController_Wayland_IVI, IfXdgRuntimeDirIsNotCorrectInitWillFail)
    {
        m_environment.setVariable(EnvironmentVariableName::XDGRuntimeDir, "/this/should/lead/nowhere");
        EXPECT_FALSE(m_scc.init());
    }

    TEST_F(ASystemCompositorController_Wayland_IVI, IfXdgRuntimeDirIsNotSetButWaylandSocketInitWillSucceed)
    {
        UnixDomainSocketHelper socketHelper = UnixDomainSocketHelper("wayland-0");

        m_environment.unsetVariable(EnvironmentVariableName::XDGRuntimeDir);

        StringOutputStream fileDescriptor;
        fileDescriptor << socketHelper.createConnectedFileDescriptor(true);
        m_environment.setVariable(EnvironmentVariableName::WaylandSocket, fileDescriptor.c_str());

        EXPECT_TRUE(m_scc.init());
    }

    TEST_F(ASystemCompositorController_Wayland_IVI, IfWaylandDisplayIsNotCorrectInitWillFail)
    {
        m_environment.setVariable(EnvironmentVariableName::WaylandDisplay, "SomeFilenameThatDoesNotExist");
        EXPECT_FALSE(m_scc.init());
    }


}
