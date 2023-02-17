//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERBACKEND_H
#define RAMSES_RENDERBACKEND_H

#include "RendererAPI/IRenderBackend.h"

namespace ramses_internal
{
    class RenderBackend : public IRenderBackend
    {
    public:
        RenderBackend(IWindow& window, IContext& context, IDevice& device, IEmbeddedCompositor& embeddedCompositor, ITextureUploadingAdapter& textureUploadingAdapter);
        virtual ~RenderBackend()  override = default;

        [[nodiscard]] virtual IWindow& getWindow() const override;
        [[nodiscard]] virtual IContext& getContext() const override;
        [[nodiscard]] virtual IDevice& getDevice() const override;
        [[nodiscard]] virtual IEmbeddedCompositor& getEmbeddedCompositor() const override;
        [[nodiscard]] virtual ITextureUploadingAdapter& getTextureUploadingAdapter() const override;

    private:
        IWindow& m_window;
        IContext& m_context;
        IDevice& m_device;
        IEmbeddedCompositor& m_embeddedCompositor;
        ITextureUploadingAdapter& m_textureUploadingAdapter;
    };
}

#endif
