//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EVISIBILITYMODE_H
#define RAMSES_EVISIBILITYMODE_H

#include <cstdint>

namespace ramses
{
    /**
     * @ingroup CoreAPI
     * Specifies the mode of node visibility.
    */

    enum class EVisibilityMode : uint32_t
    {
        Off = 0,    ///< A node shall be invisible and the node shall not trigger its resources to be loaded.
        Invisible,  ///< A node shall be invisible, but the node shall trigger its resources to be loaded.
        Visible     ///< A node shall be fully loaded and visible.
    };
}

#endif
