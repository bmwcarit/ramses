//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/RendererCommands.h"
#include <mutex>
#include <condition_variable>

namespace ramses::internal
{
    class RendererCommandBuffer
    {
    public:
        template <typename T>
        void enqueueCommand(T cmd);
        void addAndConsumeCommandsFrom(RendererCommands& cmds);
        void swapCommands(RendererCommands& cmds);

        void blockingSwapCommands(RendererCommands& cmds, std::chrono::milliseconds timeout);
        void interruptBlockingSwapCommands();

    private:
        std::mutex m_lock;
        RendererCommands m_commands;

        std::condition_variable m_newCommandsCvar;
        bool m_interruptBlockingSwapCommands = false;
    };

    template <typename T>
    void RendererCommandBuffer::enqueueCommand(T cmd)
    {
        std::lock_guard<std::mutex> lock(m_lock);
        m_commands.push_back(std::move(cmd));
        m_newCommandsCvar.notify_all();
    }
}
