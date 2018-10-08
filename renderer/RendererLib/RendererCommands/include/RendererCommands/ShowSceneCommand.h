//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SHOWSCENECOMMAND_H
#define RAMSES_SHOWSCENECOMMAND_H

#include "Ramsh/RamshCommand.h"
#include "RendererEventCollector.h"
#include "RendererLib/ESceneState.h"

namespace ramses_internal
{
    class WindowedRenderer;

    class ShowSceneCommand : public RamshCommand
    {
    public:
        explicit ShowSceneCommand(WindowedRenderer& renderer);
        virtual Bool executeInput(const RamshInput& input) override;

    private:
        WindowedRenderer& m_renderer;
        ESceneState waitForSceneState(SceneId sceneId, ESceneState targetSceneState);
        UInt32 m_timeout;
    };
}

#endif
