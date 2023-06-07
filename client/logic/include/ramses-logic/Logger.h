//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-framework-api/APIExport.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

#include <functional>
#include <string_view>

/**
 * @ingroup LogicAPI
 * Interface to interact with the internal logger. If you want to handle log messages by yourself, you can
 * register your own log handler function with #ramses::Logger::SetLogHandler, which is called each time
 * a log message is logged. In addition you can silence the standard output of the log messages
 */
namespace ramses::Logger
{
    /**
    * The #LogHandlerFunc can be used to implement a custom log handler. The
    * function is called for each log message separately. After the call to the function
    * the string data behind std::string_view is deleted. If you want to keep it, you must
    * copy it e.g. to a std::string.
    * E.g.
    * \code{.cpp}
    *   ramses::Logger::SetLogHandler([](ElogMessageType msgType, std::string_view message){
    *       std::cout << message std::endl;
    *   });
    * \endcode
    */
    using LogHandlerFunc = std::function<void(ELogLevel, std::string_view)>;

    /**
    * Controls how verbose the logging is. \p verbosityLimit has the following semantics:
    * - if log message has message type with higher or equal priority as verbosityLimit, then it is logged
    * - log priority is as documented by #ramses::ELogLevel (Errors are more important than Warnings, etc)
    * - the default value is #ramses::ELogLevel::Info, meaning that log messages are processed if they
    * have INFO priority or higher.
    *
    * @param verbosityLimit least priority a log message must have in order to be processed
    */
    RAMSES_API void SetLogVerbosityLimit(ELogLevel verbosityLimit);

    /**
    * Returns the current log verbosity limit of the logger. See #ramses::Logger::SetLogVerbosityLimit for
    * more info on semantics.
    *
    * @return current log verbosity limit
    */
    RAMSES_API ELogLevel GetLogVerbosityLimit();

    /**
    * Sets a custom log handler function, which is called each time a log message occurs.
    * Note: setting a custom logger incurs a slight performance cost because log messages
    * will be assembled and reported, even if default logging is disabled (#SetDefaultLogging).
    *
    * @ param logHandlerFunc function which is called for each log message
    */
    RAMSES_API void SetLogHandler(const LogHandlerFunc& logHandlerFunc);

    /**
    * Sets the default logging to std::out to enabled or disabled. Enabled by default.
    *
    * @param loggingEnabled true if you want to enable logging to std::out, false otherwise
    */
    RAMSES_API void SetDefaultLogging(bool loggingEnabled);
}
