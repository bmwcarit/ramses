//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/ConsoleLogAppender.h"
#include "Utils/LogMessage.h"
#include "Utils/LogContext.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "ramses-capu/os/Console.h"

namespace ramses_internal
{
    ConsoleLogAppender::ConsoleLogAppender()
        : m_callback([]() {})
    {
    }

    void ConsoleLogAppender::logMessage(const LogMessage& logMessage)
    {
        const uint64_t now = PlatformTime::GetMillisecondsAbsolute();
        ramses_capu::Console::Print(ramses_capu::Console::WHITE, "%.3f ", now/1000.0);

        switch(logMessage.getLogLevel())
        {
        case ELogLevel::Trace:
            ramses_capu::Console::Print(ramses_capu::Console::WHITE,  "[ Trace ] ");
            break;
        case ELogLevel::Debug :
            ramses_capu::Console::Print(ramses_capu::Console::WHITE,  "[ Debug ] ");
            break;
        case ELogLevel::Info :
            ramses_capu::Console::Print(ramses_capu::Console::GREEN,  "[ Info  ] ");
            break;
        case ELogLevel::Warn :
            ramses_capu::Console::Print(ramses_capu::Console::YELLOW, "[ Warn  ] ");
            break;
        case ELogLevel::Error :
            ramses_capu::Console::Print(ramses_capu::Console::RED,    "[ Error ] ");
            break;
        case ELogLevel::Fatal :
            ramses_capu::Console::Print(ramses_capu::Console::RED,    "[ Fatal ] ");
            break;
        default:
            ramses_capu::Console::Print(ramses_capu::Console::RED,    "[ ????? ] ");
            break;
        }

        ramses_capu::Console::Print(ramses_capu::Console::AQUA, logMessage.getContext().getContextName());
        ramses_capu::Console::Print(" | %s\n", logMessage.getStream().c_str());
        ramses_capu::Console::Flush();

        m_callback();
    }

    void ConsoleLogAppender::setAfterLogCallback(const std::function<void()>& callback)
    {
        m_callback = callback;
    }

    void ConsoleLogAppender::removeAfterLogCallback()
    {
        m_callback = []() {};
    }
}
