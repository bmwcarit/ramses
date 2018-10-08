//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TESTFORKINGCONTROLLER_H
#define RAMSES_TESTFORKINGCONTROLLER_H

#include "Collections/String.h"
#include "NamedPipe.h"
#include "EmbeddedCompositingTestMessages.h"
#include "RendererAPI/IEmbeddedCompositingManager.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    class TestForkingController
    {
    public:
        TestForkingController(const String& waylandSocket);
        TestForkingController(const TestForkingController&) = delete;
        TestForkingController& operator=(const TestForkingController&) = delete;

        void startTestApplication();
        void waitForTestApplicationExit();
        void sendMessageToTestApplication(ETestWaylandApplicationMessage message);
        void sendStringToTestApplication(const String& string);
        template <typename T>
        void sendMessageToTestApplication(ETestWaylandApplicationMessage message, const T& params);
        template <typename T>
        bool getAnswerFromTestApplication(T& value, IEmbeddedCompositingManager& embeddedCompositingManager);
        void deinitialize();
        void killTestApplication();

    private:
        void startForkerApplication(const String& waylandSocket);
        void stopForkerApplication();
        void sendForkRequest();
        void sendWaitForExitRequest();

        NamedPipe       m_testToForkerPipe;
        NamedPipe       m_testToWaylandClientPipe;
        NamedPipe       m_waylandClientToTestPipe;
        pid_t           m_testForkerApplicationProcessId;
    };

    template <typename T>
    void TestForkingController::sendMessageToTestApplication(ETestWaylandApplicationMessage message, const T& params)
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingController::sendMessageToTestApplication message: " << message);
        UInt32 messageAsUInt = static_cast<UInt32>(message);
        if (!m_testToWaylandClientPipe.write(&messageAsUInt, sizeof(messageAsUInt)) ||
            !m_testToWaylandClientPipe.write(&params, sizeof(params)))
        {
            LOG_ERROR(CONTEXT_RENDERER, "TestForkingControllerController::sendMessageToTestApplication(" << messageAsUInt << "): failed to write message to pipe!");
        }
    }

    template <typename T>
    bool TestForkingController::getAnswerFromTestApplication(T& value, IEmbeddedCompositingManager& embeddedCompositingManager)
    {
        LOG_INFO(CONTEXT_RENDERER, "TestForkingController::getAnswerFromTestApplication");

        UNUSED(embeddedCompositingManager)
        const uint32_t timeOutInMS = 20;
        while (!m_waylandClientToTestPipe.waitOnData(timeOutInMS))
        {
            embeddedCompositingManager.processClientRequests();
        }

        const EReadFromPipeStatus readingStatus = m_waylandClientToTestPipe.read(&value, sizeof(value));
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
