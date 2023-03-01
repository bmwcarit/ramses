//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UTILS_RAMSESLOGGER_H
#define RAMSES_UTILS_RAMSESLOGGER_H

#include "Utils/LogLevel.h"
#include "Utils/LogContext.h"
#include "Utils/ConsoleLogAppender.h"
#include "Utils/LogAppenderBase.h"
#include "Utils/UserLogAppender.h"
#include "Collections/Vector.h"
#include "Collections/String.h"
#include <mutex>
#include <memory>
#include <optional>

namespace ramses_internal
{
    class DltLogAppender;

    struct RamsesLoggerConfig
    {
        // generic argument applies to all log levels
        std::optional<ELogLevel> logLevel;
        std::optional<ELogLevel> logLevelConsole;
        // output specific loglevels can overwrite generic argument
        String logLevelContextsStr;
        String dltAppId = "RAMS";
        String dltAppDescription = "RAMS-DESC";
        bool enableSmokeTestContext = false;
    };

    struct LogContextInformation
    {
        String id;
        String name;
        ELogLevel logLevel;
    };

    class RamsesLogger
    {
    public:
        RamsesLogger();
        ~RamsesLogger();

        void initialize(const RamsesLoggerConfig& config, bool disableDLT, bool enableDLTApplicationRegistration);

        LogContext& createContext(const char* name, const char* id);

        [[nodiscard]] bool isDltAppenderActive() const;

        void log(const LogMessage& msg);

        void applyContextFilterCommand(const String& command);
        [[nodiscard]] std::vector<LogContextInformation> getAllContextsInformation() const;

        void setLogLevelForContexts(ELogLevel logLevel);

        void setConsoleLogLevel(ELogLevel logLevel);
        void setConsoleLogLevelProgrammatically(ELogLevel logLevel);
        [[nodiscard]] ELogLevel getConsoleLogLevel() const;

        void setAfterConsoleLogCallback(const std::function<void()>& callback);
        void removeAfterConsoleLogCallback();

        [[nodiscard]] bool transmitFile(const String& path, bool deleteFile) const;
        bool registerInjectionCallback(LogContext& ctx, UInt32 serviceId, int (*callback)(UInt32 serviceId, void* data, UInt32 length));

        static const char* GetLogLevelText(ELogLevel logLevel);

        void setLogHandler(const ramses::LogHandlerFunc& logHandlerFunc);

    private:
        static const ELogLevel LogLevelDefault_Contexts = ELogLevel::Info;
        static const ELogLevel LogLevelDefault_Console = ELogLevel::Info;


        static void UpdateConsoleLogLevelFromDefine(ELogLevel& loglevel);
        static void UpdateConsoleLogLevelFromEnvVar(ELogLevel& loglevel);

        void dltLogLevelChangeCallback(const String& contextId, int logLevelAsInt);
        LogContext* getLogContextById(const String& contextId);

        std::mutex m_appenderLock;
        bool m_isInitialized;
        bool m_consoleLogLevelSetProgrammatically = false;
        ConsoleLogAppender m_consoleLogAppender;
        std::unique_ptr<DltLogAppender> m_dltLogAppender;
        std::unique_ptr<LogAppenderBase> m_platformLogAppender;
        std::unique_ptr<UserLogAppender> m_userLogAppender;
        std::vector<LogContext*> m_logContexts;
        std::vector<LogAppenderBase*> m_logAppenders;
        LogContext& m_fileTransferContext;
        ELogLevel m_consoleLogLevelProgrammatically = LogLevelDefault_Console;
    };

    inline RamsesLogger& GetRamsesLogger()
    {
        static RamsesLogger logger;
        return logger;
    }
}

#endif
