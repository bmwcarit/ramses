//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PlatformAbstraction/synchronized_clock.h"

#ifdef RAMSES_LINUX_USE_DEV_PTP
#include "Utils/LogMacros.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace ramses_internal
{
    namespace synchronized_clock_impl
    {
        clockid_t GetClockId()
        {
            int fd = open("/dev/ptp0", O_RDONLY);
            if (fd < 0)
            {
                LOG_WARN(CONTEXT_FRAMEWORK, "PlatformTimePTP::GetClockId: failed to open /dev/ptp0 with error: " << strerror(errno));
                return 0;
            }
            const int baseClockFd = 3;
            const clockid_t clockId = (~static_cast<clockid_t>(fd) << 3) | baseClockFd;

            // test if getting time with clockId works
            struct timespec dummyTs;
            if (clock_gettime(clockId, &dummyTs) != 0)
            {
                LOG_WARN(CONTEXT_FRAMEWORK, "PlatformTimePTP::GetClockId: clock_gettime failed (fd:" << fd << ", clockId: " << clockId << ") with error: " << strerror(errno));
                return 0;
            }
            return clockId;
        }
    }
}
#endif
