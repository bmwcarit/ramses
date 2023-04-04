//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/RamsesRenderPassBinding.h"
#include "impl/RamsesRenderPassBindingImpl.h"

namespace rlogic
{
    RamsesRenderPassBinding::RamsesRenderPassBinding(std::unique_ptr<internal::RamsesRenderPassBindingImpl> impl) noexcept
        : RamsesBinding(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_renderPassBinding{ static_cast<internal::RamsesRenderPassBindingImpl&>(RamsesBinding::m_impl) }
    {
    }

    RamsesRenderPassBinding::~RamsesRenderPassBinding() noexcept = default;

    const ramses::RenderPass& RamsesRenderPassBinding::getRamsesRenderPass() const
    {
        return m_renderPassBinding.getRamsesRenderPass();
    }

    ramses::RenderPass& RamsesRenderPassBinding::getRamsesRenderPass()
    {
        return m_renderPassBinding.getRamsesRenderPass();
    }
}
