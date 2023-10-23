//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "internal/PlatformAbstraction/Collections/HashSet.h"

namespace ramses
{
    namespace internal
    {
        class NodeImpl;
    }

    class RamsesObject;

    using RamsesObjectVector = std::vector<RamsesObject *>;
    using NodeImplSet = internal::HashSet<internal::NodeImpl *>;
}
