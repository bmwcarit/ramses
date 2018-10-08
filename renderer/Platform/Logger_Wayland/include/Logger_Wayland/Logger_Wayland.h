//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LOGGER_WAYLAND_H
#define RAMSES_LOGGER_WAYLAND_H

#include "PlatformAbstraction/PlatformStringUtils.h"
#include "Collections/String.h"
#include "Collections/Vector.h"

namespace ramses_internal
{
    class Logger_Wayland
    {
    public:
        static void Init();
        static void Deinit();

        Logger_Wayland(const Logger_Wayland&) = delete;
        Logger_Wayland& operator=(const Logger_Wayland&) = delete;

    protected:
        Logger_Wayland();
        virtual ~Logger_Wayland();

        void         log(const char* fmt, va_list ap, const String& prefix);
        virtual void outputMessage(const String& message);

    private:
        static void ClientLog(const char* fmt, va_list ap);
        static void ServerLog(const char* fmt, va_list ap);

        static Logger_Wayland* s_instance;
    };
}

#endif
