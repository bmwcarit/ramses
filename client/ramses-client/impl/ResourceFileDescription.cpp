//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#include "ramses-client-api/ResourceFileDescription.h"
#include "ResourceFileDescriptionImpl.h"

namespace ramses
{
    ramses::ResourceFileDescription::ResourceFileDescription(const char* filename)
        : impl(new ResourceFileDescriptionImpl())
    {
        impl->m_filename = filename;
    }

    ramses::ResourceFileDescription::~ResourceFileDescription()
    {
        delete impl;
    }

    const char* ramses::ResourceFileDescription::getFilename() const
    {
        return impl->m_filename.c_str();
    }

    void ramses::ResourceFileDescription::add(const Resource* resourceObject)
    {
        bool aleadyContained = false;
        for (auto iter : impl->m_resources)
        {
            if (iter->getResourceId() == resourceObject->getResourceId())
            {
                aleadyContained = true;
                break;
            }
        }
        if (!aleadyContained)
        {
            impl->m_resources.push_back(resourceObject);
        }
    }

    bool ramses::ResourceFileDescription::contains(const Resource* resourceObject) const
    {
        return ramses_internal::contains_c(impl->m_resources, resourceObject);
    }

    uint32_t ResourceFileDescription::getNumberOfResources() const
    {
        return static_cast<uint32_t>(impl->m_resources.size());
    }

    const Resource& ResourceFileDescription::getResource(uint32_t index) const
    {
        return *impl->m_resources[index];
    }

    ResourceFileDescription::ResourceFileDescription(const ResourceFileDescription& resourceFileDescription)
        : impl(new ResourceFileDescriptionImpl())
    {
        *impl = *(resourceFileDescription.impl);
    }

    ResourceFileDescription& ResourceFileDescription::operator=(const ResourceFileDescription& resourceFileDescription)
    {
        *impl = *(resourceFileDescription.impl);
        return *this;
    }
}
