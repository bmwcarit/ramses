//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TESTFORKERAPPLICATION_H
#define RAMSES_TESTFORKERAPPLICATION_H

#include "Collections/String.h"
#include "WaylandUtilities/UnixDomainSocket.h"
#include "NamedPipe.h"

namespace ramses_internal
{
    class TestForkerApplication
    {
    public:
        TestForkerApplication(const String& embeddedCompositorDisplayName, const String& testToForkerPipeName, const String& testToWaylandClientPipeName, const String& waylandClientToTestPipeName);
        void run();

    private:
        bool handleIncomingMessage();
        void startTestApplication();
        void waitForTestApplicationExit();
        void killTestApplication();
        void setEnvironmentVariableWaylandDisplay();
        void setEnvironmentVariableWaylandSocket();

        NamedPipe m_testToForkerPipe;
        const String m_testToWaylandClientPipeName;
        const String m_waylandClientToTestPipeName;
        pid_t m_testApplicationProcessId;

        const String m_embeddedCompositorDisplayName;
        ramses_internal::UnixDomainSocket m_socket;
    };
}

#endif
