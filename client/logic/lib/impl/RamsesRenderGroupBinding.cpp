//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/RamsesRenderGroupBinding.h"
#include "impl/RamsesRenderGroupBindingImpl.h"

namespace rlogic
{
    RamsesRenderGroupBinding::RamsesRenderGroupBinding(std::unique_ptr<internal::RamsesRenderGroupBindingImpl> impl) noexcept
        : RamsesBinding(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_renderGroupBinding{ static_cast<internal::RamsesRenderGroupBindingImpl&>(RamsesBinding::m_impl) }
    {
    }

    RamsesRenderGroupBinding::~RamsesRenderGroupBinding() noexcept = default;

    const ramses::RenderGroup& RamsesRenderGroupBinding::getRamsesRenderGroup() const
    {
        return m_renderGroupBinding.getRamsesRenderGroup();
    }

    ramses::RenderGroup& RamsesRenderGroupBinding::getRamsesRenderGroup()
    {
        return m_renderGroupBinding.getRamsesRenderGroup();
    }
}
