//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RamshCommands/SetFrameTimeLimits.h"
#include "internal/RendererLib/RendererCommandBuffer.h"

namespace ramses::internal
{
    SetFrameTimeLimits::SetFrameTimeLimits(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "change values (us): limitForDynamicResourcesUpload limitForStaticResourcesUpload limitForOffscreenBufferRender";
        registerKeyword("limits");
    }

    bool SetFrameTimeLimits::execute(uint32_t& limitForDynamicResourcesUploadMicrosec, uint32_t& limitForStaticResourcesUploadMicrosec, uint32_t& limitForOffscreenBufferRenderMicrosec) const
    {
        m_rendererCommandBuffer.enqueueCommand(RendererCommand::SetLimits_FrameBudgets{ limitForDynamicResourcesUploadMicrosec, limitForStaticResourcesUploadMicrosec, limitForOffscreenBufferRenderMicrosec });
        return true;
    }
}
