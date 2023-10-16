//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/client/RenderTarget.h"

// internal
#include "impl/RenderTargetImpl.h"

namespace ramses
{
    RenderTarget::RenderTarget(std::unique_ptr<internal::RenderTargetImpl> impl)
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<internal::RenderTargetImpl&>(SceneObject::m_impl) }
    {
    }

    uint32_t RenderTarget::getWidth() const
    {
        return m_impl.getWidth();
    }

    uint32_t RenderTarget::getHeight() const
    {
        return m_impl.getHeight();
    }

    internal::RenderTargetImpl& RenderTarget::impl()
    {
        return m_impl;
    }

    const internal::RenderTargetImpl& RenderTarget::impl() const
    {
        return m_impl;
    }
}
