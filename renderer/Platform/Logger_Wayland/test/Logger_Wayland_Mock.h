//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LOGGER_WAYLAND_MOCK_H
#define RAMSES_LOGGER_WAYLAND_MOCK_H

#include "gmock/gmock.h"
#include "Logger_Wayland/Logger_Wayland.h"

namespace ramses_internal
{
    class Logger_Wayland_Mock : public Logger_Wayland
    {
    public:
        void log(const char* fmt, va_list ap, const String& prefix);

        MOCK_METHOD1(outputMessage, void(const String&));
    };
}

#endif

