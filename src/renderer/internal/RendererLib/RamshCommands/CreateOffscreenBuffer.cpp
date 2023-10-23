//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RamshCommands/CreateOffscreenBuffer.h"
#include "internal/RendererLib/RendererCommandBuffer.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    CreateOffscreenBuffer::CreateOffscreenBuffer(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "Usage: displayId bufferId width height - create an offscreen buffer";
        registerKeyword("obCreate");
    }

    bool CreateOffscreenBuffer::executeInput(const std::vector<std::string>& input)
    {
        if (input.size() != 5)
        {
            LOG_ERROR_P(CONTEXT_RAMSH, "None or too many arguments provided: {}", input.size() - 1);
            return false;
        }
        ramses::internal::RendererCommand::CreateOffscreenBuffer cmd{};
        cmd.depthStencilBufferType = EDepthBufferType::DepthStencil;
        cmd.interruptible          = false;
        cmd.sampleCount            = 0;
        if (!ArgumentConverter<uint32_t>::tryConvert(input[1], cmd.display.asMemoryHandleReference()))
        {
            LOG_ERROR_P(CONTEXT_RAMSH, "Invalid display id: {}", input[1]);
            return false;
        }
        if (!ArgumentConverter<uint32_t>::tryConvert(input[2], cmd.offscreenBuffer.asMemoryHandleReference()))
        {
            LOG_ERROR_P(CONTEXT_RAMSH, "Invalid buffer id: {}", input[2]);
            return false;
        }

        ArgumentConverter<uint32_t>::tryConvert(input[3], cmd.width);
        ArgumentConverter<uint32_t>::tryConvert(input[4], cmd.height);
        if (cmd.width == 0)
        {
            LOG_ERROR(CONTEXT_RAMSH, "Invalid width: must be larger than 0");
            return false;
        }
        if (cmd.height == 0)
        {
            LOG_ERROR(CONTEXT_RAMSH, "Invalid height: must be larger than 0");
            return false;
        }
        m_rendererCommandBuffer.enqueueCommand(std::move(cmd));
        return true;
    }
}
