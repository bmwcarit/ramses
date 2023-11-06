//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/LogLevel.h"
#include <string>

namespace ramses::internal
{
    class LogContext;

    struct LogMessage
    {
        const LogContext& m_context;
        ELogLevel m_logLevel;
        std::string m_message;
    };
}
