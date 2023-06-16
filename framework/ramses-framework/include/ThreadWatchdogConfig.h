//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_THREADWATCHDOGCONFIG_H
#define RAMSES_THREADWATCHDOGCONFIG_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/IThreadWatchdogNotification.h"

namespace ramses_internal
{
    class ThreadWatchdogConfig
    {
    public:
        ThreadWatchdogConfig()
            : m_callback(nullptr)
            , m_workerThreadsWatchdogInterval(1000)
            , m_rendererThreadWatchdogInterval(1000)
        {
        }
        void setWatchdogNotificationInterval(ramses::ERamsesThreadIdentifier thread, uint32_t interval)
        {
            switch(thread)
            {
            case ramses::ERamsesThreadIdentifier::Workers:
                m_workerThreadsWatchdogInterval = interval;
                break;
            case ramses::ERamsesThreadIdentifier::Renderer:
                m_rendererThreadWatchdogInterval = interval;
                break;
            case ramses::ERamsesThreadIdentifier::Unknown:
                break;
            }
        }

        void setThreadWatchDogCallback(ramses::IThreadWatchdogNotification* callback)
        {
            m_callback = callback;
        }

        [[nodiscard]] uint32_t getWatchdogNotificationInterval(ramses::ERamsesThreadIdentifier thread) const
        {
            switch (thread)
            {
            case ramses::ERamsesThreadIdentifier::Workers:
                return m_workerThreadsWatchdogInterval;
            case ramses::ERamsesThreadIdentifier::Renderer:
                return m_rendererThreadWatchdogInterval;
            case ramses::ERamsesThreadIdentifier::Unknown:
                return 0;
            }
            return 0;
        }

        [[nodiscard]] ramses::IThreadWatchdogNotification* getCallBack() const
        {
            return m_callback;
        }
    private:
        ramses::IThreadWatchdogNotification* m_callback;
        uint32_t m_workerThreadsWatchdogInterval;
        uint32_t m_rendererThreadWatchdogInterval;
    };
}

#endif
