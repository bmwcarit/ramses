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
    RenderBuffer::RenderBuffer(std::unique_ptr<RenderBufferImpl> impl)
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<RenderBufferImpl&>(SceneObject::m_impl) }
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

    ERenderBufferType RenderBuffer::getBufferType() const
    {
        return m_impl.getBufferType();
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
}
