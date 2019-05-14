//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UTILS_RAMSESLOGGER_H
#define RAMSES_UTILS_RAMSESLOGGER_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "Utils/LogLevel.h"
#include "Utils/LogContext.h"
#include "Utils/ConsoleLogAppender.h"
#include "Utils/LogAppenderBase.h"
#include "Collections/Vector.h"
#include "Collections/String.h"
#include "Utils/Warnings.h"
#include <functional>
#include <memory>

namespace ramses_internal
{
    class CommandLineParser;
    class DltLogAppender;

    enum class ELogAppenderType
    {
        Console = 0x1,
        Dlt
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

        void initialize(const CommandLineParser& parser, const String& idString, const String& descriptionString, Bool disableDLT);

        LogContext& createContext(const char* name, const char* id);

        void addAppender(LogAppenderBase& appender);
        void removeAppender(LogAppenderBase& appender);
        bool isAppenderTypeActive(ELogAppenderType type) const;

        void log(const LogMessage& msg);

        void applyContextFilterCommand(const String& command);
        std::vector<LogContextInformation> getAllContextsInformation() const;

        void setLogLevelForContexts(ELogLevel logLevel);
        ELogLevel getLogLevelByContextId(const String& contextId) const;

        void setLogLevelForAppenderType(ELogAppenderType type, ELogLevel logLevel);
        ELogLevel getLogLevelForAppenderType(ELogAppenderType type) const;

        void setAfterConsoleLogCallback(const std::function<void()>& callback);
        void removeAfterConsoleLogCallback();

        bool transmitFile(const String& path, bool deleteFile) const;
        bool registerInjectionCallback(LogContext& ctx, UInt32 serviceId, int (*callback)(UInt32 serviceId, void* data, UInt32 length));

        static ELogLevel GetLoglevelFromInt(Int32 logLevelInt);
        static const char* GetLogLevelText(ELogLevel logLevel);

    private:
        static const ELogLevel LogLevelDefault_Contexts = ELogLevel::Info;
        static const ELogLevel LogLevelDefault_Console = ELogLevel::Info;
        static const ELogLevel LogLevelDefault_DLT = ELogLevel::Info;

        void createDltContexts(bool pushLogLevel);
        void dltLogLevelChangeCallback(const String& contextId, int logLevelAsInt);
        const LogContext* getLogContextById(const String& contextId) const;
        LogContext* getLogContextById(const String& contextId);
        void updateDltLogAppenderLoglevel();

        PlatformLightweightLock m_appenderLock;
        bool m_isInitialized;
        ConsoleLogAppender m_consoleLogAppender;
        std::unique_ptr<DltLogAppender> m_dltLogAppender;
        std::unique_ptr<LogAppenderBase> m_platformLogAppender;
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
