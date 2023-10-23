//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/client/ClientObject.h"

// internal
#include "ClientObjectImpl.h"

namespace ramses
{
    ClientObject::ClientObject(std::unique_ptr<internal::ClientObjectImpl> impl)
        : RamsesObject{ std::move(impl) }
        , m_impl{ static_cast<internal::ClientObjectImpl&>(*RamsesObject::m_impl) }
    {
    }

    internal::ClientObjectImpl& ClientObject::impl()
    {
        return m_impl;
    }

    const internal::ClientObjectImpl& ClientObject::impl() const
    {
        return m_impl;
    }
}
