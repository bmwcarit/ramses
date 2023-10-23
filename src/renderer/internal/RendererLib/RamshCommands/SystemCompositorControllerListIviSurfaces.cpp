//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RamshCommands/SystemCompositorControllerListIviSurfaces.h"

namespace ramses::internal
{
    SystemCompositorControllerListIviSurfaces::SystemCompositorControllerListIviSurfaces(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "print out a list of ivi-ids that are registered at the system compositor";
        registerKeyword("scListSurfaces");
        registerKeyword("scl");
    }

    bool SystemCompositorControllerListIviSurfaces::executeInput([[maybe_unused]] const std::vector<std::string>& input)
    {
        m_rendererCommandBuffer.enqueueCommand(ramses::internal::RendererCommand::SCListIviSurfaces{});
        return true;
    }
}
