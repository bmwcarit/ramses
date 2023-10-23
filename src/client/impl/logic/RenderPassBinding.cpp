//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/RenderPassBinding.h"
#include "impl/logic/RenderPassBindingImpl.h"

namespace ramses
{
    RenderPassBinding::RenderPassBinding(std::unique_ptr<internal::RenderPassBindingImpl> impl) noexcept
        : RamsesBinding(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_renderPassBinding{ static_cast<internal::RenderPassBindingImpl&>(RamsesBinding::m_impl) }
    {
    }

    const ramses::RenderPass& RenderPassBinding::getRamsesRenderPass() const
    {
        return m_renderPassBinding.getRamsesRenderPass();
    }

    ramses::RenderPass& RenderPassBinding::getRamsesRenderPass()
    {
        return m_renderPassBinding.getRamsesRenderPass();
    }

    internal::RenderPassBindingImpl& RenderPassBinding::impl()
    {
        return m_renderPassBinding;
    }

    const internal::RenderPassBindingImpl& RenderPassBinding::impl() const
    {
        return m_renderPassBinding;
    }
}
