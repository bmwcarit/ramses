//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/RamsesLogger.h"
#include "Utils/ConsoleLogAppender.h"
#include "Utils/LogContext.h"
#include "Utils/LogHelper.h"
#include "Utils/Argument.h"
#include "Utils/LogMacros.h"
#include "DltLogAppender/DltLogAppender.h"
#include "PlatformAbstraction/PlatformEnvironmentVariables.h"
#include <cassert>

#ifdef __ANDROID__
    #include <AndroidLogger/AndroidLogAppender.h>
#endif

namespace ramses_internal
{
    RamsesLogger::RamsesLogger()
        : m_isInitialized(false)
        , m_consoleLogAppender()
        , m_fileTransferContext(createContext("File Transfer Context", "FILE"))
    {
        // special handling for console loglevel to allow setting console appender level to something
        // other than info even when initialize() is never called (often in tests)
        ELogLevel consoleLoglevel = LogLevelDefault_Console;
        UpdateConsoleLogLevelFromDefine(consoleLoglevel);
        UpdateConsoleLogLevelFromEnvVar(consoleLoglevel);
        m_consoleLogAppender.setLogLevel(consoleLoglevel);
        m_logAppenders.push_back(&m_consoleLogAppender);

#ifdef __ANDROID__
        m_platformLogAppender.reset(new AndroidLogAppender);
        m_logAppenders.push_back(m_platformLogAppender.get());
#endif
    }

    RamsesLogger::~RamsesLogger() = default;

    void RamsesLogger::UpdateConsoleLogLevelFromDefine(ELogLevel& loglevel)
    {
        UNUSED(loglevel);
#ifdef RAMSES_CONSOLE_LOGLEVEL_DEFAULT
        // do the macro to string dance
#define LOG_STRINGIFY1(s) #s
#define LOG_STRINGIFY2(s)  LOG_STRINGIFY1(s)
#define LOG_STRINGIFY LOG_STRINGIFY2(RAMSES_CONSOLE_LOGLEVEL_DEFAULT)
        LogHelper::StringToLogLevel(LOG_STRINGIFY, loglevel);
#undef LOG_STRINGIFY1
#undef LOG_STRINGIFY2
#undef LOG_STRINGIFY
#endif
    }

    void RamsesLogger::UpdateConsoleLogLevelFromEnvVar(ELogLevel& loglevel)
    {
        String envVarValue;
        if (PlatformEnvironmentVariables::get("CONSOLE_LOGLEVEL", envVarValue))
        {
            ELogLevel consoleLogLevelEnvVar;
            if (LogHelper::StringToLogLevel(envVarValue, consoleLogLevelEnvVar))
                loglevel = consoleLogLevelEnvVar;
        }
    }

    void RamsesLogger::setConsoleLogLevelProgrammatically(ELogLevel logLevel)
    {
        m_consoleLogAppender.setLogLevel(logLevel);
        m_consoleLogLevelSetProgrammatically = true;
        m_consoleLogLevelProgrammatically = logLevel;
    }


