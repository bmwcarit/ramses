//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ClientFactory.h"
#include "impl/FrameworkFactoryRegistry.h"
#include "impl/RamsesClientImpl.h"

namespace ramses::internal
{
    ClientUniquePtr ClientFactory::createClient(RamsesFrameworkImpl& framework, std::string_view applicationName) const
    {
        auto impl = std::make_unique<RamsesClientImpl>(framework, applicationName);
        return ClientUniquePtr{ new RamsesClient{ std::move(impl) },
            [](RamsesClient* client_) { delete client_; } };
    }

    bool ClientFactory::RegisterClientFactory()
    {
        FrameworkFactoryRegistry::GetInstance().registerClientFactory(std::make_unique<ClientFactory>());
        return true;
    }
}
