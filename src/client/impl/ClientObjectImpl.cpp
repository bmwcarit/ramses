//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/ClientObjectImpl.h"
#include "impl/RamsesClientImpl.h"
#include "impl/RamsesFrameworkImpl.h"

namespace ramses::internal
{
    ClientObjectImpl::ClientObjectImpl(RamsesClientImpl& client, ERamsesObjectType type, std::string_view name)
        : RamsesObjectImpl{ type, name }
        , m_client{ client }
    {
    }

    ClientObjectImpl::~ClientObjectImpl() = default;

    const RamsesClientImpl& ClientObjectImpl::getClientImpl() const
    {
        return m_client;
    }

    RamsesClientImpl& ClientObjectImpl::getClientImpl()
    {
        return m_client;
    }

    ErrorReporting& ClientObjectImpl::getErrorReporting() const
    {
        return m_client.getFramework().getErrorReporting();
    }

    bool ClientObjectImpl::isFromTheSameClientAs(const ClientObjectImpl& otherObject) const
    {
        return &getClientImpl() == &(otherObject.getClientImpl());
    }
}
