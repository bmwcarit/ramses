//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesFrameworkConfigImpl.h"
#include "Utils/CommandLineParser.h"
#include "Utils/LoggingUtils.h"
#include "Utils/Argument.h"
#include "Watchdog/PlatformWatchdog.h"
#include "TransportCommon/EConnectionProtocol.h"
#include "TransportCommon/RamsesTransportProtocolVersion.h"
#include "TransportCommon/SomeIPAdapter.h"

namespace ramses
{
    using namespace ramses_internal;

#if defined(HAS_TCP_COMM)
    static bool gHasTCPComm = true;
#else
    static bool gHasTCPComm = false;
#endif

    RamsesFrameworkConfigImpl::RamsesFrameworkConfigImpl(int32_t argc, char const* const* argv)
        : StatusObjectImpl()
        , m_enableSomeIPHUSafeLocalMode(false)
        , m_shellType(ERamsesShellType_Default)
        , m_periodicLogsEnabled(true)
        , m_usedProtocol(EConnectionProtocol::Invalid)
        , m_someipCommunicationUserID(SomeIPAdapter::GetInvalidCommunicationUser())
        , m_parser(argc, argv)
        , m_dltAppID("RAMS")
        , m_dltAppDescription("RAMS-DESC")
        , m_maximumTotalBytesForAsyncResourceLoading(MAXIMUM_BYTES_FOR_ASYNC_RESOURCE_LOADING)
        , m_enableProtocolVersionOffset(false)
    {
        parseCommandLine();
    }

    RamsesFrameworkConfigImpl::~RamsesFrameworkConfigImpl()
    {
    }

    void RamsesFrameworkConfigImpl::enableProtocolVersionOffset()
    {
        m_enableProtocolVersionOffset = true;
    }

    uint32_t RamsesFrameworkConfigImpl::getProtocolVersion() const
    {
        if (m_enableProtocolVersionOffset)
        {
            const uint32_t protocolVersionOffset = 99u;
            return RAMSES_TRANSPORT_PROTOCOL_VERSION_MAJOR + protocolVersionOffset;
        }
        else
        {
            return RAMSES_TRANSPORT_PROTOCOL_VERSION_MAJOR;
        }
    }

    const ramses_internal::CommandLineParser& RamsesFrameworkConfigImpl::getCommandLineParser() const
    {
        return m_parser;
    }

    EConnectionProtocol RamsesFrameworkConfigImpl::getUsedProtocol() const
    {
        return m_usedProtocol;
    }

    uint32_t RamsesFrameworkConfigImpl::getSomeipCommunicationUserID() const
    {
        return m_someipCommunicationUserID;
    }

    uint32_t RamsesFrameworkConfigImpl::getWatchdogNotificationInterval(ERamsesThreadIdentifier thread) const
    {
        return m_watchdogConfig.getWatchdogNotificationInterval(thread);
    }

    IThreadWatchdogNotification* RamsesFrameworkConfigImpl::getWatchdogNotificationCallback() const
    {
        return m_watchdogConfig.getCallBack();
    }

