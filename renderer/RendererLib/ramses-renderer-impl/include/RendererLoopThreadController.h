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
#include "Watchdog/IThreadAliveNotifier.h"
#include <mutex>
#include <condition_variable>

namespace ramses_internal
{
    class DisplayDispatcher;

    class RendererLoopThreadController : public Runnable
    {
    public:
        RendererLoopThreadController(DisplayDispatcher& displayDispatcher, IThreadAliveNotifier& watchdog);
        ~RendererLoopThreadController() override;

        Bool startRendering();
        Bool isRendering() const;
        Bool stopRendering();
        void setMaximumFramerate(Float maximumFramerate);
        Float getMaximumFramerate() const;

        void destroyRenderer();

    private:
        virtual void run() override;

        std::chrono::milliseconds sleepToControlFramerate(std::chrono::microseconds loopDuration, std::chrono::microseconds minimumFrameDuration);

        DisplayDispatcher* m_displayDispatcher;
        IThreadAliveNotifier& m_watchdog;
        const uint64_t m_aliveIdentifier;
        PlatformThread m_thread;
        mutable std::mutex m_lock;
        std::condition_variable m_sleepConditionVar;
        Bool m_doRendering;
        std::chrono::microseconds m_targetMinimumFrameDuration;
        Bool m_threadStarted;
        std::condition_variable m_rendererDestroyedCondVar;
        Bool m_destroyRenderer;
    };
}

#endif
