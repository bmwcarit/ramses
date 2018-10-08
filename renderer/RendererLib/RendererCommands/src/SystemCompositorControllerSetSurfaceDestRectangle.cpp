//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererCommands/SystemCompositorControllerSetSurfaceDestRectangle.h"
#include "Ramsh/RamshInput.h"

namespace ramses_internal
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

    Bool SystemCompositorControllerSetSurfaceDestRectangle::execute(Int32& surfaceId, Int32& x, Int32& y, Int32& width, Int32& height) const
    {
        m_rendererCommandBuffer.systemCompositorControllerSetIviSurfaceDestRectangle(WaylandIviSurfaceId(surfaceId), x, y, width ,height);
        return true;
    }
}

