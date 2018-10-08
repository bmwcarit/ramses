//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESOBJECTHANDLE_H
#define RAMSES_RAMSESOBJECTHANDLE_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "Common/TypedMemoryHandle.h"

namespace ramses
{
    struct RamsesObjectHandleTag {};
    typedef ramses_internal::TypedMemoryHandle<RamsesObjectHandleTag> RamsesObjectHandle;
}

#endif
