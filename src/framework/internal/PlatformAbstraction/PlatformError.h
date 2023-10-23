//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/LoggingUtils.h"

namespace ramses::internal
{
    enum class EStatus
    {
        Ok       = 0,
        Error    = 1,
        NotExist = 2,
        Eof      = 3
    };

    const std::array EStatusNames = {
        "Ok",
        "Error",
        "NotExist",
        "Eof"
    };
}

MAKE_ENUM_CLASS_PRINTABLE(ramses::internal::EStatus, "EStatus", ramses::internal::EStatusNames, ramses::internal::EStatus::Eof);
