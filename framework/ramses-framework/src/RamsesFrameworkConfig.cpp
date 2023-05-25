//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "RamsesFrameworkConfigImpl.h"

namespace ramses
{
    RamsesFrameworkConfig::RamsesFrameworkConfig()
        : StatusObject{ std::make_unique<RamsesFrameworkConfigImpl>() }
        , m_impl{ static_cast<RamsesFrameworkConfigImpl&>(*StatusObject::m_impl) }
    {
    }

    RamsesFrameworkConfig::~RamsesFrameworkConfig() = default;

    RamsesFrameworkConfig::RamsesFrameworkConfig(const RamsesFrameworkConfig& other)
        : StatusObject{ std::make_unique<RamsesFrameworkConfigImpl>(other.m_impl) }
        , m_impl{ static_cast<RamsesFrameworkConfigImpl&>(*StatusObject::m_impl) }
    {
    }

    RamsesFrameworkConfig::RamsesFrameworkConfig(RamsesFrameworkConfig&& other) noexcept
        : StatusObject{ std::move(other.StatusObject::m_impl) }
        , m_impl{ static_cast<RamsesFrameworkConfigImpl&>(*StatusObject::m_impl) }
    {
    }

    RamsesFrameworkConfig& RamsesFrameworkConfig::operator=(const RamsesFrameworkConfig& other)
    {
        StatusObject::m_impl = std::make_unique<RamsesFrameworkConfigImpl>(other.m_impl);
        m_impl = static_cast<RamsesFrameworkConfigImpl&>(*StatusObject::m_impl);
        return *this;
    }

    RamsesFrameworkConfig& RamsesFrameworkConfig::operator=(RamsesFrameworkConfig&& other) noexcept
    {
        StatusObject::m_impl = std::move(other.StatusObject::m_impl);
        m_impl = static_cast<RamsesFrameworkConfigImpl&>(*StatusObject::m_impl);
        return *this;
    }

    status_t RamsesFrameworkConfig::setFeatureLevel(EFeatureLevel featureLevel)
    {
        return m_impl.get().setFeatureLevel(featureLevel);
    }

    EFeatureLevel RamsesFrameworkConfig::getFeatureLevel() const
    {
        return m_impl.get().getFeatureLevel();
    }

    status_t RamsesFrameworkConfig::setRequestedRamsesShellType(ERamsesShellType requestedShellType)
    {
        const status_t status = m_impl.get().setRequestedRamsesShellType(requestedShellType);
        return status;
    }

    status_t RamsesFrameworkConfig::setWatchdogNotificationInterval(ERamsesThreadIdentifier thread, uint32_t interval)
    {
        return m_impl.get().setWatchdogNotificationInterval(thread, interval);
    }

    status_t RamsesFrameworkConfig::setWatchdogNotificationCallBack(IThreadWatchdogNotification* callback)
    {
        return m_impl.get().setWatchdogNotificationCallBack(callback);
    }

    status_t RamsesFrameworkConfig::disableDLTApplicationRegistration()
    {
        return m_impl.get().enableDLTApplicationRegistration(false);
    }

    void RamsesFrameworkConfig::setDLTApplicationID(std::string_view id)
    {
        m_impl.get().setDLTApplicationID(id);
    }

    std::string_view RamsesFrameworkConfig::getDLTApplicationID() const
    {
        return m_impl.get().getDLTApplicationID();
    }

    void RamsesFrameworkConfig::setDLTApplicationDescription(std::string_view description)
    {
        m_impl.get().setDLTApplicationDescription(description);
    }

    std::string_view RamsesFrameworkConfig::getDLTApplicationDescription() const
    {
        return m_impl.get().getDLTApplicationDescription();
    }

    void RamsesFrameworkConfig::setLogLevel(ELogLevel logLevel)
    {
        m_impl.get().setLogLevel(logLevel);
    }

    status_t RamsesFrameworkConfig::setLogLevel(std::string_view context, ELogLevel logLevel)
    {
        return m_impl.get().setLogLevel(context, logLevel);
    }

    void RamsesFrameworkConfig::setLogLevelConsole(ELogLevel logLevel)
    {
        m_impl.get().setLogLevelConsole(logLevel);
    }

    void RamsesFrameworkConfig::setPeriodicLogInterval(std::chrono::seconds interval)
    {
        m_impl.get().setPeriodicLogInterval(interval);
    }

    status_t RamsesFrameworkConfig::setParticipantGuid(uint64_t guid)
    {
        return m_impl.get().setParticipantGuid(guid);
    }

    status_t RamsesFrameworkConfig::setParticipantName(std::string_view name)
    {
        return m_impl.get().setParticipantName(name);
    }

    status_t RamsesFrameworkConfig::setConnectionSystem(EConnectionSystem connectionSystem)
    {
        return m_impl.get().setConnectionSystem(connectionSystem);
    }

    void RamsesFrameworkConfig::setInterfaceSelectionIPForTCPCommunication(std::string_view ip)
    {
        m_impl.get().m_tcpConfig.setIPAddress(ip);
    }

    void RamsesFrameworkConfig::setInterfaceSelectionPortForTCPCommunication(uint16_t port)
    {
        m_impl.get().m_tcpConfig.setPort(port);
    }

    void RamsesFrameworkConfig::setDaemonIPForTCPCommunication(std::string_view ip)
    {
        m_impl.get().m_tcpConfig.setDaemonIPAddress(ip);
    }

    void RamsesFrameworkConfig::setDaemonPortForTCPCommunication(uint16_t port)
    {
        m_impl.get().m_tcpConfig.setDaemonPort(port);
    }

    status_t RamsesFrameworkConfig::setConnectionKeepaliveSettings(std::chrono::milliseconds interval, std::chrono::milliseconds timeout)
    {
        m_impl.get().m_tcpConfig.setAliveInterval(interval);
        m_impl.get().m_tcpConfig.setAliveTimeout(timeout);
        return StatusOK;
    }
}
