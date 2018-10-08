//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestSignalHandler.h"
#include "PlatformAbstraction/PlatformSignal.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    String TestSignalHandler::ProcessName;

    void TestSignalHandler::RegisterSignalHandlersForCurrentProcess(String processName)
    {
        ProcessName = processName;

        Signal::SetSignalHandler(ramses_capu::CAPU_SIGABRT, HandleSignalCallback, true);
        Signal::SetSignalHandler(ramses_capu::CAPU_SIGFPE, HandleSignalCallback, true);
        Signal::SetSignalHandler(ramses_capu::CAPU_SIGILL, HandleSignalCallback, true);
        Signal::SetSignalHandler(ramses_capu::CAPU_SIGINT, HandleSignalCallback, true);
        Signal::SetSignalHandler(ramses_capu::CAPU_SIGSEGV, HandleSignalCallback, true);
        Signal::SetSignalHandler(ramses_capu::CAPU_SIGTERM, HandleSignalCallback, true);
    }

    TestSignalHandler::TestSignalHandler()
    {
    }

    void TestSignalHandler::HandleSignalCallback(int32_t signal)
    {
        LOG_ERROR(CONTEXT_RENDERER, "SignalHandler::HandleSignalCallback() Received signal " << Signal::SignalToString(static_cast<ramses_capu::ESignal>(signal)) << " In Process :[" << ProcessName << "]! Sending signal to process group ...");
        //send same signal to process group
        kill(0, signal);
    }
}
