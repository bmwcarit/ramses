//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RamshCommands/SystemCompositorControllerSetSurfaceVisibility.h"

namespace ramses::internal
{
    SystemCompositorControllerSetSurfaceVisibility::SystemCompositorControllerSetSurfaceVisibility(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "set visibility of a surface in the system compositor";
        registerKeyword("scSetSurfaceVisibility");
        registerKeyword("scv");

        getArgument<0>().setDescription("surface id");
        getArgument<1>().setDescription("visibility");
    }

    bool SystemCompositorControllerSetSurfaceVisibility::execute(int32_t& surfaceId, int32_t& visibility) const
    {
        m_rendererCommandBuffer.enqueueCommand(ramses::internal::RendererCommand::SCSetIviSurfaceVisibility{ WaylandIviSurfaceId(surfaceId), visibility != 0 });
        return true;
    }
}

