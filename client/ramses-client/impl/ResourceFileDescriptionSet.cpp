//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#include "ramses-client-api/ResourceFileDescriptionSet.h"
#include "ResourceFileDescriptionSetImpl.h"
#include "ramses-client-api/ResourceFileDescription.h"

namespace ramses
{

    ResourceFileDescriptionSet::ResourceFileDescriptionSet()
        : impl(new ResourceFileDescriptionSetImpl())
    {
    }

    ResourceFileDescriptionSet::~ResourceFileDescriptionSet()
    {
        delete impl;
    }

    void ResourceFileDescriptionSet::add(const ResourceFileDescription& description)
    {
        impl->descriptions.push_back(description);
    }

    uint32_t ResourceFileDescriptionSet::getNumberOfDescriptions() const
    {
        return static_cast<uint32_t>(impl->descriptions.size());
    }

    const ResourceFileDescription& ResourceFileDescriptionSet::getDescription(uint32_t index) const
    {
        return impl->descriptions[index];
    }

    ResourceFileDescriptionSet::ResourceFileDescriptionSet(const ResourceFileDescriptionSet& resourceFileDescriptionSet)
        : impl(new ResourceFileDescriptionSetImpl())
    {
        *impl = *(resourceFileDescriptionSet.impl);
    }
}
