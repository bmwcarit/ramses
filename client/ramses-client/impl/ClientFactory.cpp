//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ClientFactory.h"
#include "FrameworkFactoryRegistry.h"
#include "RamsesClientImpl.h"

namespace ramses
{
    ClientUniquePtr ClientFactory::createClient(RamsesFrameworkImpl* impl, const char* applicationName) const
    {
        ClientUniquePtr client(new RamsesClient(*new RamsesClientImpl(*impl, applicationName)),
            [](RamsesClient* client_) { delete client_; });
        return client;
    }

    bool ClientFactory::RegisterClientFactory()
    {
        FrameworkFactoryRegistry::GetInstance().registerClientFactory(std::make_unique<ClientFactory>());
        return true;
    }
}
