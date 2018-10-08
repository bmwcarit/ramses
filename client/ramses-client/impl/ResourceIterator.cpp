//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/ResourceIterator.h"
#include "ResourceIteratorImpl.h"

namespace ramses
{

    ResourceIterator::ResourceIterator(const RamsesClient& client, ERamsesObjectType objectType)
        : impl(new ResourceIteratorImpl(client.impl, objectType))
    {
    }

    ResourceIterator::~ResourceIterator()
    {
        delete impl;
    }

    Resource* ResourceIterator::getNext()
    {
        return static_cast<Resource*>(impl->getNext());
    }
}
