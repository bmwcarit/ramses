//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <utility>

#include "internal/Core/Utils/UserLogAppender.h"
#include "internal/Core/Utils/LogMessage.h"
#include "internal/Core/Utils/LogContext.h"

namespace ramses::internal
{
    UserLogAppender::UserLogAppender(LogHandlerFunc f)
        : m_func(std::move(f))
    {
    }

    void UserLogAppender::log(const LogMessage& logMessage)
    {
        m_func(logMessage.getLogLevel(), logMessage.getContext().getContextId(), logMessage.getStream().data());
    }
}
