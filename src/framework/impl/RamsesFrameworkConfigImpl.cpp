//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/RamsesFrameworkConfigImpl.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Core/Utils/LoggingUtils.h"
#include "internal/Watchdog/PlatformWatchdog.h"
#include "internal/Communication/TransportCommon/EConnectionProtocol.h"
#include "internal/Communication/TransportCommon/RamsesTransportProtocolVersion.h"
#include "impl/EFeatureLevelImpl.h"
#include <map>

namespace ramses::internal
{
#if defined(HAS_TCP_COMM)
    static bool gHasTCPComm = true;
#else
    static bool gHasTCPComm = false;
#endif

    RamsesFrameworkConfigImpl::RamsesFrameworkConfigImpl(EFeatureLevel featureLevel)
        : m_shellType(ERamsesShellType::Default)
        , m_periodicLogsEnabled(true)
        , m_usedProtocol(gHasTCPComm ? EConnectionProtocol::TCP : EConnectionProtocol::Off)
    {
        m_featureLevel = featureLevel;
        if (!IsFeatureLevel(featureLevel))
        {
            LOG_ERROR(CONTEXT_CLIENT, "Unrecognized feature level '0{}' provided, falling back to feature level 01", featureLevel);
            m_featureLevel = EFeatureLevel_01;
        }
    }

    RamsesFrameworkConfigImpl::~RamsesFrameworkConfigImpl() = default;

    bool RamsesFrameworkConfigImpl::setFeatureLevel(EFeatureLevel featureLevel)
    {
        if (!IsFeatureLevel(featureLevel))
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesFrameworkConfig::setFeatureLevel: Failed to set unsupported feature level '{}'.", featureLevel);
            return false;
        }

        m_featureLevel = featureLevel;
        return true;
    }

    EFeatureLevel RamsesFrameworkConfigImpl::getFeatureLevel() const
    {
        return m_featureLevel;
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static): design decision
    uint32_t RamsesFrameworkConfigImpl::getProtocolVersion() const
    {
        return RAMSES_TRANSPORT_PROTOCOL_VERSION_MAJOR;
    }

    const std::string& RamsesFrameworkConfigImpl::getParticipantName() const
    {
        return m_participantName;
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

    bool RamsesFrameworkConfigImpl::setRequestedRamsesShellType(ERamsesShellType shellType)
    {
        m_shellType = shellType;

        return true;
    }

    bool RamsesFrameworkConfigImpl::setWatchdogNotificationInterval(ERamsesThreadIdentifier thread, uint32_t interval)
    {
        if (0u == interval)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Could not set watchdog notification interval, interval is not valid");
            return false;
        }
        if (ERamsesThreadIdentifier::Unknown == thread)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Could not set watchdog notification interval, thread identifier is not valid");
            return false;
        }
        m_watchdogConfig.setWatchdogNotificationInterval(thread, interval);
        return true;
    }

    bool RamsesFrameworkConfigImpl::setWatchdogNotificationCallBack(IThreadWatchdogNotification* callback)
    {
        m_watchdogConfig.setThreadWatchDogCallback(callback);
        return true;
    }

    bool RamsesFrameworkConfigImpl::enableDLTApplicationRegistration(bool state)
    {
        m_enableDltApplicationRegistration = state;
        return true;
    }

    bool RamsesFrameworkConfigImpl::getDltApplicationRegistrationEnabled() const
    {
        return m_enableDltApplicationRegistration;
    }

    void RamsesFrameworkConfigImpl::setDLTApplicationID(std::string_view id)
    {
        loggerConfig.dltAppId = id;
    }

    std::string_view RamsesFrameworkConfigImpl::getDLTApplicationID() const
    {
        return loggerConfig.dltAppId;
    }

    void RamsesFrameworkConfigImpl::setDLTApplicationDescription(std::string_view description)
    {
        loggerConfig.dltAppDescription = description;
    }

    std::string_view RamsesFrameworkConfigImpl::getDLTApplicationDescription() const
    {
        return loggerConfig.dltAppDescription;
    }

    void RamsesFrameworkConfigImpl::setLogLevel(ELogLevel logLevel)
    {
        loggerConfig.logLevel = logLevel;
    }

    bool RamsesFrameworkConfigImpl::setLogLevel(std::string_view context, ELogLevel logLevel)
    {
        loggerConfig.logLevelContexts[std::string{context}] = logLevel;
        return true;
    }

    void RamsesFrameworkConfigImpl::setLogLevelConsole(ELogLevel logLevel)
    {
        loggerConfig.logLevelConsole = logLevel;
    }

    void RamsesFrameworkConfigImpl::setPeriodicLogInterval(std::chrono::seconds interval)
    {
        periodicLogTimeout    = static_cast<uint32_t>(interval.count());
        m_periodicLogsEnabled = (periodicLogTimeout > 0);
    }

    void RamsesFrameworkConfigImpl::setLoggingInstanceName(std::string_view instanceName)
    {
        m_loggingInstanceName = instanceName;
    }

    const std::string& RamsesFrameworkConfigImpl::getLoggingInstanceName() const
    {
        return m_loggingInstanceName;
    }

    bool RamsesFrameworkConfigImpl::setParticipantGuid(uint64_t guid)
    {
        m_userProvidedGuid = Guid(guid);
        if (!m_userProvidedGuid.isValid() || guid < 256)
        {
            LOG_ERROR(CONTEXT_CLIENT, "RamsesFrameworkConfig::setParticipantGuid: Failed to set invalid id '{}'.", m_userProvidedGuid);
            return false;
        }
        return true;
    }

    bool RamsesFrameworkConfigImpl::setParticipantName(std::string_view name)
    {
        m_participantName = name;
        return true;
    }

    bool RamsesFrameworkConfigImpl::setConnectionSystem(EConnectionSystem connectionSystem)
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
        return true;
    }

    Guid RamsesFrameworkConfigImpl::getUserProvidedGuid() const
    {
        return m_userProvidedGuid;
    }

    void RamsesFrameworkConfigImpl::setFeatureLevelNoCheck(EFeatureLevel featureLevel)
    {
        m_featureLevel = featureLevel;
    }
}
