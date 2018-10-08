//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestForkerApplication.h"
#include "Utils/LogMacros.h"
#include "TestWaylandApplication.h"
#include "TestSignalHandler.h"
#include <sys/wait.h>

namespace ramses_internal
{
    TestForkerApplication::TestForkerApplication(const String& testToForkerPipeName, const String& testToWaylandClientPipeName, const String& waylandClientToTestPipeName)
        : m_testToForkerPipe(testToForkerPipeName, false)
        , m_testToWaylandClientPipeName(testToWaylandClientPipeName)
        , m_waylandClientToTestPipeName(waylandClientToTestPipeName)
        , m_testApplicationProcessId(0)
    {
        TestSignalHandler::RegisterSignalHandlersForCurrentProcess("TestForkerApplication");
        m_testToForkerPipe.open();
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
        UInt32 messgageAsUInt = 0u;
        const EReadFromPipeStatus readingStatus = m_testToForkerPipe.read(&messgageAsUInt, sizeof(UInt32));
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

        switch(static_cast<ETestForkerApplicationMessage>(messgageAsUInt))
        {
        case ETestForkerApplicationMessage_StopForkerApplication:
            LOG_INFO(CONTEXT_RENDERER, "TestForkerApplication::handleIncomingMessage received stop forker application message");
            return false;
        case ETestForkerApplicationMessage_ForkTestApplication:
        {
            LOG_INFO(CONTEXT_RENDERER, "TestForkerApplication::handleIncomingMessage received fork test application message");
            startTestApplication();
            break;
        }
        case ETestForkerApplicationMessage_KillTestApplication:
        {
            LOG_INFO(CONTEXT_RENDERER, "TestForkerApplication::handleIncomingMessage received kill test application message");
            killTestApplication();
            break;
        }
        case ETestForkerApplicationMessage_WaitForTestApplicationExit:
        {
            LOG_INFO(CONTEXT_RENDERER, "TestForkerApplication::handleIncomingMessage received wait for test application exit message");
            waitForTestApplicationExit();
            break;
        }
        default:
            LOG_ERROR(CONTEXT_RENDERER, "TestForkerApplication::handleIncomingMessage(): unknown message!");
            assert(false);
            return false;
        }

        return true;
    }

    void TestForkerApplication::startTestApplication()
    {
        m_testApplicationProcessId = fork();
        if (m_testApplicationProcessId == -1)
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkerApplication::startApplication fork for application failed");
        }
        else
        {
            if (m_testApplicationProcessId == 0)
            {
                TestWaylandApplication testApplication(m_testToWaylandClientPipeName, m_waylandClientToTestPipeName);
                const Bool testApplicationExitStatus = testApplication.run();
                exit(testApplicationExitStatus);
            }
        }
    }

    void TestForkerApplication::waitForTestApplicationExit()
    {
        assert(m_testApplicationProcessId > 0);
        ::waitpid(m_testApplicationProcessId, nullptr, 0);
        m_testApplicationProcessId = 0;
    }

    void TestForkerApplication::killTestApplication()
    {
        kill(m_testApplicationProcessId, SIGKILL);
        waitForTestApplicationExit();
    }
}
