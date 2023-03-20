//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#pragma once

#include "CLI/CLI.hpp"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "ramses-renderer-api/RendererConfig.h"
#include "ramses-renderer-api/DisplayConfig.h"

namespace ramses
{
    /**
     * @brief Register command line options for the CLI11 command line parser
     *
     * Creates an option group "Framework Options" and registers command line options
     * After parsing the command line with CLI::App::parser() this config object is assigned with the values provided by command line
     *
     * @param cli CLI11 command line parser
     * @param config the configuration to parse for
     */
    inline void registerOptions(CLI::App& cli, ramses::RamsesFrameworkConfig& config)
    {
        auto* fw = cli.add_option_group("Framework Options");
        auto* logger = cli.add_option_group("Logger Options");

        std::map<std::string, EConnectionSystem> mapConn{{"tcp", EConnectionSystem::TCP}, {"off", EConnectionSystem::Off}};
        fw->add_option_function<EConnectionSystem>(
            "--connection", [&](const EConnectionSystem value) { config.setConnectionSystem(value); }, "Connection system")
            ->transform(CLI::CheckedTransformer(mapConn, CLI::ignore_case));

        fw->add_flag_function(
            "--ramsh,!--no-ramsh",
            [&](std::int64_t count) {
                if (count > 0)
                {
                    config.setRequestedRamsesShellType(ERamsesShellType_Console);
                }
                if (count < 0)
                {
                    config.setRequestedRamsesShellType(ERamsesShellType_None);
                }
            },
            "Enable Ramses Shell");

        fw->add_option_function<uint64_t>(
            "--guid", [&](uint64_t guid) { config.setParticipantGuid(guid); }, "User provided Guid");

        // TCP/IP options
        fw->add_option_function<std::string>(
            "--ip", [&](const std::string& myip) { config.setInterfaceSelectionIPForTCPCommunication(myip); }, "IP Address for TCP connection");
        fw->add_option_function<uint16_t>(
            "--port", [&](uint16_t p) { config.setInterfaceSelectionPortForTCPCommunication(p); }, "TCP port");
        fw->add_option_function<std::string>(
            "--daemon-ip", [&](const std::string& ip) { config.setDaemonIPForTCPCommunication(ip); }, "Ramses Daemon TCP port");
        fw->add_option_function<uint16_t>(
            "--daemon-port", [&](uint16_t p) { config.setDaemonPortForTCPCommunication(p); }, "Ramses Daemon TCP port");
        fw->add_option_function<std::pair<std::chrono::milliseconds,std::chrono::milliseconds>>(
            "--tcp-alive",
            [&](const auto& value) {
                config.setConnectionKeepaliveSettings(value.first, value.second);
            },
            "TCP keepalive settings in milliseconds. 1st value: interval, 2nd value: timeout");

        // Logger options
        logger->add_option_function<std::chrono::seconds>(
            "--logp", [&](const std::chrono::seconds& val) { config.setPeriodicLogInterval(val); },
            "Periodic log time interval in seconds (0 disables periodic logs)");

        const std::map<std::string, ELogLevel> logLevels = {
            {"off", ELogLevel::Off},
            {"fatal", ELogLevel::Fatal},
            {"error", ELogLevel::Error},
            {"warn", ELogLevel::Warn},
            {"info", ELogLevel::Info},
            {"debug", ELogLevel::Debug},
            {"trace", ELogLevel::Trace},
        };

        auto addLogContext = [logLevels, &config](const std::vector<std::string>& values) {
            for (const auto& value : values)
            {
                auto it = value.find(':');
                if (it == std::string::npos)
                    throw CLI::ValidationError("':' missing. Expected: CONTEXT:LOGLEVEL");
                const auto              contextStr  = value.substr(0, it);
                auto                    logLevelStr = value.substr(it + 1);
                CLI::CheckedTransformer t(logLevels, CLI::ignore_case);
                auto                    errorMsg = t(logLevelStr);
                if (!errorMsg.empty())
                    throw CLI::ValidationError(errorMsg);
                ELogLevel logLevel;
                if (!CLI::detail::lexical_assign<ELogLevel, ELogLevel>(logLevelStr, logLevel))
                    throw CLI::ValidationError("Could not convert '" + logLevelStr + "' to loglevel");
                config.setLogLevel(contextStr, logLevel);
            }
        };
        logger->add_option_function<ELogLevel>(
            "-l,--log-level", [&](ELogLevel value) { config.setLogLevel(value); }, "Log level for all contexts")
            ->transform(CLI::CheckedTransformer(logLevels, CLI::ignore_case));
        logger->add_option_function<ELogLevel>(
            "--log-level-console", [&](ELogLevel value) { config.setLogLevelConsole(value); }, "Log level for all contexts (console only)")
            ->transform(CLI::CheckedTransformer(logLevels, CLI::ignore_case));
        logger->add_option_function<std::vector<std::string>>("--log-context", addLogContext, "Apply log level for specific log context.")->type_name("CONTEXT:LOGLEVEL");
        logger->add_option_function<std::string>(
            "--dlt-app-id", [&](const std::string& value) { config.setDLTApplicationID(value); }, "DLT Application ID")
            ->default_val(config.getDLTApplicationID());
        logger->add_option_function<std::string>(
            "--dlt-app-description", [&](const std::string& value) { config.setDLTApplicationDescription(value); }, "DLT Application description")
            ->default_str(std::string(config.getDLTApplicationDescription()));

        // convert executable name to participant name
        cli.callback([&]() {
            std::string_view name  = cli.get_name();
            size_t slash = name.rfind('/');
            if (slash == std::string::npos)
            {
                slash = name.rfind('\\');
            }
            if (slash != std::string::npos)
            {
                config.setParticipantName(name.substr(slash +1));
            }
            else
            {
                config.setParticipantName(name);
            }
        });
    }

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
            "--ivi-control", [&](auto) { config.enableSystemCompositorControl(); }, "enable system compositor IVI controller");
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
            "-f,--fullscreen", [&](auto) { config.setWindowFullscreen(true); }, "enable fullscreen mode");
        grp->add_flag_function(
            "--borderless", [&](auto) { config.setWindowBorderless(true); }, "disable window borders");
        grp->add_flag_function(
            "--resizable", [&](auto) { config.setResizable(true); }, "enables resizable renderer window");
        grp->add_option_function<uint32_t>(
            "--msaa", [&](auto value) { config.setMultiSampling(value); }, "set msaa (antialiasing) sample count")
            ->check(CLI::IsMember({1, 2, 4, 8}));
        grp->add_flag_function(
            "--delete-effects", [&](auto) { config.keepEffectsUploaded(false); }, "do not keep effects uploaded");
        grp->add_flag_function(
            "--ivi-visible,!--no-ivi-visible", [&](auto) { config.setWindowIviVisible(); }, "set IVI surface visible when created");
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
                config.setClearColor(r, g, b, a);
                return !is.fail() && (is.rdbuf()->in_avail() == 0);
            }, "set clear color (rgba)");
    }
}
