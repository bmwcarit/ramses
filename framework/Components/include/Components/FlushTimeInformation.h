//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FLUSHTIMEINFORMATION_H
#define RAMSES_FLUSHTIMEINFORMATION_H

#include <chrono>

namespace ramses_internal
{
    using FlushTimeClock = std::chrono::system_clock;

    struct FlushTimeInformation
    {
        FlushTimeInformation(FlushTimeClock::duration limit = std::chrono::milliseconds(0), FlushTimeClock::time_point externalTS = FlushTimeClock::time_point(std::chrono::milliseconds(0)), FlushTimeClock::time_point internalTS = FlushTimeClock::time_point(std::chrono::milliseconds(0)))
            : latencyLimit(limit)
            , externalTimestamp(externalTS)
            , internalTimestamp(internalTS)
        {
        }

        FlushTimeClock::duration latencyLimit;
        FlushTimeClock::time_point externalTimestamp;
        FlushTimeClock::time_point internalTimestamp;
    };

    inline bool operator==(const FlushTimeInformation& a, const FlushTimeInformation& b)
    {
        return a.latencyLimit == b.latencyLimit &&
            a.externalTimestamp == b.externalTimestamp &&
            a.internalTimestamp == b.internalTimestamp;
    }

    inline bool operator!=(const FlushTimeInformation& a, const FlushTimeInformation& b)
    {
        return !(a == b);
    }
}

#endif
