//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/LogAppenderBase.h"
#include "internal/Core/Utils/LogLevel.h"
#include "ramses/framework/RamsesFrameworkTypes.h"

namespace ramses::internal
{
    class UserLogAppender : public LogAppenderBase
    {
    public:
        explicit UserLogAppender(LogHandlerFunc f);

        void log(const LogMessage& logMessage) override;

    private:
        LogHandlerFunc m_func;
    };
}

