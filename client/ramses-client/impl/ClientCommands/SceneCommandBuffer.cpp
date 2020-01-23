//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneCommandBuffer.h"

namespace ramses_internal
{
    void SceneCommandBuffer::enqueueCommand(const SceneCommand& command)
    {
        switch(command.commandType)
        {
        case ESceneCommand_ForceFallbackImage:
            enqueueCommandInternal(command.convertTo<ForceFallbackCommand>());
            break;
        case ESceneCommand_FlushSceneVersion:
            enqueueCommandInternal(command.convertTo<FlushSceneVersionCommand>());
            break;
        case ESceneCommand_ValidationRequest:
            enqueueCommandInternal(command.convertTo<ValidationRequestCommand>());
            break;
        case ESceneCommand_DumpSceneToFile:
            enqueueCommandInternal(command.convertTo<DumpSceneToFileCommand>());
            break;
        case ESceneCommand_LogResourceMemoryUsage:
            enqueueCommandInternal(command.convertTo<LogResourceMemoryUsageCommand>());
            break;
        default:
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "SceneCommandBuffer::enqueueCommand Trying to enqueue unknown command \"" << getSceneCommandName(command.commandType) << "\"" );
            break;
        }
    }

    void SceneCommandBuffer::exchangeContainerData(SceneCommandContainer& container)
    {
        std::lock_guard<std::mutex> guard(m_lock);
        m_container.swap(container);
    }
}
