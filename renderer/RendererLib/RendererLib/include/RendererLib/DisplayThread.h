//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYTHREAD_H
#define RAMSES_DISPLAYTHREAD_H

#include "RendererAPI/ELoopMode.h"
#include "RendererLib/DisplayBundle.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "Watchdog/IThreadAliveNotifier.h"

#include <mutex>
#include <condition_variable>
#include <chrono>

namespace ramses_internal
{
    constexpr static std::chrono::microseconds DefaultMinFrameDuration{ 1000000 / 60 };

    // Wrapper for a display bundle to solve dual ownership of display bundle.
    // Dispatcher owns it by default but if display threads are enabled the components of the bundle
    // must be deinitialized within the display thread.
    class DisplayBundleShared
    {
    public:
        DisplayBundleShared() = default;
        explicit DisplayBundleShared(std::unique_ptr<IDisplayBundle> displayBundle)
            : m_bundle{ std::make_shared<std::unique_ptr<IDisplayBundle>>(std::move(displayBundle)) }
        {}
        IDisplayBundle* operator->()
        {
            assert(m_bundle);
            return m_bundle->get();
        }
        IDisplayBundle& operator*()
        {
            assert(m_bundle);
            return **m_bundle;
        }
        const IDisplayBundle& operator*() const
        {
            assert(m_bundle);
            return **m_bundle;
        }
        void destroy()
        {
            m_bundle->reset();
        }
    private:
        std::shared_ptr<std::unique_ptr<IDisplayBundle>> m_bundle;
    };

    class IDisplayThread
    {
    public:
        virtual ~IDisplayThread() = default;

        virtual void startUpdating() = 0;
        virtual void stopUpdating() = 0;
        virtual void setLoopMode(ELoopMode loopMode) = 0;
        virtual void setMinFrameDuration(std::chrono::microseconds minLoopPeriod) = 0;
    };

    class DisplayThread final : public IDisplayThread, private Runnable
    {
    public:
        DisplayThread(DisplayBundleShared displayBundle, DisplayHandle displayHandle, IThreadAliveNotifier& notifier);
        virtual ~DisplayThread() override;

        virtual void startUpdating() override;
        virtual void stopUpdating() override;

        virtual void setLoopMode(ELoopMode loopMode) override;
        virtual void setMinFrameDuration(std::chrono::microseconds minLoopPeriod) override;

    private:
        virtual void run() override;

        std::chrono::milliseconds sleepToControlFramerate(std::chrono::microseconds loopDuration, std::chrono::microseconds minimumFrameDuration);

        const DisplayHandle m_displayHandle;
        DisplayBundleShared m_display;
        ELoopMode m_loopMode = ELoopMode::UpdateAndRender;
        std::chrono::microseconds m_minFrameDuration{ DefaultMinFrameDuration };

        PlatformThread m_thread;
        mutable std::mutex m_lock;
        bool m_isUpdating = false;
        std::condition_variable m_sleepConditionVar;

        IThreadAliveNotifier& m_notifier;
        const uint64_t m_aliveIdentifier;
    };
}

#endif
