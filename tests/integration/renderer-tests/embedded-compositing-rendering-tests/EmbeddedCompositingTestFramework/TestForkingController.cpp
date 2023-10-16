//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestForkingController.h"
#include "TestForkerApplication.h"
#include "EmbeddedCompositingTestMessages.h"
#include "internal/Core/Utils/RamsesLogger.h"

#include <sys/wait.h>

#include <cstdint>

namespace ramses::internal
{
    TestForkingController::TestForkingController()
    {
        for (const auto& testPipeNames : m_testPipeNames)
        {
            auto p1 = std::make_unique<NamedPipe>(testPipeNames.first, true);
            auto p2 = std::make_unique<NamedPipe>(testPipeNames.second, true);
            m_testPipes.push_back({std::move(p1), std::move(p2)});
        }

        startForkerApplication();
    }

    TestForkingController::~TestForkingController()
    {
        stopForkerApplication();
        [[maybe_unused]] pid_t processEndStatus = ::waitpid(m_testForkerApplicationProcessId, nullptr, 0);
        assert(m_testForkerApplicationProcessId == processEndStatus);
    }

    void TestForkingController::startTestApplication(uint32_t testAppIdx)
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingController::startApplication starting test application :" << testAppIdx);
        assert(testAppIdx < m_testPipes.size());
        sendForkRequest(testAppIdx);
    }

    void TestForkingController::waitForTestApplicationExit(uint32_t testAppIdx)
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingController::waitForTestApplicationExit waiting for test application to exit :" << testAppIdx);
        assert(testAppIdx < m_testPipes.size());
        sendWaitForExitRequest(testAppIdx);
    }

    void TestForkingController::sendMessageToTestApplication(const BinaryOutputStream& os, uint32_t testAppIdx)
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingController::sendMessageToTestApplication :" << testAppIdx);
        assert(testAppIdx < m_testPipes.size());

        const auto dataSize = static_cast<uint32_t>(os.getSize());
        if (!m_testPipes[testAppIdx].testToWaylandClientPipe->write(&dataSize, sizeof(dataSize)))
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingController::sendMessageToTestApplication failed to write data size to pipe!");
        }
        if (!m_testPipes[testAppIdx].testToWaylandClientPipe->write(os.getData(), dataSize))
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingController::sendMessageToTestApplication failed to write data to pipe!");
        }
    }

    void TestForkingController::startForkerApplication()
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
            TestForkerApplication forkerApplication(m_testToForkerPipe.getName(), {m_testPipeNames.cbegin(), m_testPipeNames.cend()});
            forkerApplication.run();
            exit(0);
        }

        //open pipes in parent process
        m_testToForkerPipe.open();
        for(auto& testPipe : m_testPipes)
        {
            testPipe.testToWaylandClientPipe->open();
            testPipe.waylandClientToTestPipe->open();
        }
    }

    void TestForkingController::stopForkerApplication()
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingController::stopForkerApplication(): sending message stop forker");
        const ETestForkerApplicationMessage message = ETestForkerApplicationMessage::StopForkerApplication;
        m_testToForkerPipe.write(&message, sizeof(ETestForkerApplicationMessage));
    }

    void TestForkingController::sendForkRequest(uint32_t testAppIdx)
    {
        const ETestForkerApplicationMessage message = ETestForkerApplicationMessage::ForkTestApplication;

        if (!m_testToForkerPipe.write(&message, sizeof(ETestForkerApplicationMessage))
            || !m_testToForkerPipe.write(&testAppIdx, sizeof(testAppIdx)))
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingController::sendForkRequest error " << NamedPipe::getSystemErrorStatus() << " when wrinting fork request pipe");
        }
    }

    void TestForkingController::sendWaitForExitRequest(uint32_t testAppIdx)
    {
        const ETestForkerApplicationMessage message = ETestForkerApplicationMessage::WaitForTestApplicationExit;

        if (!m_testToForkerPipe.write(&message, sizeof(ETestForkerApplicationMessage))
            || !m_testToForkerPipe.write(&testAppIdx, sizeof(testAppIdx)))
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingController::sendWaitForExitRequest error " << NamedPipe::getSystemErrorStatus() << " when writing wait for test application exit pipe");
        }
    }

    void TestForkingController::killTestApplication(uint32_t testAppIdx)
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingController::killTestApplication(): sending message kill test application");
        const ETestForkerApplicationMessage message = ETestForkerApplicationMessage::KillTestApplication;

        if (!m_testToForkerPipe.write(&message, sizeof(ETestForkerApplicationMessage))
            || !m_testToForkerPipe.write(&testAppIdx, sizeof(testAppIdx)))
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingController::killTestApplication error " << NamedPipe::getSystemErrorStatus());
        }
    }
}
