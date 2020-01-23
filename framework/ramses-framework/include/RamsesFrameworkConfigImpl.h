//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESFRAMEWORKCONFIGIMPL_H
#define RAMSES_RAMSESFRAMEWORKCONFIGIMPL_H

#include "StatusObjectImpl.h"
#include "TCPConfig.h"
#include "Utils/CommandLineParser.h"
#include "ramses-framework-api/IThreadWatchdogNotification.h"
#include "ThreadWatchdogConfig.h"
#include "TransportCommon/EConnectionProtocol.h"
#include "Collections/Guid.h"

namespace ramses
{
    const uint32_t MAXIMUM_BYTES_FOR_ASYNC_RESOURCE_LOADING = 20 * 1024 * 1024;

    class RamsesFrameworkConfigImpl : public StatusObjectImpl
    {
    public:
        RamsesFrameworkConfigImpl(int32_t argc, char const* const* argv);
        ~RamsesFrameworkConfigImpl();
        const ramses_internal::CommandLineParser& getCommandLineParser() const;

        void setDLTApplicationID(const char* id);
        const char* getDLTApplicationID() const;

        void setDLTApplicationDescription(const char* description);
        const char* getDLTApplicationDescription() const;

        uint32_t getProtocolVersion() const;
        void enableProtocolVersionOffset();

        status_t setWatchdogNotificationInterval(ramses::ERamsesThreadIdentifier thread, uint32_t interval);
        status_t setWatchdogNotificationCallBack(IThreadWatchdogNotification* callback);

        status_t setRequestedRamsesShellType(ERamsesShellType shellType);

        void setMaximumTotalBytesAllowedForAsyncResourceLoading(uint32_t maximumTotalBytesForAsynResourceLoading);
        uint32_t getMaximumTotalBytesForAsyncResourceLoading() const;
        ramses_internal::EConnectionProtocol getUsedProtocol() const;
        uint32_t getWatchdogNotificationInterval(ERamsesThreadIdentifier thread) const;
        IThreadWatchdogNotification* getWatchdogNotificationCallback() const;

        void setPeriodicLogsEnabled(bool enabled);
        ramses_internal::Guid getUserProvidedGuid() const;

        TCPConfig        m_tcpConfig;
        ERamsesShellType m_shellType;
        ramses_internal::ThreadWatchdogConfig m_watchdogConfig;
        bool m_periodicLogsEnabled;

    private:
        RamsesFrameworkConfigImpl();

        void parseCommandLine();

        ramses_internal::EConnectionProtocol m_usedProtocol;
        ramses_internal::CommandLineParser m_parser;
        ramses_internal::String m_dltAppID;
        ramses_internal::String m_dltAppDescription;
        uint32_t m_maximumTotalBytesForAsyncResourceLoading;
        bool m_enableProtocolVersionOffset;
        ramses_internal::Guid m_userProvidedGuid;
    };
}

#endif
