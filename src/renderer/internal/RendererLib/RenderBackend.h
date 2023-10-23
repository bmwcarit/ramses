//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IRenderBackend.h"

namespace ramses::internal
{
    class RenderBackend : public IRenderBackend
    {
    public:
        RenderBackend(IWindow& window, IContext& context, IDevice& device, IEmbeddedCompositor& embeddedCompositor, ITextureUploadingAdapter& textureUploadingAdapter);
        ~RenderBackend()  override = default;

        [[nodiscard]] IWindow& getWindow() const override;
        [[nodiscard]] IContext& getContext() const override;
        [[nodiscard]] IDevice& getDevice() const override;
        [[nodiscard]] IEmbeddedCompositor& getEmbeddedCompositor() const override;
        [[nodiscard]] ITextureUploadingAdapter& getTextureUploadingAdapter() const override;

    private:
        IWindow& m_window;
        IContext& m_context;
        IDevice& m_device;
        IEmbeddedCompositor& m_embeddedCompositor;
        ITextureUploadingAdapter& m_textureUploadingAdapter;
    };
}
