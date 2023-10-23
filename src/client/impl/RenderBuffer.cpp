//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/client/RenderBuffer.h"

// internal
#include "impl/RenderBufferImpl.h"

namespace ramses
{
    RenderBuffer::RenderBuffer(std::unique_ptr<internal::RenderBufferImpl> impl)
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<internal::RenderBufferImpl&>(SceneObject::m_impl) }
    {
    }

    uint32_t RenderBuffer::getWidth() const
    {
        return m_impl.getWidth();
    }

    uint32_t RenderBuffer::getHeight() const
    {
        return m_impl.getHeight();
    }

    ERenderBufferFormat RenderBuffer::getBufferFormat() const
    {
        return m_impl.getBufferFormat();
    }

    ERenderBufferAccessMode RenderBuffer::getAccessMode() const
    {
        return m_impl.getAccessMode();
    }

    uint32_t RenderBuffer::getSampleCount() const
    {
        return m_impl.getSampleCount();
    }

    internal::RenderBufferImpl& RenderBuffer::impl()
    {
        return m_impl;
    }

    const internal::RenderBufferImpl& RenderBuffer::impl() const
    {
        return m_impl;
    }
}
