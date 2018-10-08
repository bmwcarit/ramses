//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IRESOURCEPROVIDER_H
#define RAMSES_IRESOURCEPROVIDER_H

#include "Transfer/ResourceTypes.h"
#include "Components/ManagedResource.h"
#include "SceneAPI/SceneId.h"

namespace ramses_internal
{
    class IResourceProvider
    {
    public:
        virtual ~IResourceProvider() {}

        virtual void requestResourceAsyncronouslyFromFramework(const ResourceContentHashVector& ids, const RequesterID& requesterID, const SceneId& sceneId) = 0;
        virtual void cancelResourceRequest(const ResourceContentHash& resourceHash, const RequesterID& requesterID) = 0;
        virtual ManagedResourceVector popArrivedResources(const RequesterID& requesterID) = 0;

    };
}

#endif
