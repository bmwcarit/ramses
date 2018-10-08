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
        SceneActionsApply,
        OffscreenBufferRender,

        COUNT
    };

    class FrameTimer
    {
    public:
        FrameTimer()
        {
            std::fill(m_sectionBudgets.begin(), m_sectionBudgets.end(), std::numeric_limits<UInt64>::max());
        }

        void startFrame()
        {
            m_frameStartTimeStamp = PlatformTime::GetMicrosecondsMonotonic();
        }

        void setSectionTimeBudget(EFrameTimerSectionBudget section, UInt64 timeBudgetInMicrosecs)
        {
            m_sectionBudgets[static_cast<size_t>(section)] = timeBudgetInMicrosecs;
        }

        Bool isTimeBudgetExceededForSection(EFrameTimerSectionBudget section) const
        {
            const UInt64 currTime = PlatformTime::GetMicrosecondsMonotonic();
            const UInt64 sectionTime = currTime - m_frameStartTimeStamp;
            return sectionTime >= m_sectionBudgets[static_cast<size_t>(section)];
        }

        UInt64 getTimeBudgetForSection(EFrameTimerSectionBudget section) const
        {
            return m_sectionBudgets[static_cast<size_t>(section)];
        }

    private:
        UInt64 m_frameStartTimeStamp = 0u;
        std::array<UInt64, static_cast<size_t>(EFrameTimerSectionBudget::COUNT)> m_sectionBudgets;
    };
}

#endif
