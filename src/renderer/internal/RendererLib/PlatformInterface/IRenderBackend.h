//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

namespace ramses::internal
{
    class IWindow;
    class IContext;
    class IDevice;
    class IEmbeddedCompositor;
    class ITextureUploadingAdapter;

    class IRenderBackend
    {
    public:
        virtual ~IRenderBackend() = default;

        // Get components
        [[nodiscard]] virtual IWindow&                        getWindow() const = 0;
        [[nodiscard]] virtual IContext&                       getContext() const = 0;
        [[nodiscard]] virtual IDevice&                        getDevice() const = 0;
        [[nodiscard]] virtual IEmbeddedCompositor&            getEmbeddedCompositor() const = 0;
        [[nodiscard]] virtual ITextureUploadingAdapter&       getTextureUploadingAdapter()  const = 0;
    };
}
