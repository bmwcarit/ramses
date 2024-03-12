//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "RendererCommands/SetSceneState.h"
#include "RendererLib/RendererCommandBuffer.h"

namespace ramses_internal
{
    SetSceneState::SetSceneState(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "Sets the scene state";
        registerKeyword("scenestate");
        getArgument<0>().setDescription("scene id");
        getArgument<1>().setDescription("3:Rendered 2:Ready 1:Available 0:Unavailable");
    }

    Bool SetSceneState::execute(uint64_t& sceneId, uint32_t& sceneState) const
    {
        if (sceneState >= 4)
        {
            LOG_ERROR_P(CONTEXT_RAMSH, "Invalid scene state: {}", sceneState);
            return false;
        }
        m_rendererCommandBuffer.enqueueCommand(ramses_internal::RendererCommand::SetSceneState{ SceneId{sceneId}, static_cast<RendererSceneState>(sceneState) });
        return true;
    }
}
