//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IResourceUploadRenderBackend.h"

namespace ramses::internal
{
    // This class uses a shared context for upload of resources in a parallel thread
    class ResourceUploadRenderBackend : public IResourceUploadRenderBackend
    {
    public:
        ResourceUploadRenderBackend(IContext& context, IDevice& device);

        ~ResourceUploadRenderBackend() override = default;

        [[nodiscard]] IContext& getContext() const override;

        [[nodiscard]] IDevice& getDevice() const override;

    private:
        IContext& m_context;
        IDevice& m_device;
    };
}
