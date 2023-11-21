//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestSignalHandler.h"
#include "internal/PlatformAbstraction/PlatformSignal.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    std::string TestSignalHandler::ProcessName;

    void TestSignalHandler::RegisterSignalHandlersForCurrentProcess(std::string_view processName)
    {
        ProcessName = processName;

        PlatformSignal::SetSignalHandler(ESignal::ABRT, HandleSignalCallback, true);
        PlatformSignal::SetSignalHandler(ESignal::FPE, HandleSignalCallback, true);
        PlatformSignal::SetSignalHandler(ESignal::ILL, HandleSignalCallback, true);
        PlatformSignal::SetSignalHandler(ESignal::INT, HandleSignalCallback, true);
        PlatformSignal::SetSignalHandler(ESignal::SEGV, HandleSignalCallback, true);
        PlatformSignal::SetSignalHandler(ESignal::TERM, HandleSignalCallback, true);
    }

    TestSignalHandler::TestSignalHandler() = default;

    void TestSignalHandler::HandleSignalCallback(int32_t signal)
    {
        LOG_ERROR(CONTEXT_RENDERER, "SignalHandler::HandleSignalCallback() Received signal {} In Process :[{}]! Sending signal to process group ...", PlatformSignal::SignalToString(static_cast<ESignal>(signal)), ProcessName);
        //send same signal to process group
        kill(0, signal);
    }
}
