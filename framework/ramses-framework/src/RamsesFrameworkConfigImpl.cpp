//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesFrameworkConfigImpl.h"
#include "Utils/LoggingUtils.h"
#include "Watchdog/PlatformWatchdog.h"
#include "TransportCommon/EConnectionProtocol.h"
#include "TransportCommon/RamsesTransportProtocolVersion.h"
#include <map>

namespace ramses
{
    using namespace ramses_internal;

#if defined(HAS_TCP_COMM)
    static bool gHasTCPComm = true;
#else
    static bool gHasTCPComm = false;
#endif

    RamsesFrameworkConfigImpl::RamsesFrameworkConfigImpl()
        : StatusObjectImpl()
        , m_shellType(ERamsesShellType_Default)
        , m_periodicLogsEnabled(true)
        , m_usedProtocol(gHasTCPComm ? EConnectionProtocol::TCP : EConnectionProtocol::Off)
    {
    }

    RamsesFrameworkConfigImpl::~RamsesFrameworkConfigImpl() = default;

    status_t RamsesFrameworkConfigImpl::setFeatureLevel(EFeatureLevel featureLevel)
    {
        if (std::find(ramses::AllFeatureLevels.cbegin(), ramses::AllFeatureLevels.cend(), featureLevel) == ramses::AllFeatureLevels.cend())
            return addErrorEntry(fmt::format("RamsesFrameworkConfig::setFeatureLevel: Failed to set unsupported feature level '{}'.", featureLevel));

        m_featureLevel = featureLevel;
        return StatusOK;
    }

    EFeatureLevel RamsesFrameworkConfigImpl::getFeatureLevel() const
    {
        return m_featureLevel;
    }

    uint32_t RamsesFrameworkConfigImpl::getProtocolVersion() const
    {
        return RAMSES_TRANSPORT_PROTOCOL_VERSION_MAJOR;
    }

    const ramses_internal::String& RamsesFrameworkConfigImpl::getProgramName() const
    {
        return m_programName;
    }

    EConnectionProtocol RamsesFrameworkConfigImpl::getUsedProtocol() const
    {
        return m_usedProtocol;
    }

    uint32_t RamsesFrameworkConfigImpl::getWatchdogNotificationInterval(ERamsesThreadIdentifier thread) const
    {
        return m_watchdogConfig.getWatchdogNotificationInterval(thread);
    }

    IThreadWatchdogNotification* RamsesFrameworkConfigImpl::getWatchdogNotificationCallback() const
    {
        return m_watchdogConfig.getCallBack();
    }

    status_t RamsesFrameworkConfigImpl::setRequestedRamsesShellType(ERamsesShellType shellType)
    {
        m_shellType = shellType;

        return StatusOK;
    }

