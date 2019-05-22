//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "RendererCommands/ShowSceneCommand.h"
#include "Ramsh/RamshInput.h"
#include "RendererLib/WindowedRenderer.h"
#include "PlatformAbstraction/PlatformThread.h"

namespace ramses_internal
{
    ShowSceneCommand::ShowSceneCommand(WindowedRenderer& renderer)
        : m_renderer(renderer), m_timeout(5000)
    {
        description = "subscribe, map and show scene on display (unsafe) (showScene <sceneId> <displayId=0> <timeout=5000>)";
        registerKeyword("showScene");
    }

    Bool ShowSceneCommand::executeInput(const ramses_internal::RamshInput& input)
    {
        const uint32_t numArgStrings = static_cast<uint32_t>(input.size());
        UInt32 displayId(0);
        ESceneState sceneState(ESceneState::Unknown);

        LOG_INFO(CONTEXT_RAMSH, "Warning: Executing showScene command which is unsafe and only for debugging...");
        LOG_INFO(CONTEXT_RAMSH, "If showScene is not successful, you may run it again and necessarily with a higher timeout value until it successfully recovers from any intermediate states thru the target rendered scene state");

        if (numArgStrings < 2)
        {
            LOG_ERROR(CONTEXT_RAMSH, "Usage: showScene <sceneId> <displayId=0> <timeout=5000>");
            return false;
        }
        else if (numArgStrings < 3)
        {
            LOG_INFO(CONTEXT_RAMSH, "Using default displayId(0) for showScene");
        }
        else if (numArgStrings < 4)
        {
            displayId = static_cast<UInt32>(atoi(input[2u].c_str()));
        }
        else
        {
            m_timeout = static_cast<UInt32>(atoi(input[3u].c_str()));
        }

        const int sceneIdInt = atoi(input[1u].c_str());
        const SceneId sceneId(sceneIdInt);

        //checking if scene is already rendered or ready to execute
        sceneState = m_renderer.getSceneStateExecutor().getSceneState(sceneId);
        if (sceneState==ESceneState::Rendered)
        {
            LOG_ERROR(CONTEXT_RAMSH, "Scene is already rendered!");
            return false;
        }
        else if (!((sceneState==ESceneState::Published) || (sceneState==ESceneState::Subscribed) || (sceneState==ESceneState::Mapped)))
        {
            LOG_ERROR(CONTEXT_RAMSH, "Scene is not published, subscribed or mapped! Not possible to execute show scene command.");
            return false;
        }
        else
        {
            if(sceneState!=ESceneState::Published)
            {
                LOG_ERROR(CONTEXT_RAMSH, "Recovering scene " << sceneId << " from " << EnumToString(sceneState) << " state...");
            }
        }

        if (sceneState==ESceneState::Published)
        {
            m_renderer.getRendererCommandBuffer().subscribeScene(sceneId);
            sceneState=waitForSceneState(sceneId,ESceneState::Subscribed);
            if (sceneState!=ESceneState::Subscribed)
            {
                LOG_ERROR(CONTEXT_RAMSH, "Subscribing scene request" << sceneId.getValue() << " is timed out!");
                return false;
            }
        }

        if (sceneState==ESceneState::Subscribed)
        {
            m_renderer.getRendererCommandBuffer().mapSceneToDisplay(sceneId, DisplayHandle(displayId), 0);
            sceneState=waitForSceneState(sceneId,ESceneState::Mapped);
            if (sceneState!=ESceneState::Mapped)
            {
                LOG_ERROR(CONTEXT_RAMSH, "Mapping scene request" << sceneId.getValue() << " is timed out!");
                return false;
            }
        }

        if (sceneState==ESceneState::Mapped)
        {
            m_renderer.getRendererCommandBuffer().showScene(sceneId);
            sceneState=waitForSceneState(sceneId,ESceneState::Rendered);
            if (sceneState!=ESceneState::Rendered)
            {
                LOG_ERROR(CONTEXT_RAMSH, "Rendering scene request" << sceneId.getValue() << " is timed out!");
                return false;
            }
        }

        return true;
    }


    ESceneState ShowSceneCommand::waitForSceneState(SceneId sceneId, ESceneState targetSceneState)
    {
        ESceneState sceneState(ESceneState::Unknown);
        UInt32 execution_time(0);
        while (execution_time < m_timeout)
        {
            sceneState = m_renderer.getSceneStateExecutor().getSceneState(sceneId);
            if (sceneState == targetSceneState)
                return sceneState;
            PlatformThread::Sleep(100u);
            execution_time += 100;
        }

        return sceneState;
    }
}
