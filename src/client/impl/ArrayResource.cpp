//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/ArrayResource.h"

#include "impl/ArrayResourceImpl.h"

namespace ramses
{
    ArrayResource::ArrayResource(std::unique_ptr<internal::ArrayResourceImpl> impl)
        : Resource{ std::move(impl) }
        , m_impl{ static_cast<internal::ArrayResourceImpl&>(Resource::m_impl) }
    {
    }

    uint32_t ArrayResource::getNumberOfElements() const
    {
        return m_impl.getElementCount();
    }

    EDataType ArrayResource::getDataType() const
    {
        return m_impl.getElementType();
    }

    internal::ArrayResourceImpl& ArrayResource::impl()
    {
        return m_impl;
    }

    const internal::ArrayResourceImpl& ArrayResource::impl() const
    {
        return m_impl;
    }
}
