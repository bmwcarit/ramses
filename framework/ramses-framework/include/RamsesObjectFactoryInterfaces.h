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
#include <functional>
#include <string_view>

namespace ramses
{
    class RamsesClient;
    class RamsesRenderer;
    class RendererConfig;
    class RamsesFrameworkImpl;

    template <typename T>
    using UniquePtrWithDeleter = std::unique_ptr<T, std::function<void(T*)>>;
    using ClientUniquePtr = UniquePtrWithDeleter<RamsesClient>;
    using RendererUniquePtr = UniquePtrWithDeleter<RamsesRenderer>;

    class IClientFactory
    {
    public:
        virtual ~IClientFactory() = default;

        virtual ClientUniquePtr createClient(RamsesFrameworkImpl& framework, std::string_view applicationName) const = 0;
    };

    class IRendererFactory
    {
    public:
        virtual ~IRendererFactory() = default;

        virtual RendererUniquePtr createRenderer(RamsesFrameworkImpl& framework, const RendererConfig& config) const = 0;
    };
}
#endif
