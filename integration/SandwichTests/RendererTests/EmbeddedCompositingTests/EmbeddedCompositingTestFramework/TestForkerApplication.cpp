//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestForkerApplication.h"
#include "EmbeddedCompositingTestFramework/EmbeddedCompositingTestsFramework.h"
#include "Utils/LogMacros.h"
#include "TestWaylandApplication.h"
#include "TestSignalHandler.h"
#include <sys/wait.h>

namespace ramses_internal
{
    TestForkerApplication::TestForkerApplication(std::string_view testToForkerPipeName, const std::vector<std::pair<std::string,std::string>>& testPipeNames)
        : m_testToForkerPipe(testToForkerPipeName, false)
    {
        TestSignalHandler::RegisterSignalHandlersForCurrentProcess("TestForkerApplication");
        m_testToForkerPipe.open();

        for(const auto& pipeNames : testPipeNames)
            m_testApplicationInfo.push_back({pipeNames.first, pipeNames.second, 0});
    }

    void TestForkerApplication::run()
    {
        while (handleIncomingMessage())
        {
        }

        LOG_INFO(CONTEXT_RENDERER, "TestForkerApplication::run(): Stopped handling incoming messages");
    }

    bool TestForkerApplication::handleIncomingMessage()
    {
        ETestForkerApplicationMessage messgage;
        const EReadFromPipeStatus readingStatus = m_testToForkerPipe.read(&messgage, sizeof(ETestForkerApplicationMessage));
        switch(readingStatus)
        {
        case EReadFromPipeStatus_Success:
            break;
        case EReadFromPipeStatus_Closed:
            LOG_ERROR(CONTEXT_RENDERER, "TestForkerApplication::handleIncomingMessage request pipe closed, stopping forker application");
            return false;
        case EReadFromPipeStatus_Failure:
            LOG_ERROR(CONTEXT_RENDERER, "TestForkerApplication::handleIncomingMessage request pipe error " << NamedPipe::getSystemErrorStatus() << ", stopping");
            return false;
        case EReadFromPipeStatus_Empty:
            return true;
        default:
            assert(false);
            break;
        }

        switch(messgage)
        {
        case ETestForkerApplicationMessage::StopForkerApplication:
            LOG_INFO(CONTEXT_RENDERER, "TestForkerApplication::handleIncomingMessage received stop forker application message");
            return false;
        case ETestForkerApplicationMessage::ForkTestApplication:
        {
            LOG_INFO(CONTEXT_RENDERER, "TestForkerApplication::handleIncomingMessage received fork test application message");
            uint32_t testAppIdx = 0u;
            m_testToForkerPipe.read(&testAppIdx, sizeof(testAppIdx));
            startTestApplication(testAppIdx);
            break;
        }
        case ETestForkerApplicationMessage::KillTestApplication:
        {
            LOG_INFO(CONTEXT_RENDERER, "TestForkerApplication::handleIncomingMessage received kill test application message");
            uint32_t testAppIdx = 0u;
            m_testToForkerPipe.read(&testAppIdx, sizeof(testAppIdx));
            killTestApplication(testAppIdx);
            break;
        }
        case ETestForkerApplicationMessage::WaitForTestApplicationExit:
        {
            LOG_INFO(CONTEXT_RENDERER, "TestForkerApplication::handleIncomingMessage received wait for test application exit message");
            uint32_t testAppIdx = 0u;
            m_testToForkerPipe.read(&testAppIdx, sizeof(testAppIdx));
            waitForTestApplicationExit(testAppIdx);
            break;
        }
        default:
            LOG_ERROR(CONTEXT_RENDERER, "TestForkerApplication::handleIncomingMessage(): unknown message!");
            assert(false);
            return false;
        }

        return true;
    }

    void TestForkerApplication::startTestApplication(uint32_t testAppIdx)
    {
        assert(testAppIdx < m_testApplicationInfo.size());

        if(m_testApplicationInfo[testAppIdx].testApplicationProcessId > 0)
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkerApplication::startApplication trying to fork test app while it is still running");
            exit(1);
        }

        m_testApplicationInfo[testAppIdx].testApplicationProcessId = fork();
        if (m_testApplicationInfo[testAppIdx].testApplicationProcessId == -1)
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkerApplication::startApplication fork for application failed");
        }
        else if (m_testApplicationInfo[testAppIdx].testApplicationProcessId == 0)
        {
            TestWaylandApplication testApplication(m_testApplicationInfo[testAppIdx].testToWaylandClientPipeName, m_testApplicationInfo[testAppIdx].waylandClientToTestPipeName);
            const bool testApplicationExitStatus = testApplication.run();

            // TODO(tobias) this seems totally wrong as true is error condition as exit code
            exit(testApplicationExitStatus ? 1 : 0);
        }
    }

    void TestForkerApplication::waitForTestApplicationExit(uint32_t testAppIdx)
    {
        assert(testAppIdx < m_testApplicationInfo.size());
        assert(m_testApplicationInfo[testAppIdx].testApplicationProcessId > 0);
        ::waitpid(m_testApplicationInfo[testAppIdx].testApplicationProcessId, nullptr, 0);
        m_testApplicationInfo[testAppIdx].testApplicationProcessId = 0;
    }

    void TestForkerApplication::killTestApplication(uint32_t testAppIdx)
    {
        assert(testAppIdx < m_testApplicationInfo.size());
        kill(m_testApplicationInfo[testAppIdx].testApplicationProcessId, SIGKILL);
        waitForTestApplicationExit(testAppIdx);
    }
}