    status_t RamsesFrameworkConfigImpl::setWatchdogNotificationInterval(ERamsesThreadIdentifier thread, uint32_t interval)
    {
        if (0u == interval)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Could not set watchdog notification interval, interval is not valid");
            return addErrorEntry("Could not set watchdog notification interval, interval is not valid");
        }
        if (ERamsesThreadIdentifier_Unknown == thread)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Could not set watchdog notification interval, thread identifier is not valid");
            return addErrorEntry("Could not set watchdog notification interval, thread identifier is not valid");
        }
        m_watchdogConfig.setWatchdogNotificationInterval(thread, interval);
        return StatusOK;
    }

    status_t RamsesFrameworkConfigImpl::setWatchdogNotificationCallBack(IThreadWatchdogNotification* callback)
    {
        m_watchdogConfig.setThreadWatchDogCallback(callback);
        return StatusOK;
    }

    void RamsesFrameworkConfigImpl::registerOptions(CLI::App& cli)
    {
        auto* fw = cli.add_option_group("Framework Options");
        auto* logger = cli.add_option_group("Logger Options");

        std::map<std::string, EConnectionProtocol> mapConn{{"tcp", EConnectionProtocol::TCP}, {"off", EConnectionProtocol::Off}};
        fw->add_option("--connection", m_usedProtocol, "Connection system")->transform(CLI::CheckedTransformer(mapConn, CLI::ignore_case))->default_val(m_usedProtocol);

        fw->add_flag_function(
            "--ramsh,!--no-ramsh",
            [&](std::int64_t count) {
                if (count > 0)
                {
                    m_shellType = ERamsesShellType_Console;
                }
                if (count < 0)
                {
                    m_shellType = ERamsesShellType_None;
                }
            },
            "Enable Ramses Shell");
        fw->add_option_function<std::string>(
            "--guid", [&](const std::string& guid) { m_userProvidedGuid = Guid(guid.c_str()); }, "User provided Guid");

        fw->add_option_function<std::string>(
            "--ip", [&](const std::string& myip) { m_tcpConfig.setIPAddress(String(myip)); }, "IP Address for TCP connection");

        // TCP/IP options
        fw->add_option_function<uint16_t>(
            "--port", [&](uint16_t p) { m_tcpConfig.setPort(p); }, "TCP port");
        fw->add_option_function<std::string>(
            "--daemon-ip", [&](const std::string& ip) { m_tcpConfig.setDaemonIPAddress(String(ip)); }, "Ramses Daemon TCP port");
        fw->add_option_function<uint16_t>(
            "--daemon-port", [&](uint16_t p) { m_tcpConfig.setDaemonPort(p); }, "Ramses Daemon TCP port");
        fw->add_option_function<std::chrono::milliseconds>(
            "--tcp-alive", [&](const std::chrono::milliseconds& val) { m_tcpConfig.setAliveInterval(val); }, "TCP Keepalive interval in milliseconds");
        fw->add_option_function<std::chrono::milliseconds>(
            "--tcp-alive-timeout", [&](const std::chrono::milliseconds& val) { m_tcpConfig.setAliveTimeout(val); }, "TCP Keepalive timeout in milliseconds");

        // Logger options
        logger->add_option_function<std::chrono::seconds>(
            "--logp", [&](const std::chrono::seconds& val) { setPeriodicLogInterval(val); }, "Periodic log time interval in seconds (0 disables periodic logs)");

        const std::map<std::string, ELogLevel> logLevels = {
            {"off", ELogLevel::Off},
            {"fatal", ELogLevel::Fatal},
            {"error", ELogLevel::Error},
            {"warn", ELogLevel::Warn},
            {"info", ELogLevel::Info},
            {"debug", ELogLevel::Debug},
            {"trace", ELogLevel::Trace},
        };

        auto addLogContext = [logLevels, this](const std::vector<std::string>& values) {
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
                    throw CLI::ValidationError(fmt::format("Could not convert '{}' to loglevel", logLevelStr));
                loggerConfig.logLevelContexts[contextStr.c_str()] = GetELogLevelInternal(logLevel);
            }
        };
        logger->add_option("-l,--log-level", loggerConfig.logLevel, "Log level for all contexts (both console and dlt)")
            ->transform(CLI::CheckedTransformer(logLevels, CLI::ignore_case));
        logger->add_option("--log-level-console", loggerConfig.logLevelConsole, "Log level for all contexts (console only)")
            ->transform(CLI::CheckedTransformer(logLevels, CLI::ignore_case));
        logger->add_option_function<std::vector<std::string>>("--log-context", addLogContext, "Apply log level for specific log context.")->type_name("CONTEXT:LOGLEVEL");
        logger->add_option("--dlt-app-id", loggerConfig.dltAppId, "DLT Application ID")->default_str(loggerConfig.dltAppId.stdRef());
        logger->add_option("--dlt-app-description", loggerConfig.dltAppDescription, "DLT Application description")->default_str(loggerConfig.dltAppDescription.stdRef());
        logger->add_flag("--log-test", loggerConfig.enableSmokeTestContext, "Enables additional log context for smoke tests (RSMT)");
        cli.callback([&](){m_programName = cli.get_name();});
    }

    status_t RamsesFrameworkConfigImpl::enableDLTApplicationRegistration(bool state)
    {
        m_enableDltApplicationRegistration = state;
        return StatusOK;
    }

    bool RamsesFrameworkConfigImpl::getDltApplicationRegistrationEnabled() const
    {
        return m_enableDltApplicationRegistration;
    }

    void RamsesFrameworkConfigImpl::setDLTApplicationID(const char* id)
    {
        loggerConfig.dltAppId = id;
    }

    const char* RamsesFrameworkConfigImpl::getDLTApplicationID() const
    {
        return loggerConfig.dltAppId.c_str();
    }

    void RamsesFrameworkConfigImpl::setDLTApplicationDescription(const char* description)
    {
        loggerConfig.dltAppDescription = description;
    }

    const char* RamsesFrameworkConfigImpl::getDLTApplicationDescription() const
    {
        return loggerConfig.dltAppDescription.c_str();
    }

    void RamsesFrameworkConfigImpl::setLogLevel(ELogLevel logLevel)
    {
        loggerConfig.logLevel = GetELogLevelInternal(logLevel);
    }

    status_t RamsesFrameworkConfigImpl::setLogLevel(std::string_view context, ELogLevel logLevel)
    {
        loggerConfig.logLevelContexts[String(context)] = GetELogLevelInternal(logLevel);
        return StatusOK;
    }

    void RamsesFrameworkConfigImpl::setLogLevelConsole(ELogLevel logLevel)
    {
        loggerConfig.logLevelConsole = GetELogLevelInternal(logLevel);
    }

    void RamsesFrameworkConfigImpl::setPeriodicLogInterval(std::chrono::seconds interval)
    {
        periodicLogTimeout    = static_cast<uint32_t>(interval.count());
        m_periodicLogsEnabled = (periodicLogTimeout > 0);
    }

    status_t RamsesFrameworkConfigImpl::setParticipantGuid(uint64_t guid)
    {
        m_userProvidedGuid = Guid(guid);
        if (!m_userProvidedGuid.isValid() || guid < 256)
        {
            return addErrorEntry(fmt::format("RamsesFrameworkConfig::setParticipantGuid: Failed to set invalid id '{}'.", m_userProvidedGuid));
        }
        return StatusOK;
    }

    status_t RamsesFrameworkConfigImpl::setConnectionSystem(EConnectionSystem connectionSystem)
    {
        switch (connectionSystem)
        {
        case EConnectionSystem::TCP:
            m_usedProtocol = EConnectionProtocol::TCP;
            break;
        case EConnectionSystem::Off:
            m_usedProtocol = EConnectionProtocol::Off;
            break;
        }
        return StatusOK;
    }

    ramses_internal::Guid RamsesFrameworkConfigImpl::getUserProvidedGuid() const
    {
        return m_userProvidedGuid;
    }

    void RamsesFrameworkConfigImpl::setFeatureLevelNoCheck(EFeatureLevel featureLevel)
    {
        static_assert(EFeatureLevel_Latest == EFeatureLevel_01,
            "remove this method which is used only for testing while there is no valid mismatching feature level yet");
        m_featureLevel = featureLevel;
    }
}
