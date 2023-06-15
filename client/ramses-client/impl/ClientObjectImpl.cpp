//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ClientObjectImpl.h"

namespace ramses
{
    ClientObjectImpl::ClientObjectImpl(RamsesClientImpl& client, ERamsesObjectType type, std::string_view name)
        : RamsesObjectImpl(type, name)
        , m_client(client)
    {
    }

    ClientObjectImpl::~ClientObjectImpl()
    {
    }

    const RamsesClientImpl& ClientObjectImpl::getClientImpl() const
    {
        return m_client;
    }

    RamsesClientImpl& ClientObjectImpl::getClientImpl()
    {
        return m_client;
    }

    bool ClientObjectImpl::isFromTheSameClientAs(const ClientObjectImpl& otherObject) const
    {
        return &getClientImpl() == &(otherObject.getClientImpl());
    }
}
