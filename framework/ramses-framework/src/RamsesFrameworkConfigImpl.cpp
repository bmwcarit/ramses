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
        , m_shellType(ERamsesShellType::Default)
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
        if (ERamsesThreadIdentifier::Unknown == thread)
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

    status_t RamsesFrameworkConfigImpl::enableDLTApplicationRegistration(bool state)
    {
        m_enableDltApplicationRegistration = state;
        return StatusOK;
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
        loggerConfig.logLevel = GetELogLevelInternal(logLevel);
    }

    status_t RamsesFrameworkConfigImpl::setLogLevel(std::string_view context, ELogLevel logLevel)
    {
        loggerConfig.logLevelContexts[std::string{context}] = GetELogLevelInternal(logLevel);
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

    status_t RamsesFrameworkConfigImpl::setParticipantName(std::string_view name)
    {
        m_participantName = name;
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
