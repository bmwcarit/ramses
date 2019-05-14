//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MEMORYHANDLE_H
#define RAMSES_MEMORYHANDLE_H

#include "PlatformAbstraction/PlatformTypes.h"
#include <limits>

namespace ramses_internal
{
    typedef UInt32 MemoryHandle;

    constexpr MemoryHandle InvalidMemoryHandle = std::numeric_limits<MemoryHandle>::max();
}

#endif
