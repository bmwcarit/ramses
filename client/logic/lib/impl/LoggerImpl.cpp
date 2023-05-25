//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/LoggerImpl.h"

namespace ramses::internal
{
    LoggerImpl::LoggerImpl() noexcept
        : m_logHandler(nullptr)
    {
    }

    void LoggerImpl::setLogHandler(Logger::LogHandlerFunc logHandlerFunc)
    {
        m_logHandler = std::move(logHandlerFunc);
    }

    void LoggerImpl::setDefaultLogging(bool loggingEnabled)
    {
        m_defaultLogging = loggingEnabled;
    }

    void LoggerImpl::setLogVerbosityLimit(ELogLevel verbosityLimit)
    {
        m_logVerbosityLimit = verbosityLimit;
    }

    ELogLevel LoggerImpl::getLogVerbosityLimit() const
    {
        return m_logVerbosityLimit;
    }

}
