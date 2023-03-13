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
        : StatusObject(*new RamsesFrameworkConfigImpl())
        , impl(static_cast<RamsesFrameworkConfigImpl&>(StatusObject::impl))
    {
    }

    RamsesFrameworkConfig::~RamsesFrameworkConfig()
    {
    }

    void RamsesFrameworkConfig::registerOptions(CLI::App& cli)
    {
        return impl.registerOptions(cli);
    }

    status_t RamsesFrameworkConfig::setFeatureLevel(EFeatureLevel featureLevel)
    {
        return impl.setFeatureLevel(featureLevel);
    }

    EFeatureLevel RamsesFrameworkConfig::getFeatureLevel() const
    {
        return impl.getFeatureLevel();
    }

    status_t RamsesFrameworkConfig::setRequestedRamsesShellType(ERamsesShellType requestedShellType)
    {
        const status_t status = impl.setRequestedRamsesShellType(requestedShellType);
        return status;
    }

    status_t RamsesFrameworkConfig::setWatchdogNotificationInterval(ERamsesThreadIdentifier thread, uint32_t interval)
    {
        return impl.setWatchdogNotificationInterval(thread, interval);
    }

    status_t RamsesFrameworkConfig::setWatchdogNotificationCallBack(IThreadWatchdogNotification* callback)
    {
        return impl.setWatchdogNotificationCallBack(callback);
    }

    status_t RamsesFrameworkConfig::disableDLTApplicationRegistration()
    {
        return impl.enableDLTApplicationRegistration(false);
    }

    void RamsesFrameworkConfig::setDLTApplicationID(const char* id)
    {
        impl.setDLTApplicationID(id);
    }

    const char* RamsesFrameworkConfig::getDLTApplicationID() const
    {
        return impl.getDLTApplicationID();
    }

    void RamsesFrameworkConfig::setDLTApplicationDescription(const char* description)
    {
        impl.setDLTApplicationDescription(description);
    }

    const char* RamsesFrameworkConfig::getDLTApplicationDescription() const
    {
        return impl.getDLTApplicationDescription();
    }

    void RamsesFrameworkConfig::setLogLevel(ELogLevel logLevel)
    {
        impl.setLogLevel(logLevel);
    }

    status_t RamsesFrameworkConfig::setLogLevel(std::string_view context, ELogLevel logLevel)
    {
        return impl.setLogLevel(context, logLevel);
    }

    void RamsesFrameworkConfig::setLogLevelConsole(ELogLevel logLevel)
    {
        impl.setLogLevelConsole(logLevel);
    }

    void RamsesFrameworkConfig::setPeriodicLogInterval(std::chrono::seconds interval)
    {
        impl.setPeriodicLogInterval(interval);
    }

    status_t RamsesFrameworkConfig::setParticipantGuid(uint64_t guid)
    {
        return impl.setParticipantGuid(guid);
    }

    status_t RamsesFrameworkConfig::setConnectionSystem(EConnectionSystem connectionSystem)
    {
        return impl.setConnectionSystem(connectionSystem);
    }

    void RamsesFrameworkConfig::setInterfaceSelectionIPForTCPCommunication(const char* ip)
    {
        impl.m_tcpConfig.setIPAddress(ip);
    }

    void RamsesFrameworkConfig::setInterfaceSelectionPortForTCPCommunication(uint16_t port)
    {
        impl.m_tcpConfig.setPort(port);
    }

    void RamsesFrameworkConfig::setDaemonIPForTCPCommunication(const char* ip)
    {
        impl.m_tcpConfig.setDaemonIPAddress(ip);
    }

    void RamsesFrameworkConfig::setDaemonPortForTCPCommunication(uint16_t port)
    {
        impl.m_tcpConfig.setDaemonPort(port);
    }

    status_t RamsesFrameworkConfig::setConnectionKeepaliveSettings(std::chrono::milliseconds interval, std::chrono::milliseconds timeout)
    {
        impl.m_tcpConfig.setAliveInterval(interval);
        impl.m_tcpConfig.setAliveTimeout(timeout);
        return StatusOK;
    }
}
