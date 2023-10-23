//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RenderBackend.h"
#include "internal/RendererLib/PlatformInterface/IContext.h"
#include "internal/RendererLib/PlatformInterface/IEmbeddedCompositor.h"

namespace ramses::internal
{
    RenderBackend::RenderBackend(IWindow& window, IContext& context, IDevice& device, IEmbeddedCompositor& embeddedCompositor, ITextureUploadingAdapter& textureUploadingAdapter)
        : m_window(window)
        , m_context(context)
        , m_device(device)
        , m_embeddedCompositor(embeddedCompositor)
        , m_textureUploadingAdapter(textureUploadingAdapter)
    {
    }

    IWindow& RenderBackend::getWindow() const
    {
        return m_window;
    }

    IContext& RenderBackend::getContext() const
    {
        return m_context;
    }

    IDevice& RenderBackend::getDevice() const
    {
        return m_device;
    }

    IEmbeddedCompositor& RenderBackend::getEmbeddedCompositor() const
    {
        return m_embeddedCompositor;
    }

    ITextureUploadingAdapter& RenderBackend::getTextureUploadingAdapter() const
    {
        return m_textureUploadingAdapter;
    }
}
