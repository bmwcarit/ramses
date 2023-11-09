//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/LoggingUtils.h"

namespace ramses::internal
{
    enum class EConnectionProtocol
    {
        TCP,
        Off,
        Invalid, // must be last
    };

    const std::array ConectionProtocolNames =
    {
        "TCP",
        "Off",
        "Invalid"
    };
}

MAKE_ENUM_CLASS_PRINTABLE(ramses::internal::EConnectionProtocol,
                                        "EConnectionProtocol",
                                        ramses::internal::ConectionProtocolNames,
                                        ramses::internal::EConnectionProtocol::Invalid);