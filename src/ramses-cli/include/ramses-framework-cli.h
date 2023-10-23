//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#pragma once

#include "CLI/CLI.hpp"
#include "ramses/framework/RamsesFrameworkConfig.h"

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
                    config.setRequestedRamsesShellType(ERamsesShellType::Console);
                }
                if (count < 0)
                {
                    config.setRequestedRamsesShellType(ERamsesShellType::None);
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
}
