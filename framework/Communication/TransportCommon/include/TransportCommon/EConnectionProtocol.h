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
    enum EConnectionProtocol
    {
        EConnectionProtocol_TCP = 0,
        EConnectionProtocol_Fake,
        EConnectionProtocol_Invalid,
        EConnectionProtocol_NUMBER_OF_ELEMENTS
    };

    static const char* ConectionProtocolNames[] =
    {
        "TCP",
        "Fake",
        "Invalid"
    };

    ENUM_TO_STRING(EConnectionProtocol, ConectionProtocolNames, EConnectionProtocol_NUMBER_OF_ELEMENTS);
}

#endif
