//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ForceFallbackImage.h"
#include "RamsesClientImpl.h"
#include "SceneCommandTypes.h"

namespace ramses_internal
{
    ForceFallbackImage::ForceFallbackImage(ramses::RamsesClientImpl& client)
        : m_client(client)
    {
        description = "force fallback image";
        registerKeyword("forceFallbackImage");
        getArgument<0>().setDescription("fallback flag");
        getArgument<1>().setDescription("scene id");
        getArgument<2>().setDescription("stream texture name");
    }

    Bool ForceFallbackImage::execute(UInt32& forceFallback, uint64_t& sceneId, String& streamTextureName) const
    {
        ForceFallbackCommand command;
        command.streamTextureName = streamTextureName;
        command.forceFallback     = forceFallback != 0u;

        m_client.enqueueSceneCommand(ramses::sceneId_t(sceneId), command);
        return true;
    }
}