    void RamsesLogger::initialize(const CommandLineParser& parser, const String& idString, const String& descriptionString, bool disableDLT, bool enableDLTApplicationRegistration)
    {
        m_isInitialized = true;

        bool pushLogLevelToDltDaemon = false;

        ELogLevel logLevelContexts = LogLevelDefault_Contexts;
        ELogLevel logLevelConsole = LogLevelDefault_Console;

        UpdateConsoleLogLevelFromDefine(logLevelConsole);

        // programmatically set log level should override define, but will be afterwards overridden by cmdl arg "log-level-console" or "log-level"
        if (m_consoleLogLevelSetProgrammatically)
            logLevelConsole = m_consoleLogLevelProgrammatically;

        // generic "-l" argument applies to all log levels
        ArgumentString logLevelStr(parser, "l", "log-level", "");
        ELogLevel logLevel;
        if (LogHelper::StringToLogLevel(logLevelStr, logLevel))
        {
            logLevelContexts = logLevel;
            logLevelConsole = logLevel;
            pushLogLevelToDltDaemon = true;
        }

        // output specific loglevels can overwrite generic argument
        ArgumentString logLevelAllContextsStr(parser, "cl", "log-level-contexts", "");
        if (LogHelper::StringToLogLevel(logLevelAllContextsStr, logLevelContexts))
        {
            pushLogLevelToDltDaemon = true;
        }

        ArgumentString logLevelConsoleStr(parser, "lc", "log-level-console", "");
        LogHelper::StringToLogLevel(logLevelConsoleStr, logLevelConsole);

        //same for environment variable
        String envVarValue;
        if (PlatformEnvironmentVariables::get("RAMSES_LOGLEVEL", envVarValue))
        {
            ELogLevel ramsesLogLevelEnvVar;
            if (LogHelper::StringToLogLevel(envVarValue, ramsesLogLevelEnvVar))
            {
                logLevelContexts = ramsesLogLevelEnvVar;
                logLevelConsole = ramsesLogLevelEnvVar;
                pushLogLevelToDltDaemon = true;
            }
        }

        UpdateConsoleLogLevelFromEnvVar(logLevelConsole);

        // apply loglevels
        setLogLevelForContexts(logLevelContexts);
        m_consoleLogAppender.setLogLevel(logLevelConsole);

        // apply by context filter
        ArgumentString logLevelContextsStr(parser, "clf", "log-level-contexts-filter", "");
        if (logLevelContextsStr.hasValue())
        {
            applyContextFilterCommand(logLevelContextsStr);
            pushLogLevelToDltDaemon = true;
        }

        // create DLT adapter and appender
        if (!disableDLT)
        {
            const ramses_internal::ArgumentString dltAppId(parser, "dai", "dlt-app-id", idString);
            const ramses_internal::ArgumentString dltAppDescription(parser, "dad", "dlt-app-description", descriptionString);
            const String& dltAppIdAsString = dltAppId;

            if (!dltAppIdAsString.empty())
            {
                LOG_INFO(CONTEXT_FRAMEWORK, "RamsesLogger::initialize: Initializing DLT adapter");
                if (!enableDLTApplicationRegistration)
                    LOG_INFO(CONTEXT_FRAMEWORK, "RamsesLogger::initialize: Reuse exising DLT application registration");

                bool alreadyHadDltAppender = false;

                {
                    std::lock_guard<std::mutex> guard(m_appenderLock);
                    alreadyHadDltAppender = (m_dltLogAppender != nullptr);

                    if (!alreadyHadDltAppender)
                    {
                        std::vector<LogContext*> logContextPtrs;
                        for (auto& ctx : m_logContexts)
                            logContextPtrs.push_back(ctx.get());
                        DltAdapter* dltAdapter = DltAdapter::getDltAdapter();
                        if (dltAdapter->initialize(dltAppIdAsString, dltAppDescription, enableDLTApplicationRegistration,
                                            [this](const String& contextId_, int logLevel_) {
                                                dltLogLevelChangeCallback(contextId_, logLevel_);
                                            },
                                            logContextPtrs, pushLogLevelToDltDaemon))
                        {
                            m_dltLogAppender = std::make_unique<DltLogAppender>();
                            m_logAppenders.push_back(m_dltLogAppender.get());
                        }
                    }
                }

                if (alreadyHadDltAppender)
                {
                    LOG_INFO(CONTEXT_FRAMEWORK, "RamsesLogger::initialize: skipped DLT initialization because a DLT adapter already exists");
                }

                if (!m_dltLogAppender)
                {
                    LOG_WARN(CONTEXT_FRAMEWORK, "RamsesLogger::initialize: DLT disabled because initialize failed");
                }
            }
            else
            {
                LOG_WARN(CONTEXT_FRAMEWORK, "RamsesLogger::initialize: DLT disabled because applicationID not set");
            }
        }

        if (m_userLogAppender)
        {
            LOG_INFO(CONTEXT_FRAMEWORK, "RamsesLogger::initialize: a user logger was added");
        }

        ArgumentBool enableSmokeTestContext(parser, "estc", "enableSmokeTestContext", "");
        if (!enableSmokeTestContext.wasDefined())
        {
            CONTEXT_SMOKETEST.setLogLevel(ELogLevel::Off);
            CONTEXT_SMOKETEST.disableSetLogLevel();
        }

        LOG_INFO(CONTEXT_FRAMEWORK, "Ramses log levels: Contexts " << RamsesLogger::GetLogLevelText(logLevelContexts) <<
                 ", Console " << RamsesLogger::GetLogLevelText(logLevelConsole));
    }

    void RamsesLogger::applyContextFilterCommand(const String& command)
    {
        for (const auto& contextFilter : LogHelper::ParseContextFilters(command))
        {
            if (LogContext* ctx = getLogContextById(contextFilter.second))
            {
                LOG_INFO(CONTEXT_FRAMEWORK, contextFilter.second << " | " << ctx->getContextName()
                         << " | "
                         << static_cast<Int32>(contextFilter.first)
                         << " | "
                         << RamsesLogger::GetLogLevelText(contextFilter.first));
                ctx->setLogLevel(contextFilter.first);
            }
            else
            {
                LOG_INFO(CONTEXT_FRAMEWORK, "RamsesLogger::applyContextFilterCommand: unknown contextId " << contextFilter.second);
            }
        }
    }

    std::vector<LogContextInformation> RamsesLogger::getAllContextsInformation() const
    {
        std::vector<LogContextInformation> result;
        for (const auto& ctx : m_logContexts)
        {
            result.push_back({ ctx->getContextId(), ctx->getContextName(), ctx->getLogLevel() });
        }
        return result;
    }

    void RamsesLogger::setLogLevelForContexts(ELogLevel logLevel)
    {
        for (auto& ctx : m_logContexts)
        {
            ctx->setLogLevel(logLevel);
        }
    }

    void RamsesLogger::setConsoleLogLevel(ELogLevel logLevel)
    {
        m_consoleLogAppender.setLogLevel(logLevel);
    }

