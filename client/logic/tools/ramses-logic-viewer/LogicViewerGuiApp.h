//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "LogicViewerApp.h"

class ISceneSetup;

namespace ramses
{
    class DisplayConfig;
    class RendererConfig;
}

namespace rlogic
{
    class ImguiClientHelper;
    class LogicViewerGui;

    class LogicViewerGuiApp : public LogicViewerApp
    {
    public:
        enum class ExitCode
        {
            Ok                  = 0,
            ErrorRamsesClient   = 1,
            ErrorRamsesRenderer = 2,
            ErrorSceneControl   = 3,
            ErrorLoadScene      = 4,
            ErrorLoadLogic      = 5,
            ErrorLoadLua        = 6,
            ErrorNoDisplay      = 7,
            ErrorUnknown        = -1,
        };

        LogicViewerGuiApp(int argc, char const* const* argv);

        ~LogicViewerGuiApp() override;

        LogicViewerGuiApp(const LogicViewerGuiApp&) = delete;

        LogicViewerGuiApp& operator=(const LogicViewerGuiApp&) = delete;

        [[nodiscard]] bool doOneLoop() override;

        [[nodiscard]] rlogic::ImguiClientHelper* getImguiClientHelper();

    private:
        [[nodiscard]] int init(int argc, char const* const* argv);
        [[nodiscard]] ramses::displayId_t initDisplay(const Arguments& args, ramses::RamsesRenderer& renderer, const ramses::DisplayConfig& displayConfig);


        std::unique_ptr<rlogic::ImguiClientHelper>   m_imguiHelper;
        std::unique_ptr<rlogic::LogicViewerGui>      m_gui;
        std::unique_ptr<ISceneSetup>                 m_sceneSetup;

        uint32_t m_width  = 0u;
        uint32_t m_height = 0u;
        std::array<float, 4> m_defaultClearColor{ 0, 0, 0, 1 };
        bool                 m_headless = false;
    };

    inline rlogic::ImguiClientHelper* LogicViewerGuiApp::getImguiClientHelper()
    {
        return m_imguiHelper.get();
    }
}

