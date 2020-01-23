//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IRESOURCECONSUMERCOMPONENT_H
#define RAMSES_IRESOURCECONSUMERCOMPONENT_H

#include "Transfer/ResourceTypes.h"
#include "ManagedResource.h"

namespace ramses_internal
{
    class Guid;

    class IResourceConsumerComponent
    {
    public:
        virtual ~IResourceConsumerComponent() {}

        virtual void requestResourceAsynchronouslyFromFramework(const ResourceContentHashVector& ids, const RequesterID& requesterID, const Guid& providerID) = 0;
        virtual void cancelResourceRequest(const ResourceContentHash& resourceHash, const RequesterID& requesterID) = 0;
        virtual ManagedResourceVector popArrivedResources(const RequesterID& requesterID) = 0;
    };
}

#endif
