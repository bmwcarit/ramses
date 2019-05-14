//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Logger_Wayland/Logger_Wayland.h"
#include "Utils/LogMacros.h"
#include <wayland-client.h>
#include <wayland-server.h>

namespace ramses_internal
{
    Logger_Wayland* Logger_Wayland::s_instance = nullptr;

    Logger_Wayland::Logger_Wayland()
    {
        wl_log_set_handler_client(ClientLog);
        wl_log_set_handler_server(ServerLog);
    }

    Logger_Wayland::~Logger_Wayland() {}

    void Logger_Wayland::Init()
    {
        assert(s_instance == nullptr);
        s_instance = new Logger_Wayland;
    }

    void Logger_Wayland::Deinit()
    {
        delete s_instance;
        s_instance = nullptr;
    }

    void Logger_Wayland::ClientLog(const char* fmt, va_list ap)
    {
        if (s_instance != nullptr)
        {
            s_instance->log(fmt, ap, "Logger_Wayland::ClientLog: ");
        }
    }

    void Logger_Wayland::ServerLog(const char* fmt, va_list ap)
    {
        if (s_instance != nullptr)
        {
            s_instance->log(fmt, ap, "Logger_Wayland::ServerLog: ");
        }
    }

    void Logger_Wayland::log(const char* fmt, va_list ap, const String& prefix)
    {
        // Variadic list of parameters can be read only once by VSprintf,
        // first call of VSprintf only to get the length of the message,
        // for the second call, a copy of the variadic list is needed.
        va_list apCopy;
        va_copy(apCopy, ap);
        Char dummyBuffer;

        const Int32 lengthOfMessage = PlatformStringUtils::VSprintf(&dummyBuffer, 1, fmt, ap);

        if (lengthOfMessage >= 0)
        {
            std::vector<Char> buffer(lengthOfMessage + 1); // +1 for null terminator
            const Int32  numberCharacters = PlatformStringUtils::VSprintf(buffer.data(), buffer.size(), fmt, apCopy);
            if (numberCharacters == lengthOfMessage)
            {
                outputMessage(prefix + buffer.data());
            }
            else
            {
                LOG_ERROR(CONTEXT_RENDERER, "Logger_Wayland::log Error in VSprintf!");
                assert(false);
            }
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Logger_Wayland::log Encoding error in VSprintf!");
        }

        va_end(apCopy);
    }

    void Logger_Wayland::outputMessage(const String& message)
    {
        // This method is needed for Logger_Wayland_Mock, to check if strings with variadic parameters are formatted
        // right by Logger_Wayland::log.
        LOG_ERROR(CONTEXT_RENDERER, message);
    }
}
