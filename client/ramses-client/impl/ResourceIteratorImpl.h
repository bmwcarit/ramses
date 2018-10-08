//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEITERATORIMPL_H
#define RAMSES_RESOURCEITERATORIMPL_H

#include "IteratorImpl.h"
#include "ramses-client-api/RamsesObjectTypes.h"
#include "ramses-client-api/RamsesClient.h"
#include "RamsesClientImpl.h"

namespace ramses
{
    class ResourceIteratorImpl : public IteratorImpl<RamsesObject*>
    {
    public:
        ResourceIteratorImpl(const RamsesClientImpl& client, ERamsesObjectType objType)
            : IteratorImpl(client.getListOfResourceObjects(objType))
        {
        }
    };
}

#endif
