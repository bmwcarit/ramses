//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMERROR_H
#define RAMSES_PLATFORMERROR_H

#include "Utils/LoggingUtils.h"

namespace ramses_internal
{
    enum class EStatus
    {
        Ok       = 0,
        Error    = 1,
        NotExist = 2,
        Eof      = 3
    };

    static constexpr const char* EStatusNames[] = {
        "Ok",
        "Error",
        "NotExist",
        "Eof"
    };
}

MAKE_ENUM_CLASS_PRINTABLE_NO_EXTRA_LAST(ramses_internal::EStatus, "EStatus", ramses_internal::EStatusNames, ramses_internal::EStatus::Eof);

#endif
