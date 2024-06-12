//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/RamsesClient.h"
#include "impl/RamsesObjectFactoryInterfaces.h"

#include <string_view>

namespace ramses::internal
{
    class ClientFactory : public IClientFactory
    {
    public:
        static bool RegisterClientFactory();

        ClientUniquePtr createClient(RamsesFrameworkImpl& framework, std::string_view applicationName) const override;

    private:
        static void DeleteClient(RamsesClient* client);
    };
}

