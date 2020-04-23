//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CONTENTSTATES_H
#define RAMSES_CONTENTSTATES_H

#include <cassert>

namespace ramses
{
    enum class ContentState
    {
        Available,   ///< Content is known and can be requested to get ready in order to show it
        Ready,       ///< Content is ready both from Dcsm and renderer perspective (content data is fully ready to be rendered)
        Shown,       ///< Content is actively rendered on display determined from its assigned category

        Invalid
    };

    static inline const char* ContentStateName(ContentState state)
    {
        static const char* ContentStateNames[] =
        {
            "AVAILABLE",
            "READY",
            "SHOWN",
            "INVALID"
        };
        static_assert((size_t(ContentState::Invalid) + 1) == (sizeof(ContentStateNames) / sizeof(ContentStateNames[0])), "missing state name");

        return ContentStateNames[int(state)];
    }
}

#endif
