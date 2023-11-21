//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/APIExport.h"
#include "impl/RamsesObjectFactoryInterfaces.h"
#include <memory>

namespace ramses::internal
{
    class RAMSES_IMPL_EXPORT FrameworkFactoryRegistry
    {
    public:
        FrameworkFactoryRegistry(FrameworkFactoryRegistry const&) = delete;
        void operator=(FrameworkFactoryRegistry const&) = delete;

        static FrameworkFactoryRegistry& GetInstance();

        void registerClientFactory(std::unique_ptr<IClientFactory> factory);
        void registerRendererFactory(UniquePtrWithDeleter<IRendererFactory> factory);

        IClientFactory* getClientFactory();
        IRendererFactory* getRendererFactory();

    private:
        FrameworkFactoryRegistry() = default;

        std::unique_ptr<IClientFactory> m_clientFactory;
        UniquePtrWithDeleter<IRendererFactory> m_rendererFactory;
    };
}

