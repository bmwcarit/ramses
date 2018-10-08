//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANDROIDLOGAPPENDER_H
#define RAMSES_ANDROIDLOGAPPENDER_H

#include "Utils/LogAppenderBase.h"

namespace ramses_internal
{
    class AndroidLogAppender : public LogAppenderBase
    {
    public:
        void logMessage(const LogMessage& logMessage) override;
    };
}

#endif
