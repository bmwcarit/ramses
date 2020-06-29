//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestForkingController.h"
#include "TestForkerApplication.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "EmbeddedCompositingTestMessages.h"
#include "Utils/RamsesLogger.h"
#include <sys/wait.h>

namespace ramses_internal
{
    TestForkingController::TestForkingController(const String& waylandSocket)
    {
        startForkerApplication(waylandSocket);
    }

    TestForkingController::~TestForkingController()
    {
        stopForkerApplication();
        pid_t processEndStatus = ::waitpid(m_testForkerApplicationProcessId, nullptr, 0);
        UNUSED(processEndStatus)
        assert(m_testForkerApplicationProcessId == processEndStatus);
    }

    void TestForkingController::setEnvironmentVariableWaylandDisplay()
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingController::setEmbeddedCompositorWaylandSocket(): sending message to forker");
        const ETestForkerApplicationMessage message = ETestForkerApplicationMessage::SetEnvironementVariable_WaylandDisplay;
        m_testToForkerPipe.write(&message, sizeof(ETestForkerApplicationMessage));
    }

    void TestForkingController::setEnvironmentVariableWaylandSocket()
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingController::setEnvironmentVariableWaylandSocket(): sending message to forker");
        const ETestForkerApplicationMessage message = ETestForkerApplicationMessage::SetEnvironementVariable_WaylandSocket;
        m_testToForkerPipe.write(&message, sizeof(ETestForkerApplicationMessage));
    }

    void TestForkingController::startTestApplication()
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingController::startApplication starting test application ");
        sendForkRequest();
    }

    void TestForkingController::waitForTestApplicationExit()
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingController::waitForTestApplicationExit waiting for test application to exit");
        sendWaitForExitRequest();
    }

    void TestForkingController::sendMessageToTestApplication(const BinaryOutputStream& os)
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingController::sendMessageToTestApplication");

        const UInt32 dataSize = static_cast<UInt32>(os.getSize());
        if (!m_testToWaylandClientPipe.write(&dataSize, sizeof(dataSize)))
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingController::sendMessageToTestApplication failed to write data size to pipe!");
        }
        if (!m_testToWaylandClientPipe.write(os.getData(), dataSize))
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingController::sendMessageToTestApplication failed to write data to pipe!");
        }
    }

    void TestForkingController::startForkerApplication(const String& waylandSocket)
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingController::startForkerApplication starting forker");

        if(GetRamsesLogger().isDltAppenderActive())
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingController::startForkerApplication DLT logging enabled, can not fork test application, will halt and catch fire");
            exit(1);
        }

        m_testForkerApplicationProcessId = fork();
        if (m_testForkerApplicationProcessId == -1)
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingController::startForkerApplication forking forker process failed");
        }
        else if (m_testForkerApplicationProcessId == 0)
        {
            TestForkerApplication forkerApplication(waylandSocket, m_testToForkerPipe.getName(), m_testToWaylandClientPipe.getName(), m_waylandClientToTestPipe.getName());
            forkerApplication.run();
            exit(0);
        }

        //open pipes in parent process
        m_testToForkerPipe.open();
        m_testToWaylandClientPipe.open();
        m_waylandClientToTestPipe.open();
    }

    void TestForkingController::stopForkerApplication()
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingController::stopForkerApplication(): sending message stop forker");
        const ETestForkerApplicationMessage message = ETestForkerApplicationMessage::StopForkerApplication;
        m_testToForkerPipe.write(&message, sizeof(ETestForkerApplicationMessage));
    }

    void TestForkingController::sendForkRequest()
    {
        const ETestForkerApplicationMessage message = ETestForkerApplicationMessage::ForkTestApplication;

        if (!m_testToForkerPipe.write(&message, sizeof(ETestForkerApplicationMessage)))
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingController::sendForkRequest error " << NamedPipe::getSystemErrorStatus() << " when wrinting fork request pipe");
        }
    }

    void TestForkingController::sendWaitForExitRequest()
    {
        const ETestForkerApplicationMessage message = ETestForkerApplicationMessage::WaitForTestApplicationExit;

        if (!m_testToForkerPipe.write(&message, sizeof(ETestForkerApplicationMessage)))
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingController::sendWaitForExitRequest error " << NamedPipe::getSystemErrorStatus() << " when writing wait for test application exit pipe");
        }
    }

    void TestForkingController::killTestApplication()
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingController::killTestApplication(): sending message kill test application");
        const ETestForkerApplicationMessage message = ETestForkerApplicationMessage::KillTestApplication;

        if (!m_testToForkerPipe.write(&message, sizeof(ETestForkerApplicationMessage)))
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingController::killTestApplication error " << NamedPipe::getSystemErrorStatus());
        }
    }
}
