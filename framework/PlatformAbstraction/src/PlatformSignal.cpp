//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PlatformAbstraction/PlatformSignal.h"
#include "Collections/HashMap.h"
#include "Common/Cpp11Macros.h"
#include <signal.h>

namespace ramses_internal
{
    namespace {
        struct HandlerInfo
        {
            Signal::SignalHandlerFunction handler;
            Signal::SignalHandlerFunction previousHandler;
            bool chainPrevious;
            bool active;
        };

        static HashMap<ramses_capu::ESignal, HandlerInfo> gSignalMap;

        static void SignalHandlerDispatcher(int sig)
        {
            ramses_capu::ESignal enumSig = static_cast<ramses_capu::ESignal>(sig);
            HandlerInfo info;
            if (EStatus_RAMSES_OK == gSignalMap.get(enumSig, info))
            {
                info.handler(sig);
                if (info.chainPrevious && !info.active)
                {
                    info.active = true;
                    gSignalMap.put(enumSig, info);

                    signal(sig, info.previousHandler);
                    raise(sig);

                    info.active = false;
                    gSignalMap.put(enumSig, info);
                }
            }
        }
    }

    void Signal::SetSignalHandler(ramses_capu::ESignal sig, SignalHandlerFunction handler, bool chainPreviousHandler)
    {
        SignalHandlerFunction previousHandler = signal(sig, SignalHandlerDispatcher);

        HandlerInfo info;
        gSignalMap.get(sig, info); // get current state if exists

        info.previousHandler = 0;
        info.handler = handler;
        info.chainPrevious = chainPreviousHandler;
        info.active = false;

        // only store/overwrite previous handler if it wasn't us
        if (previousHandler != SignalHandlerDispatcher)
        {
            info.previousHandler = previousHandler;
        }

        gSignalMap.put(sig, info);
    }

    void Signal::RestoreSignalHandlers()
    {
        ramses_foreach(gSignalMap, it)
        {
            HandlerInfo& info = it->value;
            SignalHandlerFunction previousHandler = signal(it->key, SignalHandlerDispatcher);
            if (previousHandler != SignalHandlerDispatcher)
            {
                info.previousHandler = previousHandler;
            }
        }
    }

    const char* Signal::SignalToString(ramses_capu::ESignal sig)
    {
        switch (sig)
        {
        case ramses_capu::CAPU_SIGABRT: return "SIGABRT";
        case ramses_capu::CAPU_SIGFPE:  return "SIGFPE";
        case ramses_capu::CAPU_SIGILL:  return "SIGILL";
        case ramses_capu::CAPU_SIGINT:  return "SIGINT";
        case ramses_capu::CAPU_SIGSEGV: return "SIGSEGV";
        case ramses_capu::CAPU_SIGTERM: return "SIGTERM";
        default: return "SIG unknown";
        }
    }

    void Signal::DisableSigPipe()
    {
#ifdef __linux__
        ::signal(SIGPIPE, SIG_IGN);
#endif
    }
}
