//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RamshCommands/SystemCompositorControllerRemoveSurfaceFromLayer.h"

namespace ramses::internal
{
    SystemCompositorControllerRemoveSurfaceFromLayer::SystemCompositorControllerRemoveSurfaceFromLayer(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "remove an ivi-surface from an ivi-layer";
        registerKeyword("scRemoveSurfaceFromLayer");
        registerKeyword("scrsfl");

        getArgument<0>().setDescription("surface ivi id");
        getArgument<1>().setDescription("layer ivi id");
    }

    bool SystemCompositorControllerRemoveSurfaceFromLayer::execute(uint32_t& surfaceId, uint32_t& layerId) const
    {
        m_rendererCommandBuffer.enqueueCommand(ramses::internal::RendererCommand::SCRemoveIviSurfaceFromIviLayer{ WaylandIviSurfaceId(surfaceId), WaylandIviLayerId(layerId) });
        return true;
    }
}

