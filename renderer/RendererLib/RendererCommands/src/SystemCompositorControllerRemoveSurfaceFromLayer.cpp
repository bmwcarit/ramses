//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererCommands/SystemCompositorControllerRemoveSurfaceFromLayer.h"
#include "Ramsh/RamshInput.h"

namespace ramses_internal
{
    SystemCompositorControllerRemoveSurfaceFromLayer::SystemCompositorControllerRemoveSurfaceFromLayer(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "remove an ivi surface from an ivi layer";
        registerKeyword("scRemoveSurfaceFromLayer");
        registerKeyword("scrsfl");

        getArgument<0>().setDescription("surface ivi id");
        getArgument<1>().setDescription("layer ivi id");
    }

    Bool SystemCompositorControllerRemoveSurfaceFromLayer::execute(UInt32& surfaceId, UInt32& layerId) const
    {
        m_rendererCommandBuffer.systemCompositorControllerRemoveIviSurfaceFromIviLayer(WaylandIviSurfaceId(surfaceId), WaylandIviLayerId(layerId));
        return true;
    }
}

