//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "CommandDispatchingThread.h"
#include "internal/RendererLib/DisplayDispatcher.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/PlatformAbstraction/PlatformTime.h"
#include "internal/Watchdog/PlatformWatchdog.h"

namespace ramses::internal
{
    CommandDispatchingThread::CommandDispatchingThread(DisplayDispatcher& displayDispatcher, RendererCommandBuffer& commandBuffer, IThreadAliveNotifier& watchdog)
        : m_displayDispatcher{ displayDispatcher }
        , m_commandBuffer{ commandBuffer }
        , m_watchdog{ watchdog }
        , m_aliveIdentifier{ watchdog.registerThread() }
        , m_thread{ "R_RendererThrd" }
    {
        LOG_INFO(CONTEXT_RENDERER, "Main renderer thread starting");
        m_thread.start(*this);
        LOG_INFO(CONTEXT_RENDERER, "Main renderer thread started");
    }

    CommandDispatchingThread::~CommandDispatchingThread()
    {
        LOG_INFO(CONTEXT_RENDERER, "Main renderer thread stopping");
        m_thread.cancel();
        m_commandBuffer.interruptBlockingSwapCommands();
        m_thread.join();
        LOG_INFO(CONTEXT_RENDERER, "Main renderer thread stopped");

        m_watchdog.unregisterThread(m_aliveIdentifier);
    }

    void CommandDispatchingThread::run()
    {
        while (!isCancelRequested())
        {
            m_tmpCommands.clear();
            m_commandBuffer.blockingSwapCommands(m_tmpCommands, std::chrono::milliseconds{ m_watchdog.calculateTimeout() });
            m_watchdog.notifyAlive(m_aliveIdentifier);

            if (!m_tmpCommands.empty())
                m_displayDispatcher.dispatchCommands(m_tmpCommands);
        }
    }
}
