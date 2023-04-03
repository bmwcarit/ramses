//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/RenderBuffer.h"

// internal
#include "RenderBufferImpl.h"

namespace ramses
{
    RenderBuffer::RenderBuffer(RenderBufferImpl& pimpl)
        : SceneObject(pimpl)
        , impl(pimpl)
    {
    }

    RenderBuffer::~RenderBuffer()
    {
    }

    uint32_t RenderBuffer::getWidth() const
    {
        return impl.getWidth();
    }

    uint32_t RenderBuffer::getHeight() const
    {
        return impl.getHeight();
    }

    ERenderBufferType RenderBuffer::getBufferType() const
    {
        return impl.getBufferType();
    }

    ERenderBufferFormat RenderBuffer::getBufferFormat() const
    {
        return impl.getBufferFormat();
    }

    ERenderBufferAccessMode RenderBuffer::getAccessMode() const
    {
        return impl.getAccessMode();
    }

    uint32_t RenderBuffer::getSampleCount() const
    {
        return impl.getSampleCount();
    }
}
