//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEAPI_WAYLANDIVISURFACEID_H
#define RAMSES_SCENEAPI_WAYLANDIVISURFACEID_H

#include "Common/StronglyTypedValue.h"
#include "Utils/LoggingUtils.h"
#include <vector>
#include <unordered_set>

namespace ramses_internal
{
    struct WaylandIviSurfaceIdTag {};
    using WaylandIviSurfaceId = StronglyTypedValue<uint32_t, std::numeric_limits<uint32_t>::max(), WaylandIviSurfaceIdTag>;
    using WaylandIviSurfaceIdSet = std::unordered_set<WaylandIviSurfaceId>;
    using WaylandIviSurfaceIdVector = std::vector<WaylandIviSurfaceId>;
}

MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses_internal::WaylandIviSurfaceId)

#endif
