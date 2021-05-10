//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RendererCommandBuffer.h"

namespace ramses_internal
{
    void RendererCommandBuffer::addAndConsumeCommandsFrom(RendererCommands& cmds)
    {
        std::lock_guard<std::mutex> lock(m_lock);
        m_commands.insert(m_commands.end(), std::make_move_iterator(cmds.begin()), std::make_move_iterator(cmds.end()));
        cmds.clear();
        m_newCommandsCvar.notify_all();
    }

    void RendererCommandBuffer::swapCommands(RendererCommands& cmds)
    {
        std::lock_guard<std::mutex> lock(m_lock);
        m_commands.swap(cmds);
    }

    std::tuple<std::mutex&, std::condition_variable&> RendererCommandBuffer::getCVarForNewCommands()
    {
        return { m_lock, m_newCommandsCvar };
    }

    void RendererCommandBuffer::swapCommands_unsafe(RendererCommands& cmds)
    {
        m_commands.swap(cmds);
    }
}
