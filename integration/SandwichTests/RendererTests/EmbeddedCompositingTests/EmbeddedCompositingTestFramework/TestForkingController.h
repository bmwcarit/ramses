//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TESTFORKINGCONTROLLER_H
#define RAMSES_TESTFORKINGCONTROLLER_H

#include "NamedPipe.h"
#include "EmbeddedCompositingTestMessages.h"
#include "RendererAPI/IEmbeddedCompositingManager.h"
#include "Utils/LogMacros.h"
#include "Utils/BinaryOutputStream.h"

#include <memory>
#include <array>
#include <string>

namespace ramses_internal
{
    class TestForkingController
    {
    public:
        explicit TestForkingController();
        ~TestForkingController();
        TestForkingController(const TestForkingController&) = delete;
        TestForkingController& operator=(const TestForkingController&) = delete;

        void startTestApplication(uint32_t testAppIdx);
        void waitForTestApplicationExit(uint32_t testAppIdx);
        void sendMessageToTestApplication(const BinaryOutputStream& os, uint32_t testAppIdx);
        template <typename T>
        bool getAnswerFromTestApplication(T& value, IEmbeddedCompositingManager& embeddedCompositingManager, uint32_t testAppIdx);
        void killTestApplication(uint32_t testAppIdx);

    private:
        struct TestPipes
        {
            std::unique_ptr<NamedPipe> testToWaylandClientPipe;
            std::unique_ptr<NamedPipe> waylandClientToTestPipe;
        };

        void startForkerApplication();
        void stopForkerApplication();
        void sendForkRequest(uint32_t testAppIdx);
        void sendWaitForExitRequest(uint32_t testAppIdx);

        const std::array<std::pair<std::string,std::string>, 2> m_testPipeNames{{
                                                                    {"/tmp/ramses-ec-tests-testToWaylandClientPipe-0", "/tmp/ramses-ec-tests-waylandClientToTestPipe-0"},
                                                                    {"/tmp/ramses-ec-tests-testToWaylandClientPipe-1", "/tmp/ramses-ec-tests-waylandClientToTestPipe-1"}
                                                                }};

        NamedPipe               m_testToForkerPipe{"/tmp/ramses-ec-tests-testToForkerPipe", true};
        std::vector<TestPipes>  m_testPipes;
        pid_t                   m_testForkerApplicationProcessId = -1;
    };

    template <typename T>
    bool TestForkingController::getAnswerFromTestApplication(T& value, IEmbeddedCompositingManager& embeddedCompositingManager, uint32_t testAppIdx)
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingController::getAnswerFromTestApplication");
        assert(testAppIdx < m_testPipes.size());

        const uint32_t timeOutInMS = 20;
        while (!m_testPipes[testAppIdx].waylandClientToTestPipe->waitOnData(timeOutInMS))
        {
            embeddedCompositingManager.processClientRequests();
        }

        const EReadFromPipeStatus readingStatus = m_testPipes[testAppIdx].waylandClientToTestPipe->read(&value, sizeof(value));
        switch (readingStatus)
        {
        case EReadFromPipeStatus_Failure:
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingController::getAnswerFromTestApplication(): can not read from pipe");
            return false;
        case EReadFromPipeStatus_Closed:
            LOG_WARN(CONTEXT_RENDERER, "TestForkingController::getAnswerFromTestApplication(): pipe closed");
            return false;
        case EReadFromPipeStatus_Empty:
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingController::getAnswerFromTestApplication(): pipe empty");
            return false;
        default:
            return true;
        }
    }
}

#endif
