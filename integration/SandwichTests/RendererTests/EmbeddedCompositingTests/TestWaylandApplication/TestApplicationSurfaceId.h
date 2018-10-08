//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TESTAPPLICATIONSURFACEID_H
#define RAMSES_TESTAPPLICATIONSURFACEID_H

#include "Common/StronglyTypedValue.h"

namespace ramses_internal
{
    struct TestApplicationSurfaceIdTag
    {
    };
    typedef StronglyTypedValue<UInt32, 0xFFFFFFFF, TestApplicationSurfaceIdTag> TestApplicationSurfaceId;
}

#endif
