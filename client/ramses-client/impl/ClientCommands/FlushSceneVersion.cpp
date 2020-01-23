//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "FlushSceneVersion.h"
#include "RamsesClientImpl.h"
#include "SceneCommandTypes.h"

namespace ramses_internal
{
    FlushSceneVersion::FlushSceneVersion(ramses::RamsesClientImpl& client)
        : m_client(client)
    {
        description = "add scene version to next flush";
        registerKeyword("sceneversion");
        getArgument<0>().setDescription("version");
        getArgument<1>().setDescription("scene id");
    }

    Bool FlushSceneVersion::execute(ramses::sceneVersionTag_t& sceneVersion, uint64_t& sceneId) const
    {
        FlushSceneVersionCommand command;
        command.sceneVersion = sceneVersion;

        m_client.enqueueSceneCommand(ramses::sceneId_t(sceneId), command);
        return true;
    }
}
