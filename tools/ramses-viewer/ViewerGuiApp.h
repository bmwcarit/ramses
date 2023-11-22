//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ViewerApp.h"
#include "ramses/renderer/RendererConfig.h"
#include "ramses/renderer/DisplayConfig.h"
#include "ImguiClientHelper.h"
#include "SceneSetup.h"
#include "ViewerGui.h"
#include "SceneViewerGui.h"
#include "LogicViewerGui.h"

namespace ramses::internal
{
    class ViewerGuiApp final : public ViewerApp
    {
    public:
        ViewerGuiApp();

        void registerOptions(CLI::App& cli);

        [[nodiscard]] ExitCode run();

        // for integration tests
        [[nodiscard]] ImguiClientHelper* getImguiClientHelper();
        [[nodiscard]] ExitCode setup();
        [[nodiscard]] bool doOneLoop();

        enum class GuiMode
        {
            Off,
            On,       ///< Loaded scene rendered to offscreen buffer (configurable size and position)
            Overlay,  ///< Debugging gui overlaps the rendered scene (no offscreen buffer)
            Only,     ///< Only shows the gui, not the scene itself
        };
        [[nodiscard]] GuiMode getGuiMode() const;

    private:
        GuiMode m_guiMode = GuiMode::Overlay;

        ramses::RendererConfig m_rendererConfig;
        ramses::DisplayConfig  m_displayConfig;

        std::string m_screenshotFile;

        CLI::Option* m_width = nullptr;
        CLI::Option* m_height = nullptr;

        bool m_noSkub = false;
        bool m_headless = false;

        std::unique_ptr<ImguiClientHelper> m_imguiHelper;
        std::unique_ptr<ISceneSetup>       m_sceneSetup;
        std::unique_ptr<ViewerGui>         m_gui;
    };

    inline ImguiClientHelper* ViewerGuiApp::getImguiClientHelper()
    {
        return m_imguiHelper.get();
    }

    inline ViewerGuiApp::GuiMode ViewerGuiApp::getGuiMode() const
    {
        return m_guiMode;
    }
}
