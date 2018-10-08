//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/LogAppenderBase.h"
#include "Utils/LogMessage.h"

namespace ramses_internal
{
    void LogAppenderBase::log(const LogMessage& message)
    {
        if(static_cast<int>(message.getLogLevel()) <= static_cast<int>(m_logLevel.load()))
        {
            logMessage(message);
        }
    }
}
