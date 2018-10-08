//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TRANSPORTCOMMON_TESTUTILS_H
#define RAMSES_TRANSPORTCOMMON_TESTUTILS_H

#include <stdlib.h>
#include "Collections/String.h"


namespace ramses_internal
{

    static inline String GetTempDir()
    {
#ifdef _WIN32
        const UInt32 maxPathLength = MAX_PATH + 1;
        String result;

        char buffer[maxPathLength];
        if (GetTempPath(maxPathLength, buffer) > 0)
        {
            result = String(buffer);
        }

        return result;
#else
        return String("/tmp/");
#endif
    }
}


#endif // RAMSES_TRANSPORTCOMMON_TESTUTILS_H
