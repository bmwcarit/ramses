//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEREQUESTERID_H
#define RAMSES_RESOURCEREQUESTERID_H

#include "Common/StronglyTypedValue.h"

namespace ramses_internal
{
    typedef StronglyTypedValue<UInt32, static_cast<UInt32>(-1), struct ResourceRequesterIDTag> ResourceRequesterID;
}

MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses_internal::ResourceRequesterID)

#endif