    ELogLevel RamsesLogger::getConsoleLogLevel() const
    {
        return m_consoleLogAppender.getLogLevel();
    }

    void RamsesLogger::setAfterConsoleLogCallback(const std::function<void()>& callback)
    {
        std::lock_guard<std::mutex> guard(m_appenderLock);
        m_consoleLogAppender.setAfterLogCallback(callback);
    }

    void RamsesLogger::removeAfterConsoleLogCallback()
    {
        std::lock_guard<std::mutex> guard(m_appenderLock);
        m_consoleLogAppender.removeAfterLogCallback();
    }

    bool RamsesLogger::transmitFile(const String& path, bool deleteFile) const
    {
        if (!m_dltLogAppender)
        {
            return false;
        }

        return DltAdapter::getDltAdapter()->transmitFile(m_fileTransferContext, path, deleteFile);
    }

    bool RamsesLogger::transmit(std::vector<Byte> && data, const String &filename) const
    {
        if (!m_dltLogAppender)
        {
            return false;
        }

        return DltAdapter::getDltAdapter()->transmit(m_fileTransferContext, std::move(data), filename);
    }

    bool RamsesLogger::registerInjectionCallback(LogContext& ctx, UInt32 serviceId, int (*callback)(UInt32 serviceId, void* data, UInt32 length))
    {
        if (!m_dltLogAppender)
        {
            return false;
        }
        DltAdapter::getDltAdapter()->registerInjectionCallback(&ctx, serviceId, callback);
        return true;
    }

    bool RamsesLogger::isDltAppenderActive() const
    {
        return m_dltLogAppender != nullptr;
    }

    LogContext& RamsesLogger::createContext(const char* name, const char* id)
    {
        if (m_isInitialized)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "RamsesLogger::createContext: Context creation not allowed after initialize");
            assert(false);
        }
        m_logContexts.push_back(std::make_unique<LogContext>(name, id));

        return *m_logContexts.back();
    }

    void RamsesLogger::dltLogLevelChangeCallback(const String& contextId, int logLevelAsInt)
    {
        LogContext* ctx = getLogContextById(contextId);
        if (!ctx)
        {
            LOG_WARN(CONTEXT_RAMSH, "RamsesLogger::dltLogLevelChangeCallback: unknown contextId " << contextId);
            return;
        }

        const ELogLevel currentLogLevel = ctx->getLogLevel();
        const ELogLevel newLogLevel = LogHelper::GetLoglevelFromInt(logLevelAsInt);

        if (currentLogLevel != newLogLevel)
        {
            LOG_INFO(CONTEXT_RAMSH, contextId << " | " << ctx->getContextName()
                     << " | "
                     << static_cast<Int32>(newLogLevel)
                     << " | "
                     << GetLogLevelText(newLogLevel)
                     << " | "
                     << " Dlt changed log level from "
                     << GetLogLevelText(currentLogLevel)
                     );
            ctx->setLogLevel(newLogLevel);
        }
    }

    LogContext* RamsesLogger::getLogContextById(const String& contextId)
    {
        for (const auto& ctx : m_logContexts)
            if (contextId == ctx->getContextId())
                return ctx.get();
        return nullptr;
    }

    void RamsesLogger::log(const LogMessage& msg)
    {
        if (msg.getStream().size() > 0)
        {
            std::lock_guard<std::mutex> guard(m_appenderLock);
            for (auto& appender : m_logAppenders)
            {
                appender->log(msg);
            }
        }
    }

    const char* RamsesLogger::GetLogLevelText(ELogLevel logLevel)
    {
        switch (logLevel)
        {
        case ELogLevel::Off:    return "OFF";
        case ELogLevel::Fatal:  return "FATAL";
        case ELogLevel::Error:  return "ERROR";
        case ELogLevel::Warn:   return "WARN";
        case ELogLevel::Info:   return "INFO";
        case ELogLevel::Debug:  return "DEBUG";
        case ELogLevel::Trace:  return "TRACE";
        default:
            assert(false);
            return "<UNKNOWN>";
        }
    }

    void RamsesLogger::setLogHandler(const ramses::LogHandlerFunc& logHandlerFunc)
    {
        if (logHandlerFunc)
        {
            LOG_INFO(CONTEXT_FRAMEWORK, "RamsesLogger::setLogHandler: adding a user logger");
        }
        else
        {
            LOG_INFO(CONTEXT_FRAMEWORK, "RamsesLogger::setLogHandler: deleting a user logger");
        }

        std::lock_guard<std::mutex> guard(m_appenderLock);
        if (m_userLogAppender)
        {
            m_logAppenders.erase(std::find(m_logAppenders.begin(), m_logAppenders.end(), m_userLogAppender.get()));
            m_userLogAppender = nullptr;
        }
        if (logHandlerFunc)
        {
            m_userLogAppender = std::make_unique<UserLogAppender>(logHandlerFunc);
            m_logAppenders.push_back(m_userLogAppender.get());
        }
    }
}
