//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CONSOLELOGAPPENDER_H
#define RAMSES_CONSOLELOGAPPENDER_H

#include "Utils/LogAppenderBase.h"
#include <functional>

namespace ramses_internal
{
    class ConsoleLogAppender : public LogAppenderBase
    {
    public:
        ConsoleLogAppender();

        virtual void logMessage(const LogMessage& logMessage) override;

        void setAfterLogCallback(const std::function<void()>& callback);
        void removeAfterLogCallback();

    private:
        std::function<void()> m_callback;
    };
}

#endif
