//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/Logger_Wayland.h"
#include "internal/Core/Utils/LogMacros.h"
#include <wayland-client.h>
#include <wayland-server.h>
#include <array>

namespace ramses::internal
{
    namespace
    {
        std::array<char, 200> buffer;

        void WaylandClientLog(const char* fmt, va_list ap)
        {
            std::vsnprintf(buffer.data(), buffer.size(), fmt, ap);
            LOG_ERROR(CONTEXT_RENDERER, "Logger_Wayland::ClientLog: {}", buffer.data());
        }

        void WaylandServerLog(const char* fmt, va_list ap)
        {
            std::vsnprintf(buffer.data(), buffer.size(), fmt, ap);
            LOG_ERROR(CONTEXT_RENDERER, "Logger_Wayland::ServerLog: {}", buffer.data());
        }
    }

    namespace Logger_Wayland
    {
        void RedirectToRamsesLogger()
        {
            wl_log_set_handler_client(WaylandClientLog);
            wl_log_set_handler_server(WaylandServerLog);
        }
    }
}
