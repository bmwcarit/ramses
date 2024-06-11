//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"


namespace ramses::internal
{
    class ViewerGuiApp;
    class RendererControl;

    class RemoteScenesGui
    {
    public:
        explicit RemoteScenesGui(ViewerGuiApp& app);
        void drawContents(bool open);

    private:
        RendererControl* m_rendererControl;
        sceneId_t m_imguiScene;
        sceneId_t m_loadedScene;
    };
}

