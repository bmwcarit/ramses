//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/DisplayThread.h"
#include "RendererLib/DisplayBundle.h"
#include "Utils/ThreadLocalLog.h"

namespace ramses_internal
{
    DisplayThread::DisplayThread(DisplayBundleShared displayBundle, DisplayHandle displayHandle, IThreadAliveNotifier& notifier)
        : m_displayHandle{ displayHandle }
        , m_display{ std::move(displayBundle) }
        , m_thread{ String{ fmt::format("R_DisplayThrd{}", displayHandle) } }
        , m_notifier{ notifier }
        , m_aliveIdentifier{ notifier.registerThread() }
    {
    }

    DisplayThread::~DisplayThread()
    {
        if (m_thread.isRunning())
        {
            {
                std::lock_guard<std::mutex> lock{ m_lock };
                m_thread.cancel();
                m_sleepConditionVar.notify_one();
            }
            m_thread.join();
        }
        m_notifier.unregisterThread(m_aliveIdentifier);
    }

    void DisplayThread::startUpdating()
    {
        LOG_INFO_P(CONTEXT_RENDERER, "{}: DisplayThread starting", m_displayHandle);

        if (!m_thread.isRunning())
            m_thread.start(*this);

        std::lock_guard<std::mutex> lock{ m_lock };
        m_isUpdating = true;
        m_sleepConditionVar.notify_one();
    }

    void DisplayThread::stopUpdating()
    {
        LOG_INFO_P(CONTEXT_RENDERER, "{}: DisplayThread stopping", m_displayHandle);
        std::lock_guard<std::mutex> lock{ m_lock };
        m_isUpdating = false;
    }

    void DisplayThread::setLoopMode(ELoopMode loopMode)
    {
        LOG_INFO_P(CONTEXT_RENDERER, "{}: DisplayThread loop mode set to {}", m_displayHandle, loopMode == ELoopMode::UpdateAndRender ? "UpdateAndRender" : "UpdateOnly");
        std::lock_guard<std::mutex> lock{ m_lock };
        m_loopMode = loopMode;
    }

    void DisplayThread::setMinFrameDuration(std::chrono::microseconds minLoopPeriod)
    {
        LOG_INFO_P(CONTEXT_RENDERER, "DisplayThread min frame duration set to {} us", minLoopPeriod.count());
        std::lock_guard<std::mutex> lock{ m_lock };
        m_minFrameDuration = minLoopPeriod;
    }

    void DisplayThread::run()
    {
#if defined(__ghs__) && defined(RAMSES_RENDER_THREAD_PRIORITY)
        setThreadPriorityIntegrity(RAMSES_RENDER_THREAD_PRIORITY, "display thread");
#endif
        ThreadLocalLog::SetPrefix(static_cast<int>(m_displayHandle.asMemoryHandle()));

        std::chrono::milliseconds lastLoopSleepTime{ 0u };
        while (!isCancelRequested())
        {
            bool doUpdate = false;
            std::chrono::microseconds minimumFrameDuration{ 0 };
            ELoopMode loopMode = ELoopMode::UpdateAndRender;
            {
                std::lock_guard<std::mutex> lock{ m_lock };
                minimumFrameDuration = m_minFrameDuration;
                doUpdate = m_isUpdating;
                loopMode = m_loopMode;
            }

            m_notifier.notifyAlive(m_aliveIdentifier);
            if (!doUpdate)
            {
                std::unique_lock<std::mutex> lock{ m_lock };
                // need to recheck conditions to avoid race between start/stop/dtor
                if (!isCancelRequested() && !m_isUpdating)
                    m_sleepConditionVar.wait_for(lock, m_notifier.calculateTimeout());
            }
            else
            {
                auto loopStartTime = std::chrono::steady_clock::now();
                m_display->doOneLoop(loopMode, lastLoopSleepTime);
                const auto loopEndTime = std::chrono::steady_clock::now();

                const auto currentLoopDuration = std::chrono::duration_cast<std::chrono::microseconds>(loopEndTime - loopStartTime);
                lastLoopSleepTime = sleepToControlFramerate(currentLoopDuration, minimumFrameDuration);
            }
        }

        // release display before thread exit, it might contain platform components that need to be deinitialized in same thread
        LOG_INFO_RP(CONTEXT_RENDERER, "DisplayThread releasing display bundle components");
        m_display.destroy();
    }

    std::chrono::milliseconds DisplayThread::sleepToControlFramerate(std::chrono::microseconds loopDuration, std::chrono::microseconds minimumFrameDuration)
    {
        std::chrono::milliseconds sleepTime{ 0 };
        if (loopDuration < minimumFrameDuration)
        {
            // we use millisecond sleep precision, this will cast microseconds to whole milliseconds (floor)
            // so that we do not sleep more than necessary
            sleepTime = std::chrono::duration_cast<std::chrono::milliseconds>(minimumFrameDuration - loopDuration);
            if (sleepTime.count() > 0)
                std::this_thread::sleep_for(sleepTime);
        }

        return sleepTime;
    }
}
