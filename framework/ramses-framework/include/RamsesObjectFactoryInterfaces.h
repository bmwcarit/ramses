//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESOBJECTFACTORYINTERFACES_H
#define RAMSES_RAMSESOBJECTFACTORYINTERFACES_H

#include <memory>

namespace ramses
{
    class RamsesClient;
    class RamsesRenderer;
    class RendererConfig;
    class RamsesFrameworkImpl;

    using ClientUniquePtr = std::unique_ptr<RamsesClient, void(*)(RamsesClient*)>;
    using RendererUniquePtr = std::unique_ptr<RamsesRenderer, void(*)(RamsesRenderer*)>;

    class IClientFactory
    {
    public:
        virtual ~IClientFactory() = default;

        virtual ClientUniquePtr createClient(RamsesFrameworkImpl* impl, const char* applicationName) const = 0;
    };

    class IRendererFactory
    {
    public:
        virtual ~IRendererFactory() = default;

        virtual RendererUniquePtr createRenderer(RamsesFrameworkImpl* impl, const RendererConfig& config) const = 0;
    };
}
#endif
