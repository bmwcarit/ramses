//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "TCPConfig.h"
#include "impl/RamsesLoggerImpl.h"
#include "ramses/framework/IThreadWatchdogNotification.h"
#include "ramses/framework/EFeatureLevel.h"
#include "impl/ThreadWatchdogConfig.h"
#include "internal/Communication/TransportCommon/EConnectionProtocol.h"
#include "internal/PlatformAbstraction/Collections/Guid.h"

#include <string>

namespace ramses::internal
{
    class RamsesFrameworkConfigImpl
    {
    public:
        explicit RamsesFrameworkConfigImpl(EFeatureLevel featureLevel);
        ~RamsesFrameworkConfigImpl();

        [[nodiscard]] bool setFeatureLevel(EFeatureLevel featureLevel);
        [[nodiscard]] EFeatureLevel getFeatureLevel() const;

        [[nodiscard]] bool enableDLTApplicationRegistration(bool state);
        [[nodiscard]] bool getDltApplicationRegistrationEnabled() const;

        void setDLTApplicationID(std::string_view id);
        [[nodiscard]] std::string_view getDLTApplicationID() const;

        void setDLTApplicationDescription(std::string_view description);
        [[nodiscard]] std::string_view getDLTApplicationDescription() const;

        [[nodiscard]] uint32_t getProtocolVersion() const;

        [[nodiscard]] bool setWatchdogNotificationInterval(ERamsesThreadIdentifier thread, uint32_t interval);
        [[nodiscard]] bool setWatchdogNotificationCallBack(IThreadWatchdogNotification* callback);

        [[nodiscard]] bool setRequestedRamsesShellType(ERamsesShellType shellType);

        [[nodiscard]] EConnectionProtocol getUsedProtocol() const;
        [[nodiscard]] uint32_t getWatchdogNotificationInterval(ERamsesThreadIdentifier thread) const;
        [[nodiscard]] IThreadWatchdogNotification* getWatchdogNotificationCallback() const;

        void setLogLevel(ELogLevel logLevel);
        [[nodiscard]] bool setLogLevel(std::string_view context, ELogLevel logLevel);
        void setLogLevelConsole(ELogLevel logLevel);

        void setPeriodicLogInterval(std::chrono::seconds interval);
        void setLoggingInstanceName(std::string_view instanceName);
        [[nodiscard]] const std::string& getLoggingInstanceName() const;

        [[nodiscard]] bool setParticipantGuid(uint64_t guid);
        [[nodiscard]] Guid getUserProvidedGuid() const;

        [[nodiscard]] bool setParticipantName(std::string_view name);
        [[nodiscard]] const std::string& getParticipantName() const;

        [[nodiscard]] bool setConnectionSystem(EConnectionSystem connectionSystem);

        TCPConfig        m_tcpConfig;
        ERamsesShellType m_shellType;
        ThreadWatchdogConfig m_watchdogConfig;
        bool m_periodicLogsEnabled;

        RamsesLoggerConfig loggerConfig;
        uint32_t periodicLogTimeout = 2u;

        void setFeatureLevelNoCheck(EFeatureLevel featureLevel);

    private:
        EFeatureLevel m_featureLevel = EFeatureLevel_01;
        EConnectionProtocol m_usedProtocol;
        std::string m_participantName;
        bool m_enableDltApplicationRegistration = true;
        Guid m_userProvidedGuid;
        std::string m_loggingInstanceName = "R";
    };
}
