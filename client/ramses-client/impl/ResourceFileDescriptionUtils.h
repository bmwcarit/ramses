//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEFILEDESCRIPTIONUTILS_H
#define RAMSES_RESOURCEFILEDESCRIPTIONUTILS_H

#include "Collections/StringOutputStream.h"

#include "ramses-client-api/ResourceFileDescription.h"
#include "ramses-client-api/ResourceFileDescriptionSet.h"

namespace ramses_internal
{
    StringOutputStream& operator<<(StringOutputStream& lhs, ramses::resourceId_t const& rhs);

    class ResourceFileDescriptionUtils
    {
    public:
        static String MakeLoggingString(ramses::ResourceFileDescription const& rhs);
        static String MakeLoggingString(ramses::ResourceFileDescriptionSet const& rhs);
    };
}
#endif
