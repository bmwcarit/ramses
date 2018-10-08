//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RenderBackend.h"
#include "RendererAPI/IContext.h"
#include "RendererAPI/IEmbeddedCompositor.h"
#include "RendererAPI/ISurface.h"

namespace ramses_internal
{
    RenderBackend::RenderBackend(ISurface& surface, IDevice& device, IEmbeddedCompositor& embeddedCompositor, ITextureUploadingAdapter& textureUploadingAdapter)
        : m_surface(surface)
        , m_device(device)
        , m_embeddedCompositor(embeddedCompositor)
        , m_textureUploadingAdapter(textureUploadingAdapter)
    {
    }

    ISurface& RenderBackend::getSurface() const
    {
        return m_surface;
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
