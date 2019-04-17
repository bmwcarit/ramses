//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FRAMETIMER_H
#define RAMSES_FRAMETIMER_H

#include "PlatformAbstraction/PlatformTime.h"
#include <limits>
#include <array>

namespace ramses_internal
{
    enum class EFrameTimerSectionBudget
    {
        ClientResourcesUpload = 0,
        SceneResourcesUpload,
        SceneActionsApply,
        OffscreenBufferRender,

        COUNT
    };

    class FrameTimer
    {
    public:
        using Clock = std::chrono::steady_clock;

        FrameTimer()
        {
            std::fill(m_sectionBudgets.begin(), m_sectionBudgets.end(), PlatformTime::InfiniteDuration);
            m_frameStartTimeStamp = Clock::now();
        }

        void startFrame()
        {
            m_frameStartTimeStamp = Clock::now();
        }

        void setSectionTimeBudget(EFrameTimerSectionBudget section, UInt64 timeBudgetInMicrosecs)
        {
            // clamp given time budget to InfiniteDuration to avoid potential overflow of chrono when using numeric max of UInt64 or chrono max
            timeBudgetInMicrosecs = std::min<UInt64>(timeBudgetInMicrosecs, std::chrono::duration_cast<Duration>(PlatformTime::InfiniteDuration).count());
            m_sectionBudgets[static_cast<size_t>(section)] = Duration(timeBudgetInMicrosecs);
        }

        Bool isTimeBudgetExceededForSection(EFrameTimerSectionBudget section) const
        {
            const auto sectionDuration = Clock::now() - m_frameStartTimeStamp;
            return sectionDuration >= m_sectionBudgets[static_cast<size_t>(section)];
        }

        std::chrono::microseconds getTimeBudgetForSection(EFrameTimerSectionBudget section) const
        {
            return m_sectionBudgets[static_cast<size_t>(section)];
        }

        Clock::time_point getFrameStartTime() const
        {
            return m_frameStartTimeStamp;
        }

    private:
        using Duration = std::chrono::microseconds;

        Clock::time_point m_frameStartTimeStamp;
        std::array<Duration, static_cast<size_t>(EFrameTimerSectionBudget::COUNT)> m_sectionBudgets;
    };
}

#endif
