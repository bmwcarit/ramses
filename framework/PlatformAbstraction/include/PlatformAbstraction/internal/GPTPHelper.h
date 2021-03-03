//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_GPTPHELPER_H
#define RAMSES_GPTPHELPER_H

#include "PlatformAbstraction/synchronized_clock.h"
#include "Utils/LogMacros.h"
#include <atomic>

namespace ramses_internal
{
    class GPTPHelper
    {
        public:
            synchronized_clock::time_point evaluate(const GPTP_SyncState& state, uint64_t time)
            {
                if (state != lastState)
                {
                    LOG_INFO(CONTEXT_FRAMEWORK, "synchronized_clock::now: changed status from " << lastState << " to " << state);
                    lastState = state;
                }
                // good case
                if (state == GPTP_SyncState_Synced)
                {
                    lastSuccessfulSync = time;
                    return synchronized_clock::time_point(std::chrono::nanoseconds(time));
                }
                // never synced, always bad
                if (state == GPTP_SyncState_Unsynced)
                    return synchronized_clock::time_point(std::chrono::nanoseconds(0));

                // sync lost, only ok for 60 seconds
                if (state == GPTP_SyncState_SyncLost)
                {
                    if (std::chrono::nanoseconds(time - lastSuccessfulSync) >= std::chrono::seconds(60))
                    {
                        return synchronized_clock::time_point(std::chrono::nanoseconds(0));
                    }
                    else
                    {
                        return synchronized_clock::time_point(std::chrono::nanoseconds(time));
                    }
                }

                if (state == GPTP_SyncState_GrandMaster)
                {
                    LOG_WARN(CONTEXT_FRAMEWORK, "synchronized_clock::now: should not be grandmaster, returning 0");
                    return synchronized_clock::time_point(std::chrono::nanoseconds(0));
                }

                return synchronized_clock::time_point(std::chrono::nanoseconds(0));
        }
        private:
            GPTP_SyncState lastState = GPTP_SyncState_Unsynced;
            std::atomic<uint64_t> lastSuccessfulSync;
    };
}

#endif
