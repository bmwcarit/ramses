//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "FrameworkFactoryRegistry.h"
#include "Utils/LogMacros.h"

namespace ramses
{
    FrameworkFactoryRegistry& FrameworkFactoryRegistry::GetInstance()
    {
        static FrameworkFactoryRegistry instance;
        return instance;
    }

    void FrameworkFactoryRegistry::registerClientFactory(std::unique_ptr<IClientFactory> factory)
    {
        if (m_clientFactory)
            LOG_WARN(ramses_internal::CONTEXT_FRAMEWORK, "FrameworkFactoryRegistry::registerClientFactory called more than once");

        m_clientFactory = std::move(factory);
    }

    void FrameworkFactoryRegistry::registerRendererFactory(std::unique_ptr<IRendererFactory> factory)
    {
        if (m_rendererFactory)
            LOG_WARN(ramses_internal::CONTEXT_FRAMEWORK, "FrameworkFactoryRegistry::registerRendererFactory called more than once");

        m_rendererFactory = std::move(factory);
    }

    IClientFactory* FrameworkFactoryRegistry::getClientFactory()
    {
        return m_clientFactory.get();
    }

    IRendererFactory* FrameworkFactoryRegistry::getRendererFactory()
    {
        return m_rendererFactory.get();
    }
}
