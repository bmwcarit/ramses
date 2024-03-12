//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererCommands/CreateOffscreenBuffer.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
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
        ramses_internal::RendererCommand::CreateOffscreenBuffer cmd{};
        cmd.depthStencilBufferType = ERenderBufferType_DepthStencilBuffer;
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
        if (cmd.width == 0 || cmd.width > 4096)
        {
            LOG_ERROR_P(CONTEXT_RAMSH, "Invalid width: {}", input[3]);
            return false;
        }
        if (cmd.height == 0 || cmd.height > 4096)
        {
            LOG_ERROR_P(CONTEXT_RAMSH, "Invalid width: {}", input[4]);
            return false;
        }
        m_rendererCommandBuffer.enqueueCommand(std::move(cmd));
        return true;
    }
}
