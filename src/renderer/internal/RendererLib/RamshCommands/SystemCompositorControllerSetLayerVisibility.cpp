//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RamshCommands/SystemCompositorControllerSetLayerVisibility.h"

namespace ramses::internal
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

    bool SystemCompositorControllerSetLayerVisibility::execute(int32_t& layerId, int32_t& visibility) const
    {
        m_rendererCommandBuffer.enqueueCommand(ramses::internal::RendererCommand::SCSetIviLayerVisibility{ WaylandIviLayerId(layerId), visibility != 0 });
        return true;
    }
}

