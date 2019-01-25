//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FLUSHTIMEINFORMATION_H
#define RAMSES_FLUSHTIMEINFORMATION_H

#include "PlatformAbstraction/synchronized_clock.h"

namespace ramses_internal
{
    namespace FlushTime
    {
        using Clock = synchronized_clock;

        const Clock::time_point InvalidTimestamp{ std::chrono::milliseconds(0) };
    }

    struct FlushTimeInformation
    {
        FlushTimeInformation() = default;
        FlushTimeInformation(FlushTime::Clock::time_point expirationTS, FlushTime::Clock::time_point internalTS)
            : expirationTimestamp(expirationTS)
            , internalTimestamp(internalTS)
        {
        }

        FlushTime::Clock::time_point expirationTimestamp = FlushTime::InvalidTimestamp;
        FlushTime::Clock::time_point internalTimestamp = FlushTime::InvalidTimestamp;
    };

    inline bool operator==(const FlushTimeInformation& a, const FlushTimeInformation& b)
    {
        return
            a.expirationTimestamp == b.expirationTimestamp &&
            a.internalTimestamp == b.internalTimestamp;
    }

    inline bool operator!=(const FlushTimeInformation& a, const FlushTimeInformation& b)
    {
        return !(a == b);
    }
}

#endif
