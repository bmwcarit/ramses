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
    RendererLoopThreadController::RendererLoopThreadController(WindowedRenderer& windowedRenderer, PlatformWatchdog& watchdog)
        : m_windowedRenderer(&windowedRenderer)
        , m_watchdog(watchdog)
        , m_thread("R_RendererThrd")
        , m_lock()
        , m_doRendering(false)
        , m_minimumFrameDuration(std::chrono::microseconds(std::chrono::seconds(1)) / 60)  // 60fps
        , m_threadStarted(false)
        , m_destroyRenderer(false)
    {
    }

    RendererLoopThreadController::~RendererLoopThreadController()
    {
        if (m_threadStarted)
        {
            m_thread.cancel();
            m_sleepConditionVar.signal();
            m_thread.join();
        }
    }

    Bool RendererLoopThreadController::startRendering()
    {
        {
            PlatformLightweightGuard guard(m_lock);
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
        m_sleepConditionVar.signal();

        return true;
    }

    Bool RendererLoopThreadController::isRendering() const
    {
        PlatformLightweightGuard guard(m_lock);
        return m_doRendering;
    }

    Bool RendererLoopThreadController::stopRendering()
    {
        PlatformLightweightGuard guard(m_lock);
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
            ELoopMode loopMode = ELoopMode_UpdateAndRender;
            {
                PlatformLightweightGuard guard(m_lock);
                minimumFrameDuration = m_minimumFrameDuration;
                doRendering = m_doRendering;
                destroyRenderer = m_destroyRenderer;
                loopMode = m_loopMode;
            }

            if (destroyRenderer)
            {
                PlatformLightweightGuard guard(m_lock);
                delete m_windowedRenderer;
                m_windowedRenderer = NULL;
                m_destroyRenderer = false;
                m_rendererDestroyedCondVar.signal();
            }
            else if (!doRendering)
            {
                PlatformLightweightGuard guard(m_lock);
                m_sleepConditionVar.wait(&m_lock, m_watchdog.calculateTimeout());
            }
            else
            {
                assert(m_windowedRenderer != NULL);
                ramses::RamsesRendererUtils::DoOneLoop(*m_windowedRenderer, loopMode, lastLoopSleepTime);

                const UInt64 loopEndTime = PlatformTime::GetMicrosecondsMonotonic();
                assert(loopEndTime >= loopStartTime);
                const std::chrono::microseconds loopDuration{ loopEndTime - loopStartTime };
                lastLoopSleepTime = sleepToControlFramerate(loopDuration, minimumFrameDuration);

                loopStartTime = PlatformTime::GetMicrosecondsMonotonic();
            }

            m_watchdog.notifyWatchdog();
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
        PlatformLightweightGuard guard(m_lock);
        m_minimumFrameDuration = std::chrono::microseconds(static_cast<UInt>(1e6f / maximumFramerate));
    }


    Float RendererLoopThreadController::getMaximumFramerate() const
    {
        PlatformLightweightGuard guard(m_lock);
        using float_seconds = std::chrono::duration<float, std::ratio<1>>;
        return 1.0f / std::chrono::duration_cast<float_seconds>(m_minimumFrameDuration).count();
    }

    void RendererLoopThreadController::setLoopMode(ELoopMode loopMode)
    {
        PlatformLightweightGuard guard(m_lock);
        m_loopMode = loopMode;
    }

    void RendererLoopThreadController::destroyRenderer()
    {
        PlatformLightweightGuard guard(m_lock);
        m_destroyRenderer = true;
        m_sleepConditionVar.signal();

        while (m_windowedRenderer)
        {
            m_rendererDestroyedCondVar.wait(&m_lock);
        }
    }
}
