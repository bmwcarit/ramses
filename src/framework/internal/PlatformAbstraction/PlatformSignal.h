//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <csignal>

namespace ramses::internal
{
    enum class ESignal
    {
        ABRT = SIGABRT,
        FPE = SIGFPE,
        ILL = SIGILL,
        INT = SIGINT,
        SEGV = SIGSEGV,
        TERM = SIGTERM
    };

    class PlatformSignal final
    {
    public:
        using SignalHandlerFunction = void(*)(int32_t);

        static void SetSignalHandler(ESignal sig, SignalHandlerFunction handler, bool chainPreviousHandler);
        static void RestoreSignalHandlers();
        static const char* SignalToString(ESignal sig);
    };
}
