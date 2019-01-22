//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/Timer.h"

Timer::Timer()
{
    reset();
}

float Timer::getTime() const
{
    const auto timeDelta = std::chrono::steady_clock::now() - mStartTime;
    return std::chrono::duration_cast<std::chrono::duration<float>>(timeDelta).count();
}

float Timer::getCpuTime() const
{
    clock_t timestamp;
    timestamp = std::clock();
    return static_cast<float>(timestamp - mCpuStartTime) / CLOCKS_PER_SEC;
}

void Timer::reset()
{
    mStartTime    = std::chrono::steady_clock::now();
    mCpuStartTime = std::clock();
}
