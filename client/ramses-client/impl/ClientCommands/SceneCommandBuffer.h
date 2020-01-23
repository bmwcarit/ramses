//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENECOMMANDBUFFER_H
#define RAMSES_SCENECOMMANDBUFFER_H

#include "SceneCommandContainer.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "Utils/LogMacros.h"


namespace ramses_internal
{
    class SceneCommandBuffer
    {
    public:
        void enqueueCommand(const SceneCommand& command);
        void exchangeContainerData(SceneCommandContainer& container);
    private:
        template< typename COMMAND_TYPE >
        void enqueueCommandInternal(const COMMAND_TYPE& command);

        std::mutex m_lock;
        SceneCommandContainer m_container;
    };

    template< typename COMMAND_TYPE >
    void SceneCommandBuffer::enqueueCommandInternal(const COMMAND_TYPE& command)
    {
        std::lock_guard<std::mutex> guard(m_lock);
        m_container.addCommand(command.commandType, command);
    }
}

#endif
