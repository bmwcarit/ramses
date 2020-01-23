//  -------------------------------------------------------------------------
//  Copyright (C) 2009-2010 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMLOCK_H
#define RAMSES_PLATFORMLOCK_H

#include <mutex>

namespace ramses_internal
{
    using PlatformLock = std::recursive_mutex;
    using PlatformGuard = std::lock_guard<std::recursive_mutex>;
}

#endif
