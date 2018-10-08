/*
 * Copyright (C) 2015 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RAMSES_CAPU_WINDOWS_SIGNAL_H
#define RAMSES_CAPU_WINDOWS_SIGNAL_H

#include <signal.h>
#include "ramses-capu/Error.h"

namespace ramses_capu
{
    /// Enum of possible signals.
    enum ESignal
    {
        CAPU_SIGABRT = SIGABRT,
        CAPU_SIGFPE = SIGFPE,
        CAPU_SIGILL = SIGILL,
        CAPU_SIGINT = SIGINT,
        CAPU_SIGSEGV = SIGSEGV,
        CAPU_SIGTERM = SIGTERM
    };

    namespace os
    {
        /// Signal handler abstraction class.
        class Signal
        {
        public:

            /// Registers a signal handler.
            /** @param sig The signal.
             *  @param func Signal handler function, which is called when the signal happens.
             *  @return Returns the previous installed signal handler. */
            static void (*signal(ESignal sig, void (*func)(int32_t)))(int32_t)
            {
                return ::signal(static_cast<int32_t>(sig), (func != 0) ? func : SIG_DFL);
            }

            /// Generates a signal.
            /** @param sig The signal.
             *  @return Returns CAPU_OK when successful, and CAPU_ERROR otherwise. */
            static status_t raise(ESignal sig)
            {
                int32_t retval = ::raise(sig);
                return (retval == 0) ? CAPU_OK : CAPU_ERROR;
            }
        };
    }
}
#endif // RAMSES_CAPU_WINDOWS_SIGNAL_H
