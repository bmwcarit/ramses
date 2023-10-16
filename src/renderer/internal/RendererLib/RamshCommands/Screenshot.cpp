//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RamshCommands/Screenshot.h"
#include "internal/RendererLib/RendererCommandBuffer.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    Screenshot::Screenshot(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "saves a screenshot. Options:[-filename FILENAME -displayId DISPLAYID -sendViaDLT -ob BUFFERID]";
        registerKeyword("p");
        registerKeyword("screenshot");
    }

    bool Screenshot::executeInput(const std::vector<std::string>& input)
    {
        enum EOption
        {
            EOption_None = 0,
            EOption_Filename,
            EOption_Display,
            EOption_OffscreenBuffer,
        };

        EOption lastOption = EOption_None;

        std::string         filename = "unnamed.png";
        auto                display = DisplayHandle(0);
        OffscreenBufferHandle offscreenBuffer;
        bool                sendViaDLT = false;

        for (const auto& arg : input)
        {
            if (arg == "-filename")
            {
                lastOption = EOption_Filename;
            }
            else if (arg == "-displayId")
            {
                lastOption = EOption_Display;
            }
            else if (arg == "-sendViaDLT")
            {
                sendViaDLT = true;
            }
            else if (arg == "-ob")
            {
                lastOption = EOption_OffscreenBuffer;
            }
            else
            {
                switch( lastOption )
                {
                case EOption_Display:
                    display    = DisplayHandle(strtoul(arg.c_str(), nullptr, 0));
                    lastOption = EOption_None;
                    break;
                case EOption_Filename:
                    filename   = arg;
                    lastOption = EOption_None;
                    break;
                case EOption_OffscreenBuffer:
                    offscreenBuffer = OffscreenBufferHandle(strtoul(arg.c_str(), nullptr, 0));
                    lastOption = EOption_None;
                    break;
                case EOption_None:
                    if( contains_c(m_keywords, arg) ) // check whether a keyword is the current argument
                    {
                        continue;
                    }
                    LOG_ERROR_P(CONTEXT_RAMSH, "Unknown option: {}", arg);
                    return false;
                }
            }
        }

        m_rendererCommandBuffer.enqueueCommand(RendererCommand::ReadPixels{ display, offscreenBuffer, 0u, 0u, 0u, 0u, true, sendViaDLT, std::move(filename) });

        return true;
    }
}
