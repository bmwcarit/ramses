//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererCommands/AssignScene.h"
#include "RendererLib/RendererCommandBuffer.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    AssignScene::AssignScene(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "Usage: [-ob bufferID] [-display displayID] sceneId - assign scene to display / offscreen buffer";
        registerKeyword("assign");
    }

    bool AssignScene::executeInput(const std::vector<std::string>& input)
    {
        std::vector<std::string> positionals;
        ramses_internal::RendererCommand::SetSceneMapping cmdMap{};
        ramses_internal::RendererCommand::SetSceneDisplayBufferAssignment cmdOb{};

        enum class EOption
        {
            None,
            Display,
            OffscreenBuffer,
        };

        EOption lastOption = EOption::None;

        for (const auto& arg : input)
        {
            if (arg == "-displayId")
            {
                lastOption = EOption::Display;
            }
            else if (arg == "-ob")
            {
                lastOption = EOption::OffscreenBuffer;
            }
            else
            {
                switch( lastOption )
                {
                case EOption::Display:
                    ArgumentConverter<uint32_t>::tryConvert(arg, cmdMap.display.asMemoryHandleReference());
                    lastOption = EOption::None;
                    break;
                case EOption::OffscreenBuffer:
                    ArgumentConverter<uint32_t>::tryConvert(arg, cmdOb.buffer.asMemoryHandleReference());
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

        if (positionals.size() != 1)
        {
            LOG_ERROR_P(CONTEXT_RAMSH, "None or too many arguments provided: {}", positionals.size());
            return false;
        }

        SceneId sceneId;
        ArgumentConverter<uint64_t>::tryConvert(positionals[0], sceneId.getReference());
        if (!sceneId.isValid())
        {
            LOG_ERROR_P(CONTEXT_RAMSH, "Invalid SceneId: {}", positionals[0]);
            return false;
        }
        cmdMap.scene = sceneId;
        cmdOb.scene  = sceneId;
        if (!cmdMap.display.isValid() && !cmdOb.buffer.isValid())
        {
            LOG_ERROR(CONTEXT_RAMSH, "Expected '-ob ID' or '-displayId ID' option");
            return false;
        }
        if (cmdMap.display.isValid())
        {
            m_rendererCommandBuffer.enqueueCommand(std::move(cmdMap));
        }
        if (cmdOb.buffer.isValid())
        {
            m_rendererCommandBuffer.enqueueCommand(std::move(cmdOb));
        }

        return true;
    }
}
