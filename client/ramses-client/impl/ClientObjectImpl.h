//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CLIENTOBJECTIMPL_H
#define RAMSES_CLIENTOBJECTIMPL_H

#include "RamsesObjectImpl.h"

namespace ramses
{
    class RamsesClientImpl;

    class ClientObjectImpl : public RamsesObjectImpl
    {
    public:
        explicit ClientObjectImpl(RamsesClientImpl& client, ERamsesObjectType type, const char* name);
        ~ClientObjectImpl() override;

        // impl methods
        const RamsesClientImpl& getClientImpl() const;
        RamsesClientImpl&       getClientImpl();

        bool isFromTheSameClientAs(const ClientObjectImpl& otherObject) const;

    private:
        RamsesClientImpl& m_client;
    };
}

#endif
