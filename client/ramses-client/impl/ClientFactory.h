//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CLIENTFACTORY_H
#define RAMSES_CLIENTFACTORY_H

#include "ramses-client-api/RamsesClient.h"
#include "RamsesObjectFactoryInterfaces.h"

namespace ramses
{
    class ClientFactory : public IClientFactory
    {
    public:
        static bool RegisterClientFactory();

        virtual ClientUniquePtr createClient(RamsesFrameworkImpl* impl, const char* applicationName) const override;
    };
}
#endif
