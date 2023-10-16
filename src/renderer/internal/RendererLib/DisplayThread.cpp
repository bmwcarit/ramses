//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/DisplayThread.h"
#include "internal/RendererLib/DisplayBundle.h"
#include "internal/Core/Utils/ThreadLocalLog.h"

namespace ramses::internal
{
    DisplayThread::DisplayThread(DisplayBundleShared displayBundle, DisplayHandle displayHandle, IThreadAliveNotifier& notifier)
        : m_displayHandle{ displayHandle }
        , m_display{ std::move(displayBundle) }
        , m_thread{ fmt::format("R_DispThrd{}", displayHandle) }
        , m_notifier{ notifier }
        , m_aliveIdentifier{ notifier.registerThread() }
    {
    }

    DisplayThread::~DisplayThread()
    {
        if (m_thread.isRunning())
        {
            LOG_INFO_P(CONTEXT_RENDERER, "{}: DisplayThread stopping", m_displayHandle);
            {
                std::lock_guard<std::mutex> lock{ m_lock };
                m_thread.cancel();
                m_sleepConditionVar.notify_one();
            }
            m_thread.join();
            LOG_INFO_P(CONTEXT_RENDERER, "{}: DisplayThread stopped", m_displayHandle);
        }
        m_notifier.unregisterThread(m_aliveIdentifier);
    }

    void DisplayThread::startUpdating()
    {
        if (!m_thread.isRunning())
        {
            LOG_INFO_P(CONTEXT_RENDERER, "{}: DisplayThread starting", m_displayHandle);
            m_thread.start(*this);
            LOG_INFO_P(CONTEXT_RENDERER, "{}: DisplayThread started", m_displayHandle);
        }

        LOG_INFO_P(CONTEXT_RENDERER, "{}: DisplayThread start update", m_displayHandle);
        std::lock_guard<std::mutex> lock{ m_lock };
        m_isUpdating = true;
        m_sleepConditionVar.notify_one();
    }

    void DisplayThread::stopUpdating()
    {
        LOG_INFO_P(CONTEXT_RENDERER, "{}: DisplayThread stop update", m_displayHandle);
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
        ThreadLocalLog::SetPrefix(static_cast<int>(m_displayHandle.asMemoryHandle()));

        std::chrono::milliseconds lastLoopSleepTime{ 0u };
        while (!isCancelRequested())
        {
            bool doUpdate = false;
            std::chrono::microseconds minimumFrameDuration{ 0 };

            m_display->traceId() = 10000;
            ELoopMode loopMode = ELoopMode::UpdateAndRender;
            {
                std::lock_guard<std::mutex> lock{ m_lock };
                minimumFrameDuration = m_minFrameDuration;
                doUpdate = m_isUpdating;
                loopMode = m_loopMode;
            }

            m_display->traceId() = 10001;
            m_notifier.notifyAlive(m_aliveIdentifier);
            m_display->traceId() = 10002;
            if (!doUpdate)
            {
                std::unique_lock<std::mutex> lock{ m_lock };
                // need to recheck conditions to avoid race between start/stop/dtor
                m_display->traceId() = 10003;
                if (!isCancelRequested() && !m_isUpdating)
                    m_sleepConditionVar.wait_for(lock, m_notifier.calculateTimeout());
            }
            else
            {
                m_display->traceId() = 10004;
                auto loopStartTime = std::chrono::steady_clock::now();
                m_display->doOneLoop(loopMode, lastLoopSleepTime);
                const auto loopEndTime = std::chrono::steady_clock::now();

                m_display->traceId() = 10005;
                const auto currentLoopDuration = std::chrono::duration_cast<std::chrono::microseconds>(loopEndTime - loopStartTime);
                lastLoopSleepTime = SleepToControlFramerate(currentLoopDuration, minimumFrameDuration);
                m_display->traceId() = 10006;
            }

            m_frameCounter++;
        }

        // release display before thread exit, it might contain platform components that need to be deinitialized in same thread
        LOG_INFO_RP(CONTEXT_RENDERER, "DisplayThread releasing display bundle components");
        m_display.destroy();
    }

    std::chrono::milliseconds DisplayThread::SleepToControlFramerate(std::chrono::microseconds loopDuration, std::chrono::microseconds minimumFrameDuration)
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

    uint32_t DisplayThread::getFrameCounter() const
    {
        return m_frameCounter;
    }
}