    status_t RamsesFrameworkConfigImpl::enableSomeIPCommunication(uint32_t ramsesCommunicationUserID)
    {
        m_someipCommunicationUserID = ramsesCommunicationUserID;
        m_usedProtocol = SomeIPAdapter::GetUsedSomeIPStack(ramsesCommunicationUserID);

        if (m_usedProtocol != EConnectionProtocol::Invalid)
        {
            if (SomeIPAdapter::IsSomeIPStackCompiled(m_usedProtocol))
            {
                return StatusOK;
            }
            else
            {
                StringOutputStream error;
                error << "Specified to use " << m_usedProtocol << " but was compiled without";
                LOG_FATAL(CONTEXT_COMMUNICATION, error.c_str());
                return addErrorEntry(error.c_str());
            }
        }
        else
        {
            StringOutputStream error;
            error << "No SomeIP stack is configured for communication user ID " << ramsesCommunicationUserID;
            LOG_FATAL(CONTEXT_COMMUNICATION, error.c_str());
            return addErrorEntry(error.c_str());
        }
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

    void RamsesFrameworkConfigImpl::parseCommandLine()
    {
        const ArgumentUInt32 someipCommunicationUserID(m_parser, "someip", "enableSomeIPWithID", 0);
        const ArgumentBool useFakeConnection( m_parser, "fakeConnection", "fakeConnection");
        const ArgumentBool isRamshEnabled(m_parser, "ramsh", "ramsh");
        const ArgumentBool enableOffsetPlatformProtocolVersion(m_parser, "pvo", "protocolVersionOffset");
        const ArgumentBool disablePeriodicLogs(m_parser, "disablePeriodicLogs", "disablePeriodicLogs");
        const ArgumentString userProvidedGuid(m_parser, "guid", "guid", "");

        if (enableOffsetPlatformProtocolVersion)
        {
            enableProtocolVersionOffset();
        }
        if (isRamshEnabled)
        {
            m_shellType = ERamsesShellType_Console;
        }

        if (disablePeriodicLogs)
        {
            m_periodicLogsEnabled = false;
        }

        if (someipCommunicationUserID)
        {
            const ArgumentBool someipHuLocalMode(m_parser, "shl", "someip-hu-local");
            m_enableSomeIPHUSafeLocalMode = someipHuLocalMode;

            enableSomeIPCommunication(someipCommunicationUserID);

            if (SomeIPAdapter::GetUsedSomeIPStack(someipCommunicationUserID) == EConnectionProtocol::SomeIP_IC)
            {
                m_someipICConfig.setIPAddress(ArgumentString(m_parser, "myip", "myipaddress", m_someipICConfig.getIPAddress()));
            }

            someipKeepAliveInterval = std::chrono::milliseconds(ArgumentUInt32(m_parser, "someipAlive", "someipAlive", static_cast<uint32_t>(someipKeepAliveInterval.count())));
            someipKeepAliveTimeout = std::chrono::milliseconds(ArgumentUInt32(m_parser, "someipAliveTimeout", "someipAliveTimeout", static_cast<uint32_t>(someipKeepAliveTimeout.count())));
        }
        else if( useFakeConnection || !gHasTCPComm )
        {
            m_usedProtocol = EConnectionProtocol::Fake;
        }
        else
        {
            m_usedProtocol = EConnectionProtocol::TCP;
            ArgumentUInt16 port(m_parser, "myport", "myportnumber", m_tcpConfig.getPort());
            if( port.wasDefined() )
            {
                m_tcpConfig.setPort(port);
            }

            m_tcpConfig.setIPAddress(ArgumentString(m_parser, "myip", "myipaddress", m_tcpConfig.getIPAddress()));
            m_tcpConfig.setDaemonIPAddress(ArgumentString(m_parser, "i", "daemon-ip", m_tcpConfig.getDaemonIPAddress()));
            m_tcpConfig.setDaemonPort(ArgumentUInt16(m_parser, "p", "daemon-port", m_tcpConfig.getDaemonPort()));

            m_tcpConfig.setAliveInterval(std::chrono::milliseconds(ArgumentUInt32(m_parser, "tcpAlive", "tcpAlive", static_cast<uint32_t>(m_tcpConfig.getAliveInterval().count()))));
            m_tcpConfig.setAliveTimeout(std::chrono::milliseconds(ArgumentUInt32(m_parser, "tcpAliveTimeout", "tcpAliveTimeout", static_cast<uint32_t>(m_tcpConfig.getAliveTimeout().count()))));
        }

        if (userProvidedGuid.hasValue())
        {
            m_userProvidedGuid = Guid(userProvidedGuid);
        }
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
        m_dltAppID = id;
    }

    const char* RamsesFrameworkConfigImpl::getDLTApplicationID() const
    {
        return m_dltAppID.c_str();
    }

    void RamsesFrameworkConfigImpl::setDLTApplicationDescription(const char* description)
    {
        m_dltAppDescription = description;
    }

    const char* RamsesFrameworkConfigImpl::getDLTApplicationDescription() const
    {
        return m_dltAppDescription.c_str();
    }


    uint32_t RamsesFrameworkConfigImpl::getMaximumTotalBytesForAsyncResourceLoading() const
    {
        return m_maximumTotalBytesForAsyncResourceLoading;
    }

    void RamsesFrameworkConfigImpl::setMaximumTotalBytesAllowedForAsyncResourceLoading(uint32_t maximumTotalBytesForAsyncResourceLoading)
    {
        m_maximumTotalBytesForAsyncResourceLoading = maximumTotalBytesForAsyncResourceLoading;
    }

    void RamsesFrameworkConfigImpl::setPeriodicLogsEnabled(bool enabled)
    {
        m_periodicLogsEnabled = enabled;
    }

    ramses_internal::Guid RamsesFrameworkConfigImpl::getUserProvidedGuid() const
    {
        return m_userProvidedGuid;
    }
}
