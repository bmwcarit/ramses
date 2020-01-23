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

        PlatformSignal::SetSignalHandler(ESignal::ABRT, HandleSignalCallback, true);
        PlatformSignal::SetSignalHandler(ESignal::FPE, HandleSignalCallback, true);
        PlatformSignal::SetSignalHandler(ESignal::ILL, HandleSignalCallback, true);
        PlatformSignal::SetSignalHandler(ESignal::INT, HandleSignalCallback, true);
        PlatformSignal::SetSignalHandler(ESignal::SEGV, HandleSignalCallback, true);
        PlatformSignal::SetSignalHandler(ESignal::TERM, HandleSignalCallback, true);
    }

    TestSignalHandler::TestSignalHandler()
    {
    }

    void TestSignalHandler::HandleSignalCallback(int32_t signal)
    {
        LOG_ERROR(CONTEXT_RENDERER, "SignalHandler::HandleSignalCallback() Received signal " << PlatformSignal::SignalToString(static_cast<ESignal>(signal)) << " In Process :[" << ProcessName << "]! Sending signal to process group ...");
        //send same signal to process group
        kill(0, signal);
    }
}
