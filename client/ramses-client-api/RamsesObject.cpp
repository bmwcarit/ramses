//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/RamsesObject.h"

// internal
#include "RamsesObjectImpl.h"

namespace ramses
{
    RamsesObject::RamsesObject(std::unique_ptr<RamsesObjectImpl> impl)
        : StatusObject{ std::move(impl) }
        , m_impl{ static_cast<RamsesObjectImpl&>(*StatusObject::m_impl) }
    {
        m_impl.setRamsesObject(*this);
    }

    const char* RamsesObject::getName() const
    {
        return m_impl.getName().c_str();
    }

    status_t RamsesObject::setName(std::string_view name)
    {
        const status_t status =m_impl.setName(*this, name);
        LOG_HL_CLIENT_API1(status, name);
        return status;
    }

    ramses::ERamsesObjectType RamsesObject::getType() const
    {
        return m_impl.getType();
    }

    bool RamsesObject::isOfType(ERamsesObjectType type) const
    {
        return m_impl.isOfType(type);
    }
}
