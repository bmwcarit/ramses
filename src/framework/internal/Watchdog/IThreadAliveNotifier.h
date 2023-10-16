//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <chrono>

namespace ramses::internal
{
    class IThreadAliveNotifier
    {
    public:
        virtual ~IThreadAliveNotifier() = default;

        virtual uint64_t registerThread() = 0;
        virtual void unregisterThread(uint64_t identifier) = 0;

        virtual void notifyAlive(uint64_t identifier) = 0;
        [[nodiscard]] virtual std::chrono::milliseconds calculateTimeout() const = 0;
    };
}
