//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererCommands/SystemCompositorControllerDestroySurface.h"

namespace ramses_internal
{
    SystemCompositorControllerDestroySurface::SystemCompositorControllerDestroySurface(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "destroy an ivi-surface";
        registerKeyword("scDestroySurface");
        registerKeyword("scds");

        getArgument<0>().setDescription("surface ivi id");
    }

    Bool SystemCompositorControllerDestroySurface::execute(UInt32& surfaceId) const
    {
        m_rendererCommandBuffer.enqueueCommand(ramses_internal::RendererCommand::SCDestroyIviSurface{ WaylandIviSurfaceId(surfaceId) });
        return true;
    }
}

