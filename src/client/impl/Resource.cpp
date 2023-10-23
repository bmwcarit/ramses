//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/client/Resource.h"

// internal
#include "impl/ResourceImpl.h"

namespace ramses
{
    Resource::Resource(std::unique_ptr<internal::ResourceImpl> impl)
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<internal::ResourceImpl&>(SceneObject::m_impl) }
    {
    }

    resourceId_t Resource::getResourceId() const
    {
        return m_impl.getResourceId();
    }

    internal::ResourceImpl& Resource::impl()
    {
        return m_impl;
    }

    const internal::ResourceImpl& Resource::impl() const
    {
        return m_impl;
    }
}
