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
        : m_testToForkerPipe("/tmp/rames-ec-tests-testToForkerPipe", true)
        , m_testToWaylandClientPipe("/tmp/rames-ec-tests-testToWaylandClientPipe", true)
        , m_waylandClientToTestPipe("/tmp/rames-ec-tests-waylandClientToTestPipe", true)
        , m_testForkerApplicationProcessId(-1)
    {
        startForkerApplication(waylandSocket);
    }

    void TestForkingController::startTestApplication()
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingControllerController::startApplication starting test application ");
        sendForkRequest();
    }

    void TestForkingController::waitForTestApplicationExit()
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingControllerController::waitForTestApplicationExit waiting for test application to exit");
        sendWaitForExitRequest();
    }

    void TestForkingController::sendMessageToTestApplication(ETestWaylandApplicationMessage message)
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingController::sendMessageToTestApplication");
        UInt32 messageAsUInt = static_cast<UInt32>(message);
        if (!m_testToWaylandClientPipe.write(&messageAsUInt, sizeof(messageAsUInt)))
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingControllerController::sendMessageToTestApplication(" << messageAsUInt << "): failed to write message to pipe!");
        }
    }

    void TestForkingController::sendStringToTestApplication(const String& string)
    {
        UInt32 stringLength = static_cast<UInt32>(string.getLength());
        if (!m_testToWaylandClientPipe.write(&stringLength, sizeof(stringLength)))
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingControllerController::sendStringToTestApplciation failed to write string length to pipe!");
        }
        if (!m_testToWaylandClientPipe.write(string.c_str(), sizeof(Char) * stringLength))
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "TestForkingControllerController::sendStringToTestApplciation failed to write string to pipe!");
        }
    }

    void TestForkingController::deinitialize()
    {
        stopForkerApplication();
        pid_t processEndStatus = ::waitpid(m_testForkerApplicationProcessId, NULL, 0);
        UNUSED(processEndStatus)
        assert(m_testForkerApplicationProcessId == processEndStatus);
    }

    void TestForkingController::startForkerApplication(const String& waylandSocket)
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingControllerController::startForkerApplication starting forker");

        if(GetRamsesLogger().isAppenderTypeActive(ELogAppenderType::Dlt))
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingControllerController::startForkerApplication DLT logging enabled, can not fork test application, will halt and catch fire");
            exit(1);
        }

        m_testForkerApplicationProcessId = fork();
        if (m_testForkerApplicationProcessId == -1)
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingControllerController::startForkerApplication forking forker process failed");
        }
        else if (m_testForkerApplicationProcessId == 0)
        {
            setenv("WAYLAND_DISPLAY", waylandSocket.c_str(), 1);

            TestForkerApplication forkerApplication(m_testToForkerPipe.getName(), m_testToWaylandClientPipe.getName(), m_waylandClientToTestPipe.getName());
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
        LOG_INFO(CONTEXT_RENDERER, "TestForkingControllerController::stopForkerApplication(): sending message stop forker");

        const UInt32 messageAsUInt = static_cast<UInt32>(ETestForkerApplicationMessage_StopForkerApplication);
        m_testToForkerPipe.write(&messageAsUInt, sizeof(UInt32));
    }

    void TestForkingController::sendForkRequest()
    {
        UInt32 messageAsUInt= static_cast<UInt32>(ETestForkerApplicationMessage_ForkTestApplication);
        if (!m_testToForkerPipe.write(&messageAsUInt, sizeof(messageAsUInt)))
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingControllerController::sendForkRequest error " << NamedPipe::getSystemErrorStatus() << " when wrinting fork request pipe");
        }
    }

    void TestForkingController::sendWaitForExitRequest()
    {
        UInt32 messageAsUInt= static_cast<UInt32>(ETestForkerApplicationMessage_WaitForTestApplicationExit);
        if (!m_testToForkerPipe.write(&messageAsUInt, sizeof(messageAsUInt)))
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingControllerController::sendWaitForExitRequest error " << NamedPipe::getSystemErrorStatus() << " when writing wait for test application exit pipe");
        }
    }

    void TestForkingController::killTestApplication()
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingControllerController::killTestApplication(): sending message kill test application");
        UInt32 messageAsUInt = static_cast<UInt32>(ETestForkerApplicationMessage_KillTestApplication);
        if (!m_testToForkerPipe.write(&messageAsUInt, sizeof(messageAsUInt)))
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingControllerController::killTestApplication error " << NamedPipe::getSystemErrorStatus());
        }
    }
}
