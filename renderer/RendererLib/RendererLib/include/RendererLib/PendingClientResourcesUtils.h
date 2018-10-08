//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PENDINGCLIENTRESOURCESUTILS_H
#define RAMSES_PENDINGCLIENTRESOURCESUTILS_H

#include "Transfer/ResourceTypes.h"

namespace ramses_internal
{
    class PendingClientResourcesUtils
    {
    public:
        static void ConsolidateAddedResources(
            ResourceContentHashVector& needed,
            ResourceContentHashVector& unneeded,
            ResourceContentHashVector& newlyNeeded,
            const ResourceContentHashVector& addedResources);
        static void ConsolidateRemovedResources(
            ResourceContentHashVector& needed,
            ResourceContentHashVector& unneeded,
            ResourceContentHashVector& newlyUnneededPreviouslyNeeded,
            const ResourceContentHashVector& removedResources);

        static void ConsolidateNewlyNeededResources(
            ResourceContentHashVector& newlyNeeded,
            ResourceContentHashVector& pendingUnneeded);
        static void ConsolidateNewlyUnneededResources(
            ResourceContentHashVector& pendingUnneeded,
            const ResourceContentHashVector& newlyUnneededPreviouslyNeeded);

        static void ConsolidateNeededAndUnneededResources(
            ResourceContentHashVector& inUse,
            const ResourceContentHashVector& needed,
            const ResourceContentHashVector& unneeded);
    };
}

#endif
