//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RamshCommands/SystemCompositorControllerSetSurfaceDestRectangle.h"

namespace ramses::internal
{
    SystemCompositorControllerSetSurfaceDestRectangle::SystemCompositorControllerSetSurfaceDestRectangle(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "set rectangle of a surface in the system compositor";
        registerKeyword("scSetSurfaceRectangle");
        registerKeyword("screct");

        getArgument<0>().setDescription("surface id");
        getArgument<1>().setDescription("x");
        getArgument<2>().setDescription("y");
        getArgument<3>().setDescription("width");
        getArgument<4>().setDescription("height");
    }

    bool SystemCompositorControllerSetSurfaceDestRectangle::execute(int32_t& surfaceId, int32_t& x, int32_t& y, int32_t& width, int32_t& height) const
    {
        m_rendererCommandBuffer.enqueueCommand(ramses::internal::RendererCommand::SCSetIviSurfaceDestRectangle{ WaylandIviSurfaceId(surfaceId), x, y, width ,height });
        return true;
    }
}

