//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EDCSMOFFERINGMODE_H
#define RAMSES_EDCSMOFFERINGMODE_H

namespace ramses
{
    /**
     * Specifies mode of offering content
     * LocalOnly: Offer Content only to Consumers within the same process
     * LocalAndRemote: Offer Content to all local and remote Consumers
     */
    enum class EDcsmOfferingMode
    {
        LocalOnly = 0,
        LocalAndRemote
    };
}

#endif
