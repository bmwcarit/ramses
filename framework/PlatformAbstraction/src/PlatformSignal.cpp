//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PlatformAbstraction/PlatformSignal.h"
#include "Collections/HashMap.h"
#include <csignal>

namespace ramses_internal
{
    namespace {
        struct HandlerInfo
        {
            PlatformSignal::SignalHandlerFunction handler;
            PlatformSignal::SignalHandlerFunction previousHandler;
            bool chainPrevious;
            bool active;
        };

        HashMap<ESignal, HandlerInfo> gSignalMap;

        void SignalHandlerDispatcher(int sig)
        {
            ESignal enumSig = static_cast<ESignal>(sig);
            HandlerInfo info;
            if (EStatus::Ok == gSignalMap.get(enumSig, info))
            {
                info.handler(sig);
                if (info.chainPrevious && !info.active)
                {
                    info.active = true;
                    gSignalMap.put(enumSig, info);

                    ::signal(sig, info.previousHandler);
                    ::raise(sig);

                    info.active = false;
                    gSignalMap.put(enumSig, info);
                }
            }
        }

        PlatformSignal::SignalHandlerFunction InstallSignalHandler(ESignal sig, PlatformSignal::SignalHandlerFunction handler)
        {
#ifdef __INTEGRITY
            UNUSED(sig);
            UNUSED(handler);
            return PlatformSignal::SignalHandlerFunction(0);
#else
            return ::signal(static_cast<int>(sig), (handler != nullptr) ? handler : SIG_DFL);
#endif
        }
    }

    void PlatformSignal::SetSignalHandler(ESignal sig, SignalHandlerFunction handler, bool chainPreviousHandler)
    {
        SignalHandlerFunction previousHandler = InstallSignalHandler(sig, SignalHandlerDispatcher);

        HandlerInfo info;
        gSignalMap.get(sig, info); // get current state if exists

        info.previousHandler = nullptr;
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

    void PlatformSignal::RestoreSignalHandlers()
    {
        for(auto signalHandler : gSignalMap)
        {
            HandlerInfo& info = signalHandler.value;
            SignalHandlerFunction previousHandler = InstallSignalHandler(signalHandler.key, SignalHandlerDispatcher);
            if (previousHandler != SignalHandlerDispatcher)
            {
                info.previousHandler = previousHandler;
            }
        }
    }

    const char* PlatformSignal::SignalToString(ESignal sig)
    {
        switch (sig)
        {
        case ESignal::ABRT: return "SIGABRT";
        case ESignal::FPE:  return "SIGFPE";
        case ESignal::ILL:  return "SIGILL";
        case ESignal::INT:  return "SIGINT";
        case ESignal::SEGV: return "SIGSEGV";
        case ESignal::TERM: return "SIGTERM";
        default: return "SIG unknown";
        }
    }
}
