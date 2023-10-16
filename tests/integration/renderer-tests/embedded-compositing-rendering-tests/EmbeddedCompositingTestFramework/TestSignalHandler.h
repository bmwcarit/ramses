//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <string>
#include <string_view>

namespace ramses::internal
{
    class TestSignalHandler
    {
    public:
        static void RegisterSignalHandlersForCurrentProcess(std::string_view processName);

    private:
        TestSignalHandler();
        static void HandleSignalCallback(int32_t signal);

        static std::string ProcessName;
    };
}
