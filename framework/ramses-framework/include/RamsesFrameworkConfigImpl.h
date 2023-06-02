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

#include <string>

namespace ramses
{
    class RamsesFrameworkConfigImpl : public StatusObjectImpl
    {
    public:
        RamsesFrameworkConfigImpl();
        ~RamsesFrameworkConfigImpl() override;

        status_t setFeatureLevel(EFeatureLevel featureLevel);
        EFeatureLevel getFeatureLevel() const;

        status_t enableDLTApplicationRegistration(bool state);
        bool getDltApplicationRegistrationEnabled() const;

        void setDLTApplicationID(std::string_view id);
        std::string_view getDLTApplicationID() const;

        void setDLTApplicationDescription(std::string_view description);
        std::string_view getDLTApplicationDescription() const;

        uint32_t getProtocolVersion() const;

        status_t setWatchdogNotificationInterval(ramses::ERamsesThreadIdentifier thread, uint32_t interval);
        status_t setWatchdogNotificationCallBack(IThreadWatchdogNotification* callback);

        status_t setRequestedRamsesShellType(ERamsesShellType shellType);

        ramses_internal::EConnectionProtocol getUsedProtocol() const;
        uint32_t getWatchdogNotificationInterval(ERamsesThreadIdentifier thread) const;
        IThreadWatchdogNotification* getWatchdogNotificationCallback() const;

        void setLogLevel(ELogLevel logLevel);
        status_t setLogLevel(std::string_view context, ELogLevel logLevel);
        void setLogLevelConsole(ELogLevel logLevel);

        void setPeriodicLogInterval(std::chrono::seconds interval);

        status_t setParticipantGuid(uint64_t guid);
        ramses_internal::Guid getUserProvidedGuid() const;

        status_t setParticipantName(std::string_view name);
        const std::string& getParticipantName() const;

        status_t setConnectionSystem(EConnectionSystem connectionSystem);

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
        std::string m_participantName;
        bool m_enableDltApplicationRegistration = true;
        ramses_internal::Guid m_userProvidedGuid;
    };
}

#endif
