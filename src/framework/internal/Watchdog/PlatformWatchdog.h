//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/IThreadWatchdogNotification.h"

#include <chrono>
#include <string_view>

namespace ramses::internal
{
    inline
    static constexpr std::string_view EnumToString(const ERamsesThreadIdentifier& thread)
    {
        switch (thread)
        {
            case ERamsesThreadIdentifier::Workers: return "ERamsesThread_Workers";
            case ERamsesThreadIdentifier::Renderer: return "ERamsesThread_Renderer";
            case ERamsesThreadIdentifier::Unknown: return "ERamsesThread_Unknown";
        }
        return "";
    }

    class PlatformWatchdog
    {
    public:
        PlatformWatchdog(std::chrono::milliseconds notificationInterval, ERamsesThreadIdentifier thread, IThreadWatchdogNotification* callback);
        virtual ~PlatformWatchdog();

        void                      notifyWatchdog();
        [[nodiscard]] std::chrono::milliseconds calculateTimeout() const;

    private:
        const std::chrono::milliseconds m_interval;
        ERamsesThreadIdentifier m_thread;
        IThreadWatchdogNotification* m_watchdogCallback;
        std::chrono::milliseconds m_lastNotificationTime;
    };
}
