//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "internal/RendererLib/RamshCommands/SetClearColor.h"
#include "internal/RendererLib/RendererCommandBuffer.h"

namespace ramses::internal
{
    SetClearColor::SetClearColor(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "Usage: [-ob bufferID] displayID red green blue alpha - set clear color for display or offscreen buffer";
        registerKeyword("clc");
    }

    bool SetClearColor::executeInput(const std::vector<std::string>& input)
    {
        std::vector<std::string> positionals;
        ramses::internal::RendererCommand::SetClearColor cmd{};

        enum class EOption
        {
            None,
            OffscreenBuffer,
        };

        EOption lastOption = EOption::None;

        for (const auto& arg : input)
        {
            if (arg == "-ob")
            {
                lastOption = EOption::OffscreenBuffer;
            }
            else
            {
                switch( lastOption )
                {
                case EOption::OffscreenBuffer:
                    ArgumentConverter<uint32_t>::tryConvert(arg, cmd.offscreenBuffer.asMemoryHandleReference());
                    lastOption = EOption::None;
                    break;
                case EOption::None:
                    if (!contains_c(m_keywords, arg))
                    {
                        positionals.push_back(arg);
                    }
                    break;
                }
            }
        }

        if (positionals.size() != 5)
        {
            LOG_ERROR(CONTEXT_RAMSH, "None or too many arguments provided: {}", positionals.size());
            return false;
        }

        ArgumentConverter<uint32_t>::tryConvert(positionals[0], cmd.display.asMemoryHandleReference());
        ArgumentConverter<float>::tryConvert(positionals[1], cmd.clearColor.r);
        ArgumentConverter<float>::tryConvert(positionals[2], cmd.clearColor.g);
        ArgumentConverter<float>::tryConvert(positionals[3], cmd.clearColor.b);
        ArgumentConverter<float>::tryConvert(positionals[4], cmd.clearColor.a);
        m_rendererCommandBuffer.enqueueCommand(std::move(cmd));
        return true;
    }
}
