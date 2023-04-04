//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicViewerHeadlessApp.h"
#include "ImguiWrapper.h"
#include "Arguments.h"
#include "LogicViewer.h"
#include "LogicViewerSettings.h"
#include "ramses-logic/Logger.h"

namespace rlogic
{
    LogicViewerHeadlessApp::LogicViewerHeadlessApp(int argc, const char * const * argv)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize.x = 400.f;
        io.DisplaySize.y = 320.f;
        int texturewidth = 0;
        int textureheight = 0;
        unsigned char* pixels = nullptr;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &texturewidth, &textureheight);
        m_exitCode = init(argc, argv);
    }

    LogicViewerHeadlessApp::~LogicViewerHeadlessApp() = default;

    int LogicViewerHeadlessApp::init(int argc, char const* const* argv)
    {
        if (argc == 0 || argv == nullptr)
        {
            return static_cast<int>(ExitCode::ErrorUnknown);
        }

        CLI::App                      cli;
        Arguments                     args;

        try
        {
            CLI::deprecate_option(cli.add_flag("--headless", "headless by default - flag will be ignored"), "");
            args.registerOptions(cli);
        }
        catch (const CLI::Error& err)
        {
            // catch configuration errors
            std::cerr << err.what() << std::endl;
            return -1;
        }

        CLI11_PARSE(cli, argc, argv);

        auto exitCode = LogicViewerApp::loadScene(args);
        if (exitCode != 0)
        {
            return exitCode;
        }

        exitCode = LogicViewerApp::createViewer(args, LogicViewer::ScreenshotFunc());
        if (exitCode != 0)
        {
            return exitCode;
        }

        if (m_interactive)
        {
            std::cerr << "no interactive mode in headless viewer" << std::endl;
            return static_cast<int>(ExitCode::ErrorUnknown);
        }
        return 0;
    }

    bool LogicViewerHeadlessApp::doOneLoop()
    {
        return false; // never interactive
    }
}

