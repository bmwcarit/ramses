//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/synchronized_clock.h"

namespace ramses::internal
{
    namespace FlushTime
    {
        using Clock = synchronized_clock;

        const Clock::time_point InvalidTimestamp{ std::chrono::milliseconds(0) };
    }

    struct FlushTimeInformation
    {
        FlushTimeInformation() = default;
        FlushTimeInformation(FlushTime::Clock::time_point expirationTS, FlushTime::Clock::time_point internalTS, synchronized_clock_type clockType, bool effectTimeSync)
            : expirationTimestamp(expirationTS)
            , internalTimestamp(internalTS)
            , clock_type(clockType)
            , isEffectTimeSync(effectTimeSync)
        {
        }

        FlushTime::Clock::time_point expirationTimestamp = FlushTime::InvalidTimestamp;
        FlushTime::Clock::time_point internalTimestamp = FlushTime::InvalidTimestamp;
        synchronized_clock_type clock_type = synchronized_clock_type::SystemTime;

        /**
        * if set to true, the internalTimestamp is used to update the IScene's effect time
        * #ramses::internal::IScene::setEffectTimeSync
        */
        bool isEffectTimeSync = false;
    };

    inline bool operator==(const FlushTimeInformation& a, const FlushTimeInformation& b)
    {
        return
            a.expirationTimestamp == b.expirationTimestamp &&
            a.internalTimestamp == b.internalTimestamp &&
            a.isEffectTimeSync == b.isEffectTimeSync;
    }

    inline bool operator!=(const FlushTimeInformation& a, const FlushTimeInformation& b)
    {
        return !(a == b);
    }
}
