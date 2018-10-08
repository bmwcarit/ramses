//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Common/StronglyTypedValue.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include <type_traits>

namespace ramses_internal
{
    using StronglyTypedUInt32 = StronglyTypedValue<UInt32, 0, struct SomeTag>;

    // enforce performance guarantees
    static_assert(std::is_trivially_copyable<StronglyTypedUInt32>::value, "expectation failed");
    static_assert(std::is_trivially_destructible<StronglyTypedUInt32>::value, "expectation failed");
}
