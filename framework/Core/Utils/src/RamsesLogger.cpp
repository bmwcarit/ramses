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
#include "PlatformAbstraction/PlatformGuard.h"

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
        addAppender(m_consoleLogAppender);

#ifdef __ANDROID__
        m_platformLogAppender.reset(new AndroidLogAppender);
        addAppender(*m_platformLogAppender);
#endif
    }

    RamsesLogger::~RamsesLogger()
    {
        for (auto& ctx : m_logContexts)
        {
            delete ctx;
        }
    }

    void RamsesLogger::initialize(const CommandLineParser& parser, const String& idString, const String& descriptionString, Bool disableDLT)
    {
        if (m_isInitialized)
        {
            // should not be called multiple times but happens when more than one RamsesFramework is created
            LOG_WARN(CONTEXT_FRAMEWORK, "RamsesLogger::initialize called more than once");
        }
        m_isInitialized = true;

        bool pushLogLevelToDltDaemon = false;

        ELogLevel logLevelContexts = LogLevelDefault_Contexts;
        ELogLevel logLevelConsole = LogLevelDefault_Console;
        ELogLevel logLevelDlt = LogLevelDefault_DLT;

#ifdef RAMSES_CONSOLE_LOGLEVEL_DEFAULT
        // do the macro to string dance
#define LOG_STRINGIFY1(s) #s
#define LOG_STRINGIFY2(s)  LOG_STRINGIFY1(s)
#define LOG_STRINGIFY LOG_STRINGIFY2(RAMSES_CONSOLE_LOGLEVEL_DEFAULT)
        LogHelper::StringToLogLevel(LOG_STRINGIFY, logLevelConsole);
#undef LOG_STRINGIFY1
#undef LOG_STRINGIFY2
#undef LOG_STRINGIFY
#endif

        // generic "-l" argument applies to all log levels
        ArgumentString logLevelStr(parser, "l", "log-level", "");
        ELogLevel logLevel;
        if (LogHelper::StringToLogLevel(logLevelStr, logLevel))
        {
            logLevelContexts = logLevel;
            logLevelConsole = logLevel;
            logLevelDlt = logLevel;
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

        ArgumentString logLevelDltStr(parser, "ld", "log-level-dlt", "");
        LogHelper::StringToLogLevel(logLevelDltStr, logLevelDlt);

        //same for environment variable
        String envVarValue;
        ELogLevel ramsesLogLevelEnvVar;
        ELogLevel consoleLogLevelEnvVar;

        if (PlatformEnvironmentVariables::get("RAMSES_LOGLEVEL", envVarValue))
        {
            if (LogHelper::StringToLogLevel(envVarValue, ramsesLogLevelEnvVar))
            {
                logLevelContexts = ramsesLogLevelEnvVar;
                logLevelConsole = ramsesLogLevelEnvVar;
                logLevelDlt = ramsesLogLevelEnvVar;
                pushLogLevelToDltDaemon = true;
            }
        }

        if (PlatformEnvironmentVariables::get("CONSOLE_LOGLEVEL", envVarValue))
        {
            if (LogHelper::StringToLogLevel(envVarValue, consoleLogLevelEnvVar))
            {
                logLevelConsole = consoleLogLevelEnvVar;
            }
        }

        // apply loglevels
        setLogLevelForContexts(logLevelContexts);
        setLogLevelForAppenderType(ELogAppenderType::Console, logLevelConsole);

        // apply by context filter
        ArgumentString logLevelContextsStr(parser, "clf", "log-level-contexts-filter", "");
        if (logLevelContextsStr.hasValue())
        {
            applyContextFilterCommand(logLevelContextsStr);
            pushLogLevelToDltDaemon = true;
        }

        // create DLT adapter and appender
        if (!m_dltLogAppender && !disableDLT)
        {
            const ramses_internal::ArgumentString dltAppId(parser, "dai", "dlt-app-id", idString);
            const ramses_internal::ArgumentString dltAppDescription(parser, "dad", "dlt-app-description", descriptionString);
            const String& dltAppIdAsString = dltAppId;

            if (!dltAppIdAsString.empty())
            {
                DltAdapter* dltAdapter = DltAdapter::getDltAdapter();
                if (dltAdapter->registerApplication(dltAppIdAsString, dltAppDescription))
                {
                    dltAdapter->registerLogLevelChangeCallback([this](const String& contextId_, int logLevel_) {
                            dltLogLevelChangeCallback(contextId_, logLevel_);
                        });
                    createDltContexts(pushLogLevelToDltDaemon);

                    m_dltLogAppender.reset(new DltLogAppender);
                    addAppender(*m_dltLogAppender);
                }
                else
                {
                    LOG_WARN(CONTEXT_FRAMEWORK, "RamsesLogger::initialize: DLT disabled because registerApplication failed");
                }
            }
            else
            {
                LOG_WARN(CONTEXT_FRAMEWORK, "RamsesLogger::initialize: DLT disabled because applicationID not set");
            }
        }

        setLogLevelForAppenderType(ELogAppenderType::Dlt, logLevelDlt);

        ArgumentBool enableSmokeTestContext(parser, "estc", "enableSmokeTestContext", "");
        if (!enableSmokeTestContext.wasDefined())
        {
            CONTEXT_SMOKETEST.setLogLevel(ELogLevel::Off);
        }

        LOG_INFO(CONTEXT_FRAMEWORK, "Ramses log levels: Contexts " << RamsesLogger::GetLogLevelText(logLevelContexts) <<
                 ", Console " << RamsesLogger::GetLogLevelText(logLevelConsole) <<
                 ", DLT " << RamsesLogger::GetLogLevelText(logLevelDlt));
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

    ELogLevel RamsesLogger::getLogLevelByContextId(const String& contextId) const
    {
        if (const LogContext* ctx = getLogContextById(contextId))
        {
            return ctx->getLogLevel();
        }
        return ELogLevel::Off;
    }

    void RamsesLogger::setLogLevelForAppenderType(ELogAppenderType type, ELogLevel logLevel)
    {
        switch (type)
        {
        case ELogAppenderType::Console:
            m_consoleLogAppender.setLogLevel(logLevel);
            break;
        case ELogAppenderType::Dlt:
            if (m_dltLogAppender)
            {
                m_dltLogAppender->setLogLevel(logLevel);
            }
            break;
        default:
            break;
        }
    }

    ELogLevel RamsesLogger::getLogLevelForAppenderType(ELogAppenderType type) const
    {
        switch (type)
        {
        case ELogAppenderType::Console:
            return m_consoleLogAppender.getLogLevel();
        case ELogAppenderType::Dlt:
            return m_dltLogAppender ? m_dltLogAppender->getLogLevel() : ELogLevel::Off;
        default:
            return ELogLevel::Off;
        }
    }

    void RamsesLogger::setAfterConsoleLogCallback(const std::function<void()>& callback)
    {
        m_consoleLogAppender.setAfterLogCallback(callback);
    }

    void RamsesLogger::removeAfterConsoleLogCallback()
    {
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

    bool RamsesLogger::registerInjectionCallback(LogContext& ctx, UInt32 serviceId, int (*callback)(UInt32 serviceId, void* data, UInt32 length))
    {
        if (!m_dltLogAppender)
        {
            return false;
        }
        DltAdapter::getDltAdapter()->registerInjectionCallback(&ctx, serviceId, callback);
        return true;
    }

    bool RamsesLogger::isAppenderTypeActive(ELogAppenderType type) const
    {
        switch(type)
        {
        case ELogAppenderType::Dlt:
            return m_dltLogAppender != nullptr;
        case ELogAppenderType::Console:
        default:
            return true;
        }
    }

    LogContext& RamsesLogger::createContext(const char* name, const char* id)
    {
        if (m_isInitialized)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "RamsesLogger::createContext: Context creation not allowed after initialize");
            assert(false);
        }
        LogContext* ctx = new LogContext(name, id);
        m_logContexts.push_back(ctx);
        return *ctx;
    }

    void RamsesLogger::createDltContexts(bool pushLogLevel)
    {
        for (auto& ctx : m_logContexts)
        {
            assert(ctx->getUserData() == nullptr);
            DltAdapter::getDltAdapter()->registerContext(ctx, pushLogLevel, ctx->getLogLevel());
        }
    }

    void RamsesLogger::dltLogLevelChangeCallback(const String& contextId, int logLevelAsInt)
    {
        const ELogLevel currentLogLevel = getLogLevelByContextId(contextId);
        const ELogLevel newLogLevel = GetLoglevelFromInt(logLevelAsInt);

        if (currentLogLevel != newLogLevel)
        {
            if (LogContext* ctx = getLogContextById(contextId))
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

                updateDltLogAppenderLoglevel();
            }
            else
            {
                LOG_WARN(CONTEXT_RAMSH, "RamsesLogger::dltLogLevelChangeCallback: unknown contextId " << contextId);
            }
        }
    }

    void RamsesLogger::updateDltLogAppenderLoglevel()
    {
        // update to highest context level
        if (m_dltLogAppender)
        {
            ELogLevel newDltLogLevel = ELogLevel::Off;
            for (const auto ctx : m_logContexts)
            {
                if (ctx->getLogLevel() > newDltLogLevel)
                {
                    newDltLogLevel = ctx->getLogLevel();
                }
            }
            m_dltLogAppender->setLogLevel(newDltLogLevel);
        }
    }

    const LogContext* RamsesLogger::getLogContextById(const String& contextId) const
    {
        for (const auto& ctx : m_logContexts)
        {
            if (contextId == ctx->getContextId())
            {
                return ctx;
            }
        }
        return nullptr;
    }

    LogContext* RamsesLogger::getLogContextById(const String& contextId)
    {
        // reuse const version
        return const_cast<LogContext*>(const_cast<const RamsesLogger*>(this)->getLogContextById(contextId));
    }

    void RamsesLogger::addAppender(LogAppenderBase& appender)
    {
        PlatformLightweightGuard guard(m_appenderLock);
        if (find_c(m_logAppenders, &appender) == m_logAppenders.end())
        {
            m_logAppenders.push_back(&appender);
        }
    }

    void RamsesLogger::removeAppender(LogAppenderBase& appender)
    {
        PlatformLightweightGuard guard(m_appenderLock);
        auto it = find_c(m_logAppenders, &appender);
        if (it != m_logAppenders.end())
        {
            m_logAppenders.erase(it);
        }
    }

    void RamsesLogger::log(const LogMessage& msg)
    {
        if (msg.getStream().length() > 0)
        {
            PlatformLightweightGuard guard(m_appenderLock);
            for (auto& appender : m_logAppenders)
            {
                appender->log(msg);
            }
        }
    }

    ELogLevel RamsesLogger::GetLoglevelFromInt(Int32 logLevelInt)
    {
        return LogHelper::GetLoglevelFromInt(logLevelInt);
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
}
