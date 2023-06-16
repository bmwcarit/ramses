//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TESTFORKERAPPLICATION_H
#define RAMSES_TESTFORKERAPPLICATION_H

#include "NamedPipe.h"

#include <string>
#include <string_view>

namespace ramses_internal
{
    class TestForkerApplication
    {
    public:
        TestForkerApplication(std::string_view testToForkerPipeName, const std::vector<std::pair<std::string, std::string>>& testPipeNames);
        void run();

    private:
        bool handleIncomingMessage();
        void startTestApplication(uint32_t testAppIdx);
        void waitForTestApplicationExit(uint32_t testAppIdx);
        void killTestApplication(uint32_t testAppIdx);

        NamedPipe m_testToForkerPipe;

        struct TestApplicationInfo
        {
            const std::string testToWaylandClientPipeName;
            const std::string waylandClientToTestPipeName;
            pid_t testApplicationProcessId;
        };
        std::vector<TestApplicationInfo> m_testApplicationInfo;
    };
}

#endif
