//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERCOMMANDEXECUTOR_H
#define RAMSES_RENDERERCOMMANDEXECUTOR_H

#include "SceneAPI/SceneId.h"
#include "RendererLib/RendererCommandTypes.h"
#include "RendererLib/RendererCommandContainer.h"
#include "RendererLogger.h"

namespace ramses_internal
{
    class Renderer;
    class RendererCommandBuffer;
    class RendererEventCollector;
    class RendererSceneUpdater;
    class FrameTimer;

    class RendererCommandExecutor
    {
    public:
        RendererCommandExecutor(Renderer& renderer, RendererCommandBuffer& rendererCommandBuffer, RendererSceneUpdater& rendererSceneUpdater, RendererEventCollector& rendererEventCollector, FrameTimer& frameTimer);

        void executePendingCommands();

    private:

        Renderer&                       m_renderer;
        RendererSceneUpdater&           m_rendererSceneUpdater;
        RendererCommandBuffer&          m_rendererCommandBuffer;
        RendererEventCollector&         m_rendererEventCollector;
        FrameTimer&                     m_frameTimer;

        SceneIdVector                   m_activeScenes;
        RendererCommandContainer        m_executedCommands;
    };
}

#endif
