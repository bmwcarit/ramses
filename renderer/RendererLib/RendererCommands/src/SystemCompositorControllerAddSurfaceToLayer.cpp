//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererCommands/SystemCompositorControllerAddSurfaceToLayer.h"

namespace ramses_internal
{
    SystemCompositorControllerAddSurfaceToLayer::SystemCompositorControllerAddSurfaceToLayer(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "adds an ivi surface to an ivi layer";
        registerKeyword("scAddSurfaceToLayer");
        registerKeyword("scastl");

        getArgument<0>().setDescription("surface ivi id");
        getArgument<1>().setDescription("layer ivi id");
    }

    Bool SystemCompositorControllerAddSurfaceToLayer::execute(UInt32& surfaceId, UInt32& layerId) const
    {
        m_rendererCommandBuffer.enqueueCommand(ramses_internal::RendererCommand::SCAddIviSurfaceToIviLayer{ WaylandIviSurfaceId(surfaceId), WaylandIviLayerId(layerId) });
        return true;
    }
}

