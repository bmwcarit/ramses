//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/RenderBufferBinding.h"
#include "impl/logic/RenderBufferBindingImpl.h"

namespace ramses
{
    RenderBufferBinding::RenderBufferBinding(std::unique_ptr<internal::RenderBufferBindingImpl> impl) noexcept
        : RamsesBinding{ std::move(impl) }
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_renderBufferBinding{ static_cast<internal::RenderBufferBindingImpl&>(RamsesBinding::m_impl) }
    {
    }

    const ramses::RenderBuffer& RenderBufferBinding::getRenderBuffer() const
    {
        return m_renderBufferBinding.getRenderBuffer();
    }

    ramses::RenderBuffer& RenderBufferBinding::getRenderBuffer()
    {
        return m_renderBufferBinding.getRenderBuffer();
    }

    internal::RenderBufferBindingImpl& RenderBufferBinding::impl()
    {
        return m_renderBufferBinding;
    }

    const internal::RenderBufferBindingImpl& RenderBufferBinding::impl() const
    {
        return m_renderBufferBinding;
    }
}
