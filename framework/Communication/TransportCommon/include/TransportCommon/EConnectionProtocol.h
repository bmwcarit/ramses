//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ECONNECTIONPROTOCOL_H
#define RAMSES_ECONNECTIONPROTOCOL_H

#include "Utils/LoggingUtils.h"

namespace ramses_internal
{
    enum class EConnectionProtocol
    {
        TCP,
        Off,
        Invalid, // must be last
    };

    static const char* ConectionProtocolNames[] =
    {
        "TCP",
        "Off",
        "Invalid"
    };
}

MAKE_ENUM_CLASS_PRINTABLE_NO_EXTRA_LAST(ramses_internal::EConnectionProtocol,
                                        "EConnectionProtocol",
                                        ramses_internal::ConectionProtocolNames,
                                        ramses_internal::EConnectionProtocol::Invalid);

#endif
