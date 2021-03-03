//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogResourceMemoryUsage.h"
#include "RamsesClientImpl.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    LogResourceMemoryUsage::LogResourceMemoryUsage(ramses::RamsesClientImpl& client)
        : m_client(client)
    {
        description = "Log memory usage of resources in scene";
        registerKeyword("logmem");
        registerKeyword("lm");
    }

    Bool LogResourceMemoryUsage::execute(uint64_t& sceneId) const
    {
        LOG_INFO(CONTEXT_CLIENT,"LogResourceMemoryUsage");
        SceneCommandLogResourceMemoryUsage command;
        m_client.enqueueSceneCommand(ramses::sceneId_t(sceneId), std::move(command));
        return true;
    }
}
