//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererCommands/SystemCompositorControllerScreenshot.h"
#include "Ramsh/RamshInput.h"

namespace ramses_internal
{
    SystemCompositorControllerScreenshot::SystemCompositorControllerScreenshot(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "makes a screenshot from the system compositor";
        registerKeyword("scScreenshot");
        registerKeyword("scs");

        getArgument<0>().setDescription("file name including path, where to store the screenshot");
        getArgument<1>().setDescription("wayland IVI screen id");
    }

    Bool SystemCompositorControllerScreenshot::execute(String& fileName, int32_t& screenIviId) const
    {
        m_rendererCommandBuffer.systemCompositorControllerScreenshot(fileName, screenIviId);
        return true;
    }
}

