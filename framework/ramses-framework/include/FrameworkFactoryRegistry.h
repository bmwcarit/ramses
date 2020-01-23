//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FRAMEWORKFACTORYREGISTRY_H
#define RAMSES_FRAMEWORKFACTORYREGISTRY_H

#include "RamsesObjectFactoryInterfaces.h"
#include <memory>

namespace ramses
{
    class FrameworkFactoryRegistry
    {
    public:
        FrameworkFactoryRegistry(FrameworkFactoryRegistry const&) = delete;
        void operator=(FrameworkFactoryRegistry const&) = delete;

        static FrameworkFactoryRegistry& GetInstance();

        void registerClientFactory(std::unique_ptr<IClientFactory> factory);
        void registerRendererFactory(std::unique_ptr<IRendererFactory> factory);

        IClientFactory* getClientFactory();
        IRendererFactory* getRendererFactory();

    private:
        FrameworkFactoryRegistry() = default;

        std::unique_ptr<IClientFactory> m_clientFactory;
        std::unique_ptr<IRendererFactory> m_rendererFactory;
    };
}
#endif
