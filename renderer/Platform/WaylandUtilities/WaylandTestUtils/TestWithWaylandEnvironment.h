//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TESTWITHWAYLANDENVIRONMENT_H
#define RAMSES_TESTWITHWAYLANDENVIRONMENT_H

#include "gtest/gtest.h"
#include "WaylandUtilities/WaylandEnvironmentUtils.h"

namespace ramses_internal
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

        ~TestWithWaylandEnvironment()
        {
            //restore environment before running test
            WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, m_initialValueOfXdgRuntimeDir);
            WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandDisplay, m_initialValueOfWaylandDisplay);
            WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::WaylandSocket, m_initialValueOfWaylandSocket);
        }

        //initial value of env vars before running test
        const String m_initialValueOfXdgRuntimeDir;
        const String m_initialValueOfWaylandDisplay;
        const String m_initialValueOfWaylandSocket;
    };
}

#endif

