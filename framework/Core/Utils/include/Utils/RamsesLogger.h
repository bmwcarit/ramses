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
#include "PlatformAbstraction/PlatformTypes.h"

#include <string>
#include <mutex>
#include <memory>
#include <optional>
#include <map>

namespace ramses_internal
{
    class DltLogAppender;

    struct RamsesLoggerConfig
    {
        // generic argument applies to all log levels
        std::optional<ELogLevel> logLevel;
        std::optional<ELogLevel> logLevelConsole;
        // output specific loglevels can overwrite generic argument
        std::map<std::string, ELogLevel> logLevelContexts; // TODO: std::unordered_map<std::string, ELogLevel>
        std::string dltAppId = "RAMS";
        std::string dltAppDescription = "RAMS-DESC";
    };

    struct LogContextInformation
    {
        std::string id;
        std::string name;
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

        void applyContextFilterCommand(const std::string& command);
        [[nodiscard]] std::vector<LogContextInformation> getAllContextsInformation() const;

        void setLogLevelForContexts(ELogLevel logLevel);

        void setConsoleLogLevel(ELogLevel logLevel);
        [[nodiscard]] ELogLevel getConsoleLogLevel() const;

        void setAfterConsoleLogCallback(const std::function<void()>& callback);
        void removeAfterConsoleLogCallback();

        [[nodiscard]] bool transmitFile(const std::string& path, bool deleteFile) const;
        bool registerInjectionCallback(LogContext& ctx, uint32_t serviceId, int (*callback)(uint32_t serviceId, void* data, uint32_t length));

        static const char* GetLogLevelText(ELogLevel logLevel);

        void setLogHandler(const ramses::LogHandlerFunc& logHandlerFunc);

    private:
        static const ELogLevel LogLevelDefault_Contexts = ELogLevel::Info;
        static const ELogLevel LogLevelDefault_Console = ELogLevel::Info;

        void applyContextFilter(const std::string& context, ELogLevel logLevel);

        void dltLogLevelChangeCallback(const std::string& contextId, int logLevelAsInt);
        LogContext* getLogContextById(const std::string& contextId);

        std::mutex m_appenderLock;
        bool m_isInitialized;
        ConsoleLogAppender m_consoleLogAppender;
        std::unique_ptr<DltLogAppender> m_dltLogAppender;
        std::unique_ptr<LogAppenderBase> m_platformLogAppender;
        std::unique_ptr<UserLogAppender> m_userLogAppender;
        std::vector<LogContext*> m_logContexts;
        std::vector<LogAppenderBase*> m_logAppenders;
        LogContext& m_fileTransferContext;
    };

    inline RamsesLogger& GetRamsesLogger()
    {
        static RamsesLogger logger;
        return logger;
    }
}

#endif
