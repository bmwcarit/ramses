//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

namespace ramses::internal
{
    class IDevice;
    class IContext;

    // This class uses a shared context for upload of resources in a parallel thread
    class IResourceUploadRenderBackend
    {
    public:
        virtual ~IResourceUploadRenderBackend() = default;

        [[nodiscard]] virtual IContext&                       getContext() const = 0;
        [[nodiscard]] virtual IDevice&                        getDevice() const = 0;
    };
}
