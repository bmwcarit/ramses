//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gtest/gtest.h"
#include "internal/Platform/Wayland/WaylandEnvironmentUtils.h"

namespace ramses::internal
{
    class TestWithWaylandEnvironment : public ::testing::Test
    {
    protected:
        TestWithWaylandEnvironment()
            : m_initialValueOfXdgRuntimeDir(WaylandEnvironmentUtils::GetVariable(WaylandEnvironmentVariable::XDGRuntimeDir))
            , m_initialValueOfWaylandDisplay(WaylandEnvironmentUtils::GetVariable(WaylandEnvironmentVariable::WaylandDisplay))
            , m_initialValueOfWaylandSocket(WaylandEnvironmentUtils::GetVariable(WaylandEnvironmentVariable::WaylandSocket))
        {
            //cleanup environment before running test
            WaylandEnvironmentUtils::UnsetVariable(WaylandEnvironmentVariable::XDGRuntimeDir);
            WaylandEnvironmentUtils::UnsetVariable(WaylandEnvironmentVariable::WaylandDisplay);
            WaylandEnvironmentUtils::UnsetVariable(WaylandEnvironmentVariable::WaylandSocket);
        }

        ~TestWithWaylandEnvironment() override
        {
            //restore environment before running test
            WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, m_initialValueOfXdgRuntimeDir);
            WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandDisplay, m_initialValueOfWaylandDisplay);
            WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandSocket, m_initialValueOfWaylandSocket);
        }

        //initial value of env vars before running test
        const std::string m_initialValueOfXdgRuntimeDir;
        const std::string m_initialValueOfWaylandDisplay;
        const std::string m_initialValueOfWaylandSocket;
    };
}

