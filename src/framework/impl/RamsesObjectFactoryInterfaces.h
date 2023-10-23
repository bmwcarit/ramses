//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <memory>
#include <functional>
#include <string_view>

namespace ramses
{
    namespace internal
    {
        class RamsesFrameworkImpl;
    }

    class RamsesClient;
    class RamsesRenderer;
    class RendererConfig;

    template <typename T>
    using UniquePtrWithDeleter = std::unique_ptr<T, std::function<void(T*)>>;
    using ClientUniquePtr = UniquePtrWithDeleter<RamsesClient>;
    using RendererUniquePtr = UniquePtrWithDeleter<RamsesRenderer>;

    class IClientFactory
    {
    public:
        virtual ~IClientFactory() = default;

        virtual ClientUniquePtr createClient(internal::RamsesFrameworkImpl& framework, std::string_view applicationName) const = 0;
    };

    class IRendererFactory
    {
    public:
        virtual ~IRendererFactory() = default;

        virtual RendererUniquePtr createRenderer(internal::RamsesFrameworkImpl& framework, const RendererConfig& config) const = 0;
    };
}

