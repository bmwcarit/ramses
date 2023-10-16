//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/framework/IThreadWatchdogNotification.h"

namespace ramses::internal
{
    class ThreadWatchdogConfig
    {
    public:
        ThreadWatchdogConfig() = default;

        void setWatchdogNotificationInterval(ERamsesThreadIdentifier thread, uint32_t interval)
        {
            switch(thread)
            {
            case ERamsesThreadIdentifier::Workers:
                m_workerThreadsWatchdogInterval = interval;
                break;
            case ERamsesThreadIdentifier::Renderer:
                m_rendererThreadWatchdogInterval = interval;
                break;
            case ERamsesThreadIdentifier::Unknown:
                break;
            }
        }

        void setThreadWatchDogCallback(IThreadWatchdogNotification* callback)
        {
            m_callback = callback;
        }

        [[nodiscard]] uint32_t getWatchdogNotificationInterval(ERamsesThreadIdentifier thread) const
        {
            switch (thread)
            {
            case ERamsesThreadIdentifier::Workers:
                return m_workerThreadsWatchdogInterval;
            case ERamsesThreadIdentifier::Renderer:
                return m_rendererThreadWatchdogInterval;
            case ERamsesThreadIdentifier::Unknown:
                return 0;
            }
            return 0;
        }

        [[nodiscard]] IThreadWatchdogNotification* getCallBack() const
        {
            return m_callback;
        }
    private:
        IThreadWatchdogNotification* m_callback{nullptr};
        uint32_t m_workerThreadsWatchdogInterval{1000u};
        uint32_t m_rendererThreadWatchdogInterval{1000u};
    };
}
