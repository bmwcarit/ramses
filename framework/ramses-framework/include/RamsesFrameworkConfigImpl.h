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
#include "Utils/RamsesLogger.h"
#include "ramses-framework-api/IThreadWatchdogNotification.h"
#include "ramses-framework-api/EFeatureLevel.h"
#include "ThreadWatchdogConfig.h"
#include "TransportCommon/EConnectionProtocol.h"
#include "Collections/Guid.h"
#include "CLI/CLI.hpp"

namespace ramses
{
    class RamsesFrameworkConfigImpl : public StatusObjectImpl
    {
    public:
        RamsesFrameworkConfigImpl(int32_t argc, char const* const* argv);
        ~RamsesFrameworkConfigImpl();

        void registerOptions(CLI::App& cli);

        const ramses_internal::String& getProgramName() const;

        status_t setFeatureLevel(EFeatureLevel featureLevel);
        EFeatureLevel getFeatureLevel() const;

        status_t enableDLTApplicationRegistration(bool state);
        bool getDltApplicationRegistrationEnabled() const;

        void setDLTApplicationID(const char* id);
        const char* getDLTApplicationID() const;

        void setDLTApplicationDescription(const char* description);
        const char* getDLTApplicationDescription() const;

        uint32_t getProtocolVersion() const;
        void enableProtocolVersionOffset();

        status_t setWatchdogNotificationInterval(ramses::ERamsesThreadIdentifier thread, uint32_t interval);
        status_t setWatchdogNotificationCallBack(IThreadWatchdogNotification* callback);

        status_t setRequestedRamsesShellType(ERamsesShellType shellType);

        ramses_internal::EConnectionProtocol getUsedProtocol() const;
        uint32_t getWatchdogNotificationInterval(ERamsesThreadIdentifier thread) const;
        IThreadWatchdogNotification* getWatchdogNotificationCallback() const;

        void setPeriodicLogsEnabled(bool enabled);
        ramses_internal::Guid getUserProvidedGuid() const;

        TCPConfig        m_tcpConfig;
        ERamsesShellType m_shellType;
        ramses_internal::ThreadWatchdogConfig m_watchdogConfig;
        bool m_periodicLogsEnabled;

        ramses_internal::RamsesLoggerConfig loggerConfig;
        uint32_t periodicLogTimeout = 2u;

        void setFeatureLevelNoCheck(EFeatureLevel featureLevel);

    private:
        EFeatureLevel m_featureLevel = EFeatureLevel_01;
        ramses_internal::EConnectionProtocol m_usedProtocol;
        ramses_internal::String m_programName;
        bool m_enableDltApplicationRegistration = true;
        bool m_enableProtocolVersionOffset;
        ramses_internal::Guid m_userProvidedGuid;
        bool m_dltAppIdSet = false;
        bool m_dltDescriptionSet = false;
    };
}

#endif
