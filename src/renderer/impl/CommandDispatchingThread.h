//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Enums/ELoopMode.h"
#include "internal/RendererLib/RendererCommands.h"
#include "internal/PlatformAbstraction/PlatformThread.h"
#include "internal/Watchdog/IThreadAliveNotifier.h"
#include <mutex>
#include <condition_variable>

namespace ramses::internal
{
    class DisplayDispatcher;
    class RendererCommandBuffer;

    class CommandDispatchingThread : public Runnable
    {
    public:
        CommandDispatchingThread(DisplayDispatcher& displayDispatcher, RendererCommandBuffer& commandBuffer, IThreadAliveNotifier& watchdog);
        ~CommandDispatchingThread() override;

    private:
        void run() override;

        DisplayDispatcher& m_displayDispatcher;
        RendererCommandBuffer& m_commandBuffer;
        IThreadAliveNotifier& m_watchdog;
        const uint64_t m_aliveIdentifier;

        PlatformThread m_thread;

        // to avoid re-allocs
        RendererCommands m_tmpCommands;
    };
}
