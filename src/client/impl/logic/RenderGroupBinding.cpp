//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/RenderGroupBinding.h"
#include "impl/logic/RenderGroupBindingImpl.h"

namespace ramses
{
    RenderGroupBinding::RenderGroupBinding(std::unique_ptr<internal::RenderGroupBindingImpl> impl) noexcept
        : RamsesBinding(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_renderGroupBinding{ static_cast<internal::RenderGroupBindingImpl&>(RamsesBinding::m_impl) }
    {
    }

    const ramses::RenderGroup& RenderGroupBinding::getRamsesRenderGroup() const
    {
        return m_renderGroupBinding.getRamsesRenderGroup();
    }

    ramses::RenderGroup& RenderGroupBinding::getRamsesRenderGroup()
    {
        return m_renderGroupBinding.getRamsesRenderGroup();
    }

    internal::RenderGroupBindingImpl& RenderGroupBinding::impl()
    {
        return m_renderGroupBinding;
    }

    const internal::RenderGroupBindingImpl& RenderGroupBinding::impl() const
    {
        return m_renderGroupBinding;
    }
}
