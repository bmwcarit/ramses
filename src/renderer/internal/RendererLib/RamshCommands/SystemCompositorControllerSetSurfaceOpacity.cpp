//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RamshCommands/SystemCompositorControllerSetSurfaceOpacity.h"

namespace ramses::internal
{
    SystemCompositorControllerSetSurfaceOpacity::SystemCompositorControllerSetSurfaceOpacity(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "set opacity of a surface via the system compositor";
        registerKeyword("scSetSurfaceOpacity");
        registerKeyword("sco");

        getArgument<0>().setDescription("surface id");
        getArgument<1>().setDescription("opacity");
    }

    bool SystemCompositorControllerSetSurfaceOpacity::execute(int32_t& surfaceId, float& opacity) const
    {
        m_rendererCommandBuffer.enqueueCommand(ramses::internal::RendererCommand::SCSetIviSurfaceOpacity{ WaylandIviSurfaceId(surfaceId), opacity });
        return true;
    }
}

