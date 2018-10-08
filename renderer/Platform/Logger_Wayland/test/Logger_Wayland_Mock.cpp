//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Logger_Wayland_Mock.h"

namespace ramses_internal
{
    void Logger_Wayland_Mock::log(const char* fmt, va_list ap, const String& prefix)
    {
        // Just for making log accessible for the test.
        Logger_Wayland::log(fmt, ap, prefix);
    }
}
