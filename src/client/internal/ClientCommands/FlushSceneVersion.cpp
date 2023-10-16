//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "FlushSceneVersion.h"
#include "impl/RamsesClientImpl.h"
#include "SceneCommandBuffer.h"

namespace ramses::internal
{
    FlushSceneVersion::FlushSceneVersion(RamsesClientImpl& client)
        : m_client(client)
    {
        description = "add scene version to next flush";
        registerKeyword("sceneversion");
        getArgument<0>().setDescription("version");
        getArgument<1>().setDescription("scene id");
    }

    bool FlushSceneVersion::execute(sceneVersionTag_t& sceneVersion, uint64_t& sceneId) const
    {
        SceneCommandFlushSceneVersion command;
        command.sceneVersion = sceneVersion;

        m_client.enqueueSceneCommand(sceneId_t(sceneId), std::move(command));
        return true;
    }
}
