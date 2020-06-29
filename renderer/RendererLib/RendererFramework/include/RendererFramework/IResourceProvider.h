//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IRESOURCEPROVIDER_H
#define RAMSES_IRESOURCEPROVIDER_H

#include "Components/ManagedResource.h"
#include "SceneAPI/SceneId.h"
#include "Components/ResourceRequesterID.h"

namespace ramses_internal
{
    class IResourceProvider
    {
    public:
        virtual ~IResourceProvider() {}

        virtual void requestResourceAsyncronouslyFromFramework(const ResourceContentHashVector& ids, const ResourceRequesterID& requesterID, const SceneId& sceneId) = 0;
        virtual void cancelResourceRequest(const ResourceContentHash& resourceHash, const ResourceRequesterID& requesterID) = 0;
        virtual ManagedResourceVector popArrivedResources(const ResourceRequesterID& requesterID) = 0;

    };
}

#endif
