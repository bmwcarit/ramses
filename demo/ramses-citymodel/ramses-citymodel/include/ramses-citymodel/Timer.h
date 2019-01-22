//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_TIMER_H
#define RAMSES_CITYMODEL_TIMER_H

#include "chrono"
#include "ctime"

/// Class for time measuring.
class Timer
{
public:
    /// Constructor.
    Timer();

    /// Returns the elapsed time in seconds.
    /** @return The elapsed time. */
    float getTime() const;

    /// Returns the elapsed CPU time in seconds.
    /** @return The elapsed cpu time. */
    float getCpuTime() const;

    /// Reset the start time.
    void reset();

private:
    /// Start (real) time of the measured period.
    std::chrono::steady_clock::time_point mStartTime;

    /// Start (CPU) time of the measured period.
    std::clock_t mCpuStartTime = 0;
};

#endif
