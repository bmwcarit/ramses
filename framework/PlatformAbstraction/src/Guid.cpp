//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Collections/Guid.h"
#include <cinttypes>

namespace ramses_internal
{
    uint64_t Guid::getFromString(const char* guid, size_t len) const
    {
        if (len == 36)
        {
            // backward compatible 128bit guid parsing
            uint32_t data1 = 0;
            uint32_t data2 = 0;
            uint32_t data3 = 0;
            uint64_t data4 = 0;
            uint64_t data5 = 0;

            const int retVal = sscanf(guid, "%08x-%04x-%04x-%04" SCNx64 "-%012" SCNx64,
                                &data1,
                                &data2,
                                &data3,
                                &data4,
                                &data5);
            if (retVal == 5 && data4 <= 0xFFFF && data5 <= 0xFFFFFFFFFFFF)
                return (data4 << 48) + data5;
        }
        else if (len == 17)
        {
            // modern 64bit parsing
            uint64_t data4 = 0;
            uint64_t data5 = 0;

            const int retVal = sscanf(guid, "%04" SCNx64 "-%012" SCNx64, &data4, &data5);
            if (retVal == 2 && data4 <= 0xFFFF && data5 <= 0xFFFFFFFFFFFF)
                return (data4 << 48) + data5;
        }
        else if (len == 4)
        {
            // shortened format parsing (to always allow e.g. Guid(fmt::to_string(Guid(1))))
            uint64_t data5 = 0;
            const int retVal = sscanf(guid, "%04" SCNx64, &data5);
            if (retVal == 1 && data5 <= 0xff)
                return data5;
        }
        return 0;
    }

    String Guid::toString() const
    {
        return String{fmt::to_string(*this)};
    }
}
