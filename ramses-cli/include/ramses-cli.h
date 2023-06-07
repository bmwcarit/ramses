//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#pragma once

#include "ramses-framework-cli.h"
#include "ramses-renderer-api/RendererConfig.h"
#include "ramses-renderer-api/DisplayConfig.h"

namespace ramses
{
    /**
    * @brief Register command line options for the CLI11 command line parser
    *
    * Creates an option group "Renderer Options" and registers command line options
    * After parsing the command line with CLI::App::parse() this config object is assigned with the values provided by command line
    *
    * @param[in] cli CLI11 command line parser
    */
    inline void registerOptions(CLI::App& cli, ramses::RendererConfig& config)
    {
        auto* grp = cli.add_option_group("Renderer Options");
        grp->add_flag_function(
            "--ivi-control", [&](auto /*unused*/) { config.enableSystemCompositorControl(); }, "enable system compositor IVI controller");
    }

    /**
    * @brief Register command line options for the CLI11 command line parser
    *
    * Creates an option group "Display Options" and registers command line options
    * After parsing the command line with CLI::App::parse() this config object is assigned with the values provided by command line
    *
    * @param[in] cli CLI11 command line parser
    */
    inline void registerOptions(CLI::App& cli, ramses::DisplayConfig& config)
    {
        auto* grp = cli.add_option_group("Display Options");

        const std::map<std::string, EWindowType> windowTypes = {
            {"windows"          , EWindowType::Windows},
            {"x11"              , EWindowType::X11},
            {"android"          , EWindowType::Android},
            {"wayland-ivi"      , EWindowType::Wayland_IVI},
            {"wayland-wl-shell" , EWindowType::Wayland_Shell},
        };
        grp->add_option_function<EWindowType>(
            "--window-type", [&](auto value) {
                config.setWindowType(value);
            },
            "set window type used to create display")->transform(CLI::CheckedTransformer(windowTypes, CLI::ignore_case, CLI::ignore_underscore));

        const std::map<std::string, EDeviceType> deviceTypes = {
            {"gles30"   , EDeviceType::GLES_3_0},
            {"gl42"     , EDeviceType::GL_4_2},
            {"gl45"     , EDeviceType::GL_4_5},
        };

        grp->add_option_function<EDeviceType>(
            "--device-type", [&](auto value) {
                config.setDeviceType(value);
            },
            "set device type used to create display")->transform(CLI::CheckedTransformer(deviceTypes, CLI::ignore_case, CLI::ignore_underscore))->default_val("gles30");

        grp->add_option_function<int32_t>(
            "--xpos", [&](auto value) {
                int32_t x = 0;
                int32_t y = 0;
                uint32_t w = 0;
                uint32_t h = 0;
                config.getWindowRectangle(x, y, w, h);
                x = value;
                config.setWindowRectangle(x, y, w, h);
            },
            "set x position of window");
        grp->add_option_function<int32_t>(
            "--ypos", [&](auto value) {
                int32_t x = 0;
                int32_t y = 0;
                uint32_t w = 0;
                uint32_t h = 0;
                config.getWindowRectangle(x, y, w, h);
                y = value;
                config.setWindowRectangle(x, y, w, h);
            }, "set y position of window");
        grp->add_option_function<uint32_t>(
            "--width", [&](auto value) {
                int32_t  x = 0;
                int32_t  y = 0;
                uint32_t w = 0;
                uint32_t h = 0;
                config.getWindowRectangle(x, y, w, h);
                w = value;
                config.setWindowRectangle(x, y, w, h);
            }, "set window width");
        grp->add_option_function<uint32_t>(
            "--height", [&](auto value) {
                int32_t  x = 0;
                int32_t  y = 0;
                uint32_t w = 0;
                uint32_t h = 0;
                config.getWindowRectangle(x, y, w, h);
                h = value;
                config.setWindowRectangle(x, y, w, h);
            }, "set window height");
        grp->add_flag_function(
            "-f,--fullscreen", [&](auto /*unused*/) { config.setWindowFullscreen(true); }, "enable fullscreen mode");
        grp->add_flag_function(
            "--borderless", [&](auto /*unused*/) { config.setWindowBorderless(true); }, "disable window borders");
        grp->add_flag_function(
            "--resizable", [&](auto /*unused*/) { config.setResizable(true); }, "enables resizable renderer window");
        grp->add_option_function<uint32_t>(
            "--msaa", [&](auto value) { config.setMultiSampling(value); }, "set msaa (antialiasing) sample count")
            ->check(CLI::IsMember({1, 2, 4, 8}));
        grp->add_flag_function(
            "--delete-effects", [&](auto /*unused*/) { config.keepEffectsUploaded(false); }, "do not keep effects uploaded");
        grp->add_flag_function(
            "--ivi-visible,!--no-ivi-visible", [&](auto /*unused*/) { config.setWindowIviVisible(); }, "set IVI surface visible when created");
        grp->add_option_function<uint32_t>(
            "--ivi-layer", [&](auto value) { config.setWaylandIviLayerID(waylandIviLayerId_t(value)); }, "set id of IVI layer the display surface will be added to")
            ->type_name("LAYER");
        grp->add_option_function<uint32_t>(
            "--ivi-surface", [&](auto value) { config.setWaylandIviSurfaceID(waylandIviSurfaceId_t(value)); }, "set id of IVI surface the display will be composited on")
            ->type_name("SURFACE");
        auto* ec = grp->add_option_function<std::string>(
            "--ec-display", [&](const auto& value) { config.setWaylandEmbeddedCompositingSocketName(value); }, "set wayland display (socket name) that is created by the embedded compositor")
            ->type_name("NAME");
        grp->add_option_function<std::string>(
            "--ec-socket-group", [&](const auto& value) { config.setWaylandEmbeddedCompositingSocketGroup(value); }, "group name for wayland display socket")
            ->needs(ec)
            ->type_name("NAME");
        grp->add_option_function<uint32_t>(
            "--ec-socket-permissions", [&](auto value) { config.setWaylandEmbeddedCompositingSocketPermissions(value); }, "file permissions for wayland display socket")
            ->needs(ec)
            ->type_name("MODE");
        grp->add_option_function<std::string>(
            "--clear", [&](const auto& value) {
                std::istringstream is;
                is.str(value);
                float r = 0.f;
                float g = 0.f;
                float a = 0.f;
                float b = 0.f;
                char separator = 0;
                is >> r;
                is >> separator;
                is >> g;
                is >> separator;
                is >> b;
                is >> separator;
                is >> a;
                config.setClearColor({r, g, b, a});
                return !is.fail() && (is.rdbuf()->in_avail() == 0);
            }, "set clear color (rgba)");
    }
}
