//  -------------------------------------------------------------------------
//  Copyright (C) 2009-2010 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMTYPES_H
#define RAMSES_PLATFORMTYPES_H

#include <cstdint>

#define UNUSED(x) ((void)(x))

namespace ramses_internal
{
    using Int = std::intptr_t;
    using Byte = unsigned char;
}

#endif
