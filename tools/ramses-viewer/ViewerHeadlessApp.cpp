//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#include "ViewerHeadlessApp.h"
#include "ImguiWrapper.h"

namespace ramses::internal
{
    ViewerHeadlessApp::ViewerHeadlessApp()
    {
        // context needed for saving default lua file (uses imgui logging)
        ImGuiIO& io                  = ImGui::GetIO();
        io.DisplaySize.x             = 400.f;
        io.DisplaySize.y             = 320.f;
        int            texturewidth  = 0;
        int            textureheight = 0;
        unsigned char* pixels        = nullptr;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &texturewidth, &textureheight);
    }

    void ViewerHeadlessApp::registerOptions(CLI::App& cli)
    {
        ViewerApp::registerOptions(cli);
    }

    ViewerHeadlessApp::ExitCode ViewerHeadlessApp::run()
    {
        auto exitCode = loadScene();
        if (exitCode != ExitCode::Ok)
            return exitCode;

        exitCode = createLogicViewer(LogicViewer::ScreenshotFunc());
        if (exitCode != ExitCode::Ok)
            return exitCode;

        return ExitCode::Ok;
    }
}
