//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "internal/RendererLib/RamshCommands/TriggerPickEvent.h"
#include "internal/RendererLib/RendererCommandBuffer.h"

namespace ramses::internal
{
    TriggerPickEvent::TriggerPickEvent(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "Pick scene at X and Y coordinates normalized to -1,1 with bottom left being -1,-1";
        registerKeyword("pick");
        getArgument<0>().setDescription("scene id");
        getArgument<1>().setDescription("normalized pick coordinate X");
        getArgument<2>().setDescription("normalized pick coordinate Y");
    }

    bool TriggerPickEvent::execute(uint64_t& sceneId, float& pickCoordX, float& pickCoordY) const
    {
        m_rendererCommandBuffer.enqueueCommand(RendererCommand::PickEvent{ SceneId{sceneId}, glm::vec2{pickCoordX, pickCoordY} });
        return true;
    }
}
