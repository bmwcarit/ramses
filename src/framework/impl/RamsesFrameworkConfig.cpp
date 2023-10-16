//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/framework/RamsesFrameworkConfig.h"
#include "impl/RamsesFrameworkConfigImpl.h"

namespace ramses
{
    RamsesFrameworkConfig::RamsesFrameworkConfig(EFeatureLevel featureLevel)
        : m_impl{ std::make_unique<internal::RamsesFrameworkConfigImpl>(featureLevel) }
    {
    }

    RamsesFrameworkConfig::~RamsesFrameworkConfig() = default;

    RamsesFrameworkConfig::RamsesFrameworkConfig(const RamsesFrameworkConfig& other)
        : m_impl{ std::make_unique<internal::RamsesFrameworkConfigImpl>(*other.m_impl) }
    {
    }

    RamsesFrameworkConfig::RamsesFrameworkConfig(RamsesFrameworkConfig&& other) noexcept = default;

    RamsesFrameworkConfig& RamsesFrameworkConfig::operator=(const RamsesFrameworkConfig& other)
    {
        if (this != &other)
        {
            m_impl = std::make_unique<internal::RamsesFrameworkConfigImpl>(*other.m_impl);
        }
        return *this;
    }

    RamsesFrameworkConfig& RamsesFrameworkConfig::operator=(RamsesFrameworkConfig&& other) noexcept = default;

    bool RamsesFrameworkConfig::setFeatureLevel(EFeatureLevel featureLevel)
    {
        return m_impl->setFeatureLevel(featureLevel);
    }

    EFeatureLevel RamsesFrameworkConfig::getFeatureLevel() const
    {
        return m_impl->getFeatureLevel();
    }

    bool RamsesFrameworkConfig::setRequestedRamsesShellType(ERamsesShellType requestedShellType)
    {
        return m_impl->setRequestedRamsesShellType(requestedShellType);
    }

    bool RamsesFrameworkConfig::setWatchdogNotificationInterval(ERamsesThreadIdentifier thread, uint32_t interval)
    {
        return m_impl->setWatchdogNotificationInterval(thread, interval);
    }

    bool RamsesFrameworkConfig::setWatchdogNotificationCallBack(IThreadWatchdogNotification* callback)
    {
        return m_impl->setWatchdogNotificationCallBack(callback);
    }

    bool RamsesFrameworkConfig::disableDLTApplicationRegistration()
    {
        return m_impl->enableDLTApplicationRegistration(false);
    }

    void RamsesFrameworkConfig::setDLTApplicationID(std::string_view id)
    {
        m_impl->setDLTApplicationID(id);
    }

    std::string_view RamsesFrameworkConfig::getDLTApplicationID() const
    {
        return m_impl->getDLTApplicationID();
    }

    void RamsesFrameworkConfig::setDLTApplicationDescription(std::string_view description)
    {
        m_impl->setDLTApplicationDescription(description);
    }

    std::string_view RamsesFrameworkConfig::getDLTApplicationDescription() const
    {
        return m_impl->getDLTApplicationDescription();
    }

    void RamsesFrameworkConfig::setLogLevel(ELogLevel logLevel)
    {
        m_impl->setLogLevel(logLevel);
    }

    bool RamsesFrameworkConfig::setLogLevel(std::string_view context, ELogLevel logLevel)
    {
        return m_impl->setLogLevel(context, logLevel);
    }

    void RamsesFrameworkConfig::setLogLevelConsole(ELogLevel logLevel)
    {
        m_impl->setLogLevelConsole(logLevel);
    }

    void RamsesFrameworkConfig::setPeriodicLogInterval(std::chrono::seconds interval)
    {
        m_impl->setPeriodicLogInterval(interval);
    }

    bool RamsesFrameworkConfig::setParticipantGuid(uint64_t guid)
    {
        return m_impl->setParticipantGuid(guid);
    }

    bool RamsesFrameworkConfig::setParticipantName(std::string_view name)
    {
        return m_impl->setParticipantName(name);
    }

    bool RamsesFrameworkConfig::setConnectionSystem(EConnectionSystem connectionSystem)
    {
        return m_impl->setConnectionSystem(connectionSystem);
    }

    void RamsesFrameworkConfig::setInterfaceSelectionIPForTCPCommunication(std::string_view ip)
    {
        m_impl->m_tcpConfig.setIPAddress(ip);
    }

    void RamsesFrameworkConfig::setInterfaceSelectionPortForTCPCommunication(uint16_t port)
    {
        m_impl->m_tcpConfig.setPort(port);
    }

    void RamsesFrameworkConfig::setDaemonIPForTCPCommunication(std::string_view ip)
    {
        m_impl->m_tcpConfig.setDaemonIPAddress(ip);
    }

    void RamsesFrameworkConfig::setDaemonPortForTCPCommunication(uint16_t port)
    {
        m_impl->m_tcpConfig.setDaemonPort(port);
    }

    bool RamsesFrameworkConfig::setConnectionKeepaliveSettings(std::chrono::milliseconds interval, std::chrono::milliseconds timeout)
    {
        m_impl->m_tcpConfig.setAliveInterval(interval);
        m_impl->m_tcpConfig.setAliveTimeout(timeout);
        return true;
    }

    internal::RamsesFrameworkConfigImpl& RamsesFrameworkConfig::impl()
    {
        return *m_impl;
    }

    const internal::RamsesFrameworkConfigImpl& RamsesFrameworkConfig::impl() const
    {
        return *m_impl;
    }
}
