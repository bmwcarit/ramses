//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMABSTRACTION_PLATFORMSIGNAL_H
#define RAMSES_PLATFORMABSTRACTION_PLATFORMSIGNAL_H

#include <ramses-capu/os/Signal.h>

namespace ramses_internal
{
    class Signal : public ramses_capu::os::Signal
    {
    public:
        typedef void (*SignalHandlerFunction)(int32_t);

        using ramses_capu::os::Signal::signal;
        using ramses_capu::os::Signal::raise;

        static void SetSignalHandler(ramses_capu::ESignal sig, SignalHandlerFunction handler, bool chainPreviousHandler);
        static void RestoreSignalHandlers();
        static const char* SignalToString(ramses_capu::ESignal sig);

        static void DisableSigPipe();
    };

    typedef ramses_capu::ESignal ESignal;
}

#endif
