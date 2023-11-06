//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/Utils/RamsesLogger.h"
#include "internal/Core/Utils/ConsoleLogAppender.h"
#include "internal/Core/Utils/LogContext.h"
#include "internal/Core/Utils/LogHelper.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/DltLogAppender/DltLogAppender.h"
#include "internal/PlatformAbstraction/PlatformEnvironmentVariables.h"
#include <cassert>

#ifdef __ANDROID__
    #include "internal/Core/Utils/AndroidLogger/AndroidLogAppender.h"
#endif

namespace ramses::internal
{
    thread_local std::string RamsesLogger::PrefixInstance = "R";
    thread_local std::string RamsesLogger::PrefixThread = "main";
    thread_local std::string RamsesLogger::PrefixAdditional;
    thread_local std::string RamsesLogger::PrefixCombined = "R.main: ";

    RamsesLogger::RamsesLogger()
        : m_isInitialized(false)
        , m_fileTransferContext(createContext("File Transfer Context", "FILE"))
    {
        m_consoleLogAppender.setLogLevel(LogLevelDefault_Console);
        m_logAppenders.push_back(&m_consoleLogAppender);

#ifdef __ANDROID__
        m_platformLogAppender.reset(new AndroidLogAppender);
        m_logAppenders.push_back(m_platformLogAppender.get());
#endif
    }

    RamsesLogger::~RamsesLogger() = default;

    void RamsesLogger::initialize(const RamsesLoggerConfig& config, bool disableDLT, bool enableDLTApplicationRegistration)
    {
        m_isInitialized = true;

        bool pushLogLevelToDltDaemon = false;

        ELogLevel logLevelContexts = LogLevelDefault_Contexts;
        ELogLevel logLevelConsole = LogLevelDefault_Console;

        // generic argument applies to all log levels
        if (config.logLevel.has_value())
        {
            logLevelContexts = config.logLevel.value();
            logLevelConsole = config.logLevel.value();
            pushLogLevelToDltDaemon = true;
        }

        if (config.logLevelConsole.has_value())
        {
            logLevelConsole = config.logLevelConsole.value();
        }

        // apply loglevels
        setLogLevelForContexts(logLevelContexts);
        m_consoleLogAppender.setLogLevel(logLevelConsole);

        // apply by context filter
        if (!config.logLevelContexts.empty())
        {
            for (const auto& cmd : config.logLevelContexts)
            {
                applyContextFilter(cmd.first, cmd.second);
            }
            pushLogLevelToDltDaemon = true;
        }

        // create DLT adapter and appender
        if (!disableDLT)
        {
            if (!config.dltAppId.empty())
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
                        if (dltAdapter->initialize(config.dltAppId, config.dltAppDescription, enableDLTApplicationRegistration,
                                            [this](const std::string& contextId_, int logLevel_) {
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

        LOG_INFO(CONTEXT_FRAMEWORK, "Ramses log levels: Contexts " << RamsesLogger::GetLogLevelText(logLevelContexts) <<
                 ", Console " << RamsesLogger::GetLogLevelText(logLevelConsole));
    }

    void RamsesLogger::applyContextFilterCommand(const std::string& command)
    {
        for (const auto& contextFilter : LogHelper::ParseContextFilters(command))
        {
            applyContextFilter(contextFilter.second, contextFilter.first);
        }
    }

    void RamsesLogger::applyContextFilter(const std::string& contextId, ELogLevel logLevel)
    {
        if (LogContext* ctx = getLogContextById(contextId))
        {
            LOG_INFO(CONTEXT_FRAMEWORK, contextId << " | " << ctx->getContextName()
                     << " | "
                     << static_cast<int32_t>(logLevel)
                     << " | "
                     << RamsesLogger::GetLogLevelText(logLevel));
            ctx->setLogLevel(logLevel);
        }
        else
        {
            LOG_INFO(CONTEXT_FRAMEWORK, "RamsesLogger::applyContextFilterCommand: unknown contextId " << contextId);
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

    bool RamsesLogger::transmitFile(const std::string& path, bool deleteFile) const
    {
        if (!m_dltLogAppender)
        {
            return false;
        }

        return DltAdapter::getDltAdapter()->transmitFile(m_fileTransferContext, path, deleteFile);
    }

    bool RamsesLogger::transmit(std::vector<std::byte> && data, const std::string &filename) const
    {
        if (!m_dltLogAppender)
        {
            return false;
        }

        return DltAdapter::getDltAdapter()->transmit(m_fileTransferContext, std::move(data), filename);
    }

    bool RamsesLogger::registerInjectionCallback(LogContext& ctx, uint32_t serviceId, int (*callback)(uint32_t serviceId, void* data, uint32_t length))
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

    void RamsesLogger::dltLogLevelChangeCallback(const std::string& contextId, int logLevelAsInt)
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
                     << static_cast<int32_t>(newLogLevel)
                     << " | "
                     << GetLogLevelText(newLogLevel)
                     << " | "
                     << " Dlt changed log level from "
                     << GetLogLevelText(currentLogLevel)
                     );
            ctx->setLogLevel(newLogLevel);
        }
    }

    LogContext* RamsesLogger::getLogContextById(const std::string& contextId)
    {
        for (const auto& ctx : m_logContexts)
        {
            if (contextId == ctx->getContextId())
                return ctx.get();
        }
        return nullptr;
    }

    void RamsesLogger::log(LogMessage&& msg)
    {
        if (!msg.m_message.empty())
        {
            msg.m_message.insert(0, PrefixCombined);

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

    void RamsesLogger::setLogHandler(const LogHandlerFunc& logHandlerFunc)
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

    void RamsesLogger::SetPrefixes(std::string_view instance, std::string_view thread, std::string_view additional)
    {
        PrefixInstance = instance;
        PrefixThread = thread;
        PrefixAdditional = additional;

        PrefixCombined.clear();
        if (!PrefixAdditional.empty())
        {
            fmt::format_to(std::back_inserter(PrefixCombined), "{}.{}.{}: ", PrefixInstance, PrefixThread, PrefixAdditional);
        }
        else
        {
            fmt::format_to(std::back_inserter(PrefixCombined), "{}.{}: ", PrefixInstance, PrefixThread);
        }
    }

    void RamsesLogger::SetPrefixAdditional(std::string_view additional)
    {
        SetPrefixes(PrefixInstance, PrefixThread, additional);
    }

    const std::string& RamsesLogger::GetPrefixInstance()
    {
        return PrefixInstance;
    }
}
