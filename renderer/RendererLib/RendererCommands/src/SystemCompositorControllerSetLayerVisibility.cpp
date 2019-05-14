//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererCommands/SystemCompositorControllerSetLayerVisibility.h"
#include "Ramsh/RamshInput.h"

namespace ramses_internal
{
    SystemCompositorControllerSetLayerVisibility::SystemCompositorControllerSetLayerVisibility(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "set visibility of a layer in the system compositor";
        registerKeyword("scSetLayerVisibility");
        registerKeyword("sclv");

        getArgument<0>().setDescription("layer id");
        getArgument<1>().setDescription("visibility");
    }

    Bool SystemCompositorControllerSetLayerVisibility::execute(Int32& layerId, Int32& visibility) const
    {
        m_rendererCommandBuffer.systemCompositorControllerSetIviLayerVisibility(WaylandIviLayerId(layerId), visibility != 0);
        return true;
    }
}

