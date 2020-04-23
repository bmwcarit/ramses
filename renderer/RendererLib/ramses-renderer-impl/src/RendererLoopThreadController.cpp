//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "RendererLoopThreadController.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "Watchdog/PlatformWatchdog.h"
#include "RamsesRendererUtils.h"
#include "RendererLib/WindowedRenderer.h"

namespace ramses_internal
{
    RendererLoopThreadController::RendererLoopThreadController(WindowedRenderer& windowedRenderer, PlatformWatchdog& watchdog, std::chrono::milliseconds loopCountPeriod)
        : m_windowedRenderer(&windowedRenderer)
        , m_watchdog(watchdog)
        , m_thread("R_RendererThrd")
        , m_lock()
        , m_doRendering(false)
        , m_targetMinimumFrameDuration(std::chrono::microseconds(std::chrono::seconds(1)) / 60)  // 60fps
        , m_threadStarted(false)
        , m_destroyRenderer(false)
        , m_loopCountPeriod(loopCountPeriod)
    {
    }

    RendererLoopThreadController::~RendererLoopThreadController()
    {
        if (m_threadStarted)
        {
            m_thread.cancel();
            m_sleepConditionVar.notify_one();
            m_thread.join();
        }
    }

    Bool RendererLoopThreadController::startRendering()
    {
        {
            std::lock_guard<std::mutex> guard(m_lock);
            if (m_doRendering)
            {
                return false;
            }

            m_doRendering = true;
        }

        if (!m_threadStarted)
        {
            m_thread.start(*this);
            m_threadStarted = true;
        }
        m_sleepConditionVar.notify_one();

        return true;
    }

    Bool RendererLoopThreadController::isRendering() const
    {
        std::lock_guard<std::mutex> guard(m_lock);
        return m_doRendering;
    }

    Bool RendererLoopThreadController::stopRendering()
    {
        std::lock_guard<std::mutex> guard(m_lock);
        if (!m_doRendering)
        {
            return false;
        }

        m_doRendering = false;
        return true;
    }

    void RendererLoopThreadController::run()
    {
        UInt64 loopStartTime = PlatformTime::GetMicrosecondsMonotonic();
        std::chrono::milliseconds lastLoopSleepTime{ 0u };

        while (!isCancelRequested())
        {
            Bool doRendering = false;
            Bool destroyRenderer = false;
            std::chrono::microseconds minimumFrameDuration{ 0 };
            ELoopMode loopMode = ELoopMode::UpdateAndRender;
            {
                std::lock_guard<std::mutex> guard(m_lock);
                minimumFrameDuration = m_targetMinimumFrameDuration;
                doRendering = m_doRendering;
                destroyRenderer = m_destroyRenderer;
                loopMode = m_loopMode;
            }

            if (destroyRenderer)
            {
                std::lock_guard<std::mutex> guard(m_lock);
                delete m_windowedRenderer;
                m_windowedRenderer = nullptr;
                m_destroyRenderer = false;
                m_rendererDestroyedCondVar.notify_one();
            }
            else if (!doRendering)
            {
                std::unique_lock<std::mutex> l(m_lock);
                m_sleepConditionVar.wait_for(l, std::chrono::milliseconds{m_watchdog.calculateTimeout()});
            }
            else
            {
                assert(m_windowedRenderer != nullptr);
                m_windowedRenderer->doOneLoop(loopMode, lastLoopSleepTime);

                const UInt64 loopEndTime = PlatformTime::GetMicrosecondsMonotonic();
                assert(loopEndTime >= loopStartTime);
                const std::chrono::microseconds currentLoopDuration{ loopEndTime - loopStartTime };
                lastLoopSleepTime = sleepToControlFramerate(currentLoopDuration, minimumFrameDuration);
                calculateLooptimeAverage(currentLoopDuration + lastLoopSleepTime, loopEndTime);
                loopStartTime = PlatformTime::GetMicrosecondsMonotonic();
            }

            m_watchdog.notifyWatchdog();
        }
    }

    void RendererLoopThreadController::calculateLooptimeAverage(const std::chrono::microseconds currentLoopDuration, const uint64_t loopEndTime)
    {
        if (m_loopCountPeriod.count() > 0)
        {
            m_sumOfLoopTimeInPeriod += currentLoopDuration;
            m_numberOfLoopsInPeriod++;
            m_maximumLoopTimeInPeriod = std::max(m_maximumLoopTimeInPeriod, currentLoopDuration);


            if (loopEndTime >= m_lastPeriodLoopCountReportingTimeMicroseconds + std::chrono::microseconds(m_loopCountPeriod).count())
            {
                m_windowedRenderer->fireLoopTimingReportRendererEvent(m_maximumLoopTimeInPeriod, std::chrono::microseconds(m_sumOfLoopTimeInPeriod.count() / m_numberOfLoopsInPeriod));
                m_sumOfLoopTimeInPeriod = std::chrono::microseconds(0);
                m_numberOfLoopsInPeriod = 0;
                m_maximumLoopTimeInPeriod = std::chrono::microseconds(0);
                m_lastPeriodLoopCountReportingTimeMicroseconds = loopEndTime;
            }
        }
    }

    std::chrono::milliseconds RendererLoopThreadController::sleepToControlFramerate(std::chrono::microseconds loopDuration, std::chrono::microseconds minimumFrameDuration)
    {
        if (minimumFrameDuration > loopDuration)
        {
            // we use millisecond sleep precision, this will cast microseconds to whole milliseconds (floor)
            // so that we do not sleep more than necessary
            const std::chrono::milliseconds neededSleepDuration = std::chrono::duration_cast<std::chrono::milliseconds>(minimumFrameDuration - loopDuration);
            if (neededSleepDuration.count() > 0)
            {
                PlatformThread::Sleep(static_cast<UInt32>(neededSleepDuration.count()));
                return neededSleepDuration;
            }
        }

        return std::chrono::milliseconds{ 0u };
    }

    void RendererLoopThreadController::setMaximumFramerate(Float maximumFramerate)
    {
        std::lock_guard<std::mutex> guard(m_lock);
        m_targetMinimumFrameDuration = std::chrono::microseconds(static_cast<UInt>(1e6f / maximumFramerate));
    }


    Float RendererLoopThreadController::getMaximumFramerate() const
    {
        std::lock_guard<std::mutex> guard(m_lock);
        using float_seconds = std::chrono::duration<float, std::ratio<1>>;
        return 1.0f / std::chrono::duration_cast<float_seconds>(m_targetMinimumFrameDuration).count();
    }

    void RendererLoopThreadController::setLoopMode(ELoopMode loopMode)
    {
        std::lock_guard<std::mutex> guard(m_lock);
        m_loopMode = loopMode;
    }

    void RendererLoopThreadController::destroyRenderer()
    {
        std::unique_lock<std::mutex> l(m_lock);
        m_destroyRenderer = true;
        m_sleepConditionVar.notify_one();
        m_rendererDestroyedCondVar.wait(l, [&]() { return !m_windowedRenderer; });
    }
}
