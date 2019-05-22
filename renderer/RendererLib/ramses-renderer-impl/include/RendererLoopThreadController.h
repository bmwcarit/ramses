//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERLOOPTHREADCONTROLLER_H
#define RAMSES_RENDERERLOOPTHREADCONTROLLER_H

#include "RendererAPI/ELoopMode.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "PlatformAbstraction/PlatformConditionVariable.h"

namespace ramses_internal
{
    class PlatformWatchdog;
    class WindowedRenderer;

    class RendererLoopThreadController : public Runnable
    {
    public:
        RendererLoopThreadController(WindowedRenderer& windowedRenderer, PlatformWatchdog& watchdog);
        ~RendererLoopThreadController();

        Bool startRendering();
        Bool isRendering() const;
        Bool stopRendering();
        void setMaximumFramerate(Float maximumFramerate);
        Float getMaximumFramerate() const;
        void setLoopMode(ELoopMode loopMode);

        void destroyRenderer();

    private:
        virtual void run() override;
        std::chrono::milliseconds sleepToControlFramerate(std::chrono::microseconds loopDuration, std::chrono::microseconds minimumFrameDuration);

        WindowedRenderer* m_windowedRenderer;
        PlatformWatchdog& m_watchdog;
        PlatformThread m_thread;
        mutable PlatformLightweightLock m_lock;
        PlatformConditionVariable m_sleepConditionVar;
        Bool m_doRendering;
        std::chrono::microseconds m_minimumFrameDuration;
        Bool m_threadStarted;
        ELoopMode m_loopMode = ELoopMode_UpdateAndRender;
        PlatformConditionVariable m_rendererDestroyedCondVar;

        Bool m_destroyRenderer;
    };
}

#endif
