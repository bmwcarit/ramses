//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ScopedLogContextLevel.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    struct TestLog
    {
        ELogLevel type;
        std::string message;
    };

    class TestLogCollector
    {
    public:
        explicit TestLogCollector(LogContext& context, ELogLevel verbosityLimit)
            : m_logCollector(context, verbosityLimit, [this](ELogLevel type, std::string_view message)
                {
                    logs.emplace_back(TestLog{type, std::string{message}});
                })
        {
        }

        std::vector<TestLog> logs;

    private:
        ScopedLogContextLevel m_logCollector;
    };
}
